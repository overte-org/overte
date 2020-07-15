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

#include <QtCore/QtGlobal>

namespace udt4 {

class PacketID {
public:
    // Base type of sequence numbers
    using Type = qint32;
    using UType = quint32;
    
    // Values are for 31 bit PacketID
    static const Type MAX = 0x7FFFFFFF; // maximum packet ID used in UDT
    
    PacketID() = default;
    inline PacketID(const PacketID& other);
    inline PacketID(PacketID&& other);
    
    // Only explicit conversions
    explicit inline PacketID(Type value);
    explicit inline PacketID(UType value);
    explicit inline operator Type() const;
    explicit inline operator UType() const;
    
    inline PacketID& operator++();
    inline PacketID& operator--();
    inline PacketID operator++(int);
    inline PacketID operator--(int);
    inline PacketID& operator=(const PacketID& other);
    inline PacketID& operator=(PacketID&& other);
    inline PacketID& operator+=(Type inc);
    inline PacketID& operator-=(Type dec);
    
    inline bool operator==(const PacketID& other) const;
    inline bool operator!=(const PacketID& other) const;

    inline Type blindDifference(const PacketID& rhs) const;
    
    inline bool operator<(const PacketID& rhs) const;
    inline bool operator>(const PacketID& rhs) const;
    inline bool operator<=(const PacketID& rhs) const;
    inline bool operator>=(const PacketID& rhs) const;
    
private:
    UType _value { 0 };
};
static_assert(sizeof(PacketID) == sizeof(uint32_t), "PacketID invalid size");

inline PacketID operator+(PacketID a, const PacketID::Type& b);
inline PacketID operator+(const PacketID::Type& a, PacketID b);
inline PacketID operator-(PacketID a, const PacketID::Type& b);
inline PacketID operator-(const PacketID::Type& a, PacketID b);
    
} // namespace udt4

#include "PacketID.inl"
#endif  // hifi_udt4_PacketID_h
