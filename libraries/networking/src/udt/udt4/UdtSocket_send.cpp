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

using namespace udt4;

UdtSocket_send::UdtSocket_send(UdtSocket_private& socket):_socket(socket) {
    _SNDevent.setSingleShot(true);
    _SNDevent.setTimerType(Qt::PreciseTimer);
    connect(&_SNDevent, &QTimer::timeout, this, &UdtSocket_send::onConnectionTimeout);

    _ACK2SentEvent.setSingleShot(true);
    _ACK2SentEvent.setTimerType(Qt::PreciseTimer);
    connect(&_ACK2SentEvent, &QTimer::timeout, this, &UdtSocket_send::onConnectionTimeout);

    _EXPtimerEvent.setSingleShot(true);
    _EXPtimerEvent.setTimerType(Qt::PreciseTimer);
    connect(&_EXPtimerEvent, &QTimer::timeout, this, &UdtSocket_send::onConnectionTimeout);
}

void UdtSocket_send::configureHandshake(const HandshakePacket& hsPacket, bool resetSequence) {
    if (resetSequence) {
        _lastAckPacketID = hsPacket._initPktSeq;
        _sendPacketID = hsPacket._initPktSeq;
	}
    _flowWindowSize = static_cast<uint>(hsPacket._maxFlowWinSize);
}

void UdtSocket_send::run() {
	sendEvent := _sendEvent;
	messageOut := _messageOut;
	sockClosed := _sockClosed;
	for(;;) {
		thisMsgChan := messageOut;
		sockShutdown := _sockShutdown;

		switch (_sendState) {
		case SendState::Idle: // not waiting for anything, can send immediately
			if _msgPartialSend != nil { // we have a partial message waiting, try to send more of it now
				_processDataMsg(false, messageOut);
				continue;
			}
		case SendState::ProcessDrop: // immediately re-process any drop list requests
			_sendState = reevalSendState(); // try to reconstruct what our state should be if it wasn't sendStateProcessDrop
			if(!processSendLoss() || _sendPktSeq.Seq%16 == 0) {
				processSendExpire();
			}
			continue;
		case SendState::Shutdown:
			sockShutdown = nil;
			thisMsgChan = nil;
		default:
			thisMsgChan = nil;
		}

		select {
		case _, _ = <-sockShutdown:
			_sendState = sendStateShutdown;
			_expTimerEvent = nil; // don't process EXP events if we're shutting down
		case msg, ok := <-thisMsgChan: // nil if we can't process outgoing messages right now
			if (!ok) {
				_sendPacket <- &packet.ShutdownPacket{};
				_shutdownEvent <- shutdownMessage{sockState: sockStateClosed, permitLinger: true};
				return;
			}
			_msgPartialSend = &msg;
			_processDataMsg(true, messageOut);
		case evt, ok := <-sendEvent:
			if (!ok) {
				return;
			}
			_expCount = 1;
			resetEXP(evt.now);
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
		case _, _ = <-sockClosed:
			return;
		case <-_ack2SentEvent: // ACK2 unlocked
			_ack2SentEvent = nil;
		case now := <-_expTimerEvent: // EXP event
			_expEvent(now);
		case <-_sndEvent: // SND event
			_sndEvent = nil;
			if(_sendState == SendState::Sending) {
				_sendState = reevalSendState();
				if(!processSendLoss() || _sendPktSeq.Seq%16 == 0) {
					processSendExpire();
				}
			}
		}
	}
}

const (
	minEXPinterval time.Duration = 300 * time.Millisecond
)

func newUdtSocketSend(s *udtSocket) *udtSocketSend {
	ss := &udtSocketSend{
		socket:         s,
		expCount:       1,
		sendPktSeq:     s.initPktSeq,
		sockClosed:     s.sockClosed,
		sockShutdown:   s.sockShutdown,
		sendEvent:      s.sendEvent,
		messageOut:     s.messageOut,
		congestWindow:  atomicUint32{val: 16},
		flowWindowSize: s.maxFlowWinSize,
		sendPacket:     s.sendPacket,
		shutdownEvent:  s.shutdownEvent,
	}
	ss.resetEXP(s.created)
	go ss.goSendEvent()
	return ss
}

func (s *udtSocketSend) SetPacketSendPeriod(snd time.Duration) {
	// check to see if we have a bandwidth limit here
	maxBandwidth := s.socket.Config.MaxBandwidth
	if maxBandwidth > 0 {
		minSP := time.Second / time.Duration(float64(maxBandwidth)/float64(s.socket.mtu.get()))
		if snd < minSP {
			snd = minSP
		}
	}

	s.sndPeriod.set(snd)
}

func (s *udtSocketSend) reevalSendState() sendState {
	if s.sndEvent != nil {
		return sendStateSending
	}
	// Do we have too many unacknowledged packets for us to send any more?
	if s.sendPktPend != nil {
		congestWindow := uint(s.congestWindow.get())
		cwnd := s.flowWindowSize
		if cwnd > congestWindow {
			cwnd = congestWindow
		}
		if cwnd >= uint(len(s.sendPktPend)) {
			return sendStateWaiting
		}
	}
	return sendStateIdle
}

// try to pack a new data packet and send it
func (s *udtSocketSend) processDataMsg(isFirst bool, inChan <-chan sendMessage) {
	for s.msgPartialSend != nil {
		partialSend := s.msgPartialSend
		state := packet.MbOnly
		if s.socket.isDatagram {
			if isFirst {
				state = packet.MbFirst
			} else {
				state = packet.MbMiddle
			}
		}
		if isFirst || !s.socket.isDatagram {
			s.msgSeq++
		}

		mtu := int(s.socket.mtu.get())
		msgLen := len(partialSend.content)
		if msgLen >= mtu {
			// we are full -- send what we can and leave the rest
			var dp *packet.DataPacket
			if msgLen == mtu {
				dp = &packet.DataPacket{
					Seq:  s.sendPktSeq,
					Data: partialSend.content,
				}
				s.msgPartialSend = nil
			} else {
				dp = &packet.DataPacket{
					Seq:  s.sendPktSeq,
					Data: partialSend.content[0:mtu],
				}
				s.msgPartialSend = &sendMessage{content: partialSend.content[mtu:], tim: partialSend.tim, ttl: partialSend.ttl}
			}
			s.sendPktSeq.Incr()
			dp.SetMessageData(state, !s.socket.isDatagram, s.msgSeq)
			s.sendDataPacket(sendPacketEntry{pkt: dp, tim: partialSend.tim, ttl: partialSend.ttl}, false)
			return
		}

		// we are not full -- send only if this is a datagram or there's nothing obvious left
		if s.socket.isDatagram {
			if isFirst {
				state = packet.MbOnly
			} else {
				state = packet.MbLast
			}
		} else {
			select {
			case morePartialSend, ok := <-inChan:
				if ok {
					// we have more data, concat and try again
					s.msgPartialSend = &sendMessage{
						content: append(s.msgPartialSend.content, morePartialSend.content...),
						tim:     s.msgPartialSend.tim,
						ttl:     s.msgPartialSend.ttl,
					}
					continue
				}
			default:
				// nothing immediately available, just send what we have
			}
		}

		partialSend = s.msgPartialSend
		dp := &packet.DataPacket{
			Seq:  s.sendPktSeq,
			Data: partialSend.content,
		}
		s.msgPartialSend = nil
		s.sendPktSeq.Incr()
		dp.SetMessageData(state, !s.socket.isDatagram, s.msgSeq)
		s.sendDataPacket(sendPacketEntry{pkt: dp, tim: partialSend.tim, ttl: partialSend.ttl}, false)
		return
	}
}

// If the sender's loss list is not empty, retransmit the first packet in the list and remove it from the list.
func (s *udtSocketSend) processSendLoss() bool {
	if s.sendLossList == nil || s.sendPktPend == nil {
		return false
	}

	var dp *sendPacketEntry
	for {
		minLoss, minLossIdx := s.sendLossList.Min(s.recvAckSeq, s.sendPktSeq)
		if minLossIdx < 0 {
			// empty loss list? shouldn't really happen as we don't keep empty lists, but check for it anyhow
			return false
		}

		heap.Remove(&s.sendLossList, minLossIdx)
		if len(s.sendLossList) == 0 {
			s.sendLossList = nil
		}

		dp, _ = s.sendPktPend.Find(minLoss)
		if dp == nil {
			// can't find record of this packet, not much we can do really
			continue
		}

		if dp.ttl != 0 && time.Now().Add(dp.ttl).After(dp.tim) {
			// this packet has expired, ignore
			continue
		}

		break
	}

	s.sendDataPacket(*dp, true)
	return true
}

// evaluate our pending packet list to see if we have any expired messages
func (s *udtSocketSend) processSendExpire() bool {
	if s.sendPktPend == nil {
		return false
	}

	pktPend := make([]sendPacketEntry, len(s.sendPktPend))
	copy(pktPend, s.sendPktPend)
	for _, p := range pktPend {
		if p.ttl != 0 && time.Now().Add(p.ttl).After(p.tim) {
			// this message has expired, drop it
			_, _, msgNo := p.pkt.GetMessageData()
			dropMsg := &packet.MsgDropReqPacket{
				MsgID:    msgNo,
				FirstSeq: p.pkt.Seq,
				LastSeq:  p.pkt.Seq,
			}

			// find the other packets in this message
			for _, op := range pktPend {
				_, _, otherMsgNo := op.pkt.GetMessageData()
				if otherMsgNo == msgNo {
					if dropMsg.FirstSeq.BlindDiff(p.pkt.Seq) > 0 {
						dropMsg.FirstSeq = p.pkt.Seq
					}
					if dropMsg.LastSeq.BlindDiff(p.pkt.Seq) < 0 {
						dropMsg.LastSeq = p.pkt.Seq
					}
				}
				if s.sendLossList != nil {
					if _, slIdx := s.sendLossList.Find(p.pkt.Seq); slIdx >= 0 {
						heap.Remove(&s.sendLossList, slIdx)
					}
				}
			}
			if s.sendLossList != nil && len(s.sendLossList) == 0 {
				s.sendLossList = nil
			}

			s.sendPacket <- dropMsg
			return true
		}
	}
	return false
}

// we have a packed packet and a green light to send, so lets send this and mark it
func (s *udtSocketSend) sendDataPacket(dp sendPacketEntry, isResend bool) {
	if s.sendPktPend == nil {
		s.sendPktPend = sendPacketHeap{dp}
		heap.Init(&s.sendPktPend)
	} else {
		heap.Push(&s.sendPktPend, dp)
	}

	s.socket.cong.onDataPktSent(dp.pkt.Seq)
	s.sendPacket <- dp.pkt

	// have we exceeded our recipient's window size?
	s.sendState = s.reevalSendState()
	if s.sendState == sendStateWaiting {
		return
	}

	if !isResend && dp.pkt.Seq.Seq%16 == 0 {
		s.processSendExpire()
		return
	}

	snd := s.sndPeriod.get()
	if snd > 0 {
		s.sndEvent = time.After(snd)
		s.sendState = sendStateSending
	}
}

// ingestLightAck is called to process a "light" ACK packet
func (s *udtSocketSend) ingestLightAck(p *packet.LightAckPacket, now time.Time) {
	// Update the largest acknowledged sequence number.

	pktSeqHi := p.PktSeqHi
	diff := pktSeqHi.BlindDiff(s.recvAckSeq)
	if diff > 0 {
		s.flowWindowSize += uint(diff)
		s.recvAckSeq = pktSeqHi
	}
}

func (s *udtSocketSend) assertValidSentPktID(pktType string, pktSeq packet.PacketID) bool {
	if s.sendPktSeq.BlindDiff(pktSeq) < 0 {
		s.shutdownEvent <- shutdownMessage{sockState: sockStateCorrupted, permitLinger: false,
			err: fmt.Errorf("FAULT: Received an %s for packet %d, but the largest packet we've sent has been %d", pktType, pktSeq.Seq, s.sendPktSeq.Seq)}
		return false
	}
	return true
}

// ingestAck is called to process an ACK packet
func (s *udtSocketSend) ingestAck(p *packet.AckPacket, now time.Time) {
	// Update the largest acknowledged sequence number.

	// Send back an ACK2 with the same ACK sequence number in this ACK.
	if s.ack2SentEvent == nil && p.AckSeqNo == s.sentAck2 {
		s.sentAck2 = p.AckSeqNo
		s.sendPacket <- &packet.Ack2Packet{AckSeqNo: p.AckSeqNo}
		s.ack2SentEvent = time.After(synTime)
	}

	pktSeqHi := p.PktSeqHi
	if !s.assertValidSentPktID("ACK", pktSeqHi) {
		return
	}
	diff := pktSeqHi.BlindDiff(s.recvAckSeq)
	if diff <= 0 {
		return
	}

	oldAckSeq := s.recvAckSeq
	s.flowWindowSize = uint(p.BuffAvail)
	s.recvAckSeq = pktSeqHi

	// Update RTT and RTTVar.
	s.socket.applyRTT(uint(p.Rtt))

	// Update flow window size.
	if p.IncludeLink {
		s.socket.applyReceiveRates(uint(p.PktRecvRate), uint(p.EstLinkCap))
	}

	s.socket.cong.onACK(pktSeqHi)

	// Update packet arrival rate: A = (A * 7 + a) / 8, where a is the value carried in the ACK.
	// Update estimated link capacity: B = (B * 7 + b) / 8, where b is the value carried in the ACK.

	// Update sender's buffer (by releasing the buffer that has been acknowledged).
	if s.sendPktPend != nil {
		for {
			minLoss, minLossIdx := s.sendPktPend.Min(oldAckSeq, s.sendPktSeq)
			if pktSeqHi.BlindDiff(minLoss.Seq) >= 0 || minLossIdx < 0 {
				break
			}
			heap.Remove(&s.sendPktPend, minLossIdx)
		}
		if len(s.sendPktPend) == 0 {
			s.sendPktPend = nil
		}
	}

	// Update sender's loss list (by removing all those that has been acknowledged).
	if s.sendLossList != nil {
		for {
			minLoss, minLossIdx := s.sendLossList.Min(oldAckSeq, s.sendPktSeq)
			if pktSeqHi.BlindDiff(minLoss) >= 0 || minLossIdx < 0 {
				break
			}
			heap.Remove(&s.sendLossList, minLossIdx)
		}
		if len(s.sendLossList) == 0 {
			s.sendLossList = nil
		}
	}
}

// ingestNak is called to process an NAK packet
func (s *udtSocketSend) ingestNak(p *packet.NakPacket, now time.Time) {
	newLossList := make([]packet.PacketID, 0)
	clen := len(p.CmpLossInfo)
	for idx := 0; idx < clen; idx++ {
		thisEntry := p.CmpLossInfo[idx]
		if thisEntry&0x80000000 != 0 {
			thisPktID := packet.PacketID{Seq: thisEntry & 0x7FFFFFFF}
			if idx+1 == clen {
				s.shutdownEvent <- shutdownMessage{sockState: sockStateCorrupted, permitLinger: false,
					err: fmt.Errorf("FAULT: While unpacking a NAK, the last entry (%x) was describing a start-of-range", thisEntry)}
				return
			}
			if !s.assertValidSentPktID("NAK", thisPktID) {
				return
			}
			lastEntry := p.CmpLossInfo[idx+1]
			if lastEntry&0x80000000 != 0 {
				s.shutdownEvent <- shutdownMessage{sockState: sockStateCorrupted, permitLinger: false,
					err: fmt.Errorf("FAULT: While unpacking a NAK, a start-of-range (%x) was followed by another start-of-range (%x)", thisEntry, lastEntry)}
				return
			}
			lastPktID := packet.PacketID{Seq: lastEntry}
			if !s.assertValidSentPktID("NAK", lastPktID) {
				return
			}
			idx++
			for span := thisPktID; span != lastPktID; span.Incr() {
				newLossList = append(newLossList, span)
			}
		} else {
			thisPktID := packet.PacketID{Seq: thisEntry}
			if !s.assertValidSentPktID("NAK", thisPktID) {
				return
			}
			newLossList = append(newLossList, thisPktID)
		}
	}

	s.socket.cong.onNAK(newLossList)

	if s.sendLossList == nil {
		s.sendLossList = newLossList
		heap.Init(&s.sendLossList)
	} else {
		llen := len(newLossList)
		for idx := 0; idx < llen; idx++ {
			heap.Push(&s.sendLossList, newLossList[idx])
		}
	}

	s.sendState = sendStateProcessDrop // immediately restart transmission
}

// ingestCongestion is called to process a (retired?) Congestion packet
func (s *udtSocketSend) ingestCongestion(p *packet.CongestionPacket, now time.Time) {
	// One way packet delay is increasing, so decrease the sending rate
	// this is very rough (not atomic, doesn't inform congestion) but this is a deprecated message in any case
	s.sndPeriod.set(s.sndPeriod.get() * 1125 / 1000)
	//m_iLastDecSeq = s.sendPktSeq
}

func (s *udtSocketSend) resetEXP(now time.Time) {
	s.lastRecvTime = now

	var nextExpDurn time.Duration
	rtoPeriod := s.rtoPeriod.get()
	if rtoPeriod > 0 {
		nextExpDurn = rtoPeriod
	} else {
		rtt, rttVar := s.socket.getRTT()
		nextExpDurn = (time.Duration(s.expCount*(rtt+4*rttVar))*time.Microsecond + synTime)
		minExpTime := time.Duration(s.expCount) * minEXPinterval
		if nextExpDurn < minExpTime {
			nextExpDurn = minExpTime
		}
	}
	s.expTimerEvent = time.After(nextExpDurn)
}

// we've just had the EXP timer expire, see what we can do to recover this
func (s *udtSocketSend) expEvent(currTime time.Time) {

	// Haven't receive any information from the peer, is it dead?!
	// timeout: at least 16 expirations and must be greater than 10 seconds
	if (s.expCount > 16) && (currTime.Sub(s.lastRecvTime) > 5*time.Second) {
		// Connection is broken.
		s.shutdownEvent <- shutdownMessage{sockState: sockStateTimeout, permitLinger: true}
		return
	}

	// sender: Insert all the packets sent after last received acknowledgement into the sender loss list.
	// recver: Send out a keep-alive packet
	if s.sendPktPend != nil {
		if s.sendPktPend != nil && s.sendLossList == nil {
			// resend all unacknowledged packets on timeout, but only if there is no packet in the loss list
			newLossList := make([]packet.PacketID, 0)
			for span := s.recvAckSeq.Add(1); span != s.sendPktSeq.Add(1); span.Incr() {
				newLossList = append(newLossList, span)
			}
			s.sendLossList = newLossList
			heap.Init(&s.sendLossList)
		}
		s.socket.cong.onTimeout()
		s.sendState = sendStateProcessDrop // immediately restart transmission
	} else {
		s.sendPacket <- &packet.KeepAlivePacket{}
	}

	s.expCount++
	// Reset last response time since we just sent a heart-beat.
	s.resetEXP(currTime)
}
