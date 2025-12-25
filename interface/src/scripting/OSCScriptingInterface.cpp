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
#include <QNetworkDatagram>

#include "ScriptContext.h"
#include "ScriptEngine.h"
#include "ScriptValue.h"
#include "OSCScriptingInterface.h"

#include <array>
#include <bit>
#include <optional>
#include <QByteArray>

// TODO: Split this out into libshared later
// This might be handy to reuse in other modules too
namespace DataHelpers {

template<typename T, std::endian endianness = std::endian::big>
    // don't try to write pointers, references, structs, etc
    requires (std::is_arithmetic_v<T>)
inline auto write(QByteArray& ba, T v) {
    std::array<char, sizeof(v)> buffer;

    std::copy_n(reinterpret_cast<char*>(&v), sizeof(v), buffer.begin());

    if constexpr (std::endian::native != endianness) {
        std::reverse(buffer.begin(), buffer.end());
    }

    return ba.append(buffer.data(), buffer.size());
}

template<typename T>
inline auto writeUTF8NullTerminated(QByteArray& ba, const T& v);

template<>
inline auto writeUTF8NullTerminated(QByteArray& ba, const QString& v) {
    ba.append(v.toUtf8());
    return ba.append('\x00');
}

template<typename T, std::endian endianness = std::endian::big>
    // don't try to read pointers, references, structs, etc
    requires (std::is_arithmetic_v<T>)
inline std::optional<T> read(QByteArray& ba, int& offset) {
    if (offset < 0) { return std::nullopt; }

    std::array<char, sizeof(T)> buffer;

    // the byte array doesn't have enough
    // data left to fulfill the read
    if (offset + static_cast<int>(sizeof(T)) > ba.size()) {
        return std::nullopt;
    }

    std::copy_n(ba.constData() + offset, sizeof(T), buffer.begin());

    if constexpr (std::endian::native != endianness) {
        std::reverse(buffer.begin(), buffer.end());
    }

    T value;
    std::copy_n(buffer.data(), buffer.size(), reinterpret_cast<char*>(&value));

    offset += sizeof(T);

    return value;
}

template<typename T, int N>
    requires (sizeof(T) == 1 && N > 0)
inline std::optional<std::array<T, N>> read(QByteArray& ba, int& offset) {
    if (offset < 0) { return std::nullopt; }

    std::array<T, N> buffer;

    // the byte array doesn't have enough
    // data left to fulfill the read
    if (offset + N > ba.size()) {
        return std::nullopt;
    }

    std::copy_n(ba.constData() + offset, N, buffer.begin());

    return buffer;
}

inline std::optional<QByteArray> read(QByteArray& ba, int offset, int length) {
    if (offset < 0 || length < 1) { return std::nullopt; }

    // the byte array doesn't have enough
    // data left to fulfill the read
    if (offset + length > ba.size()) {
        return std::nullopt;
    }

    return QByteArray(ba.constData() + offset, length);
}

template<typename T>
inline std::optional<T>
readUTF8NullTerminated(QByteArray& ba, int& offset);

template<>
inline std::optional<QString>
readUTF8NullTerminated(QByteArray& ba, int& offset) {
    auto cursor = offset;

    for (int i = 0; (i + cursor) < ba.size(); i++) {
        if (ba[i + cursor] == '\x00') {
            offset = offset + i;
            return QString::fromUtf8(ba.constData() + cursor, i);
        }
    }

    return std::nullopt;
}

template<>
inline std::optional<QByteArray>
readUTF8NullTerminated(QByteArray& ba, int& offset) {
    auto cursor = offset;

    for (int i = 0; (i + cursor) < ba.size(); i++) {
        if (ba[i + cursor] == '\x00') {
            offset = offset + i;
            return QByteArray(ba.constData() + cursor, i);
        }
    }

    return std::nullopt;
}

}


Q_LOGGING_CATEGORY(osc_cat, "overte.osc")

enum OSCTag: char {
    // standard types
    Int = 'i',
    Float = 'f',
    String = 's',
    Blob = 'b',

    // extension types
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

    if (
        auto s = DataHelpers::readUTF8NullTerminated<QString>(data, cursor);
        s.has_value() && !s->isEmpty()
    ) {
        addr = *s;
    } else {
        qCritical(osc_cat, "Early EOF in packet, OSC address missing");
        return;
    }
    cursor = next4(cursor);

    if (addr == "#bundle") {
        qCritical(osc_cat, "Received OSC bundle, which isn't supported");
        return;
    }

    if (
        auto s = DataHelpers::readUTF8NullTerminated<QByteArray>(data, cursor);
        s.has_value() && !s->isEmpty()
    ) {
        argTypes = *s;
    } else {
        qCritical(osc_cat, "Early EOF in packet, argument types string missing");
        return;
    }
    cursor = next4(cursor);

    if (argTypes[0] != ',') {
        qCritical(osc_cat, "Argument type string does not start with ','");
        return;
    }

    // +1 to exclude the comma
    for (int arg = 1; arg < argTypes.length(); arg++) {
        switch (argTypes[arg]) {
            case OSCTag::Int: {
                auto value = DataHelpers::read<qint32>(data, cursor);

                if (!value.has_value()) {
                    qCritical(osc_cat, "Early EOF in packet (%s), reading int at offset %d", qUtf8Printable(addr), cursor);
                    return;
                }

                args.append(QVariantMap {
                    {"type", QString(OSCTag::Int)},
                    {"value", value.value()},
                });
            } break;

            case OSCTag::Float: {
                auto value = DataHelpers::read<float>(data, cursor);

                if (!value.has_value()) {
                    qCritical(osc_cat, "Early EOF in packet (%s), reading float at offset %d", qUtf8Printable(addr), cursor);
                    return;
                }

                args.append(QVariantMap {
                    {"type", QString(OSCTag::Float)},
                    {"value", value.value()},
                });
            } break;

            case OSCTag::String: {
                int startCursor = cursor;
                auto value = DataHelpers::readUTF8NullTerminated<QString>(data, cursor);

                if (!value.has_value()) {
                    qCritical(osc_cat, "Early EOF in packet (%s), string starting at offset %d", qUtf8Printable(addr), startCursor);
                    return;
                }

                cursor = next4(cursor);

                args.append(QVariantMap {
                    {"type", QString(OSCTag::String)},
                    {"value", value.value()},
                });
            } break;

            case OSCTag::Blob: {
                auto len = DataHelpers::read<qint32>(data, cursor);

                if (!len.has_value()) {
                    qCritical(osc_cat, "Early EOF in packet (%s), reading blob at offset %d (length prefix)", qUtf8Printable(addr), cursor);
                    return;
                }

                auto startCursor = cursor;
                auto bytes = DataHelpers::read(data, cursor, len.value());

                if (!bytes.has_value()) {
                    qCritical(osc_cat, "Early EOF in packet (%s), reading blob at offset %d (data)", qUtf8Printable(addr), startCursor);
                    return;
                }

                args.append(QVariantMap {
                    {"type", QString(OSCTag::Blob)},
                    {"value", bytes.value()},
                });

                cursor = next4(cursor + len.value());
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


    if (address == "#bundle") {
        engine->raiseException("OSC bundles are not supported");
        return engine->undefinedValue();
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
                DataHelpers::write(bodyBytes, static_cast<qint32>(value.toDouble()));
            } break;

            case QVariant::Double: {
                bytes.append(OSCTag::Float);
                DataHelpers::write(bodyBytes, static_cast<float>(value.toDouble()));
            } break;

            case QVariant::String: {
                bytes.append(OSCTag::String);

                auto stringArg = value.toString();

                if (stringArg.contains(QChar(0))) {
                    engine->raiseException("String contains null (0x00) character");
                    return engine->undefinedValue();
                }

                DataHelpers::writeUTF8NullTerminated(bodyBytes, stringArg);
                pad4(bodyBytes);
            } break;

            case QVariant::ByteArray: {
                bytes.append(OSCTag::Blob);

                auto bytesArg = value.toByteArray();

                DataHelpers::write(bodyBytes, bytesArg.length());
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
