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

#include <algorithm>
#include "UdtSocket_recv.h"
#include "UdtSocket.h"
#include "../../NetworkLogging.h"
#include <QtCore/QLoggingCategory>
#include <vector>

using namespace udt4;

// an implementation of the Floyd-Rivest SELECT algorithm
namespace floydRivest {
    // left is the left index for the interval
    // right is the right index for the interval
    // k is the desired index value, where array[k] is the k+1 smallest element
    // when left = 0
    template<class T>
    void select(std::vector<T>& target, unsigned k, unsigned left, unsigned right) {
        size_t length = target.size();
   	    while (right > left) {
		    if (right - left > 600) {
                // Choosing a small subarray S based on sampling.
                // 600, 0.5 and 0.5 are arbitrary constants 
			    unsigned n = right - left + 1;
			    unsigned i = k - left + 1;
                double z = log(n);
			    double s = 0.5 * exp(2 * z / 3);
                double sign = 1.0;
			    if (i - n / 2 < 0) {
                    sign = -1;
			    }
			    double sd = 0.5 * sqrt(z * s * (n - s) / n) * sign;
			    unsigned newLeft = std::max(left, static_cast<unsigned>(floor(k - i * s / n + sd)));
			    unsigned newRight = std::min(right, static_cast<unsigned>(floor(k + (n - i) * s / n + sd)));
			    select(target, k, newLeft, newRight);
		    }

            // Partition the subarray S[left..right] with arr[k] as pivot 
            T t = target[k];
		    unsigned i = left;
		    unsigned j = right;
		    std::swap(target[left], target[k]);
		    if(target[right] > t) {
			    std::swap(target[left], target[right]);
		    }

		    while(i < j) {
			    std::swap(target[i++], target[j--]);
			    while(i < length && target[i] < t) {
				    i++;
			    }
			    while(j >= 0 && target[j] > t) {
				    j--;
			    }
		    }

		    // All equal points
		    if(target[left] == t) {
			    std::swap(target[left], target[j]);
		    } else {
			    j++;
			    std::swap(target[right], target[j]);
		    }
		    if (j <= k) {
			    left = j + 1;
		    }
		    if (k <= j) {
			    right = j - 1;
		    }
	    }
    }

    // Buckets. Sort a slice into buckets of given size. All elements from one bucket are smaller than any element  from the next one.
    // elements at position i * bucketSize are guaranteed to be the (i * bucketSize) th smallest elements
    // s := // some slice
    // FloydRivest.Buckets(sort.Interface(s), 5)
    // s is now sorted into buckets of size 5
    // max(s[0:5]) < min(s[5:10])
    // max(s[10: 15]) < min(s[15:20])
    // ...
    template <class T>
    void buckets(std::vector<T>& target, size_t bucketSize) {
        unsigned left = 0;
        unsigned right = static_cast<unsigned>(target.size() - 1);
        QList<unsigned> s;
        s.append(left);
        s.append(right);
	    while(!s.isEmpty()) {
            right = s.takeFirst();
            left = s.takeFirst();
		    if (right - left <= bucketSize) {
			    continue;
		    }
		    // + bucketSize - 1 is to do math ceil
		    unsigned mid = static_cast<unsigned>(left + ((right - left + bucketSize - 1) / bucketSize / 2) * bucketSize);
		    select(target, mid, left, right);
		    s.append(left);
		    s.append(mid);
		    s.append(mid);
		    s.append(right);
	    }
    }

}  // namespace FloydRivest

UdtSocket_receive::UdtSocket_receive(UdtSocket_private& socket) : _socket(socket) {
    _ACKtimerEvent.setSingleShot(true);
    _ACKtimerEvent.setTimerType(Qt::PreciseTimer);
    connect(&_ACKtimerEvent, &QTimer::timeout, this, &UdtSocket_receive::ACKevent);
}

void UdtSocket_receive::configureHandshake(const HandshakePacket& hsPacket) {
    _farNextPktSeq = hsPacket._initPktSeq;
    _farRecdPktSeq = hsPacket._initPktSeq - 1;
    _sentACK = hsPacket._initPktSeq;
    _recvACK2 = hsPacket._initPktSeq;
}

void UdtSocket_receive::setState(UdtSocketState newState) {
    bool shouldBeRunning = false;
    switch (newState) {
    case UdtSocketState::HalfConnected:
    case UdtSocketState::Connected:
        shouldBeRunning = true;
        break;
    }

    QMutexLocker guard(&_eventMutex);
    _flagListenerShutdown = !shouldBeRunning;
    if (shouldBeRunning && !isRunning()) {
        start();
    }
}

void UdtSocket_receive::startupInit() {
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
    _ACKsentTimer.setRemainingTime(0);   // default expired 
    _fullACKsentTimer.setRemainingTime(0);  // default expired
    _ACKtimer.setRemainingTime(0);          // default expired
}

void UdtSocket_receive::run() {
    startupInit();
    _ACKtimer.setRemainingTime(UdtSocket::SYN, Qt::PreciseTimer);
    _ACKtimerEvent.start(UdtSocket::SYN);

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
    ReceivedPacket packet(udtPacket, timeReceived);
    QMutexLocker guard(&_eventMutex);
    _receivedPacketList.push_back(std::move(packet));
    _eventCondition.notify_all();
}

// the main event loop for the "receive" side of the socket, this controls the behavior and permitted actions
bool UdtSocket_receive::processEvent(QMutexLocker& eventGuard) {
    if (!_receivedPacketList.empty()) {
        ReceivedPacket packet;  // using std::swap so we can use move semantics here
        std::swap(packet, _receivedPacketList.front());
        _receivedPacketList.pop_front();

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
                _socket.ingestErrorPacket(packet.udtPacket);
                return true;
            case PacketType::Shutdown:  // sent by either peer
                eventGuard.unlock();
                _socket.requestShutdown(UdtSocketState::HalfClosed, "Shutdown requested by peer");
                return true;
        }
    }

	if (_flagRecentACKevent && _ACKtimer.hasExpired() && !_flagListenerShutdown) {
        _flagRecentACKevent = false;
        eventGuard.unlock();
		ackEvent();
        return true;
	}

    return _flagRecentACKevent;
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

    if (_recvACK2 > lookup->lastPacket) {
		_recvACK2 = lookup->lastPacket;
	}

	// Update the largest ACK number ever been acknowledged.
	if(_largestACK < ackSeq) {
		_largestACK = ackSeq;
	}

	_socket.applyRTT(std::chrono::microseconds((timeReceived.nsecsElapsed() - lookup->sendTime.nsecsElapsed()) / 1000));
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
        DataPacketMap::const_iterator nextPacketLookup = findFirstEntry(_recvPktPend, stopSeq, _farNextPktSeq);
		if(nextPacketLookup == _recvPktPend.end() || !attemptProcessPacket(nextPacketLookup->second)) {
			break;
		}
	}
}

// ingestData is called to process a data packet
void UdtSocket_receive::ingestData(DataPacket&& dataPacket, const QElapsedTimer& timeReceived) {
	const PacketID& packetID = dataPacket._packetID;

	/* If the sequence number of the current data packet is 16n + 1,
	where n is an integer, record the time interval between this
	packet and the last data packet in the Packet Pair Window. */
	if(((static_cast<quint32>(packetID)-1)&0xf) == 0) {
		if(_recvLastProbe.isValid()) {
			_recvPktPairHistory.append(std::chrono::microseconds((timeReceived.nsecsElapsed() - _recvLastProbe.nsecsElapsed())/1000));
			while(_recvPktPairHistory.length() > 16) {
				_recvPktPairHistory.removeFirst();
			}
		}
		_recvLastProbe = timeReceived;
	}

	// Record the packet arrival time in PKT History Window.
	if(_recvLastArrival.isValid()) {
		_recvPktHistory.append(std::chrono::microseconds((timeReceived.nsecsElapsed() - _recvLastArrival.nsecsElapsed())/1000));
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
			newLoss.insert(ReceiveLossMap::value_type(packetID, ReceiveLossEntry(packetID)));
		}

		if(_recvLossList.empty()) {
			_recvLossList = newLoss;
		} else {
			for(PacketID idx = _farNextPktSeq; idx != packetID; idx++) {
                _recvLossList.insert(ReceiveLossMap::value_type(packetID, ReceiveLossEntry(packetID)));
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
            ReceiveLossMap::const_iterator nextLoss = findFirstEntry(_recvLossList, _farRecdPktSeq, _farNextPktSeq);
            assert(nextLoss != _recvLossList.end());
			_farRecdPktSeq = nextLoss->first;
		}
	}

    ReceivedDataPacket receivedDataPacket;
    receivedDataPacket.dataPacket = std::move(dataPacket);
    receivedDataPacket.timeReceived = timeReceived;
    if (!attemptProcessPacket(receivedDataPacket)) {
        _recvPktPend.insert(DataPacketMap::value_type(dataPacket._packetID, std::move(receivedDataPacket)));
    }
}

bool UdtSocket_receive::attemptProcessPacket(const ReceivedDataPacket& receivedDataPacket) {
    const DataPacket& dataPacket = receivedDataPacket.dataPacket;
    const PacketID& packetID = dataPacket._packetID;

	// can we process this packet?
	if(!_recvLossList.empty() && dataPacket._isOrdered && _farRecdPktSeq + 1 != packetID) {
		// we're required to order these packets and we're missing prior packets, so push and return
		return false;
	}

	// can we find the start of this message?
    typedef QList<DataPacket> DataPacketList;
	DataPacketList pieces;
    QElapsedTimer firstReceived = receivedDataPacket.timeReceived;
    QElapsedTimer lastReceived = receivedDataPacket.timeReceived;

    size_t pieceLength = 0;
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
                    qCInfo(networking) << _socket.localAddressDebugString() << ": Message with id " << static_cast<quint32>(dataPacket._messageNumber)
                                                           << "appears to be a broken fragment";
					break;
				}
                const DataPacket& prevPiece = findPrevPiece->second.dataPacket;
				if(prevPiece._messageNumber != dataPacket._messageNumber) {
					// ...oops? previous piece isn't in the same message
                    qCInfo(networking) << _socket.localAddressDebugString() << ": Message with id " << static_cast<quint32>(dataPacket._messageNumber)
                                                           << "appears to be a broken fragment";
					break;
				}
				pieces.prepend(prevPiece);
                firstReceived = findPrevPiece->second.timeReceived;
                pieceLength += prevPiece._contents.length();
				if(prevPiece._messagePosition == DataPacket::MessagePosition::First) {
					break;
				}
				pieceSeq--;
			}
		}
	}
	if(!cannotContinue) {
		pieces.append(dataPacket);
        pieceLength += dataPacket._contents.length();

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
                            qCInfo(networking) << _socket.localAddressDebugString() << ": Message with id " << static_cast<quint32>(dataPacket._messageNumber)
                                                                   << "appears to be a broken fragment";
						}
						// in any case we can't continue with this
						break;
					}
                    const DataPacket& nextPiece = findNextPiece->second.dataPacket;
                    if (nextPiece._messageNumber != dataPacket._messageNumber) {
						// ...oops? previous piece isn't in the same message
                        qCInfo(networking) << _socket.localAddressDebugString() << ": Message with id " << static_cast<quint32>(dataPacket._messageNumber)
                                                                << "appears to be a broken fragment";
						break;
					}
                    pieces.append(nextPiece);
                    lastReceived = findNextPiece->second.timeReceived;
                    pieceLength += nextPiece._contents.length();
					if(nextPiece._messagePosition == DataPacket::MessagePosition::Last) {
						break;
					}
				}
			}
		}
	}

	// we've received a data packet, do we need to send an ACK for it?
	_unackPktCount++;
	unsigned ackInterval = _ackInterval.load();
	if((ackInterval > 0) && (ackInterval <= _unackPktCount)) {
		// ACK timer expired or ACK interval is reached
		ackEvent();
	} else if(ACK_SELF_CLOCK_INTERVAL * _lightAckCount <= _unackPktCount) {
		//send a "light" ACK
		sendLightACK();
		_lightAckCount++;
	}

	if(cannotContinue) {
		// we need to wait for more packets, store and return
		return false;
	}

	// we have a message, pull it from the pending heap (if necessary), assemble it into a message, and return it
	if(!_recvPktPend.empty()) {
		for(DataPacketList::const_iterator trans = pieces.begin(); trans != pieces.end(); trans++) {
            _recvPktPend.erase(trans->_packetID);
		}
	}

    ReceiveMessageEntryPointer message = ReceiveMessageEntryPointer::create();
    message->messageNumber = dataPacket._messageNumber;
    message->isOrdered = dataPacket._isOrdered;
    message->firstReceived = firstReceived;
    message->lastReceived = lastReceived;
    message->numPackets = pieces.count();

    quint8* messageBytes = static_cast<quint8*>(message->content.create(pieceLength));
    size_t offset = 0;
    for (DataPacketList::const_iterator trans; trans != pieces.end(); trans++) {
        size_t len = trans->_contents.length();
        memcpy(messageBytes + offset, trans->_contents.constData(), len);
        offset += len;
	}

	_socket.receivedMessage(message);
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

void UdtSocket_receive::getReceiveSpeeds(unsigned& recvSpeed, unsigned& bandwidth) {

	// get median value, but cannot change the original value order in the window
	if(!_recvPktHistory.isEmpty()) {
        std::vector<std::chrono::microseconds> ourPktHistory(_recvPktHistory.begin(), _recvPktHistory.end());
		unsigned n = static_cast<unsigned>(ourPktHistory.size());

		unsigned cutPos = n / 2;
		floydRivest::buckets(ourPktHistory, cutPos);
		std::chrono::microseconds median = ourPktHistory[cutPos];

		std::chrono::microseconds upper(median.count() << 3);  // upper bounds
		std::chrono::microseconds lower(median.count() >> 3);  // lower bounds
		unsigned count = 0;            // number of entries inside bounds
		std::chrono::microseconds sum{ 0 };               // sum of values inside bounds

		// median filtering
		unsigned idx = 0;
		for(unsigned i = 0; i < n; i++) {
			if((ourPktHistory[idx] < upper) && (ourPktHistory[idx] > lower)) {
				count++;
				sum += ourPktHistory[idx];
			}
			idx++;
		}

		// do we have enough valid values to return a value?
		// calculate speed
		if(count > (n >> 1)) {
			recvSpeed = std::chrono::seconds(count) / sum;
		}
	}

	// get median value, but cannot change the original value order in the window
	if(!_recvPktPairHistory.isEmpty()) {
        std::vector<std::chrono::microseconds> ourProbeHistory(_recvPktPairHistory.begin(), _recvPktPairHistory.end());
		unsigned n = static_cast<unsigned>(ourProbeHistory.size());

		unsigned cutPos = n / 2;
        floydRivest::buckets(ourProbeHistory, cutPos);
		std::chrono::microseconds median = ourProbeHistory[cutPos];

		std::chrono::microseconds upper(median.count() << 3);  // upper bounds
		std::chrono::microseconds lower(median.count() >> 3); // lower bounds
		unsigned count = 1;           // number of entries inside bounds
		std::chrono::microseconds sum = median;         // sum of values inside bounds

		// median filtering
		unsigned idx = 0;
		for(unsigned i = 0; i < n; i++) {
			if((ourProbeHistory[idx] < upper) && (ourProbeHistory[idx] > lower)) {
				count++;
				sum += ourProbeHistory[idx];
			}
			idx++;
		}

		bandwidth = std::chrono::seconds(count) / sum;
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
	if(packetID == _sentACK && !_ACKsentTimer.hasExpired()) {
		return;
	}
	_sentACK = packetID;

	_lastACK++;
    ACKHistoryEntry ackHistoryEntry;
    ackHistoryEntry.ackID = _lastACK;
    ackHistoryEntry.lastPacket = packetID;
    ackHistoryEntry.sendTime.start();
    _ackHistory.insert(_lastACK, ackHistoryEntry);

    std::chrono::microseconds rtt, rttVariance;
    _socket.getRTT(rtt, rttVariance);

	int numPendPackets = _farNextPktSeq.blindDifference(_farRecdPktSeq) - 1;
	int availWindow = _socket.getMaxFlowWinSize() - numPendPackets;
	if(availWindow < 2) {
		availWindow = 2;
	}

	ACKPacket ackPacket;
    ackPacket._ackSequence = _lastACK;
    ackPacket._lastPacketReceived = packetID;
    ackPacket._rtt = rtt;
    ackPacket._rttVariance = rttVariance;
    ackPacket._availBufferSize = availWindow;

    if (_fullACKsentTimer.hasExpired()) {
        unsigned recvSpeed, bandwidth;
        getReceiveSpeeds(recvSpeed, bandwidth);
        ackPacket._ackType = ACKPacket::AckType::Full;
		ackPacket._packetReceiveRate = recvSpeed;
		ackPacket._estimatedLinkCapacity = bandwidth;
		_fullACKsentTimer.setRemainingTime(UdtSocket::SYN, Qt::PreciseTimer);
	}
    _socket.sendPacket(ackPacket.toPacket());

    std::chrono::microseconds microsecs = rtt + 4 * rttVariance;
    _ACKsentTimer.setPreciseRemainingTime(microsecs.count() / 1000, (microsecs.count() % 1000) * 1000, Qt::PreciseTimer);
}

void UdtSocket_receive::sendNAK(const ReceiveLossMap& receiveLoss) {
    NAKPacket::IntegerList lossInfo;

	PacketID currentPacket = _farRecdPktSeq;
    while (currentPacket != _farNextPktSeq) {
        ReceiveLossMap::const_iterator minPacketLookup = findFirstEntry(receiveLoss, currentPacket, _farRecdPktSeq);
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

        currentPacket = lastPacket + 1;
	}

    NAKPacket packet;
    packet._lossData = lossInfo;
	_socket.sendPacket(packet.toPacket());
}

// assuming some condition has occured (ACK timer expired, ACK interval), send an ACK and reset the appropriate timer
void UdtSocket_receive::ackEvent() {
    sendACK();
    std::chrono::microseconds ackTime = UdtSocket::SYN;
    std::chrono::microseconds ackPeriod(_ackPeriod.load());
	if(ackPeriod.count() > 0) {
		ackTime = ackPeriod;
	}
    _ACKtimer.setPreciseRemainingTime(ackTime.count()/1000, (ackTime.count()%1000)*1000, Qt::PreciseTimer);
    if (ackTime >= std::chrono::milliseconds{ 2 }) {
        _ACKtimerEvent.start(std::chrono::duration_cast<std::chrono::milliseconds>(ackTime) - std::chrono::milliseconds{ 1 });
    } else {
        QMutexLocker guard(&_eventMutex);
        _flagRecentACKevent = true;
    }
	_unackPktCount = 0;
	_lightAckCount = 1;
}

// Generally called by congestion control to set the time between ACKs
void UdtSocket_receive::setACKperiod(std::chrono::microseconds ack) {
    _ackPeriod.store(ack.count());
}

// Generally called by congestion control to set the number of packets before we send an ACK
void UdtSocket_receive::setACKinterval(unsigned ack) {
    _ackInterval.store(ack);
}
