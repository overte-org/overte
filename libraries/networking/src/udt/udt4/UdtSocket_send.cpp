//
//  UdtSocket_send.cpp
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-06-20.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "UdtSocket_send.h"
#include "UdtSocket.h"

using namespace udt4;

UdtSocket_send::UdtSocket_send(UdtSocket_private& socket):_socket(socket) {
    _SNDtimer.setSingleShot(true);
    _SNDtimer.setTimerType(Qt::PreciseTimer);
    connect(&_SNDtimer, &QTimer::timeout, this, &UdtSocket_send::SNDevent);

    _EXPtimer.setSingleShot(true);
    _EXPtimer.setTimerType(Qt::PreciseTimer);
    connect(&_EXPtimer, &QTimer::timeout, this, &UdtSocket_send::EXPevent);
}

void UdtSocket_send::configureHandshake(const HandshakePacket& hsPacket, bool resetSequence) {
    if (resetSequence) {
        _lastAckPacketID = hsPacket._initPktSeq;
        _sendPacketID = hsPacket._initPktSeq;
	}
    _flowWindowSize = static_cast<uint>(hsPacket._maxFlowWinSize);
}

void UdtSocket_send::startupInit() {
    _flagRecentReceivedPacket = false;
    _flagRecentEXPevent = false;
    _flagRecentSNDevent = false;
    _flagSendDisconnect = false;
    _lastReceiveTime.start();
    _msgPartialSend.reset();
    _expCount = 1;
    _messageSequence = 0UL;
    _sentAck2 = 0UL;
    _ACK2SentTimer.setRemainingTime(0); // default expired
    _receivedPacketList.clear();
    resetEXP();
}

// the main event loop for the "send" side of the socket, this controls the behavior and permitted actions
void UdtSocket_send::run() {
    startupInit();
	for(;;) {
        QMutexLocker guard(&_eventMutex);
        while (!processEvent(guard)) {
            if (_sendState == SendState::Closed) {
                // socket is closed, leave this thread
                return;
            }
            _eventCondition.wait(&_eventMutex);
        }
    }
}

void UdtSocket_send::setState(UdtSocketState newState) {
    QMutexLocker guard(&_eventMutex);
    _socketState = newState;
    _eventCondition.notify_all();
}

void UdtSocket_send::resetReceiveTimer() {
    QMutexLocker guard(&_eventMutex);
    _flagRecentReceivedPacket = true;
    _eventCondition.notify_all();
}

void UdtSocket_send::EXPevent() {
    QMutexLocker guard(&_eventMutex);
    _flagRecentEXPevent = true;
    _eventCondition.notify_all();
}

void UdtSocket_send::SNDevent() {
    QMutexLocker guard(&_eventMutex);
    _flagRecentSNDevent = true;
    _eventCondition.notify_all();
}

void UdtSocket_send::queueDisconnect() {
    QMutexLocker guard(&_eventMutex);
    _flagSendDisconnect = true;
    _eventCondition.notify_all();
}

void UdtSocket_send::sendMessage(ByteSlice content, QDeadlineTimer expireTime) {
    MessageEntryPointer message = MessageEntryPointer::create(content);
    message->expireTime = expireTime;

    QMutexLocker guard(&_eventMutex);
    _pendingMessages.append(message);
    _eventCondition.notify_all();
}

void UdtSocket_send::packetReceived(const Packet& udtPacket, const QElapsedTimer& timeReceived) {
    ReceivedPacket packet(udtPacket, timeReceived);
    QMutexLocker guard(&_eventMutex);
    _receivedPacketList.append(packet);
    _eventCondition.notify_all();
}

// the main event loop for the "send" side of the socket, this controls the behavior and permitted actions
bool UdtSocket_send::processEvent(QMutexLocker& eventGuard) {

    if (_flagRecentReceivedPacket && _sendState != SendState::Shutdown) {
        _flagRecentReceivedPacket = false;
        _flagRecentEXPevent = false;
        eventGuard.unlock();
        _expCount = 1;
        resetEXP();
    }

    bool canSendPacket = false;
    switch (_sendState) {
	case SendState::Idle: // not waiting for anything, can send immediately
        canSendPacket = true;
        break;
	case SendState::ProcessDrop: // immediately re-process any drop list requests
        eventGuard.unlock();
		_sendState = reevalSendState(); // try to reconstruct what our state should be if it wasn't sendStateProcessDrop
        if (!processSendLoss() || (static_cast<quint32>(_sendPacketID) % 16) == 0) {
			processSendExpire();
		}
		return true;
	}

    switch (_socketState) {
    case UdtSocketState::Connected: // this is the expected state while we are running
        break;
    case UdtSocketState::HalfClosed:
        if (_sendState != SendState::Shutdown) {
            _sendState = SendState::Shutdown;
            _flagRecentEXPevent = false;
            eventGuard.unlock();
            if (_EXPtimer.isActive()) {  // don't process EXP events if we're shutting down
                _EXPtimer.stop();
            }
            return true;
        }
        break;
    default: // not a running state
        _sendState = SendState::Closed;
        return false;
    }

    if (canSendPacket) {
        if (_msgPartialSend != nullptr) {  // we have a partial message waiting, try to send more of it now
            eventGuard.unlock();
            processDataMsg(false);
            return true;
        }
        if (!_pendingMessages.isEmpty()) {
            _msgPartialSend = _pendingMessages.takeFirst();
            eventGuard.unlock();
            processDataMsg(true);
            return true;
        }
        if (_flagSendDisconnect) {
            _flagSendDisconnect = false;
            eventGuard.unlock();
            Packet shutdownPacket;
            shutdownPacket._type = PacketType::Shutdown;
            _socket.sendPacket(shutdownPacket);
            return true;
        }
    }

    if (!_receivedPacketList.isEmpty()) {
        ReceivedPacket recvPacket = _receivedPacketList.takeFirst();
        switch (recvPacket.udtPacket._type) {
        case PacketType::Ack:
			ingestAck(ACKPacket(recvPacket.udtPacket), recvPacket.timeReceived);
            break;
        case PacketType::Nak:
            ingestNak(NAKPacket(recvPacket.udtPacket), recvPacket.timeReceived);
            break;
        case PacketType::Congestion:
            ingestCongestion(recvPacket.udtPacket, recvPacket.timeReceived);
            break;
        }
		_sendState = reevalSendState();
    }

    if (_flagRecentEXPevent) {
        _flagRecentEXPevent = false;
        eventGuard.unlock();
        processExpEvent();
        return true;
    }

    if (_flagRecentSNDevent) {
        _flagRecentSNDevent = false;
        if (_sendState == SendState::Sending) {
            eventGuard.unlock();
            _sendState = reevalSendState();
            if (!processSendLoss() || (static_cast<quint32>(_sendPacketID) % 16) == 0) {
				processSendExpire();
			}
            return true;
        }
	}

    // no events seen to process
    return false;
}

void UdtSocket_send::setPacketSendPeriod(quint32 SND) { // exported
	// check to see if we have a bandwidth limit here
	maxBandwidth := _socket.Config.MaxBandwidth;
	if (maxBandwidth > 0) {
		minSP := time.Second / time.Duration(float64(maxBandwidth)/float64(_socket.mtu.get()));
		if (snd < minSP) {
			snd = minSP;
		}
	}

	_sndPeriod.set(snd);
}

UdtSocket_send::SendState UdtSocket_send::reevalSendState() const {
    if (_SNDtimer.isActive()) {
		return SendState::Sending;
	}

	// Do we have too many unacknowledged packets for us to send any more?
	if (_sendPktPend != nullptr) {
		congestWindow := uint(_congestWindow.get());
		uint cwnd = _flowWindowSize;
		if (cwnd > congestWindow) {
			cwnd = congestWindow;
		}
		if (cwnd >= uint(len(_sendPktPend))) {
			return SendState::Waiting;
		}
	}
	return SendState::Idle;
}

// try to pack a new data packet and send it
void UdtSocket_send::processDataMsg(bool isFirst) {
	while (_msgPartialSend != nullptr) {
        MessageEntryPointer partialSend = _msgPartialSend;
		DataPacket::MessagePosition state = DataPacket::MessagePosition::Only;
		if (_socket.isDatagram) {
			if(isFirst) {
				state = DataPacket::MessagePosition::First;
			} else {
				state = DataPacket::MessagePosition::Middle;
			}
		}
		if (isFirst || !_socket.isDatagram) {
			_messageSequence++;
		}

		unsigned mtu = _socket.mtu.get();
		unsigned msgLen = partialSend->content.length();
		if (msgLen >= mtu) {
			// we are full -- send what we can and leave the rest
			DataPacket dataPacket;
            dataPacket._packetID = _sendPacketID;
			if (msgLen == mtu) {
                dataPacket._contents = partialSend->content;
				_msgPartialSend.reset();
			} else {
                dataPacket._contents = partialSend->content.substring(0, mtu);
				_msgPartialSend->content = partialSend->content.substring(mtu);
			}
			_sendPacketID++;
            dataPacket._messagePosition = state;
            dataPacket._isOrdered = !_socket.isDatagram;
            dataPacket._messageNumber = _messageSequence;
			sendDataPacket(sendPacketEntry{pkt: dataPacket, tim: partialSend->sendTime, ttl: partialSend->expireTime}, false);
			return;
		}

		// we are not full -- send only if this is a datagram or there's nothing obvious left
		if (_socket.isDatagram) {
			if(isFirst) {
				state = DataPacket::MessagePosition::Only;
			} else {
				state = DataPacket::MessagePosition::Last;
			}
		} else {
            QMutexLocker guard(&_eventMutex);
            if(!_pendingMessages.isEmpty()) {
                MessageEntryPointer morePartialSend = _pendingMessages.takeFirst();
                _msgPartialSend->content.append(morePartialSend->content);
				continue;
			}
		}

		partialSend = _msgPartialSend;
		DataPacket dataPacket;
        dataPacket._packetID = _sendPacketID;
        dataPacket._contents = partialSend->content;
		_msgPartialSend.reset();
        _sendPacketID++;
        dataPacket._messagePosition = state;
        dataPacket._isOrdered = !_socket.isDatagram;
        dataPacket._messageNumber = _messageSequence;
		sendDataPacket(sendPacketEntry{pkt: dp, tim: partialSend->sendTime, ttl: partialSend->expireTime}, false);
		return;
	}
}

// If the sender's loss list is not empty, retransmit the first packet in the list and remove it from the list.
bool UdtSocket_send::processSendLoss() {
    if (_sendLossList == nullptr || _sendPktPend == nullptr) {
		return false;
	}

	SendPacketEntryPointer dataPacket;
	for(;;) {
		minLoss, minLossIdx := _sendLossList.Min(_lastAckPacketID, _sendPacketID);
		if (minLossIdx < 0) {
			// empty loss list? shouldn't really happen as we don't keep empty lists, but check for it anyhow
			return false;
		}

		heap.Remove(&_sendLossList, minLossIdx);
		if (len(_sendLossList) == 0) {
			_sendLossList = nullptr;
		}

		dataPacket, _ = _sendPktPend.Find(minLoss);
		if (dataPacket == nullptr) {
			// can't find record of this packet, not much we can do really
			continue;
		}

		if (dataPacket->expireTime.hasExpired()) {
			// this packet has expired, ignore
			continue;
		}

		break;
	}

	sendDataPacket(dataPacket, true);
	return true;
}

// evaluate our pending packet list to see if we have any expired messages
bool UdtSocket_send::processSendExpire() {
	if (_sendPktPend == nullptr) {
		return false;
	}

	pktPend := make([]sendPacketEntry, len(_sendPktPend));
	copy(pktPend, _sendPktPend);
	for (_, p := range pktPend) {
		if (p.ttl != 0 && time.Now().Add(p.ttl).After(p.tim)) {
			// this message has expired, drop it
			_, _, msgNo := p.pkt.GetMessageData();

            MessageDropRequestPacket dropMessage;
            dropMessage._messageID = msgNo;
            dropMessage._firstPacketID = p.pkt;
            dropMessage._lastPacketID = p.pkt;

			// find the other packets in this message
			for (_, op := range pktPend) {
				_, _, otherMsgNo := op.pkt.GetMessageData();
				if (otherMsgNo == msgNo) {
					if (dropMsg.FirstSeq.blindDifference(p.pkt.Seq) > 0) {
						dropMsg.FirstSeq = p.pkt.Seq;
					}
					if (dropMsg.LastSeq.blindDifference(p.pkt.Seq) < 0) {
						dropMsg.LastSeq = p.pkt.Seq;
					}
				}
				if (_sendLossList != nullptr) {
					if (_, slIdx := _sendLossList.Find(p.pkt.Seq); slIdx >= 0) {
						heap.Remove(&_sendLossList, slIdx);
					}
				}
			}
			if (_sendLossList != nullptr && len(_sendLossList) == 0) {
				_sendLossList = nullptr;
			}

			_sendPacket <- dropMessage;
			return true;
		}
	}
	return false;
}

// we have a packed packet and a green light to send, so lets send this and mark it
void UdtSocket_send::sendDataPacket(SendPacketEntryPointer dataPacket, bool isResend) {
	_sendPktPend.insert(dataPacket);
	_socket.cong.onDataPktSent(dataPacket->packet._packetID);
	_sendPacket <- dataPacket->packet;

	// have we exceeded our recipient's window size?
	_sendState = reevalSendState();
	if (_sendState == SendState::Waiting) {
		return;
	}

	if (!isResend && (static_cast<quint32>(dataPacket->packet._packetID)%16) == 0) {
		processSendExpire();
		return;
	}

	snd := _sndPeriod.get();
	if (snd > 0) {
		_SNDtimer.start(snd);
		_sendState = SendState::Sending;
	}
}

bool UdtSocket_send::assertValidSentPktID(const char* pktType, const PacketID& packetID) const {
    if (_sendPacketID.blindDifference(packetID) < 0) {
		_shutdownEvent <- shutdownMessage{sockState: sockStateCorrupted, permitLinger: false,
			err: fmt.Errorf("FAULT: Received an %s for packet %d, but the largest packet we've sent has been %d", pktType, packetID.Seq, _sendPacketID.Seq)};
		return false;
	}
	return true;
}

// ingestAck is called to process an ACK packet
void UdtSocket_send::ingestAck(const ACKPacket& ackPacket, const QElapsedTimer& timeReceived) {
	// Update the largest acknowledged sequence number.

    if (ackPacket._ackType == ACKPacket::AckType::Light) {
	    PacketID lastPacketReceived = ackPacket._lastPacketReceived;
        if (!assertValidSentPktID("ACK", lastPacketReceived)) {
		    return;
	    }
        qint32 diff = lastPacketReceived.blindDifference(_lastAckPacketID);
	    if (diff > 0) {
		    _flowWindowSize += diff;
		    _lastAckPacketID = lastPacketReceived;
	    }
    }

	// Send back an ACK2 with the same ACK sequence number in this ACK.
    if (_ACK2SentTimer.hasExpired() && ackPacket._ackSequence == _sentAck2) {
        _sentAck2 = ackPacket._ackSequence;
		_sendPacket <- &packet.Ack2Packet{AckSeqNo: ackPacket._ackSequence};
        _ACK2SentTimer.setRemainingTime(UdtSocket::SYN, Qt::PreciseTimer);
	}

	PacketID lastPacketReceived = ackPacket._lastPacketReceived;
        if (!assertValidSentPktID("ACK", lastPacketReceived)) {
		return;
	}
	qint32 diff = lastPacketReceived.blindDifference(_lastAckPacketID);
	if (diff <= 0) {
		return;
	}

	PacketID oldAckSeq = _lastAckPacketID;
	_flowWindowSize = ackPacket._availBufferSize;
	_lastAckPacketID = lastPacketReceived;

	// Update RTT and RTTVar.
	_socket.applyRTT(ackPacket._rtt);

	// Update flow window size.
	if (ackPacket._ackType == ACKPacket::AckType::Full) {
		_socket.applyReceiveRates(ackPacket._packetReceiveRate, ackPacket._estimatedLinkCapacity);
	}

	_socket.cong.onACK(lastPacketReceived);

	// Update packet arrival rate: A = (A * 7 + a) / 8, where a is the value carried in the ACK.
	// Update estimated link capacity: B = (B * 7 + b) / 8, where b is the value carried in the ACK.

	// Update sender's buffer (by releasing the buffer that has been acknowledged).
	if (_sendPktPend != nullptr) {
		for(;;) {
			minLoss, minLossIdx := _sendPktPend.Min(oldAckSeq, _sendPacketID);
			if (lastPacketReceived.blindDifference(minLoss.Seq) >= 0 || minLossIdx < 0) {
				break;
			}
			heap.Remove(&_sendPktPend, minLossIdx);
		}
	}

	// Update sender's loss list (by removing all those that has been acknowledged).
	if (_sendLossList != nullptr) {
		for(;;) {
			minLoss, minLossIdx := _sendLossList.Min(oldAckSeq, _sendPacketID);
			if (lastPacketReceived.blindDifference(minLoss) >= 0 || minLossIdx < 0) {
				break;
			}
			heap.Remove(&_sendLossList, minLossIdx);
		}
	}
}

// ingestNak is called to process an NAK packet
void UdtSocket_send::ingestNak(const NAKPacket& nakPacket, const QElapsedTimer& timeReceived) {
	newLossList := make([]packet.PacketID, 0);
	for (NAKPacket::IntegerList::const_iterator trans = nakPacket._lossData.begin(); trans != nakPacket._lossData.end(); trans++) {
		quint32 thisEntry = *trans;
		if ((thisEntry&0x80000000) != 0) {
			PacketID thisPacketID(thisEntry);
            trans++;
            if(trans == nakPacket._lossData.end()) {
				_shutdownEvent <- shutdownMessage{sockState: sockStateCorrupted, permitLinger: false,
					err: fmt.Errorf("FAULT: While unpacking a NAK, the last entry (%x) was describing a start-of-range", thisEntry)};
				return;
			}
            if (!assertValidSentPktID("NAK", thisPacketID)) {
				return;
			}
            PacketID lastEntry = *trans;
			if ((lastEntry&0x80000000) != 0) {
				_shutdownEvent <- shutdownMessage{sockState: sockStateCorrupted, permitLinger: false,
					err: fmt.Errorf("FAULT: While unpacking a NAK, a start-of-range (%x) was followed by another start-of-range (%x)", thisEntry, lastEntry)};
				return;
			}
			PacketID lastPacketID(lastEntry);
			if (!assertValidSentPktID("NAK", lastPacketID)) {
				return;
			}
			for (PacketID span = thisPacketID; span != lastPacketID; span++) {
				newLossList = append(newLossList, span);
			}
		} else {
			PacketID thisPacketID(thisEntry);
			if (!assertValidSentPktID("NAK", thisEntry)) {
				return;
			}
			newLossList = append(newLossList, thisPacketID);
		}
	}

	_socket.cong.onNAK(newLossList);

	if(_sendLossList == nullptr) {
		_sendLossList = newLossList;
		heap.Init(&_sendLossList);
	} else {
		llen := len(newLossList);
		for (idx := 0; idx < llen; idx++) {
			heap.Push(&_sendLossList, newLossList[idx]);
		}
	}

	_sendState = SendState::ProcessDrop; // immediately restart transmission
}

// ingestCongestion is called to process a (retired?) Congestion packet
void UdtSocket_send::ingestCongestion(const Packet& udtPacket, const QElapsedTimer& timeReceived) {
	// One way packet delay is increasing, so decrease the sending rate
	// this is very rough (not atomic, doesn't inform congestion) but this is a deprecated message in any case
    _sndPeriod.set(_sndPeriod.get() * 1125 / 1000);
	//m_iLastDecSeq = s.sendPktSeq;
}

void UdtSocket_send::resetEXP() {
    _lastReceiveTime.start();

	std::chrono::milliseconds nextExpDurn;
    std::chrono::milliseconds rtoPeriod(_rtoPeriod.get());
	if (rtoPeriod.count() > 0) {
		nextExpDurn = rtoPeriod;
	} else {
        std::chrono::microseconds rtt, rttVariance;
        _socket.getRTT(rtt, rttVariance);

        nextExpDurn = std::chrono::milliseconds(_expCount * (rtt.count() + 4 * rttVariance.count()) / 1000) + UdtSocket::SYN;
        std::chrono::milliseconds minExpTime(_expCount * MIN_EXP_INTERVAL);
		if (nextExpDurn.count() < minExpTime.count()) {
			nextExpDurn = minExpTime;
		}
	}
	_EXPtimer.start(nextExpDurn);
}

// we've just had the EXP timer expire, see what we can do to recover this
void UdtSocket_send::processExpEvent() {

	// Haven't receive any information from the peer, is it dead?!
	// timeout: at least 16 expirations and must be greater than 10 seconds
    if ((_expCount > 16) && (currTime.Sub(_lastReceiveTime) > 5 * time.Second)) {
		// Connection is broken.
		_shutdownEvent <- shutdownMessage{sockState: sockStateTimeout, permitLinger: true};
		return;
	}

	// sender: Insert all the packets sent after last received acknowledgement into the sender loss list.
	// recver: Send out a keep-alive packet
	if (_sendPktPend != nullptr) {
		if (_sendPktPend != nullptr && _sendLossList == nullptr) {
			// resend all unacknowledged packets on timeout, but only if there is no packet in the loss list
			newLossList := make([]packet.PacketID, 0);
			for(PacketID span = _lastAckPacketID + 1; span != _sendPacketID + 1; span++) {
				newLossList = append(newLossList, span);
			}
			_sendLossList = newLossList;
			heap.Init(&_sendLossList);
		}
		_socket.cong.onTimeout();
		_sendState = SendState::ProcessDrop; // immediately restart transmission
	} else {
		_sendPacket <- &packet.KeepAlivePacket{};
	}

	_expCount++;
	// Reset last response time since we just sent a heart-beat.
	resetEXP();
}
