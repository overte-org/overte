//
//  NativeCC.cpp
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-05-25.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include <cmath>
#include "NativeCC.h"
#include "UdtSocket.h"

using namespace udt4;

NativeCongestionControl::NativeCongestionControl() : _rcInterval(UdtSocket::SYN) {
}

// Init to be called (only) at the start of a UDT connection.
void NativeCongestionControl::init(CongestionControlParms& parms) {
    _rcInterval = UdtSocket::SYN;
	_lastRCTime.start();
    parms.setACKPeriod(std::chrono::duration_cast<std::chrono::milliseconds>(_rcInterval));

	_slowStart = true;
    _lastAck = parms.getSendCurrentPacketID();
	_loss = false;
    _decCount = 0;
	_lastDecSeq = _lastAck - 1;
    _lastDecPeriod = std::chrono::microseconds{ 1 };
	_avgNAKNum = 0;
	_nakCount = 0;
	_decRandom = 1;

	parms.setCongestionWindowSize(16);
    parms.setPacketSendPeriod(std::chrono::milliseconds{ 1 });
}

// Close to be called when a UDT connection is closed.
void NativeCongestionControl::close(CongestionControlParms&) {
	// nothing done for this event
}

// OnACK to be called when an ACK packet is received
void NativeCongestionControl::onACK(CongestionControlParms& parms, PacketID packetID) {
	if (std::chrono::milliseconds(_lastRCTime.elapsed()) < _rcInterval) {
		return;
	}
	_lastRCTime.start();
	unsigned cWndSize = parms.getCongestionWindowSize();
	std::chrono::microseconds pktSendPeriod = parms.getPacketSendPeriod();
    unsigned recvRate, bandwidth;
	parms.getReceiveRates(recvRate, bandwidth);
    std::chrono::microseconds rtt = parms.getRTT();

	// If the current status is in the slow start phase, set the congestion window
	// size to the product of packet arrival rate and (RTT + SYN). Slow Start ends. Stop.
	if (_slowStart) {
		cWndSize = uint(int(cWndSize) + int(packetID.blindDifference(_lastAck)));
		_lastAck = packetID;

		if (cWndSize > parms.getMaxFlowWindow()) {
			_slowStart = false;
			if (recvRate > 0) {
				parms.setPacketSendPeriod(std::chrono::milliseconds(std::chrono::milliseconds{ONE_SECOND}.count() / recvRate));
			} else {
				parms.setPacketSendPeriod(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::microseconds((rtt + _rcInterval).count() / cWndSize)));
			}
		} else {
			// During Slow Start, no rate increase
			parms.setCongestionWindowSize(cWndSize);
			return;
		}
	} else {
		// Set the congestion window size (CWND) to: CWND = A * (RTT + SYN) + 16.
		cWndSize = static_cast<unsigned>(static_cast<double>(recvRate)/std::chrono::microseconds{ONE_SECOND}.count()*(rtt+_rcInterval).count() + 16.0);
	}
	if (_loss) {
		_loss = false;
		parms.setCongestionWindowSize(cWndSize);
		return;
	}
	/*
		The number of sent packets to be increased in the next SYN period
		(inc) is calculated as:
		   if (B <= C)
			  inc = 1/PS;
		   else
			  inc = max(10^(ceil(log10((B-C)*PS*8))) * Beta/PS, 1/PS);
		where B is the estimated link capacity and C is the current
		sending speed. All are counted as packets per second. PS is the
		fixed size of UDT packet counted in bytes. Beta is a constant
		value of 0.0000015.
	*/

	// Note: 1/24/2012
	// The minimum increase parameter is increased from "1.0 / m_iMSS" to 0.01
	// because the original was too small and caused sending rate to stay at low level
	// for long time.
	double inc;
	static constexpr double MIN_INC = 0.01;

	B := time.Duration(bandwidth) - time.Second/time.Duration(pktSendPeriod);
	bandwidth9 := time.Duration(bandwidth / 9);
	if ((pktSendPeriod > _lastDecPeriod) && (bandwidth9 < B)) {
		B = bandwidth9;
	}
	if (B <= 0) {
		inc = MIN_INC;
	} else {
		// inc = max(10 ^ ceil(log10( B * MSS * 8 ) * Beta / MSS, 1/MSS)
		// Beta = 1.5 * 10^(-6)

		unsigned mss = parms.getMSS();
		inc = pow(int(ceil(log10(B*mss*8.0))), 10) * 0.0000015 / mss;

		if (inc < MIN_INC) {
			inc = MIN_INC;
		}
	}

	// The SND period is updated as: SND = (SND * SYN) / (SND * inc + SYN).
	parms.setPacketSendPeriod(time.Duration(float64(pktSendPeriod*_rcInterval) / (float64(pktSendPeriod)*inc + float64(_rcInterval))));
}

// OnNAK to be called when a loss report is received
void NativeCongestionControl::onNAK(CongestionControlParms& parms, const QList<PacketID>& packetIDs) {
	// If it is in slow start phase, set inter-packet interval to 1/recvrate. Slow start ends. Stop.
	if (_slowStart) {
		_slowStart = false;
        unsigned recvRate, bandwidth;
	    parms.getReceiveRates(recvRate, bandwidth);
		if (recvRate > 0) {
			// Set the sending rate to the receiving rate.
			parms.setPacketSendPeriod(time.Second / time.Duration(recvRate));
			return;
		}
		// If no receiving rate is observed, we have to compute the sending
		// rate according to the current window size, and decrease it
		// using the method below.
		parms.setPacketSendPeriod(time.Duration(float64(time.Microsecond) * float64(parms.getCongestionWindowSize()) / float64(parms.getRTT()+_rcInterval)));
	}

	_loss = true;

	/*
		2) If this NAK starts a new congestion period, increase inter-packet
			interval (snd) to snd = snd * 1.125; Update AvgNAKNum, reset
			NAKCount to 1, and compute DecRandom to a random (average
			distribution) number between 1 and AvgNAKNum. Update LastDecSeq.
			Stop.
		3) If DecCount <= 5, and NAKCount == DecCount * DecRandom:
			a. Update SND period: SND = SND * 1.125;
			b. Increase DecCount by 1;
			c. Record the current largest sent sequence number (LastDecSeq).
	*/
	std::chrono::microseconds pktSendPeriod = parms.getPacketSendPeriod();
	if (_lastDecSeq.blindDifference(packetIDs.front()) > 0) {
		_lastDecPeriod = pktSendPeriod;
		parms.setPacketSendPeriod(pktSendPeriod * 1125 / 1000);

		_avgNAKNum = int(ceil(_avgNAKNum*0.875 + _nakCount*0.125));
		_nakCount = 1;
		_decCount = 1;

		_lastDecSeq = parms.getSendCurrentPacketID();

		// remove global synchronization using randomization
		rand := float64(randUint32()) / math.MaxUint32;
		_decRandom = int(ceil(float64(_avgNAKNum) * rand));
		if (_decRandom < 1) {
			_decRandom = 1;
		}
	} else {
		if (_decCount < 5) {
			_nakCount++;
			if(_nakCount%_decRandom != 0) {
				_decCount++;
				return;
			}
		}
		_decCount++;

		// 0.875^5 = 0.51, rate should not be decreased by more than half within a congestion period
		parms.setPacketSendPeriod(pktSendPeriod * 1125 / 1000);
		_lastDecSeq = parms.getSendCurrentPacketID();
	}
}

// OnTimeout to be called when a timeout event occurs
void NativeCongestionControl::onTimeout(CongestionControlParms& parms) {
	if (_slowStart) {
		_slowStart = false;
        unsigned recvRate, bandwidth;
	    parms.getReceiveRates(recvRate, bandwidth);
		if (recvRate > 0) {
			parms.setPacketSendPeriod(std::chrono::milliseconds(std::chrono::milliseconds{ONE_SECOND}.count() / recvRate));
		} else {
			parms.setPacketSendPeriod(time.Duration(float64(time.Microsecond) * float64(parms.getCongestionWindowSize()) / float64(parms.getRTT()+_rcInterval)));
		}
	} else {
		/*
			std::chrono::microseconds pktSendPeriod = parms.getPacketSendPeriod();
			_lastDecPeriod = pktSendPeriod;
			parms.setPacketSendPeriod(std::chrono::microseconds(static_cast<qint64>(ceil(pktSendPeriod.count() * 2)));
			_lastDecSeq = _lastAck;
		*/
	}
}

// OnPktSent to be called when data is sent
void NativeCongestionControl::onPacketSent(CongestionControlParms&, const Packet&) {
	// nothing done for this event
}

// OnPktRecv to be called when a data is received
void NativeCongestionControl::onPacketReceived(CongestionControlParms&, const Packet&, const QElapsedTimer&) {
	// nothing done for this event
}

// OnCustomMsg to process a user-defined packet
void NativeCongestionControl::onCustomMessageReceived(CongestionControlParms&, const Packet&, const QElapsedTimer&) {
	// nothing done for this event
}
