//
//  UdtSocket_recv.cpp
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-07-19.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "UdtSocket_recv.h"
#include "UdtSocket.h"

using namespace udt4;

UdtSocket_receive::UdtSocket_receive(UdtSocket_private& socket) : _socket(socket) {
    _ACKtimer.setSingleShot(true);
    _ACKtimer.setTimerType(Qt::PreciseTimer);
    connect(&_ACKtimer, &QTimer::timeout, this, &UdtSocket_receive::ACKevent);
}

/*
const (
	ackSelfClockInterval = 64
);
*/

void UdtSocket_receive::configureHandshake(const HandshakePacket& hsPacket) {
    _farNextPktSeq = hsPacket._initPktSeq;
    _farRecdPktSeq = hsPacket._initPktSeq - 1;
    _sentACK = hsPacket._initPktSeq;
    _recvACK2 = hsPacket._initPktSeq;
}

void UdtSocket_receive::run() {
    _ackHistory.clear();
    _recvLossList.clear();
    _recvPktPend.clear();
    _recvPktPairHistory.clear();
    _recvPktHistory.clear();
    _largestACK = 0UL;
    _lastACK = 0UL;
    _flagRecentACKevent = false;
    _lightAckCount = 0;
    _unackPktCount = 0;
    _recvLastArrival.invalidate();
    _recvLastProbe.invalidate();
    _ACKsentEvent.setRemainingTime(-1);
    _ACKsentEvent2.setRemainingTime(-1);
    _ACKtimer.start(UdtSocket::SYN);

    for (;;) {
        QMutexLocker guard(&_eventMutex);
        while (!processEvent(guard)) {
            if (_flagListenerShutdown) {
                // listener is closed, leave this thread
                return;
            }
            _eventCondition.wait(&_eventMutex);
        }
    }
}

void UdtSocket_receive::ACKevent() {
    QMutexLocker guard(&_eventMutex);
    _flagRecentACKevent = true;
    _eventCondition.notify_all();
}

void UdtSocket_receive::packetReceived(const Packet& udtPacket, const QElapsedTimer& timeReceived) {
    QMutexLocker guard(&_eventMutex);
    ReceivedPacket packet(udtPacket, timeReceived);
    _receivedPacketList.append(packet);
    _eventCondition.notify_all();
}

// the main event loop for the "receive" side of the socket, this controls the behavior and permitted actions
bool UdtSocket_receive::processEvent(QMutexLocker& eventGuard) {
    if (!_receivedPacketList.isEmpty()) {
        ReceivedPacket packet = _receivedPacketList.takeFirst();
        switch (packet.udtPacket._type) {
            case PacketType::Ack2:
                eventGuard.unlock();
                ingestACK2(packet.udtPacket, packet.timeReceived);
                return true;
            case PacketType::MsgDropReq:
                eventGuard.unlock();
                ingestMsgDropReq(MessageDropRequestPacket(packet.udtPacket), packet.timeReceived);
                return true;
            case PacketType::Data:
                eventGuard.unlock();
                ingestData(DataPacket(packet.udtPacket), packet.timeReceived);
                return true;
            case PacketType::SpecialErr:
                eventGuard.unlock();
                ingestError(packet.udtPacket);
                return true;
            case PacketType::Shutdown:  // sent by either peer
                eventGuard.unlock();
                ingestShutdown();
                return true;
        }
    }

	if (_flagRecentACKevent && !_flagListenerShutdown) {
        eventGuard.unlock();
		ackEvent();
        return true;
	}

    return false;
}

/*  Excerpt from the spec:

    ACK is used to trigger an acknowledgement (ACK). Its period is set by
    the congestion control module. However, UDT will send an ACK no
    longer than every 0.01 second, even though the congestion control
    does not need timer-based ACK. Here, 0.01 second is defined as the
    SYN time, or synchronization time, and it affects many of the other
    timers used in UDT.

    NAK is used to trigger a negative acknowledgement (NAK). Its period
    is dynamically updated to 4 * RTT_+ RTTVar + SYN, where RTTVar is the
    variance of RTT samples.

    EXP is used to trigger data packets retransmission and maintain
    connection status. Its period is dynamically updated to N * (4 * RTT
    + RTTVar + SYN), where N is the number of continuous timeouts. To
    avoid unnecessary timeout, a minimum threshold (e.g., 0.5 second)
    should be used in the implementation.
*/

// ingestAck2 is called to process an ACK2 packet
void UdtSocket_receive::ingestACK2(const Packet& udtPacket, const QElapsedTimer& timeReceived) {

    SequenceNumber ackSeq(udtPacket._additionalInfo);
    ACKHistoryMap::iterator lookup = _ackHistory.find(ackSeq);
    if (lookup == _ackHistory.end()) {
        return;  // this ACK not found
    }

    if (_recvACK2 < lookup->lastPacket) {
		_recvACK2 = lookup->lastPacket;
	}

	// Update the largest ACK number ever been acknowledged.
	if(_largestACK < ackSeq) {
		_largestACK = ackSeq;
	}

	_socket.applyRTT(static_cast<quint32>((timeReceived.nsecsElapsed() - lookup->sendTime.nsecsElapsed()) / 1000));
    _ackHistory.erase(lookup);

	//s.rto = 4 * s.rtt + s.rttVar
}

// ingestMsgDropReq is called to process an message drop request packet
void UdtSocket_receive::ingestMsgDropReq(const MessageDropRequestPacket& dropPacket, const QElapsedTimer& timeReceived) {
    PacketID stopSeq = dropPacket._lastPacketID + 1;
    for (PacketID packetID = dropPacket._firstPacketID; packetID != stopSeq; packetID++) {

		// remove all these packets from the loss list
        ReceiveLossMap::iterator lossLookup = _recvLossList.find(packetID);
        if(lossLookup != _recvLossList.end()) {
            _recvLossList.erase(lossLookup);
		}

		// remove all pending packets with this message
        DataPacketMap::iterator findPendPiece = _recvPktPend.find(packetID);
        if (findPendPiece != _recvPktPend.end()) {
            _recvPktPend.erase(findPendPiece);
        }
	}

	if (dropPacket._firstPacketID == _farRecdPktSeq + 1) {
        _farRecdPktSeq = dropPacket._lastPacketID;
	}
	if(_recvLossList.empty()) {
		_farRecdPktSeq = _farNextPktSeq - 1;
	}

	// try to push any pending packets out, now that we have dropped any blocking packets
	while(!_recvPktPend.empty() && stopSeq != _farNextPktSeq) {
        DataPacketMap::const_iterator nextPacketLookup = findFirst(_recvPktPend, stopSeq, _farNextPktSeq);
		if(nextPacketLookup == _recvPktPend.end() || !attemptProcessPacket(nextPacketLookup->second, false)) {
			break;
		}
	}
}

// ingestData is called to process a data packet
void UdtSocket_receive::ingestData(const DataPacket& dataPacket, const QElapsedTimer& timeReceived) {
    _socket.cong.onPktRecv(dataPacket);

	const PacketID& packetID = dataPacket._packetID;

	/* If the sequence number of the current data packet is 16n + 1,
	where n is an integer, record the time interval between this
	packet and the last data packet in the Packet Pair Window. */
	if(((static_cast<quint32>(packetID)-1)&0xf) == 0) {
		if(_recvLastProbe.isValid()) {
			_recvPktPairHistory.append(static_cast<quint32>((timeReceived.nsecsElapsed() - _recvLastProbe.nsecsElapsed())/1000));
			while(_recvPktPairHistory.length() > 16) {
				_recvPktPairHistory.removeFirst();
			}
		}
		_recvLastProbe = timeReceived;
	}

	// Record the packet arrival time in PKT History Window.
	if(_recvLastArrival.isValid()) {
		_recvPktHistory.append(static_cast<quint32>((timeReceived.nsecsElapsed() - _recvLastArrival.nsecsElapsed())/1000));
		while(_recvPktHistory.length() > 16) {
			_recvPktHistory.removeFirst();
		}
	}
	_recvLastArrival = timeReceived;

	/* If the sequence number of the current data packet is greater
	than LRSN + 1, put all the sequence numbers between (but
	excluding) these two values into the receiver's loss list and
	send them to the sender in an NAK packet. */
	qint32 seqDiff = packetID.blindDifference(_farNextPktSeq);
	if(seqDiff > 0) {
		ReceiveLossMap newLoss;
		for(PacketID idx = _farNextPktSeq; idx != packetID; idx++) {
			newLoss.insert(ReceiveLossMap::value_type(packetID, ReceiveLossEntry{packetID: packetID});
		}

		if(_recvLossList.empty()) {
			_recvLossList = newLoss;
		} else {
			for(PacketID idx = _farNextPktSeq; idx != packetID; idx++) {
                _recvLossList.insert(ReceiveLossMap::value_type(packetID, ReceiveLossEntry{packetID: packetID});
			}
		}

		sendNAK(newLoss);
		_farNextPktSeq = packetID + 1;

	} else if(seqDiff < 0) {
		// If the sequence number is less than LRSN, remove it from the receiver's loss list.
        ReceiveLossMap::iterator lossLookup = _recvLossList.find(packetID);
		if(lossLookup == _recvLossList.end()) {
			return; // already previously received packet -- ignore
		}
        _recvLossList.erase(lossLookup);

		if(_recvLossList.empty()) {
			_farRecdPktSeq = _farNextPktSeq - 1;
		} else {
            ReceiveLossMap::const_iterator nextLoss = findFirst(_recvLossList, _farRecdPktSeq, _farNextPktSeq);
            assert(nextLoss != _recvLossList.end());
			_farRecdPktSeq = nextLoss->first;
		}
	}

	attemptProcessPacket(dataPacket, true);
}

bool UdtSocket_receive::attemptProcessPacket(const DataPacket& dataPacket, bool isNew) {
    const PacketID& packetID = dataPacket._packetID;

	// can we process this packet?
	if(!_recvLossList.empty() && dataPacket._isOrdered && _farRecdPktSeq + 1 != packetID) {
		// we're required to order these packets and we're missing prior packets, so push and return
		if(isNew) {
			_recvPktPend.insert(DataPacketMap::value_type(dataPacket._packetID, dataPacket));
		}
		return false;
	}

	// can we find the start of this message?
    typedef QList<DataPacket> DataPacketList;
	DataPacketList pieces;
	bool cannotContinue = false;
	switch(dataPacket._messagePosition) {
	case DataPacket::MessagePosition::Last:
	case DataPacket::MessagePosition::Middle:
		// we need prior packets, let's make sure we have them
		if(!_recvPktPend.empty()) {
			PacketID pieceSeq = packetID - 1;
			for(;;) {
                DataPacketMap::const_iterator findPrevPiece = _recvPktPend.find(pieceSeq);
				if(findPrevPiece == _recvPktPend.end()) {
					// we don't have the previous piece, is it missing?
					ReceiveLossMap::const_iterator lossLookup = _recvLossList.find(pieceSeq);
					if(lossLookup != _recvLossList.end()) {
						// it's missing, stop processing
						cannotContinue = true;
					}
					// in any case we can't continue with this
					log.Printf("Message with id %d appears to be a broken fragment", static_cast<quint32>(dataPacket._messageNumber));
					break;
				}
                const DataPacket& prevPiece = findPrevPiece->second;
				if(prevPiece._messageNumber != dataPacket._messageNumber) {
					// ...oops? previous piece isn't in the same message
					log.Printf("Message with id %d appears to be a broken fragment", static_cast<quint32>(dataPacket._messageNumber));
					break;
				}
				pieces.prepend(prevPiece);
				if(prevPiece._messagePosition == DataPacket::MessagePosition::First) {
					break;
				}
				pieceSeq--;
			}
		}
	}
	if(!cannotContinue) {
		pieces.append(dataPacket);

    	switch(dataPacket._messagePosition) {
	    case DataPacket::MessagePosition::First:
	    case DataPacket::MessagePosition::Middle:
			// we need following packets, let's make sure we have them
    		if(!_recvPktPend.empty()) {
				PacketID pieceSeq = packetID + 1;
				for(;;) {
                    DataPacketMap::const_iterator findNextPiece = _recvPktPend.find(pieceSeq);
    				if(findNextPiece == _recvPktPend.end()) {
						// we don't have the previous piece, is it missing?
						if(pieceSeq == _farNextPktSeq) {
							// hasn't been received yet
							cannotContinue = true;
						} else if(!_recvLossList.empty()) {
							ReceiveLossMap::const_iterator lossLookup = _recvLossList.find(pieceSeq);
							if(lossLookup != _recvLossList.end()) {
								// it's missing, stop processing
								cannotContinue = true;
							}
						} else {
							log.Printf("Message with id %d appears to be a broken fragment", static_cast<quint32>(dataPacket._messageNumber));
						}
						// in any case we can't continue with this
						break;
					}
                    const DataPacket& nextPiece = findNextPiece->second;
                    if (nextPiece._messageNumber != dataPacket._messageNumber) {
						// ...oops? previous piece isn't in the same message
						log.Printf("Message with id %d appears to be a broken fragment", static_cast<quint32>(dataPacket._messageNumber));
						break;
					}
					pieces.append(nextPiece);
					if(nextPiece._messagePosition == DataPacket::MessagePosition::Last) {
						break;
					}
				}
			}
		}
	}

	// we've received a data packet, do we need to send an ACK for it?
	_unackPktCount++;
	unsigned ackInterval = _ackInterval.get();
	if((ackInterval > 0) && (ackInterval <= _unackPktCount)) {
		// ACK timer expired or ACK interval is reached
		ackEvent();
	} else if(ackSelfClockInterval*_lightAckCount <= _unackPktCount) {
		//send a "light" ACK
		sendLightACK();
		_lightAckCount++;
	}

	if(cannotContinue) {
		// we need to wait for more packets, store and return
		if(isNew) {
			_recvPktPend.insert(DataPacketMap::value_type(dataPacket._packetID, dataPacket));
		}
		return false;
	}

	// we have a message, pull it from the pending heap (if necessary), assemble it into a message, and return it
	if(!_recvPktPend.empty()) {
		for(_, piece := range pieces) {
			_recvPktPend.Remove(piece.Seq);
		}
	}

	msg := make([]byte, 0);
	for(_, piece := range pieces) {
		msg = append(msg, piece.Data...);
	}
	_socket.receivedMessage(msg);
	return true;
}

void UdtSocket_receive::sendLightACK() {
    PacketID packetID;

	// If there is no loss, the ACK is the current largest sequence number plus 1;
	// Otherwise it is the smallest sequence number in the receiver loss list.
	if(_recvLossList.empty()) {
        packetID = _farNextPktSeq;
	} else {
		packetID = _farRecdPktSeq + 1;
	}

	if (packetID != _recvACK2) {
		// send out a lite ACK
		// to save time on buffer processing and bandwidth/AS measurement, a lite ACK only feeds back an ACK number
	    ACKPacket ackPacket;
        ackPacket._lastPacketReceived = packetID;
        ackPacket._ackType = ACKPacket::AckType::Light;
		_socket.sendPacket(ackPacket.toPacket());
	}
}

void UdtSocket_receive::getReceiveSpeeds(int& recvSpeed, int& bandwidth) {

	// get median value, but cannot change the original value order in the window
	if(!_recvPktHistory.isEmpty()) {
		ourPktHistory := make(sortableDurnArray, len(_recvPktHistory));
		copy(ourPktHistory, _recvPktHistory);
		n := len(ourPktHistory);

		cutPos := n / 2;
		FloydRivest.Buckets(ourPktHistory, cutPos);
		median := ourPktHistory[cutPos];

		upper := median << 3;  // upper bounds
		lower := median >> 3;  // lower bounds
		count := 0;            // number of entries inside bounds
		var sum time.Duration; // sum of values inside bounds

		// median filtering
		idx := 0;
		for(i := 0; i < n; i++) {
			if((ourPktHistory[idx] < upper) && (ourPktHistory[idx] > lower)) {
				count++;
				sum += ourPktHistory[idx];
			}
			idx++;
		}

		// do we have enough valid values to return a value?
		// calculate speed
		if(count > (n >> 1)) {
			recvSpeed = int(time.Second * time.Duration(count) / sum);
		}
	}

	// get median value, but cannot change the original value order in the window
	if(!_recvPktPairHistory.isEmpty()) {
		ourProbeHistory := make(sortableDurnArray, len(s.recvPktPairHistory));
		copy(ourProbeHistory, _recvPktPairHistory);
		n := len(ourProbeHistory);

		cutPos := n / 2;
		FloydRivest.Buckets(ourProbeHistory, cutPos);
		median := ourProbeHistory[cutPos];

		upper := median << 3; // upper bounds
		lower := median >> 3; // lower bounds
		count := 1;           // number of entries inside bounds
		sum := median;        // sum of values inside bounds

		// median filtering
		idx := 0;
		for(i := 0; i < n; i++) {
			if((ourProbeHistory[idx] < upper) && (ourProbeHistory[idx] > lower)) {
				count++;
				sum += ourProbeHistory[idx];
			}
			idx++;
		}

		bandwidth = static_cast<int>(time.Second * time.Duration(count) / sum);
	}
}

void UdtSocket_receive::sendACK() {

	// If there is no loss, the ACK is the current largest sequence number plus 1;
	// Otherwise it is the smallest sequence number in the receiver loss list.
    PacketID packetID;
    if (_recvLossList.empty()) {
        packetID = _farNextPktSeq;
	} else {
        packetID = _farRecdPktSeq + 11;
	}

	if (packetID == _recvACK2) {
		return;
	}

	// only send out an ACK if we either are saying something new or the ackSentEvent has expired
	if(packetID == _sentACK && !_ACKsentEvent.hasExpired() && !_ACKsentEvent.isForever()) {
		return;
	}
	_sentACK = packetID;

	_lastACK++;
	ackHist := &ackHistoryEntry{
		ackID:      _lastACK,
		lastPacket: ack,
		sendTime:   time.Now(),
	}
	if(_ackHistory == nil) {
		_ackHistory = ackHistoryHeap{ackHist};
		heap.Init(&_ackHistory);
	} else {
		heap.Push(&_ackHistory, ackHist);
	}

	rtt, rttVar := _socket.getRTT();

	int numPendPackets = static_cast<int>(_farNextPktSeq.blindDifference(_farRecdPktSeq) - 1);
	int availWindow = static_cast<int>(_socket.maxFlowWinSize) - numPendPackets;
	if(availWindow < 2) {
		availWindow = 2;
	}

	ACKPacket ackPacket;
    ackPacket._ackSequence = _lastACK;
    ackPacket._lastPacketReceived = packetID;
    ackPacket._rtt = static_cast<quint32>(rtt);
    ackPacket._rttVariance = static_cast<quint32>(rttVariance);
    ackPacket._availBufferSize = static_cast<quint32>(availWindow);

    if (_ACKsentEvent2.hasExpired() || _ACKsentEvent2.isForever()) {
        getReceiveSpeeds(recvSpeed, bandwidth);
        ackPacket._ackType = ACKPacket::AckType::Full;
		ackPacket._packetReceiveRate = static_cast<quint32>(recvSpeed);
		ackPacket._estimatedLinkCapacity = static_cast<quint32>(bandwidth);
		_ACKsentEvent2.setRemainingTime(UdtSocket::SYN, Qt::PreciseTimer);
	}
    _socket.sendPacket(ackPacket.toPacket());

    quint64 microsecs = rtt+4*rttVar;
	_ACKsentEvent.setPreciseRemainingTime(microsecs/1000, (microsecs%1000)*1000, Qt::PreciseTimer);
}

void UdtSocket_receive::sendNAK(const ReceiveLossMap& receiveLoss) {
    NAKPacket::IntegerList lossInfo;

	PacketID currentPacket = _farRecdPktSeq;
    while (currentPacket != _farNextPktSeq) {
        ReceiveLossMap::const_iterator minPacketLookup = findFirst(receiveLoss, currentPacket, _farRecdPktSeq);
        if (minPacketLookup == receiveLoss.end()) {
			break;
		}

		PacketID lastPacket = minPacketLookup->first;
		for(;;) {
			PacketID nextPacket = lastPacket + 1;
            ReceiveLossMap::const_iterator lookup = receiveLoss.find(nextPacket);
            if(lookup == receiveLoss.end()) {
				break;
			}
            lastPacket = nextPacket;
		}

		if (lastPacket == minPacketLookup->first) {
			lossInfo.append(static_cast<quint32>(minPacketLookup->first)&0x7FFFFFFF);
		} else {
			lossInfo.append(static_cast<quint32>(minPacketLookup->first)|0x80000000);
            lossInfo.append(static_cast<quint32>(lastPacket)&0x7FFFFFFF);
		}

        error: need to set curPkt to something different?
	}

    NAKPacket packet;
    packet._lossData = lossInfo;
	_socket.sendPacket(packet.toPacket());
}

// ingestData is called to process an (undocumented) OOB error packet
void UdtSocket_receive::ingestError(const Packet& udtPacket) {
	// TODO: umm something
}

// assuming some condition has occured (ACK timer expired, ACK interval), send an ACK and reset the appropriate timer
void UdtSocket_receive::ackEvent() {
    sendACK();
    int ackTime = UdtSocket::SYN;
	int ackPeriod = _ackPeriod.get();
	if(ackPeriod > 0) {
		ackTime = ackPeriod;
	}
	_ACKtimer.start(ackTime);
	_unackPktCount = 0;
	_lightAckCount = 1;
}
