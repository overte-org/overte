//
//  PacketID.h
//  libraries/networking/src/udt/udt4
//
//  Created by Clement on 7/23/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_udt4_PacketID_h
#define hifi_udt4_PacketID_h

#include <functional>
#include <QtCore/QHash>
#include <QtCore/QtGlobal>

namespace udt4 {

template <int BITS>
class WrappedSequence {
public:
    // Base type of sequence numbers
    using Type = qint32;
    using UType = quint32;

    // Values are for 31 bit PacketID
    static constexpr UType MAX = (1UL << BITS) - 1;   // maximum value that can be stored
    static constexpr UType SIGN = 1UL << (BITS - 1);  // the "sign" bit when doing comparisons

    WrappedSequence() = default;
    inline WrappedSequence(const WrappedSequence& other) : _value(other._value) {}
    inline WrappedSequence(WrappedSequence&& other) : _value(std::move(other._value)) {}

    // Only explicit conversions
    explicit inline WrappedSequence(UType value) : _value(value & MAX) {}
    explicit inline operator UType() const { return static_cast<UType>(_value); }

    inline WrappedSequence& operator++() { _value = (_value + 1) & MAX; return *this; }
    inline WrappedSequence& operator--() { _value = (_value - 1) & MAX; return *this; }
    inline WrappedSequence operator++(int);
    inline WrappedSequence operator--(int);
    inline WrappedSequence& operator=(const WrappedSequence& other) { _value = other._value; return *this; }
    inline WrappedSequence& operator=(UType value) { _value = value & MAX; return *this; }
    inline WrappedSequence& operator=(WrappedSequence&& other) noexcept;
    inline WrappedSequence& operator+=(Type inc) { _value = (_value + inc) & MAX; return *this; }
    inline WrappedSequence& operator-=(Type dec) { _value = (_value - dec) & MAX; return *this; }

    inline bool operator==(const WrappedSequence& other) const { return _value == other._value; }
    inline bool operator!=(const WrappedSequence& other) const { return _value != other._value; }

    inline Type blindDifference(const WrappedSequence& rhs) const;

    // these do not provide a strict ordering, please do not use this as a key in QMap (although QHash is okay)
    inline bool operator<(const WrappedSequence& rhs) const { return blindDifference(rhs) < 0; }
    inline bool operator>(const WrappedSequence& rhs) const { return blindDifference(rhs) > 0; }
    inline bool operator<=(const WrappedSequence& rhs) const { return blindDifference(rhs) <= 0; }
    inline bool operator>=(const WrappedSequence& rhs) const { return blindDifference(rhs) >= 0; }

private:
    UType _value{ 0 };
};

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
inline WrappedSequence<BITS>& WrappedSequence<BITS>::operator=(WrappedSequence&& other) noexcept {
    if (&other != this) {
        _value = other._value;
    }
    return *this;
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
inline uint qHash(const WrappedSequence<BITS>& key) {
    return ::qHash(static_cast<quint32>(key));
}

template <int BITS>
inline uint qHash(const WrappedSequence<BITS>& key, uint seed) {
    return ::qHash(static_cast<quint32>(key), seed);
}

// WrappedSequenceLess is a functor used when using WrappedSequence objects inside of std::map objects
// (which is permitted, unlike QMap which doesn't permit explicitly specified ordering
template <class T>
struct WrappedSequenceLess : std::binary_function<T, T, bool> {
    inline bool operator()(const T& lhs, const T& rhs) const { return static_cast<quint32>(lhs) < static_cast<quint32>(rhs); }
};

typedef WrappedSequence<31> PacketID;
static_assert(sizeof(PacketID) == sizeof(quint32), "PacketID invalid size");

typedef WrappedSequence<29> SequenceNumber;  // either an ACK number or a Message number
static_assert(sizeof(SequenceNumber) == sizeof(quint32), "SequenceNumber invalid size");

using MessageNumber = SequenceNumber;
using ACKSequence = SequenceNumber;

}  // namespace udt4

#endif  // hifi_udt4_PacketID_h
