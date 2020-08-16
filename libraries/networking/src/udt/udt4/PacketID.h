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
    inline WrappedSequence(const WrappedSequence& other);
    inline WrappedSequence(WrappedSequence&& other);

    // Only explicit conversions
    explicit inline WrappedSequence(UType value);
    explicit inline operator UType() const;

    inline WrappedSequence& operator++();
    inline WrappedSequence& operator--();
    inline WrappedSequence operator++(int);
    inline WrappedSequence operator--(int);
    inline WrappedSequence& operator=(const WrappedSequence& other);
    inline WrappedSequence& operator=(UType value);
    inline WrappedSequence& operator=(WrappedSequence&& other) noexcept;
    inline WrappedSequence& operator+=(Type inc);
    inline WrappedSequence& operator-=(Type dec);

    inline bool operator==(const WrappedSequence& other) const;
    inline bool operator!=(const WrappedSequence& other) const;

    inline Type blindDifference(const WrappedSequence& rhs) const;

    // these do not provide a strict ordering, please do not use this as a key in QMap (although QHash is okay)
    inline bool operator<(const WrappedSequence& rhs) const;
    inline bool operator>(const WrappedSequence& rhs) const;
    inline bool operator<=(const WrappedSequence& rhs) const;
    inline bool operator>=(const WrappedSequence& rhs) const;

private:
    UType _value{ 0 };
};

template <int BITS>
inline WrappedSequence<BITS> operator+(WrappedSequence<BITS> a, qint32 b);
template <int BITS>
inline WrappedSequence<BITS> operator+(qint32 a, WrappedSequence<BITS> b);
template <int BITS>
inline WrappedSequence<BITS> operator-(WrappedSequence<BITS> a, qint32 b);
template <int BITS>
inline WrappedSequence<BITS> operator-(qint32 a, WrappedSequence<BITS> b);

template <int BITS>
inline uint qHash(const WrappedSequence<BITS>& key);
template <int BITS>
inline uint qHash(const WrappedSequence<BITS>& key, uint seed);

// WrappedSequenceLess is a functor used when using WrappedSequence objects inside of std::map objects
// (which is permitted, unlike QMap which doesn't permit explicitly specified ordering
template <class T>
struct WrappedSequenceLess : std::binary_function<T, T, bool> {
    inline bool operator()(const T& lhs, const T& rhs) const;
};

typedef WrappedSequence<31> PacketID;
static_assert(sizeof(PacketID) == sizeof(quint32), "PacketID invalid size");

typedef WrappedSequence<29> SequenceNumber;  // either an ACK number or a Message number
static_assert(sizeof(SequenceNumber) == sizeof(quint32), "SequenceNumber invalid size");

using MessageNumber = SequenceNumber;
using ACKSequence = SequenceNumber;

}  // namespace udt4

#include "PacketID.inl"
#endif  // hifi_udt4_PacketID_h
