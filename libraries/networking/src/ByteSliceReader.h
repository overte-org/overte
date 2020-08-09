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

#include "ByteSlice.h"
#include "ExtendedIODevice.h"
/* ByteSliceReader

   This class represents an IODevice that can be used to stream the contents of a ByteSlices.
*/
class ByteSliceReader : public ExtendedIODevice {
    Q_OBJECT
public:
    inline ByteSliceReader();
    inline ByteSliceReader(const ByteSlice& data);
    inline ByteSliceReader(ByteSlice&& data);

    inline ByteSlice getContents() const;
    void setContents(const ByteSlice& data);
    void setContents(ByteSlice&& data);

    inline ByteSlice remainingByteSlice() const;

    // QIODevice virtual functions
    virtual bool isSequential() const override;
    virtual qint64 size() const override;

protected:
    // QIODevice virtual functions
    virtual qint64 writeData(const char* data, qint64 maxSize) override;
    virtual qint64 readData(char* data, qint64 maxSize) override;

private:
    ByteSlice _contents;
};

#include "BytesliceReader.inl"
#endif /* networking_BytesliceReader_h */