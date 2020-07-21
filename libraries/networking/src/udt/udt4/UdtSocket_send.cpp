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
    _ACK2SentTimer.setRemainingTime(-1);
    resetEXP(evt.now);
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

void UdtSocket_send::sendMessage(MessageEntryPointer message) {
    QMutexLocker guard(&_eventMutex);
    _pendingMessages.append(message);
    _eventCondition.notify_all();
}

// the main event loop for the "send" side of the socket, this controls the behavior and permitted actions
bool UdtSocket_send::processEvent(QMutexLocker& eventGuard) {

    if (_flagRecentReceivedPacket && _sendState != SendState::Shutdown) {
        _flagRecentReceivedPacket = false;
        _flagRecentEXPevent = false;
        eventGuard.unlock();
        _expCount = 1;
        resetEXP(evt.now);
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
            processDataMsg(true);
            return true;
        }
        if (_flagSendDisconnect) {
            _flagSendDisconnect = false;
            _sendPacket <- &packet.ShutdownPacket{};
            return true;
        }
    }

	select {
	case evt, ok := <-sendEvent:
		switch sp := evt.pkt.(type) {
		case *packet.AckPacket:
			ingestAck(sp, evt.now);
		case *packet.LightAckPacket:
			ingestLightAck(sp, evt.now);
		case *packet.NakPacket:
			ingestNak(sp, evt.now);
		case *packet.CongestionPacket:
			ingestCongestion(sp, evt.now);
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
            if (!processSendLoss() || _sendPacketID.Seq % 16 == 0) {
				processSendExpire();
			}
            return true;
        }
	}

    // no events seen to process
    return false;
}
/*
const (
	minEXPinterval time.Duration = 300 * time.Millisecond
)
*/
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
void UdtSocket_send::processDataMsg(bool isFirst, <-chan sendMessage inChan) {
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
		minLoss, minLossIdx := _sendLossList.Min(_recvAckSeq, _sendPacketID);
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

// ingestLightAck is called to process a "light" ACK packet
void UdtSocket_send::ingestLightAck(LightAckPacket* p, time.Time now) {
	// Update the largest acknowledged sequence number.

	pktSeqHi := p.PktSeqHi;
	diff := pktSeqHi.blindDifference(_recvAckSeq);
	if (diff > 0) {
		_flowWindowSize += uint(diff);
		_recvAckSeq = pktSeqHi;
	}
}

bool UdtSocket_send::assertValidSentPktID(QString pktType, PacketID packetID) {
	if (_sendPacketID.blindDifference(pktSeq) < 0) {
		_shutdownEvent <- shutdownMessage{sockState: sockStateCorrupted, permitLinger: false,
			err: fmt.Errorf("FAULT: Received an %s for packet %d, but the largest packet we've sent has been %d", pktType, pktSeq.Seq, _sendPacketID.Seq)};
		return false;
	}
	return true;
}

// ingestAck is called to process an ACK packet
void UdtSocket_send::ingestAck(const ACKPacket& ackPacket, time.Time now) {
	// Update the largest acknowledged sequence number.

	// Send back an ACK2 with the same ACK sequence number in this ACK.
	if (_ack2SentEvent == nullptr && ackPacket.AckSeqNo == s.sentAck2) {
		_sentAck2 = ackPacket.AckSeqNo;
		_sendPacket <- &packet.Ack2Packet{AckSeqNo: ackPacket.AckSeqNo};
		_ack2SentEvent = time.After(synTime);
	}

	PacketID pktSeqHi := ackPacket._lastPacketReceived;
	if (!assertValidSentPktID("ACK", pktSeqHi)) {
		return;
	}
	diff := pktSeqHi.blindDifference(_recvAckSeq);
	if (diff <= 0) {
		return;
	}

	oldAckSeq := _recvAckSeq;
	_flowWindowSize = uint(ackPacket._availBufferSize);
	_recvAckSeq = pktSeqHi;

	// Update RTT and RTTVar.
	_socket.applyRTT(ackPacket._rtt);

	// Update flow window size.
	if (ackPacket._ackType == ACKPacket::AckType::Full) {
		_socket.applyReceiveRates(ackPacket._packetReceiveRate, ackPacket._estimatedLinkCapacity);
	}

	_socket.cong.onACK(pktSeqHi);

	// Update packet arrival rate: A = (A * 7 + a) / 8, where a is the value carried in the ACK.
	// Update estimated link capacity: B = (B * 7 + b) / 8, where b is the value carried in the ACK.

	// Update sender's buffer (by releasing the buffer that has been acknowledged).
	if (_sendPktPend != nullptr) {
		for(;;) {
			minLoss, minLossIdx := _sendPktPend.Min(oldAckSeq, _sendPacketID);
			if (pktSeqHi.blindDifference(minLoss.Seq) >= 0 || minLossIdx < 0) {
				break;
			}
			heap.Remove(&_sendPktPend, minLossIdx);
		}
	}

	// Update sender's loss list (by removing all those that has been acknowledged).
	if (_sendLossList != nullptr) {
		for(;;) {
			minLoss, minLossIdx := _sendLossList.Min(oldAckSeq, _sendPacketID);
			if (pktSeqHi.blindDifference(minLoss) >= 0 || minLossIdx < 0) {
				break;
			}
			heap.Remove(&_sendLossList, minLossIdx);
		}
	}
}

// ingestNak is called to process an NAK packet
void UdtSocket_send::ingestNak(NakPacket* p, time.Time now) {
	newLossList := make([]packet.PacketID, 0);
	clen := len(p.CmpLossInfo);
	for (idx := 0; idx < clen; idx++) {
		thisEntry := p.CmpLossInfo[idx];
		if (thisEntry&0x80000000 != 0) {
			thisPktID := packet.PacketID{Seq: thisEntry & 0x7FFFFFFF};
			if (idx+1 == clen) {
				_shutdownEvent <- shutdownMessage{sockState: sockStateCorrupted, permitLinger: false,
					err: fmt.Errorf("FAULT: While unpacking a NAK, the last entry (%x) was describing a start-of-range", thisEntry)};
				return;
			}
			if (!assertValidSentPktID("NAK", thisPktID)) {
				return;
			}
			lastEntry := p.CmpLossInfo[idx+1];
			if (lastEntry&0x80000000 != 0) {
				_shutdownEvent <- shutdownMessage{sockState: sockStateCorrupted, permitLinger: false,
					err: fmt.Errorf("FAULT: While unpacking a NAK, a start-of-range (%x) was followed by another start-of-range (%x)", thisEntry, lastEntry)};
				return;
			}
			lastPktID := packet.PacketID{Seq: lastEntry};
			if (!assertValidSentPktID("NAK", lastPktID)) {
				return;
			}
			idx++;
			for (span := thisPktID; span != lastPktID; span++) {
				newLossList = append(newLossList, span);
			}
		} else {
			thisPktID := packet.PacketID{Seq: thisEntry};
			if (!assertValidSentPktID("NAK", thisPktID)) {
				return;
			}
			newLossList = append(newLossList, thisPktID);
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
void UdtSocket_send::ingestCongestion(CongestionPacket* p, time.Time now) {
	// One way packet delay is increasing, so decrease the sending rate
	// this is very rough (not atomic, doesn't inform congestion) but this is a deprecated message in any case
    _sndPeriod.set(_sndPeriod.get() * 1125 / 1000);
	//m_iLastDecSeq = s.sendPktSeq;
}

void UdtSocket_send::resetEXP(time.Time now) {
    _lastRecvTime = now;

	var nextExpDurn time.Duration;
	rtoPeriod := _rtoPeriod.get();
	if (rtoPeriod > 0) {
		nextExpDurn = rtoPeriod;
	} else {
		rtt, rttVar := _socket.getRTT();
		nextExpDurn = (time.Duration(_expCount*(rtt+4*rttVar))*time.Microsecond + synTime);
		minExpTime := time.Duration(_expCount) * minEXPinterval;
		if (nextExpDurn < minExpTime) {
			nextExpDurn = minExpTime;
		}
	}
	_expTimerEvent = time.After(nextExpDurn);
}

// we've just had the EXP timer expire, see what we can do to recover this
void UdtSocket_send::expEvent(time.Time currTime) {

	// Haven't receive any information from the peer, is it dead?!
	// timeout: at least 16 expirations and must be greater than 10 seconds
	if ((_expCount > 16) && (currTime.Sub(_lastRecvTime) > 5*time.Second)) {
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
			for(PacketID span = _recvAckSeq + 1; span != _sendPacketID + 1; span++) {
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
	resetEXP(currTime);
}
