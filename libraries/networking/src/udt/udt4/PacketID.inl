//
//  PacketID.inl
//  libraries/networking/src/udt/udt4
//
//  Created by Clement on 7/23/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_udt4_PacketID_inl
#define hifi_udt4_PacketID_inl

#include "PacketID.h"

namespace udt4 {

inline PacketID::PacketID(const PacketID& other) : _value(other._value & MAX) {
}

inline PacketID::PacketID(PacketID&& other) : _value(other._value) {
}

inline PacketID::PacketID(Type value) {
    _value = (value <= MAX) ? ((value >= 0) ? value : 0) : MAX;
}

inline PacketID::PacketID(UType value) {
    _value = (value <= MAX) ? value : MAX;
}

inline PacketID::operator Type() const {
    return _value;
}

inline PacketID::operator UType() const {
    return static_cast<UType>(_value);
}
    
inline PacketID& PacketID::operator++() {
    _value = (_value + 1) & MAX;
    return *this;
}
inline PacketID& PacketID::operator--() {
    _value = (_value - 1) & MAX;
    return *this;
}

inline PacketID PacketID::operator++(int) {
    PacketID before = *this;
    ++(*this);
    return before;
}

inline PacketID PacketID::operator--(int) {
    PacketID before = *this;
    --(*this);
    return before;
}
    
inline PacketID& PacketID::operator=(const PacketID& other) {
    _value = other._value;
    return *this;
}

inline PacketID& PacketID::operator=(PacketID&& other) {
    if (&other != this) {
        _value = other._value;
    }
    return *this;
}

inline PacketID& PacketID::operator+=(Type inc) {
    _value = (_value + inc) & MAX;
    return *this;
}

inline PacketID& PacketID::operator-=(Type dec) {
    _value = (_value - dec) & MAX;
    return *this;
}
    
inline bool PacketID::operator==(const PacketID& other) const {
    return _value == other._value;
}

inline bool PacketID::operator!=(const PacketID& other) const {
    return _value != other._value;
}

inline PacketID::Type PacketID::blindDifference(const PacketID& rhs) const {
    UType diff = (_value - rhs._value) & PacketID::MAX;
    if ((diff & 0x40000000) != 0) {
        diff = diff | 0x80000000;
	}
	return static_cast<Type>(diff);
}
    
inline bool PacketID::operator<(const PacketID& rhs) const {
    return blindDifference(rhs) < 0;
}

inline bool PacketID::operator>(const PacketID& rhs) const {
    return blindDifference(rhs) > 0;
}

inline bool PacketID::operator<=(const PacketID& rhs) const {
    return blindDifference(rhs) <= 0;
}

inline bool PacketID::operator>=(const PacketID& rhs) const {
    return blindDifference(rhs) >= 0;
}
    
inline PacketID operator+(PacketID a, const PacketID::Type& b) {
    a += b;
    return a;
}

inline PacketID operator+(const PacketID::Type& a, PacketID b) {
    b += a;
    return b;
}

inline PacketID operator-(PacketID a, const PacketID::Type& b) {
    a -= b;
    return a;
}

inline PacketID operator-(const PacketID::Type& a, PacketID b) {
    b -= a;
    return b;
}

}  // namespace udt4

#endif // hifi_udt4_PacketID_inl
