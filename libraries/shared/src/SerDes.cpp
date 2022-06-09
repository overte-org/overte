//
//  SerDes.h
//
//
//  Created by Dale Glass on 5/6/2022
//  Copyright 2022 Dale Glass
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <cctype>

#include "SerDes.h"
const int  SerDes::DEFAULT_SIZE;
const char SerDes::PADDING_CHAR;

QDebug operator<<(QDebug debug, const SerDes &ds) {
    debug << "{ capacity =" << ds.capacity() << "; length = " << ds.length() << "; pos = " << ds.pos() << "}";
    debug << "\n";

    QString literal;
    QString hex;

    for(size_t i=0;i<ds.length();i++) {
        char c = ds._store[i];

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

        if ( literal.length() == 16 || (i+1 == ds.length()) ) {
            while( literal.length() < 16 ) {
                literal.append(" ");
                hex.append("   ");
            }

            debug << literal << "  " << hex << "\n";
            literal.clear();
            hex.clear();
        }
    }

    return debug;
}


void SerDes::changeAllocation(size_t new_size) {
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
