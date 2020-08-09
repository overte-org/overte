//
//  BytesliceReader.inl
//  libraries/networking/src
//
//  Created by Heather Anderson on 2020-08-08.
//  Copyright 2020 Vircadia.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once

#ifndef networking_BytesliceReader_inl
#define networking_BytesliceReader_inl

#include "BytesliceReader.h"

ByteSliceReader::ByteSliceReader() {
    open(QIODevice::ReadOnly);
}

ByteSliceReader::ByteSliceReader(const ByteSlice& data) : _contents(data) {
    open(QIODevice::ReadOnly);
}

ByteSliceReader::ByteSliceReader(ByteSlice&& data) : _contents(std::move(data)) {
    open(QIODevice::ReadOnly);
}

ByteSlice ByteSliceReader::getContents() const {
    return _contents;
}

ByteSlice ByteSliceReader::remainingByteSlice() const {
    return _contents.substring(pos());
}

#endif /* networking_BytesliceReader_inl */