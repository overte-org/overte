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

template <int BITS>
inline WrappedSequence<BITS>::WrappedSequence(const WrappedSequence& other) : _value(other._value) {
}

template <int BITS>
inline WrappedSequence<BITS>::WrappedSequence(WrappedSequence&& other) : _value(other._value) {
}

template <int BITS>
inline WrappedSequence<BITS>::WrappedSequence(UType value) {
    _value = value & MAX;
}

template <int BITS>
inline WrappedSequence<BITS>::operator UType() const {
    return static_cast<UType>(_value);
}
    
template <int BITS>
inline WrappedSequence<BITS>& WrappedSequence<BITS>::operator++() {
    _value = (_value + 1) & MAX;
    return *this;
}

template <int BITS>
inline WrappedSequence<BITS>& WrappedSequence<BITS>::operator--() {
    _value = (_value - 1) & MAX;
    return *this;
}

template <int BITS>
inline WrappedSequence<BITS> WrappedSequence<BITS>::operator++(int) {
    WrappedSequence before = *this;
    ++(*this);
    return before;
}

template <int BITS>
inline WrappedSequence<BITS> WrappedSequence<BITS>::operator--(int) {
    WrappedSequence before = *this;
    --(*this);
    return before;
}
    
template <int BITS>
inline WrappedSequence<BITS>& WrappedSequence<BITS>::operator=(const WrappedSequence& other) {
    _value = other._value;
    return *this;
}

template <int BITS>
inline WrappedSequence<BITS>& WrappedSequence<BITS>::operator=(UType value) {
    _value = value & MAX;
    return *this;
}

template <int BITS>
inline WrappedSequence<BITS>& WrappedSequence<BITS>::operator=(WrappedSequence&& other) noexcept {
    if (&other != this) {
        _value = other._value;
    }
    return *this;
}

template <int BITS>
inline WrappedSequence<BITS>& WrappedSequence<BITS>::operator+=(Type inc) {
    _value = (_value + inc) & MAX;
    return *this;
}

template <int BITS>
inline WrappedSequence<BITS>& WrappedSequence<BITS>::operator-=(Type dec) {
    _value = (_value - dec) & MAX;
    return *this;
}
    
template <int BITS>
inline bool WrappedSequence<BITS>::operator==(const WrappedSequence& other) const {
    return _value == other._value;
}

template <int BITS>
inline bool WrappedSequence<BITS>::operator!=(const WrappedSequence& other) const {
    return _value != other._value;
}

template <int BITS>
inline typename WrappedSequence<BITS>::Type WrappedSequence<BITS>::blindDifference(const WrappedSequence& rhs) const {
    UType diff = (_value - rhs._value) & WrappedSequence::MAX;
    if ((diff & SIGN) != 0) {
        diff = diff | ~MAX;
	}
	return static_cast<Type>(diff);
}
    
template <int BITS>
inline bool WrappedSequence<BITS>::operator<(const WrappedSequence& rhs) const {
    return blindDifference(rhs) < 0;
}

template <int BITS>
inline bool WrappedSequence<BITS>::operator>(const WrappedSequence& rhs) const {
    return blindDifference(rhs) > 0;
}

template <int BITS>
inline bool WrappedSequence<BITS>::operator<=(const WrappedSequence& rhs) const {
    return blindDifference(rhs) <= 0;
}

template <int BITS>
inline bool WrappedSequence<BITS>::operator>=(const WrappedSequence& rhs) const {
    return blindDifference(rhs) >= 0;
}
    
template <int BITS>
inline WrappedSequence<BITS> operator+(WrappedSequence<BITS> a, qint32 b) {
    a += b;
    return a;
}

template <int BITS>
inline WrappedSequence<BITS> operator+(qint32 a, const WrappedSequence<BITS> b) {
    b += a;
    return b;
}

template <int BITS>
inline WrappedSequence<BITS> operator-(WrappedSequence<BITS> a, qint32 b) {
    a -= b;
    return a;
}

template <int BITS>
inline WrappedSequence<BITS> operator-(qint32 a, WrappedSequence<BITS> b) {
    b -= a;
    return b;
}

template <int BITS>
uint qHash(const WrappedSequence<BITS>& key) {
    return ::qHash(static_cast<quint32>(key));
}

template <int BITS>
uint qHash(const WrappedSequence<BITS> & key, uint seed) {
    return ::qHash(static_cast<quint32>(key), seed);
}

template <class T>
bool WrappedSequenceLess<T>::operator()(const T& lhs, const T& rhs) const {
    return static_cast<quint32>(lhs) < static_cast<quint32>(rhs);
}

}  // namespace udt4

#endif // hifi_udt4_PacketID_inl
