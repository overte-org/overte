//
//  OSCScriptingInterface.cpp
//  interface/src/scripting
//
//  Created by Ada <ada@thingvellir.net> on 2025-12-13
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <QLoggingCategory>
#include <QRegularExpression>
#include <QtEndian>

#include "ScriptEngine.h"
#include "OSCScriptingInterface.h"


Q_LOGGING_CATEGORY(osc, "osc")

enum OSCTag: char {
    Int = 'i',
    Float = 'f',
    String = 's',
    Blob = 'b',
};

static const QRegularExpression invalidCharacters = QRegularExpression("([ #*,?\\[\\]{}])");
static const QMap<QChar, QVariant::Type> typeNameMap = {
    { OSCTag::Int, QVariant::Int },
    { OSCTag::Float, QVariant::Double },
    { OSCTag::String, QVariant::String },
    { OSCTag::Blob, QVariant::ByteArray },
};


OSCScriptingInterface::OSCScriptingInterface(QObject* parent) :
    QObject(parent),
    _receivePort("osc/receivePort", 9000),
    _receiveHost("osc/receiveHost", "127.0.0.1"),
    _sendPort("osc/sendPort", 9001),
    _sendHost("osc/sendHost", "127.0.0.1"),
    _socket(std::make_shared<QUdpSocket>(this))
{
    _socket->bind(QHostAddress(_receiveHost.get()), _receivePort.get());

    connect(_socket.get(), &QUdpSocket::readyRead, this, &OSCScriptingInterface::receivePacket);

    qCInfo(osc) << "Listening on" << _socket->localAddress();
}

OSCScriptingInterface::~OSCScriptingInterface() {
    _socket->close();
    qCInfo(osc) << "Closed listen socket";
}

void OSCScriptingInterface::receivePacket() {
    // TODO
    emit messageReceived("/test", "", QVariantList());
}

void OSCScriptingInterface::sendMessage(const QString& address, const QString &argumentTypes, const QVariantList& arguments) {
    Q_ASSERT(_socket);

    // FIXME: how do you get the script engine???
    struct HackRemoveThis {
        void raiseException(QString msg) {
            qCCritical(osc) << msg;
        }
    };

    auto scriptEngine = std::make_unique<HackRemoveThis>(); //_scriptManager->engine();

    if (address.length() < 2 || address[0] != '/') {
        scriptEngine->raiseException("address must be at least two characters and start with '/'");
        return;
    }

    if (auto match = invalidCharacters.match(address); match.hasMatch()) {
        scriptEngine->raiseException(QString("address contains invalid character '%1'").arg(match.captured()));
        return;
    }

    if (argumentTypes.length() != arguments.length()) {
        scriptEngine->raiseException(QString("argumentTypes.length (%1) does not match arguments.length (%2)").arg(argumentTypes.length()).arg(arguments.length()));
        return;
    }

    auto pad4 = [](QByteArray& array) {
        array.append((4 - array.length() % 4) % 4, 0);
    };

    QByteArray bytes = address.toUtf8();

    // pad to 4-byte boundary with at least one zero for the address string
    bytes.append('\x00');
    pad4(bytes);

    // comma at the start of the argument list, even if empty
    bytes.append(',');

    QByteArray bodyBytes;

    for (int i = 0; i < arguments.length(); i++) {
        auto arg = arguments[i];

        if (!typeNameMap.contains(argumentTypes[i])) {
            scriptEngine->raiseException(QString("Unknown type tag '%1'").arg(argumentTypes[i]));
            return;
        }

        auto expectedType = typeNameMap[argumentTypes[i]];
        if (
            expectedType != arg.type() &&
            // JS doesn't have ints, so allow doubles to be interpreted as ints
            arg.type() != QVariant::Double
        ) {
            scriptEngine->raiseException(QString("Specified argument type '%1' does not match value of type '%2'").arg(QVariant::typeToName(expectedType)).arg(arg.typeName()));
            return;
        }

        switch (expectedType) {
            case QVariant::Int: {
                bytes.append(OSCTag::Int);

                auto* ptr = bodyBytes.data() + bodyBytes.length();
                bodyBytes.append(4, 0);

                qToBigEndian(static_cast<qint32>(arg.toDouble()), ptr);
            } break;

            case QVariant::Double: {
                bytes.append(OSCTag::Float);

                auto* ptr = bodyBytes.data() + bodyBytes.length();
                bodyBytes.append(4, 0);

                // FIXME: use std::bit_cast instead of this hack once c++20 is in
                auto tmp = static_cast<float>(arg.toDouble());
                qToBigEndian(*reinterpret_cast<quint32*>(&tmp), ptr);
            } break;

            case QVariant::String: {
                bytes.append(OSCTag::String);

                auto stringArg = arg.toString();

                if (stringArg.contains(QChar(0))) {
                    scriptEngine->raiseException("String contains null (0x00) character");
                    return;
                }

                bodyBytes.append(stringArg.toUtf8());

                // pad to 4-byte boundary with at least one zero for the string
                bodyBytes.append('\x00');
                pad4(bodyBytes);
            } break;

            case QVariant::ByteArray: {
                bytes.append(OSCTag::Blob);

                auto bytesArg = arg.toByteArray();

                // write length prefix
                auto* ptr = bodyBytes.data() + bodyBytes.length();
                bodyBytes.append(4, 0);
                qToBigEndian(bytesArg.length(), ptr);

                // write body
                bodyBytes.append(bytesArg);

                // pad to 4-byte boundary
                pad4(bodyBytes);
            } break;

            // unreachable
            default:
                Q_ASSERT(false);
                break;
        }
    }

    // pad argument type string to 4-byte boundary with at least one zero
    bytes.append('\x00');
    pad4(bytes);

    // pack the header + body data together for the datagram
    bytes.append(bodyBytes);

    _socket->writeDatagram(bytes, QHostAddress(_sendHost.get()), _sendPort.get());

    qCDebug(osc) << QHostAddress(_sendHost.get()) << _sendPort.get() << ":" << bytes.toHex(' ');
}
