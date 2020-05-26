//
//  Byteslice.inl
//  libraries/networking/src/serialization
//
//  Created by Heather Anderson on 2020-05-01.
//  Copyright 2020 Vircadia.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once

#ifndef serialization_Byteslice_inl
#define serialization_Byteslice_inl

#include "Byteslice.h"

ByteSlice::ByteSlice() : _offset(0), _length(0) {
}

ByteSlice::ByteSlice(const QByteArray& data) :
    _offset(0), _length(data.length()),
    _content(BytestringPointer::create(reinterpret_cast<const quint8*>(data.constData()), _length)) {
}

ByteSlice::ByteSlice(const std::string& data) :
    _offset(0), _length(data.length()),
    _content(BytestringPointer::create(reinterpret_cast<const quint8*>(data.c_str()), _length)) {
}

ByteSlice::ByteSlice(BytestringPointer content, size_t offset, size_t length) :
    _offset(offset), _length(length), _content(content) {
}

// internal-only, for creating substrings
ByteSlice::ByteSlice(const ByteSlice& data) : _offset(data._offset), _length(data._length), _content(data._content) {
}

bool ByteSlice::empty() const {
    return !_length;
}

size_t ByteSlice::length() const {
    return _length;
}

const quint8& ByteSlice::operator[](size_t idx) {
    if (_content.isNull() || idx > _length) {
        return gl_fallback;
    } else {
        return _content->_content[idx];
    }
}

#endif /* serialization_Byteslice_inl */