//
//  NodeConnectionData.cpp
//  domain-server/src
//
//  Created by Stephen Birarda on 2015-08-24.
//  Copyright 2015 High Fidelity, Inc.
//  Copyright 2021 Vircadia contributors.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "NodeConnectionData.h"

#include <QtCore/QDataStream>

NodeConnectionData NodeConnectionData::fromDataStream(QDataStream& dataStream, const SockAddr& senderSockAddr,
                                                      bool isConnectRequest) {
    NodeConnectionData newHeader;

    if (isConnectRequest) {
        dataStream >> newHeader.connectUUID;

        // Read out the protocol version signature from the connect message
        char* rawBytes;
        uint length;

        dataStream.readBytes(rawBytes, length);
        newHeader.protocolVersion = QByteArray(rawBytes, length);

        // NOTE: QDataStream::readBytes() - The buffer is allocated using new []. Destroy it with the delete [] operator.
        delete[] rawBytes;

        // read the hardware address sent by the client
        dataStream >> newHeader.hardwareAddress;

        // now the machine fingerprint
        dataStream >> newHeader.machineFingerprint;

        // and the operating system type
        QByteArray compressedSystemInfo;
        dataStream >> compressedSystemInfo;
        if (!compressedSystemInfo.isEmpty()) {
            newHeader.SystemInfo = qUncompress(compressedSystemInfo);
        }

        dataStream >> newHeader.connectReason;

        dataStream >> newHeader.previousConnectionUpTime;
    }

    dataStream >> newHeader.lastPingTimestamp;

    // TODO(IPv6): publicSockAddrIPv4.getType() is specific to v4, is it ok that v6 uses same type?
    SocketType publicSocketType, localSocketType;
    dataStream >> newHeader.nodeType
        >> publicSocketType >> newHeader.publicSockAddrIPv4 >> newHeader.publicSockAddrIPv6 // TODO(IPv6): make sure that it's serialized the same way
        >> localSocketType >> newHeader.localSockAddrIPv4 >> newHeader.localSockAddrIPv6
        >> newHeader.interestList >> newHeader.placeName;
    newHeader.publicSockAddrIPv4.setType(publicSocketType);
    newHeader.publicSockAddrIPv6.setType(publicSocketType);
    newHeader.localSockAddrIPv4.setType(localSocketType);
    newHeader.localSockAddrIPv6.setType(localSocketType);

    // For WebRTC connections, the user client's signaling channel WebSocket address is used instead of the actual data 
    // channel's address.
    if (senderSockAddr.getType() == SocketType::WebRTC) {
        if (newHeader.publicSockAddrIPv4.getType() != SocketType::WebRTC
            || newHeader.publicSockAddrIPv6.getType() != SocketType::WebRTC
            || newHeader.localSockAddrIPv4.getType() != SocketType::WebRTC
            || newHeader.localSockAddrIPv6.getType() != SocketType::WebRTC) {
            qDebug() << "Inconsistent WebRTC socket types!";
        }

        // We don't know whether it's a public or local connection so set both the same.
        // TODO(IPv6):
        //auto address =
        //    !senderSockAddr.getAddressIPv6().isNull() ? senderSockAddr.getAddressIPv6() : senderSockAddr.getAddressIPv4();
        auto port = senderSockAddr.getPort();
        Q_ASSERT(!(!senderSockAddr.getAddressIPv6().isNull() && !senderSockAddr.getAddressIPv4().isNull()));
        if (!senderSockAddr.getAddressIPv6().isNull()) {
            newHeader.publicSockAddrIPv6.setAddress(senderSockAddr.getAddressIPv6());
            newHeader.publicSockAddrIPv6.setPort(port);
            newHeader.localSockAddrIPv6.setAddress(senderSockAddr.getAddressIPv6());
            newHeader.localSockAddrIPv6.setPort(port);
        } else {
            newHeader.publicSockAddrIPv4.setAddress(senderSockAddr.getAddressIPv4());
            newHeader.publicSockAddrIPv4.setPort(port);
            newHeader.localSockAddrIPv4.setAddress(senderSockAddr.getAddressIPv4());
            newHeader.localSockAddrIPv4.setPort(port);
        }
    }

    newHeader.senderSockAddr = senderSockAddr;
    
    // TODO(IPv6): test this
    if (newHeader.publicSockAddrIPv4.getAddressIPv4().isNull() && newHeader.publicSockAddrIPv6.getAddressIPv6().isNull()) {
        // this node wants to use us as its STUN server
        // set the node's public address to whatever we perceive the public address to be

        // if the sender is on our box then leave its public address to 0 so that
        // other users attempt to reach it on the same address they have for the domain-server
        if (senderSockAddr.getAddressIPv4().isLoopback() || senderSockAddr.getAddressIPv6().isLoopback()) {
            newHeader.publicSockAddrIPv4.setAddress(QHostAddress());
            newHeader.publicSockAddrIPv6.setAddress(QHostAddress());
        } else {
            // prefer IPv6 if available
            if (!senderSockAddr.getAddressIPv6().isNull()) {
                newHeader.publicSockAddrIPv6.setAddress(senderSockAddr.getAddressIPv6());
            } else {
                newHeader.publicSockAddrIPv4.setAddress(senderSockAddr.getAddressIPv4());
            }
        }
    }
    
    return newHeader;
}
