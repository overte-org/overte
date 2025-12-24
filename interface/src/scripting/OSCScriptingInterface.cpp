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
#include <QNetworkDatagram>

#include "ScriptContext.h"
#include "ScriptEngine.h"
#include "ScriptValue.h"
#include "OSCScriptingInterface.h"


Q_LOGGING_CATEGORY(osc_cat, "osc")

enum OSCTag: char {
    Int = 'i',
    Float = 'f',
    String = 's',
    Blob = 'b',
    False = 'F',
    True = 'T',
    Null = 'N',
};

static const QRegularExpression invalidCharacters = QRegularExpression("([ #*,?\\[\\]{}])");
static const QMap<QChar, QVariant::Type> typeNameMap = {
    { OSCTag::Int, QVariant::Int },
    { OSCTag::Float, QVariant::Double },
    { OSCTag::String, QVariant::String },
    { OSCTag::Blob, QVariant::ByteArray },
    { OSCTag::False, QVariant::Bool },
    { OSCTag::True, QVariant::Bool },
    { OSCTag::Null, QVariant::Invalid },
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

    connect(_socket.get(), &QUdpSocket::readyRead, this, &OSCScriptingInterface::readPacket);

    qCInfo(osc_cat) << "Listening on" << _socket->localAddress();
}

OSCScriptingInterface::~OSCScriptingInterface() {
    _socket->close();
    qCInfo(osc_cat) << "Closed listen socket";
}

void OSCScriptingInterface::readPacket() {
    auto datagram = _socket->receiveDatagram();

    if (!datagram.isValid()) { return; }

    auto next4 = [](int i) { return i + (4 - i % 4); };

    auto data = datagram.data();
    int cursor = 0;

    QString addr;
    QByteArray argTypes;
    QVariantList args;

    // address
    for (int i = 0; (i + cursor) < data.length(); i++) {
        if (data[i + cursor] == '\x00') {
            addr = QString::fromUtf8(data.constData(), i);
            cursor = next4(cursor + i);
            break;
        }
    }

    if (addr.isEmpty()) { return; }

    // argument types
    for (int i = 0; (i + cursor) < data.length(); i++) {
        if (data[i + cursor] == '\x00') {
            argTypes = QByteArray(data.constData() + cursor, i);
            cursor = next4(cursor + i);
            break;
        }
    }

    // +1 to exclude the comma
    for (int arg = 1; arg < argTypes.length(); arg++) {
        switch (argTypes[arg]) {
            case OSCTag::Int: {
                const char* ptr = data.constData() + cursor;
                qint32 tmp = qFromBigEndian<qint32>(ptr);
                args.append(QVariantMap {
                    {"type", QString(OSCTag::Int)},
                    {"value", tmp},
                });
                cursor += 4;
            } break;

            case OSCTag::Float: {
                const char* ptr = data.constData() + cursor;
                qint32 tmp = qFromBigEndian<qint32>(ptr);
                args.append(QVariantMap {
                    {"type", QString(OSCTag::Float)},
                    {"value", *reinterpret_cast<float*>(&tmp)},
                });
                cursor += 4;
            } break;

            case OSCTag::String: {
                QString tmp;

                for (int i = 0; i + cursor < data.length(); i++) {
                    if (data[i + cursor] == '\x00') {
                        tmp = QString::fromUtf8(data.constData() + cursor, i);
                        cursor = next4(cursor + i);
                        break;
                    }
                }

                args.append(QVariantMap {
                    {"type", QString(OSCTag::String)},
                    {"value", tmp},
                });
            } break;

            case OSCTag::Blob: {
                const char* ptr = data.constData() + cursor;
                auto len = qFromBigEndian<qint32>(ptr);
                cursor += 4;

                args.append(QVariantMap {
                    {"type", QString(OSCTag::Blob)},
                    {"value", QByteArray(data.constData() + cursor, len)},
                });

                cursor = next4(cursor + len);
            } break;

            case OSCTag::False:
                args.append(QVariantMap {
                    {"type", QString(OSCTag::False)},
                    {"value", false},
                });
                break;

            case OSCTag::True:
                args.append(QVariantMap {
                    {"type", QString(OSCTag::True)},
                    {"value", true},
                });
                break;

            case OSCTag::Null:
                args.append(QVariantMap {
                    {"type", QString(OSCTag::Null)},
                    {"value", QVariant()},
                });
                break;

            default:
                qCCritical(osc_cat, "Unknown type '%c'", static_cast<char>(argTypes[arg]));
                return;
        }
    }

    emit packetReceived(addr, args);
}

ScriptValue OSCScriptingInterface::sendPacket(ScriptContext* context, ScriptEngine* engine) {
    auto instance = DependencyManager::get<OSCScriptingInterface>();

    if (context->argumentCount() == 0) {
        engine->raiseException("Address is required");
        return engine->undefinedValue();
    }

    auto address = context->argument(0).toString();
    QVariantList arguments;

    for (int i = 1; i < context->argumentCount(); i++) {
        arguments.append(context->argument(i).toVariant());
    }

    if (address.length() < 2 || !address.startsWith('/') || address.endsWith('/')) {
        engine->raiseException("Address must be at least two characters and start with '/'");
        return engine->undefinedValue();
    }

    if (auto match = invalidCharacters.match(address); match.hasMatch()) {
        engine->raiseException(QString("Address contains invalid character '%1'").arg(match.captured()));
        return engine->undefinedValue();
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

        QVariant::Type expectedType;
        QVariant value;

        if (arg.type() == QVariant::Map) {
            auto map = arg.toMap();
            auto typeName = map.value("type").toString();
            if (!typeNameMap.contains(typeName[0])) {
                engine->raiseException(QString("Unknown type '%c' on argument %d").arg(typeName).arg(i));
                return engine->undefinedValue();
            }
            expectedType = typeNameMap[typeName[0]];
            value = map.value("value");
        } else {
            expectedType = arg.type();
            value = arg;
        }

        if (value.isNull()) {
            bytes.append(OSCTag::Null);
            continue;
        }

        switch (expectedType) {
            case QVariant::Int: {
                bytes.append(OSCTag::Int);

                auto* ptr = bodyBytes.data() + bodyBytes.length();
                bodyBytes.append(4, 0);

                qToBigEndian(static_cast<qint32>(value.toDouble()), ptr);
            } break;

            case QVariant::Double: {
                bytes.append(OSCTag::Float);

                auto* ptr = bodyBytes.data() + bodyBytes.length();
                bodyBytes.append(4, 0);

                // FIXME: use std::bit_cast instead of this hack once c++20 is in
                auto tmp = static_cast<float>(value.toDouble());
                qToBigEndian(*reinterpret_cast<quint32*>(&tmp), ptr);
            } break;

            case QVariant::String: {
                bytes.append(OSCTag::String);

                auto stringArg = value.toString();

                if (stringArg.contains(QChar(0))) {
                    engine->raiseException("String contains null (0x00) character");
                    return engine->undefinedValue();
                }

                bodyBytes.append(stringArg.toUtf8());

                // pad to 4-byte boundary with at least one zero for the string
                bodyBytes.append('\x00');
                pad4(bodyBytes);
            } break;

            case QVariant::ByteArray: {
                bytes.append(OSCTag::Blob);

                auto bytesArg = value.toByteArray();

                // write length prefix
                auto* ptr = bodyBytes.data() + bodyBytes.length();
                bodyBytes.append(4, 0);
                qToBigEndian(bytesArg.length(), ptr);

                // write body
                bodyBytes.append(bytesArg);

                // pad to 4-byte boundary
                pad4(bodyBytes);
            } break;

            case QVariant::Bool:
                bytes.append(value.toBool() ? OSCTag::True : OSCTag::False);
                break;

            default:
                engine->raiseException(QString("Unserializable type %1").arg(value.typeName()));
                return engine->undefinedValue();
        }
    }

    // pad argument type string to 4-byte boundary with at least one zero
    bytes.append('\x00');
    pad4(bytes);

    // pack the header + body data together for the datagram
    bytes.append(bodyBytes);

    instance->_socket->writeDatagram(bytes, QHostAddress(instance->_sendHost.get()), instance->_sendPort.get());

    qCDebug(osc_cat) << QHostAddress(instance->_sendHost.get()) << instance->_sendPort.get() << ":" << bytes.toHex(' ');

    return engine->undefinedValue();
}
