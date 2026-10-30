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

Q_LOGGING_CATEGORY(osc_cat, "overte.osc")

class WriteHelper {
    std::vector<uint8_t> data {};

public:
    const auto& getData() const { return data; }

    /// If data.size is already aligned to alignment, then nothing extra will be pushed
    void pushToAlign(size_t alignment) {
        auto delta = alignment - (data.size() % alignment);
        data.resize(data.size() + delta);
    }

    void pushU8(uint8_t x) {
        data.push_back(x);
    }

    void pushU32(uint32_t x) {
        std::array<uint8_t, 4> a {
            static_cast<uint8_t>((x >> 24) & 0xff),
            static_cast<uint8_t>((x >> 16) & 0xff),
            static_cast<uint8_t>((x >> 8) & 0xff),
            static_cast<uint8_t>((x >> 0) & 0xff),
        };
        data.insert(data.end(), std::begin(a), std::end(a));
    }

    void pushU64(uint64_t x) {
        std::array<uint8_t, 8> a {
            static_cast<uint8_t>((x >> 56) & 0xff),
            static_cast<uint8_t>((x >> 48) & 0xff),
            static_cast<uint8_t>((x >> 40) & 0xff),
            static_cast<uint8_t>((x >> 32) & 0xff),
            static_cast<uint8_t>((x >> 24) & 0xff),
            static_cast<uint8_t>((x >> 16) & 0xff),
            static_cast<uint8_t>((x >> 8) & 0xff),
            static_cast<uint8_t>((x >> 0) & 0xff),
        };
        data.insert(data.end(), std::begin(a), std::end(a));
    }

    void pushF32(float x) {
        pushU32(std::bit_cast<uint32_t>(x));
    }

    void pushString(const std::string_view& s) {
        data.insert(data.end(), std::begin(s), std::end(s));
        data.push_back(0);
        pushToAlign(4);
    }

    void pushI32(int32_t x) {
        pushU32(static_cast<uint32_t>(x));
    }

    void pushBytes(const QByteArray& bytes) {
        data.insert(data.end(), std::begin(bytes), std::end(bytes));
    }

    void pushBytes(const std::vector<uint8_t>& bytes) {
        data.insert(data.end(), std::begin(bytes), std::end(bytes));
    }
};

class ReadHelper {
    const uint8_t* data;
    size_t dataLen;
    size_t cursor { 0 };

public:
    ReadHelper(const uint8_t* data, size_t dataLen) :
        data(data), dataLen(dataLen) {}

    ReadHelper(const char* data, size_t dataLen) :
        data(reinterpret_cast<const uint8_t*>(data)), dataLen(dataLen) {}

    ReadHelper slice(size_t len) {
        if (cursor < dataLen) {
            auto minLen = std::min(dataLen - cursor, len);
            auto r = ReadHelper(data + cursor, minLen);
            cursor += minLen;
            return r;
        } else {
            return ReadHelper(data + dataLen, 0);
        }
    }

    size_t getCursor() const { return cursor; }

    /// If cursor is already aligned to alignment, nothing changes
    void skipToAlign(size_t alignment) {
        auto delta = alignment - (cursor % alignment);
        cursor = cursor + delta;
    }

    std::optional<uint8_t> readU8() {
        if (cursor + 1 > dataLen) { return std::nullopt; }

        auto x = data[cursor];

        cursor += 1;
        return x;
    }

    std::optional<uint32_t> readU32() {
        if (cursor + 4 > dataLen) { return std::nullopt; }

        std::array<uint8_t, 4> raw {
            data[cursor + 0],
            data[cursor + 1],
            data[cursor + 2],
            data[cursor + 3],
        };

        uint32_t x = (
            (static_cast<uint32_t>(raw[0]) << 24) |
            (static_cast<uint32_t>(raw[1]) << 16) |
            (static_cast<uint32_t>(raw[2]) << 8) |
            (static_cast<uint32_t>(raw[3]) << 0)
        );

        cursor += 4;
        return x;
    }

    std::optional<int32_t> readI32() {
        if (auto x = readU32(); x.has_value()) {
            return static_cast<int32_t>(*x);
        } else {
            return std::nullopt;
        }
    }

    std::optional<uint32_t> readU64() {
        if (cursor + 8 > dataLen) { return std::nullopt; }

        std::array<uint8_t, 8> raw {
            data[cursor + 0],
            data[cursor + 1],
            data[cursor + 2],
            data[cursor + 3],
            data[cursor + 4],
            data[cursor + 5],
            data[cursor + 6],
            data[cursor + 7],
        };

        uint64_t x = (
            (static_cast<uint64_t>(raw[0]) << 24) |
            (static_cast<uint64_t>(raw[1]) << 24) |
            (static_cast<uint64_t>(raw[2]) << 24) |
            (static_cast<uint64_t>(raw[3]) << 24) |
            (static_cast<uint64_t>(raw[4]) << 24) |
            (static_cast<uint64_t>(raw[5]) << 16) |
            (static_cast<uint64_t>(raw[6]) << 8) |
            (static_cast<uint64_t>(raw[7]) << 0)
        );

        cursor += 8;
        return x;
    }

    std::optional<float> readF32() {
        if (auto x = readU32(); x.has_value()) {
            return std::bit_cast<float>(*x);
        } else {
            return std::nullopt;
        }
    }

    std::optional<std::string> readString() {
        // there needs to be at least 4 bytes, 1 nul terminator and padding
        if (cursor + 4 > dataLen) { return std::nullopt; }

        size_t endpoint = cursor;

        while (data[endpoint] != 0) {
            endpoint += 1;

            // there wasn't a nul terminator in the buffer, bail
            if (endpoint >= dataLen) {
                return std::nullopt;
            }
        }

        auto s = std::string(data + cursor, data + endpoint);

        cursor += endpoint - cursor;
        skipToAlign(4);
        return s;
    }

    std::optional<QString> readQString() {
        if (auto x = readString(); x.has_value()) {
            return QString::fromStdString(*x);
        } else {
            return std::nullopt;
        }
    }

    std::optional<QByteArray> readQByteArray(size_t len) {
        if (cursor + len > dataLen) { return std::nullopt; }

        auto bytes = QByteArray(reinterpret_cast<const char*>(data + cursor), len);

        cursor += len;
        return bytes;
    }
};

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
    connect(this, &OSCScriptingInterface::receiveHostChanged, this, [this] {
        this->rebindSocket();
    });

    connect(_socket.get(), &QUdpSocket::readyRead, this, [this] {
        // multiple datagrams might arrive at once
        while (_socket->hasPendingDatagrams()) {
            readPacket();
        }
    });

    rebindSocket();
}

OSCScriptingInterface::~OSCScriptingInterface() {
    _socket->close();
    qCInfo(osc_cat) << "Closed listen socket";
}

void OSCScriptingInterface::rebindSocket() {
    _socket->abort();

    _socket->bind(
        QHostAddress(_receiveHost.get()),
        _receivePort.get(),
        QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint
    );

    qCInfo(
        osc_cat,
        "Listening on %s : %d",
        qUtf8Printable(_socket->localAddress().toString()),
        _socket->localPort()
    );
}

static bool readOscMessage(
    std::vector<std::pair<QString, QVariantList>>& messageList,
    ReadHelper& r
) {
    std::string addr;
    std::string argTypes;
    QVariantList args;

    if (
        auto s = r.readString();
        s.has_value() && !s->empty()
    ) {
        addr = *s;
    } else {
        qCritical(osc_cat, "Early EOF in packet, OSC address missing");
        return false;
    }

    if (addr == "#bundle") {
        // we can't actually do anything with this, but it's part of the bundle struct
        [[maybe_unused]] auto timetag = r.readU64();

        auto elemSize = r.readU32();

        while (elemSize.has_value()) {
            auto innerR = r.slice(*elemSize);

            if (!readOscMessage(messageList, innerR)) {
                return false;
            }

            elemSize = r.readU32();
        }

        return true;
    }

    if (
        auto s = r.readString();
        s.has_value() && !s->empty()
    ) {
        argTypes = *s;
    } else {
        qCritical(osc_cat, "Early EOF in packet, argument types string missing");
        return false;
    }

    if (argTypes[0] != ',') {
        qCritical(osc_cat, "Argument type string does not start with ','");
        return false;
    }

    // +1 to exclude the comma
    for (size_t arg = 1; arg < argTypes.length(); arg++) {
        auto startCursor = r.getCursor();

        switch (argTypes[arg]) {
            case OSCTag::Int: {
                auto value = r.readI32();

                if (!value.has_value()) {
                    qCritical(osc_cat, "Early EOF in packet (%s), reading int at offset %zu", addr.c_str(), startCursor);
                    return false;
                }

                args.append(QVariantMap {
                    {"type", QString(OSCTag::Int)},
                    {"value", value.value()},
                });
            } break;

            case OSCTag::Float: {
                auto value = r.readF32();

                if (!value.has_value()) {
                    qCritical(osc_cat, "Early EOF in packet (%s), reading float at offset %zu", addr.c_str(), startCursor);
                    return false;
                }

                args.append(QVariantMap {
                    {"type", QString(OSCTag::Float)},
                    {"value", value.value()},
                });
            } break;

            case OSCTag::String: {
                auto value = r.readQString();

                if (!value.has_value()) {
                    qCritical(osc_cat, "Early EOF in packet (%s), string starting at offset %zu", addr.c_str(), startCursor);
                    return false;
                }

                args.append(QVariantMap {
                    {"type", QString(OSCTag::String)},
                    {"value", value.value()},
                });
            } break;

            case OSCTag::Blob: {
                auto len = r.readU32();

                if (!len.has_value()) {
                    qCritical(osc_cat, "Early EOF in packet (%s), reading blob at offset %zu (length prefix)", addr.c_str(), startCursor);
                    return false;
                }

                startCursor = r.getCursor();

                auto bytes = r.readQByteArray(*len);

                if (!bytes.has_value()) {
                    qCritical(osc_cat, "Early EOF in packet (%s), reading blob at offset %zu (data)", addr.c_str(), startCursor);
                    return false;
                }

                args.append(QVariantMap {
                    {"type", QString(OSCTag::Blob)},
                    {"value", bytes.value()},
                });
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
                return false;
        }
    }

    messageList.push_back(std::make_pair(QString::fromStdString(addr), args));
    return true;
}

void OSCScriptingInterface::readPacket() {
    auto datagram = _socket->receiveDatagram();

    if (!datagram.isValid()) { return; }

    auto r = ReadHelper(datagram.data().constData(), datagram.data().size());

    std::vector<std::pair<QString, QVariantList>> q {};

    if (readOscMessage(q, r)) {
        if (q.size() == 1) {
            emit packetReceived(std::get<0>(q[0]), std::get<1>(q[0]));
        } else {
            QVariantList list {};

            for (const auto& [address, args] : q) {
                QVariantList msgList {};
                msgList.append(address);

                for (const auto& arg : args) {
                    msgList.append(arg);
                }
            }

            emit bundleReceived(list);
        }
    }
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
        engine->raiseException("Sending OSC bundles is unsupported");
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

    auto msgBytes = WriteHelper();

    msgBytes.pushString(address.toStdString());

    // comma at the start of the argument list, even if empty
    msgBytes.pushU8(',');

    auto bodyBytes = WriteHelper();

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
            msgBytes.pushU8(OSCTag::Null);
            continue;
        }

        switch (expectedType) {
            case QVariant::Int: {
                msgBytes.pushU8(OSCTag::Int);
                bodyBytes.pushI32(static_cast<int32_t>(value.toDouble()));
            } break;

            case QVariant::Double: {
                msgBytes.pushU8(OSCTag::Float);
                bodyBytes.pushF32(static_cast<float>(value.toDouble()));
            } break;

            case QVariant::String: {
                msgBytes.pushU8(OSCTag::String);

                auto stringArg = value.toString();

                if (stringArg.contains(QChar(0))) {
                    engine->raiseException("String contains null (0x00) character");
                    return engine->undefinedValue();
                }

                bodyBytes.pushString(stringArg.toStdString());
            } break;

            case QVariant::ByteArray: {
                msgBytes.pushU8(OSCTag::Blob);

                auto bytesArg = value.toByteArray();

                bodyBytes.pushU32(bytesArg.size());
                bodyBytes.pushBytes(bytesArg);
                bodyBytes.pushToAlign(4);
            } break;

            case QVariant::Bool:
                msgBytes.pushU8(value.toBool() ? OSCTag::True : OSCTag::False);
                break;

            default:
                engine->raiseException(QString("Unserializable type %1").arg(value.typeName()));
                return engine->undefinedValue();
        }
    }

    // pad argument type string to 4-byte boundary with at least one zero
    msgBytes.pushU8('\x00');
    msgBytes.pushToAlign(4);

    // pack the header + body data together for the datagram
    msgBytes.pushBytes(bodyBytes.getData());

    instance->_socket->writeDatagram(
        QByteArray(reinterpret_cast<const char*>(msgBytes.getData().data()), msgBytes.getData().size()),
        QHostAddress(instance->_sendHost.get()), instance->_sendPort.get()
    );

    return engine->undefinedValue();
}

void OSCScriptingInterface::setReceiveHost(QString host) {
    if (_receiveHost.get() == host) { return; }

    QHostAddress address;

    // only save if it's a valid address
    if (address.setAddress(host)) {
        _receiveHost.set(host);
    }

    emit receiveHostChanged();
}

void OSCScriptingInterface::setSendHost(QString host) {
    if (_sendHost.get() == host) { return; }

    QHostAddress address;

    // only save if it's a valid address
    if (address.setAddress(host)) {
        _sendHost.set(host);
    }
}

void OSCScriptingInterface::setSendPort(int port) {
    if (_sendPort.get() == port) { return; }

    if (port < 1 || port > 65535) {
        qCritical(osc_cat) << "setSendPort: invalid port" << port;
        return;
    }

    _sendPort.set(port);
}

void OSCScriptingInterface::setReceivePort(int port) {
    if (_receivePort.get() == port) { return; }

    if (port < 1024 || port > 65535) {
        qCritical(osc_cat) << "setReceivePort: invalid port" << port;
        return;
    }

    _receivePort.set(port);

    emit receiveHostChanged();
}
