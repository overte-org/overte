//
//  Byteslice.cpp
//  libraries/networking/src/serialization
//
//  Created by Heather Anderson on 2020-05-01.
//  Copyright 2020 Vircadia.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "Byteslice.h"

ByteSlice::Bytestring::Bytestring(const quint8* source, size_t length) {
    quint8* myContent = new quint8[length];
    if (source != nullptr) {
        memcpy(myContent, source, length);
    } else {
        memset(myContent, 0, length);
    }
    _content = myContent;
    _fullLength = length;
}

ByteSlice::Bytestring::~Bytestring() {
    delete[] _content;
}

quint8 ByteSlice::pop_front() {
    if (!_length) {
        return 0;
    }
    --_length;
    return _content->_content[_offset++];
}

ByteSlice ByteSlice::substring(size_t offset, size_t length /* = static_cast<size_t>(-1) */) const {
    if (offset >= _length) {
        return ByteSlice();
    }
    size_t maxLen = _length - offset;
    if (length == static_cast<size_t>(-1)) {
        return ByteSlice(_content, _offset + offset, maxLen);
    } else {
        return ByteSlice(_content, _offset + offset, std::min(maxLen, length));
    }
}

// create a new buffer and return a pointer to it
void* ByteSlice::create(size_t length) {
    _offset = 0;
    _length = length;
    _content = BytestringPointer::create(nullptr, length);
    return const_cast<quint8*>(_content->_content);
}
