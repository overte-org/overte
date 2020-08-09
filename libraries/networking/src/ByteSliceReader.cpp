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

bool ByteSliceReader::isSequential() const {
    return false;
}

qint64 ByteSliceReader::size() const {
    return _contents.length();
}

qint64 ByteSliceReader::writeData(const char*, qint64) {
    // ByteSlice is read only.  Shouldn't get here unless someone re-open()ed the device as read-write
    return -1;
}

qint64 ByteSliceReader::readData(char* data, qint64 maxSize) {
    // we're either reading what is left from the current position or what was asked to be read
    qint64 currentPosition = pos();
    qint64 numBytesToRead = std::min(std::max(size() - currentPosition, 0LL), maxSize);

    if (numBytesToRead > 0) {
        // read out the data
        memcpy(data, _contents.constData() + currentPosition, numBytesToRead);
    }

    return numBytesToRead;
}
