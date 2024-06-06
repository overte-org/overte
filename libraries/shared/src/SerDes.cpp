//
//  SerDes.h
//
//
//  Created by Dale Glass on 5/6/2022
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <cctype>

#include "SerDes.h"
const int  DataSerializer::DEFAULT_SIZE;
const char DataSerializer::PADDING_CHAR;


static void dumpHex(QDebug &debug, const char*buf, size_t len) {
  QString literal;
    QString hex;

    for(size_t i=0;i<len;i++) {
        char c = buf[i];

        if (std::isalnum(c)) {
            literal.append(c);
        } else {
            literal.append(".");
        }

        QString hnum = QString::number( static_cast<unsigned char>(c), 16 );
        if ( hnum.length() == 1 ) {
            hnum.prepend("0");
        }

        hex.append(hnum  + " ");

        if ( literal.length() == 16 || (i+1 == len) ) {
            while( literal.length() < 16 ) {
                literal.append(" ");
                hex.append("   ");
            }

            debug << literal << "  " << hex << "\n";
            literal.clear();
            hex.clear();
        }
    }
}


QDebug operator<<(QDebug debug, const DataSerializer &ser) {
    debug << "{ capacity =" << ser.capacity() << "; length = " << ser.length() << "; pos = " << ser.pos() << "}";
    debug << "\n";

    dumpHex(debug, ser.buffer(), ser.length());
    return debug;
}


QDebug operator<<(QDebug debug, const DataDeserializer &des) {
    debug << "{ length = " << des.length() << "; pos = " << des.pos() << "}";
    debug << "\n";


    dumpHex(debug, des.buffer(), des.length());
    return debug;
}


void DataSerializer::changeAllocation(size_t new_size) {
    while ( _capacity < new_size) {
        _capacity *= 2;
    }

    char *new_buf = new char[_capacity];
    assert( *new_buf );

    memcpy(new_buf, _store, _length);
    char *prev_buf = _store;
    _store = new_buf;

    delete []prev_buf;
}
