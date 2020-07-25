//
//  UdtSocket_cc.cpp
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-07-23.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "UdtSocket_cc.h"
#include "UdtSocket.h"

using namespace udt4;

PacketID UdtSocket_CongestionControl::getSendCurrentPacketID() const {
    return PacketID(_lastSentPacketID.load());
}

void UdtSocket_CongestionControl::setCongestionWindowSize(unsigned pkt) {
    _congestionWindow = pkt;
	_socket.setCongestionWindow(pkt);
}

unsigned UdtSocket_CongestionControl::getCongestionWindowSize() const {
    return _congestionWindow;
}

std::chrono::milliseconds UdtSocket_CongestionControl::getPacketSendPeriod() const {
    return _sndPeriod;
}

void UdtSocket_CongestionControl::setPacketSendPeriod(std::chrono::milliseconds snd) {
    _sndPeriod = snd;
	_socket.setPacketSendPeriod(snd);
}

unsigned UdtSocket_CongestionControl::getMaxFlowWindow() const {
    return _socket.getMaxFlowWinSize();
}

void UdtSocket_CongestionControl::getReceiveRates(unsigned& recvSpeed, unsigned& bandwidth) const {
    _socket.getReceiveRates(recvSpeed, bandwidth);
}

std::chrono::microseconds UdtSocket_CongestionControl::getRTT() const {
    std::chrono::microseconds rtt, rttVariance;
    _socket.getRTT(rtt, rttVariance);
    return rtt;
}

unsigned UdtSocket_CongestionControl::getMSS() const {
    return _socket.getMTU();
}

void UdtSocket_CongestionControl::setACKPeriod(std::chrono::milliseconds ack) {
    _socket.setACKperiod(ack);
}

void UdtSocket_CongestionControl::setACKInterval(unsigned ack) {
    _socket.setACKinterval(ack);
}

void UdtSocket_CongestionControl::setRTOPeriod(std::chrono::milliseconds rto) {
    _socket.setRTOperiod(rto);
}
