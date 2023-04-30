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

#include "NativeCC.h"
#include <QtCore/QMutexLocker>
#include "UdtSocket.h"

using namespace udt4;

UdtSocket_CongestionControl::UdtSocket_CongestionControl(UdtSocket_private& socket) : _socket(socket) {
    _congestion = CongestionControlPointer(new NativeCongestionControl);
    start();
}

UdtSocket_CongestionControl::~UdtSocket_CongestionControl() {
    _inShutdown = true;
    wait();
}

void UdtSocket_CongestionControl::setCongestionControl(CongestionControlPointer congestionControl) {
    _congestion = congestionControl;
}

void UdtSocket_CongestionControl::run() {
    for (;;) {
        QMutexLocker guard(&_eventProtect);
        while (!_inShutdown && _events.empty()) {
            _eventCondition.wait(&_eventProtect);
        }
        if (_inShutdown) {
            return;
        }

        // using std::swap so we can use move semantics here
        Event thisEvent;
        std::swap(thisEvent, _events.front());
        _events.pop_front();

        guard.unlock();
        handleEvent(thisEvent);
    }
}

void UdtSocket_CongestionControl::handleEvent(const Event& event) {
    switch (event.type) {
        case EventType::init:
            _congestion->init(*this);
            break;
        case EventType::close:
            _congestion->close(*this);
            break;
        case EventType::onACK:
            _congestion->onACK(*this, event.packetIDs.front());
            break;
        case EventType::onNAK:
            _congestion->onNAK(*this, event.packetIDs);
            break;
        case EventType::onTimeout:
            _congestion->onTimeout(*this);
            break;
        case EventType::onPacketSent:
            _congestion->onPacketSent(*this, event.udtPacket);
            break;
        case EventType::onPacketReceived:
            _congestion->onPacketReceived(*this, event.udtPacket, event.timeReceived);
            break;
        case EventType::onCustomMessageReceived:
            _congestion->onCustomMessageReceived(*this, event.udtPacket, event.timeReceived);
            break;
    }
}

void UdtSocket_CongestionControl::submitEvent(Event&& event) {
    QMutexLocker guard(&_eventProtect);
    _events.push_back(std::move(event));
    _eventCondition.wakeAll();
}

/* Implementing calls in from UDTSocket and its child objects */

void UdtSocket_CongestionControl::init(const PacketID& packetID, unsigned mtu) {
    _lastSentPacketID.store(static_cast<quint32>(packetID));
    _mtu.store(mtu);

    Event event;
    event.type = EventType::init;
    submitEvent(std::move(event));
}

void UdtSocket_CongestionControl::close() {
    Event event;
    event.type = EventType::close;
    submitEvent(std::move(event));
}

void UdtSocket_CongestionControl::onACK(const PacketID& lastPacketReceived) {
    Event event;
    event.type = EventType::onACK;
    event.packetIDs.push_back(lastPacketReceived);
    submitEvent(std::move(event));
}

void UdtSocket_CongestionControl::onNAK(QList<PacketID>&& packetIDs) {
    Event event;
    event.type = EventType::onNAK;
    event.packetIDs = std::move(packetIDs);
    submitEvent(std::move(event));
}

void UdtSocket_CongestionControl::onTimeout() {
    Event event;
    event.type = EventType::onTimeout;
    submitEvent(std::move(event));
}

void UdtSocket_CongestionControl::onDataPacketSent(const PacketID& packetID) {
    _lastSentPacketID.store(static_cast<quint32>(packetID));
}

void UdtSocket_CongestionControl::onPacketSent(const Packet& udtPacket) {
    Event event;
    event.type = EventType::onPacketSent;
    event.udtPacket = udtPacket;
    submitEvent(std::move(event));
}

void UdtSocket_CongestionControl::onPacketReceived(const Packet& udtPacket, const QElapsedTimer& timeReceived) {
    Event event;
    event.type = EventType::onPacketReceived;
    event.udtPacket = udtPacket;
    event.timeReceived = timeReceived;
    submitEvent(std::move(event));
}

void UdtSocket_CongestionControl::onCustomMessageReceived(const Packet& udtPacket, const QElapsedTimer& timeReceived) {
    Event event;
    event.type = EventType::onCustomMessageReceived;
    event.udtPacket = udtPacket;
    event.timeReceived = timeReceived;
    submitEvent(std::move(event));
}

/* CongestionControlParms interface implementation */

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

std::chrono::microseconds UdtSocket_CongestionControl::getPacketSendPeriod() const {
    return _sndPeriod;
}

void UdtSocket_CongestionControl::setPacketSendPeriod(std::chrono::microseconds snd) {
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
    return _mtu.load();
}

void UdtSocket_CongestionControl::setACKPeriod(std::chrono::microseconds ack) {
    _socket.setACKperiod(ack);
}

void UdtSocket_CongestionControl::setACKInterval(unsigned ack) {
    _socket.setACKinterval(ack);
}

void UdtSocket_CongestionControl::setRTOPeriod(std::chrono::microseconds rto) {
    _socket.setRTOperiod(rto);
}
