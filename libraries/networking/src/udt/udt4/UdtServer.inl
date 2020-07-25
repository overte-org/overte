//
//  UdtServer.inl
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-05-25.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_udt4_UdtServer_inl
#define hifi_udt4_UdtServer_inl

#include "UdtServer.h"

namespace udt4 {

inline UdtServer::AcceptFlags UdtServer::acceptFlags() const {
    return _acceptFlags;
}

inline QString UdtServer::errorString() const {
    return _errorString;
}

inline bool UdtServer::isListening() const {
    return !_multiplexer.isNull();
}

inline qint64 UdtServer::listenReplayWindow() const {
    return _listenReplayWindow;
}

inline int UdtServer::maxPendingConnections() const {
    return _maxPendingConn.load();
}

inline void UdtServer::pauseAccepting() {
    _pausePendingConn.store(1);
}

inline void UdtServer::resumeAccepting() {
    _pausePendingConn.store(0);
}

inline QAbstractSocket::SocketError UdtServer::serverError() const {
    return _serverError;
}

inline void UdtServer::setAcceptFlags(AcceptFlags flags) {
    _acceptFlags = flags;
}

inline void UdtServer::setListenReplayWindow(qint64 msecs) {
    _listenReplayWindow = msecs;
}

}  // namespace udt4

#endif /* hifi_udt4_UdtServer_inl */