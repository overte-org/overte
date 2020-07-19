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
    _largestACK = 0UL;
    _flagRecentACKevent = false;
    _recvLastArrival.invalidate();
    _recvLastProbe.invalidate();
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
                ingestData(packet.udtPacket, packet.timeReceived);
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

    SequenceNumber ackSeq = udtPacket._additionalInfo;
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
		if(_recvPktPend != nil) {
			if(lossEntry, idx := _recvPktPend.Find(packetID); lossEntry != nil) {
				heap.Remove(&_recvPktPend, idx);
			}
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
		nextPkt, _ := _recvPktPend.Min(stopSeq, _farNextPktSeq);
		if(nextPkt == nil || !attemptProcessPacket(nextPkt, false)) {
			break;
		}
	}
}

// ingestData is called to process a data packet
void UdtSocket_receive::ingestData(const Packet& udtPacket, const QElapsedTimer& timeReceived) {
    _socket.cong.onPktRecv(udtPacket);

	PacketID packetID = udtPacket._sequence;

	/* If the sequence number of the current data packet is 16n + 1,
	where n is an integer, record the time interval between this
	packet and the last data packet in the Packet Pair Window. */
	if((static_cast<quint32>(packetID)-1)&0xf == 0) {
		if(_recvLastProbe.isValid()) {
			_recvPktPairHistory = append(_recvPktPairHistory, timeReceived.Sub(_recvLastProbe));
			if(len(_recvPktPairHistory) > 16) {
				_recvPktPairHistory = _recvPktPairHistory[len(s.recvPktPairHistory)-16:];
			}
		}
		_recvLastProbe = timeReceived;
	}

	// Record the packet arrival time in PKT History Window.
	if(_recvLastArrival.isValid()) {
		_recvPktHistory = append(_recvPktHistory, timeReceived.Sub(s.recvLastArrival));
		if(len(s.recvPktHistory) > 16) {
			_recvPktHistory = _recvPktHistory[len(s.recvPktHistory)-16:];
		}
	}
	_recvLastArrival = timeReceived;

	/* If the sequence number of the current data packet is greater
	than LRSN + 1, put all the sequence numbers between (but
	excluding) these two values into the receiver's loss list and
	send them to the sender in an NAK packet. */
	qint32 seqDiff = packetID.blindDifference(_farNextPktSeq);
	if(seqDiff > 0) {
		newLoss := make(receiveLossHeap, 0, seqDiff);
		for(PacketID idx = _farNextPktSeq; idx != packetID; idx++) {
			newLoss = append(newLoss, ReceiveLossEntry{packetID: packetID});
		}

		if(_recvLossList == nil) {
			_recvLossList = newLoss;
			heap.Init(&_recvLossList);
		} else {
			for(PacketID idx = _farNextPktSeq; idx != packetID; idx+) {
				heap.Push(&_recvLossList, ReceiveLossEntry{packetID: packetID});
			}
			heap.Init(&newLoss);
		}

		sendNAK(newLoss);
		_farNextPktSeq = packetID + 1;

	} else if(seqDiff < 0) {
		// If the sequence number is less than LRSN, remove it from the receiver's loss list.
		if(!_recvLossList.Remove(packetID)) {
			return; // already previously received packet -- ignore
		}

		if(len(_recvLossList) == 0) {
			_farRecdPktSeq = _farNextPktSeq - 1;
			_recvLossList = nil;
		} else {
			_farRecdPktSeq, _ = _recvLossList.Min(_farRecdPktSeq, _farNextPktSeq);
		}
	}

	attemptProcessPacket(udtPacket, true);
        }

bool UdtSocket_receive::attemptProcessPacket(const Packet& udtPacket, bool isNew) {
	PacketID packetID = udtPacket._sequence;

	// can we process this packet?
	boundary, mustOrder, msgID := p.GetMessageData();
	if(s.recvLossList != nil && mustOrder && s.farRecdPktSeq.Add(1) != seq) {
		// we're required to order these packets and we're missing prior packets, so push and return
		if(isNew) {
			if(s.recvPktPend == nil) {
				s.recvPktPend = dataPacketHeap{p};
				heap.Init(&s.recvPktPend);
			} else {
				heap.Push(&s.recvPktPend, p);
			}
		}
		return false;
	}

	// can we find the start of this message?
	pieces := make([]*packet.DataPacket, 0);
	cannotContinue := false;
	switch(boundary) {
	case packet.MbLast, packet.MbMiddle:
		// we need prior packets, let's make sure we have them
		if(s.recvPktPend != nil) {
			pieceSeq := seq.Add(-1);
			for(;;) {
				prevPiece, _ := s.recvPktPend.Find(pieceSeq);
				if(prevPiece == nil) {
					// we don't have the previous piece, is it missing?
					if(s.recvLossList != nil) {
						if(lossEntry, _ := s.recvLossList.Find(pieceSeq); lossEntry != nil) {
							// it's missing, stop processing
							cannotContinue = true;
						}
					}
					// in any case we can't continue with this
					log.Printf("Message with id %d appears to be a broken fragment", msgID);
					break;
				}
				prevBoundary, _, prevMsg := prevPiece.GetMessageData();
				if(prevMsg != msgID) {
					// ...oops? previous piece isn't in the same message
					log.Printf("Message with id %d appears to be a broken fragment", msgID);
					break;
				}
				pieces = append([]*packet.DataPacket{prevPiece}, pieces...);
				if(prevBoundary == packet.MbFirst) {
					break;
				}
				pieceSeq.Decr();
			}
		}
	}
	if(!cannotContinue) {
		pieces = append(pieces, p);

		switch(boundary) {
		case packet.MbFirst, packet.MbMiddle:
			// we need following packets, let's make sure we have them
			if(s.recvPktPend != nil) {
				pieceSeq := seq.Add(1);
				for {
					nextPiece, _ := s.recvPktPend.Find(pieceSeq);
					if(nextPiece == nil) {
						// we don't have the previous piece, is it missing?
						if(pieceSeq == s.farNextPktSeq) {
							// hasn't been received yet
							cannotContinue = true;
						} else if(s.recvLossList != nil) {
							if(lossEntry, _ := s.recvLossList.Find(pieceSeq); lossEntry != nil) {
								// it's missing, stop processing
								cannotContinue = true;
							}
						} else {
							log.Printf("Message with id %d appears to be a broken fragment", msgID);
						}
						// in any case we can't continue with this
						break;
					}
					nextBoundary, _, nextMsg := nextPiece.GetMessageData();
					if(nextMsg != msgID) {
						// ...oops? previous piece isn't in the same message
						log.Printf("Message with id %d appears to be a broken fragment", msgID);
						break;
					}
					pieces = append(pieces, nextPiece);
					if(nextBoundary == packet.MbLast) {
						break;
					}
				}
			}
		}
	}

	// we've received a data packet, do we need to send an ACK for it?
	s.unackPktCount++;
	ackInterval := uint(s.ackInterval.get());
	if((ackInterval > 0) && (ackInterval <= s.unackPktCount)) {
		// ACK timer expired or ACK interval is reached
		s.ackEvent();
	} else if(ackSelfClockInterval*s.lightAckCount <= s.unackPktCount) {
		//send a "light" ACK
		s.sendLightACK();
		s.lightAckCount++;
	}

	if(cannotContinue) {
		// we need to wait for more packets, store and return
		if(isNew) {
			if(s.recvPktPend == nil) {
				s.recvPktPend = dataPacketHeap{p};
				heap.Init(&s.recvPktPend);
			} else {
				heap.Push(&s.recvPktPend, p);
			}
		}
		return false;
	}

	// we have a message, pull it from the pending heap (if necessary), assemble it into a message, and return it
	if(s.recvPktPend != nil) {
		for(_, piece := range pieces) {
			s.recvPktPend.Remove(piece.Seq);
		}
		if(len(s.recvPktPend) == 0) {
			s.recvPktPend = nil;
		}
	}

	msg := make([]byte, 0);
	for(_, piece := range pieces) {
		msg = append(msg, piece.Data...);
	}
	s.messageIn <- msg;
	return true;
}

void UdtSocket_receive::sendLightACK() {
	var ack packet.PacketID

	// If there is no loss, the ACK is the current largest sequence number plus 1;
	// Otherwise it is the smallest sequence number in the receiver loss list.
	if s.recvLossList == nil {
		ack = s.farNextPktSeq
	} else {
		ack = s.farRecdPktSeq.Add(1)
	}

	if ack != s.recvAck2 {
		// send out a lite ACK
		// to save time on buffer processing and bandwidth/AS measurement, a lite ACK only feeds back an ACK number
		s.sendPacket <- &packet.LightAckPacket{PktSeqHi: ack}
	}
}

void UdtSocket_receive::getReceiveSpeeds(int& recvSpeed, int& bandwidth) {

	// get median value, but cannot change the original value order in the window
	if s.recvPktHistory != nil {
		ourPktHistory := make(sortableDurnArray, len(s.recvPktHistory))
		copy(ourPktHistory, s.recvPktHistory)
		n := len(ourPktHistory)

		cutPos := n / 2
		FloydRivest.Buckets(ourPktHistory, cutPos)
		median := ourPktHistory[cutPos]

		upper := median << 3  // upper bounds
		lower := median >> 3  // lower bounds
		count := 0            // number of entries inside bounds
		var sum time.Duration // sum of values inside bounds

		// median filtering
		idx := 0
		for i := 0; i < n; i++ {
			if (ourPktHistory[idx] < upper) && (ourPktHistory[idx] > lower) {
				count++
				sum += ourPktHistory[idx]
			}
			idx++
		}

		// do we have enough valid values to return a value?
		// calculate speed
		if count > (n >> 1) {
			recvSpeed = int(time.Second * time.Duration(count) / sum)
		}
	}

	// get median value, but cannot change the original value order in the window
	if s.recvPktPairHistory == nil {
		ourProbeHistory := make(sortableDurnArray, len(s.recvPktPairHistory))
		copy(ourProbeHistory, s.recvPktPairHistory)
		n := len(ourProbeHistory)

		cutPos := n / 2
		FloydRivest.Buckets(ourProbeHistory, cutPos)
		median := ourProbeHistory[cutPos]

		upper := median << 3 // upper bounds
		lower := median >> 3 // lower bounds
		count := 1           // number of entries inside bounds
		sum := median        // sum of values inside bounds

		// median filtering
		idx := 0
		for i := 0; i < n; i++ {
			if (ourProbeHistory[idx] < upper) && (ourProbeHistory[idx] > lower) {
				count++
				sum += ourProbeHistory[idx]
			}
			idx++
		}

		bandwidth = int(time.Second * time.Duration(count) / sum)
	}

	return
}

void UdtSocket_receive::sendACK() {
	var ack packet.PacketID

	// If there is no loss, the ACK is the current largest sequence number plus 1;
	// Otherwise it is the smallest sequence number in the receiver loss list.
	if s.recvLossList == nil {
		ack = s.farNextPktSeq
	} else {
		ack = s.farRecdPktSeq.Add(1)
	}

	if ack == s.recvAck2 {
		return
	}

	// only send out an ACK if we either are saying something new or the ackSentEvent has expired
	if ack == s.sentAck && s.ackSentEvent != nil {
		return
	}
	s.sentAck = ack

	s.lastACK++
	ackHist := &ackHistoryEntry{
		ackID:      s.lastACK,
		lastPacket: ack,
		sendTime:   time.Now(),
	}
	if s.ackHistory == nil {
		s.ackHistory = ackHistoryHeap{ackHist}
		heap.Init(&s.ackHistory)
	} else {
		heap.Push(&s.ackHistory, ackHist)
	}

	rtt, rttVar := s.socket.getRTT()

	numPendPackets := int(s.farNextPktSeq.BlindDiff(s.farRecdPktSeq) - 1)
	availWindow := int(s.socket.maxFlowWinSize) - numPendPackets
	if availWindow < 2 {
		availWindow = 2
	}

	p := &packet.AckPacket{
		AckSeqNo:  s.lastACK,
		PktSeqHi:  ack,
		Rtt:       uint32(rtt),
		RttVar:    uint32(rttVar),
		BuffAvail: uint32(availWindow),
	}
	if s.ackSentEvent2 == nil {
		recvSpeed, bandwidth := s.getRcvSpeeds()
		p.IncludeLink = true
		p.PktRecvRate = uint32(recvSpeed)
		p.EstLinkCap = uint32(bandwidth)
		s.ackSentEvent2 = time.After(synTime)
	}
	s.sendPacket <- p
	s.ackSentEvent = time.After(time.Duration(rtt+4*rttVar) * time.Microsecond)
}

void UdtSocket_receive::sendNAK(receiveLossHeap rl) {
	lossInfo := make([]uint32, 0)

	curPkt := s.farRecdPktSeq
	for curPkt != s.farNextPktSeq {
		minPkt, idx := rl.Min(curPkt, s.farRecdPktSeq)
		if idx < 0 {
			break
		}

		lastPkt := minPkt
		for {
			nextPkt := lastPkt.Add(1)
			_, idx = rl.Find(nextPkt)
			if idx < 0 {
				break
			}
			lastPkt = nextPkt
		}

		if lastPkt == minPkt {
			lossInfo = append(lossInfo, minPkt.Seq&0x7FFFFFFF)
		} else {
			lossInfo = append(lossInfo, minPkt.Seq|0x80000000, lastPkt.Seq&0x7FFFFFFF)
		}
	}

	s.sendPacket <- &packet.NakPacket{CmpLossInfo: lossInfo}
}

// ingestData is called to process an (undocumented) OOB error packet
void UdtSocket_receive::ingestError(const Packet& udtPacket) {
	// TODO: umm something
}

// assuming some condition has occured (ACK timer expired, ACK interval), send an ACK and reset the appropriate timer
void UdtSocket_receive::ackEvent() {
    sendACK();
	ackTime := synTime;
	ackPeriod := _ackPeriod.get();
	if(ackPeriod > 0) {
		ackTime = ackPeriod;
	}
	_ackTimerEvent = time.After(ackTime);
	_unackPktCount = 0;
	_lightAckCount = 1;
}
