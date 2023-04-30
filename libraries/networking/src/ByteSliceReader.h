//
//  BytesliceReader.h
//  libraries/networking/src
//
//  Created by Heather Anderson on 2020-08-08.
//  Copyright 2020 Vircadia.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once

#ifndef networking_BytesliceReader_h
#define networking_BytesliceReader_h

#include <QtCore/QObject>
#include <QtCore/QByteArray>

#include "ByteSlice.h"

/* ByteSliceReader

   This class represents an IODevice that can be used to stream the contents of a ByteSlices.
*/
class ByteSliceReader : public QObject {
    Q_OBJECT
public:
    inline ByteSliceReader(){};
    inline ByteSliceReader(const ByteSlice& data) : _contents(data){};
    inline ByteSliceReader(ByteSlice&& data) : _contents(std::move(data)){};

    inline qint64 size() const { return _contents.length(); }
    inline ByteSlice getContents() const { return _contents; }
    void setContents(const ByteSlice& data);
    void setContents(ByteSlice&& data);
    inline void reset() { _pos = 0; }

    inline ByteSlice remainingByteSlice() const { return _contents.substring(_pos); };

    template<typename T> inline qint64 peekPrimitive(T* data) const;
    template<typename T> inline qint64 readPrimitive(T* data);

    qint64 peek(char* data, qint64 maxlen) const;
    qint64 read(char* data, qint64 maxlen);
    QByteArray peekByteArrayRef(qint64 maxlen) const;
    QByteArray readByteArrayRef(qint64 maxlen);
    qint64 getBytesLeftToRead() const;

private:
    ByteSlice _contents;
    size_t _pos{ 0 };
};

template<typename T> qint64 ByteSliceReader::peekPrimitive(T* data) const {
    return peek(reinterpret_cast<char*>(data), sizeof(T));
}

template<typename T> qint64 ByteSliceReader::readPrimitive(T* data) {
    return read(reinterpret_cast<char*>(data), sizeof(T));
}

#endif /* networking_BytesliceReader_h */