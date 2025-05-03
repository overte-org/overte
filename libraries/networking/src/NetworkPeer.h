//
//  NetworkPeer.h
//  libraries/networking/src
//
//  Created by Stephen Birarda on 2014-10-02.
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2021 Vircadia contributors.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_NetworkPeer_h
#define hifi_NetworkPeer_h

#include <atomic>

#include <QtCore/QObject>
#include <QtCore/QSharedPointer>
#include <QtCore/QTimer>
#include <QtCore/QUuid>

#include "SockAddr.h"
#include "UUID.h"

const QString ICE_SERVER_HOSTNAME = "localhost";
const quint16 ICE_SERVER_DEFAULT_PORT = 7337;
const int ICE_HEARBEAT_INTERVAL_MSECS = 1 * 1000;
const int MAX_ICE_CONNECTION_ATTEMPTS = 5;

const int UDP_PUNCH_PING_INTERVAL_MS = 250;

class NetworkPeer : public QObject {
    Q_OBJECT
public:
    NetworkPeer(QObject* parent = 0);
    NetworkPeer(const QUuid& uuid, const SockAddr& publicSocketIPv4, const SockAddr& publicSocketIPv6,
        const SockAddr& localSocketIPv4, const SockAddr& localSocketIPv6, QObject* parent = 0);

    bool isNull() const { return _uuid.isNull(); }
    bool hasSockets() const { return (!_localSocketIPv4.isNull() || !_localSocketIPv6.isNull())
        && (!_publicSocketIPv4.isNull() || !_publicSocketIPv6.isNull()); }

    const QUuid& getUUID() const { return _uuid; }
    void setUUID(const QUuid& uuid) { _uuid = uuid; }

    using LocalID = NetworkLocalID;
    static const LocalID NULL_LOCAL_ID = 0;

    LocalID getLocalID() const { return _localID; }
    void setLocalID(LocalID localID) { _localID = localID; }

    void softReset();
    void reset();

    //const SockAddr& getPublicSocket() const { return _publicSocket; }
    //const SockAddr& getLocalSocket() const { return _localSocket; }
    //const SockAddr& getSymmetricSocket() const { return _symmetricSocket; }

    const SockAddr& getPublicSocketIPv4() const { return _publicSocketIPv4; }
    const SockAddr& getLocalSocketIPv4() const { return _localSocketIPv4; }
    const SockAddr& getSymmetricSocketIPv4() const { return _symmetricSocketIPv4; }
    const SockAddr& getPublicSocketIPv6() const { return _publicSocketIPv6; }
    const SockAddr& getLocalSocketIPv6() const { return _localSocketIPv6; }
    const SockAddr& getSymmetricSocketIPv6() const { return _symmetricSocketIPv6; }

    void setPublicSocketIPv4(const SockAddr& publicSocket);
    void setPublicSocketIPv6(const SockAddr& publicSocket);
    void setLocalSocketIPv4(const SockAddr& localSocket);
    void setLocalSocketIPv6(const SockAddr& localSocket);
    void setSymmetricSocket(const SockAddr& symmetricSocket);

    const SockAddr* getActiveSocket() const { return _activeSocket; }

    void activatePublicSocket(QAbstractSocket::NetworkLayerProtocol protocol);
    void activateLocalSocket(QAbstractSocket::NetworkLayerProtocol protocol);
    void activateSymmetricSocket(QAbstractSocket::NetworkLayerProtocol protocol);

    void activateMatchingOrNewSymmetricSocket(const SockAddr& matchableSockAddr);

    quint64 getWakeTimestamp() const { return _wakeTimestamp; }
    void setWakeTimestamp(quint64 wakeTimestamp) { _wakeTimestamp = wakeTimestamp; }

    quint64 getLastHeardMicrostamp() const { return _lastHeardMicrostamp; }
    void setLastHeardMicrostamp(quint64 lastHeardMicrostamp) { _lastHeardMicrostamp = lastHeardMicrostamp; }

    QByteArray toByteArray() const;

    int getConnectionAttempts() const  { return _connectionAttempts; }
    void incrementConnectionAttempts() { ++_connectionAttempts; }
    void resetConnectionAttempts() { _connectionAttempts = 0; }

    // Typically the LimitedNodeList removes nodes after they are "silent"
    // meaning that we have not received any packets (including simple keepalive pings) from them for a set interval.
    // The _isForcedNeverSilent flag tells the LimitedNodeList that a Node should never be killed by removeSilentNodes()
    // even if its the timestamp of when it was last heard from has never been updated.

    bool isForcedNeverSilent() const { return _isForcedNeverSilent; }
    void setIsForcedNeverSilent(bool isForcedNeverSilent) { _isForcedNeverSilent = isForcedNeverSilent; }

    friend QDataStream& operator<<(QDataStream& out, const NetworkPeer& peer);
    friend QDataStream& operator>>(QDataStream& in, NetworkPeer& peer);
public slots:
    void startPingTimer();
    void stopPingTimer();

signals:
    void pingTimerTimeout();
    void socketActivated(const SockAddr& sockAddr);
    void socketUpdated(SockAddr previousAddress, SockAddr currentAddress);

protected:
    void setActiveSocket(SockAddr* discoveredSocket);

    QUuid _uuid;
    LocalID _localID { 0 };

    // TODO(IPv6): these should be available separately for IPv4 and IPv6
    //SockAddr _publicSocket;
    SockAddr _publicSocketIPv4;
    SockAddr _publicSocketIPv6;
    //SockAddr _localSocket;
    SockAddr _localSocketIPv4;
    SockAddr _localSocketIPv6;
    //SockAddr _symmetricSocket;
    SockAddr _symmetricSocketIPv4;
    SockAddr _symmetricSocketIPv6;
    SockAddr* _activeSocket;

    quint64 _wakeTimestamp;
    std::atomic_ullong _lastHeardMicrostamp;

    QTimer* _pingTimer = NULL;

    int _connectionAttempts;

    bool _isForcedNeverSilent { false };
};

QDebug operator<<(QDebug debug, const NetworkPeer &peer);
typedef QSharedPointer<NetworkPeer> SharedNetworkPeer;

#endif // hifi_NetworkPeer_h
