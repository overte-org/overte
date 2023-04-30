//
//  ByteSliceReader.cpp
//  libraries/networking/src
//
//  Created by Heather Anderson on 2020-08-08.
//  Copyright 2020 Vircadia.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <algorithm>
#include "BytesliceReader.h"

void ByteSliceReader::setContents(const ByteSlice& data) {
    quint64 oldSize = _contents.length();
    _contents = data;
    if (oldSize != _contents.length()) {
        reset();
    }
}

void ByteSliceReader::setContents(ByteSlice&& data) {
    quint64 oldSize = _contents.length();
    _contents = std::move(data);
    if (oldSize != _contents.length()) {
        reset();
    }
}

qint64 ByteSliceReader::peek(char* data, qint64 maxSize) const {
    // we're either reading what is left from the current position or what was asked to be read
    qint64 numBytesToRead = std::min(static_cast<qint64>(std::max(size() - _pos, 0ULL)), maxSize);

    if (numBytesToRead > 0) {
        // read out the data
        memcpy(data, _contents.constData() + _pos, numBytesToRead);
    }

    return numBytesToRead;
}

qint64 ByteSliceReader::read(char* data, qint64 maxSize) {
    qint64 numBytesToRead = peek(data, maxSize);
    if (numBytesToRead > 0) {
        _pos += numBytesToRead;
    }
    return numBytesToRead;
}

QByteArray ByteSliceReader::peekByteArrayRef(qint64 maxSize) const {
    // we're either reading what is left from the current position or what was asked to be read
    qint64 numBytesToRead = std::min(static_cast<qint64>(std::max(size() - _pos, 0ULL)), maxSize);

    return QByteArray { QByteArray::fromRawData(reinterpret_cast<const char *>(_contents.constData() + _pos), numBytesToRead) };
}

QByteArray ByteSliceReader::readByteArrayRef(qint64 maxSize) {
    QByteArray data = peekByteArrayRef(maxSize);
    qint64 numBytesToRead = data.length();

    if (numBytesToRead > 0) {
        _pos += numBytesToRead;
    }
    return data;
}
