//
//  UdtSocket_send.h
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-06-20.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_udt4_UdtSocket_send_h
#define hifi_udt4_UdtSocket_send_h

#include <chrono>
#include <list>
#include <map>
#include "Packet.h"
#include "PacketID.h"
#include <set>
#include <QtCore/QDeadlineTimer>
#include <QtCore/QElapsedTimer>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QSharedPointer>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QWaitCondition>

namespace udt4 {

class UdtSocket_private;
enum class UdtSocketState;

/* UdtSocket_send

Implements the "outgoing" side of a UDT socket connection, scheduling outgoing packets and listening
for packet-loss and acknowledgements from the far side

This class is private and not user-accessible
*/
class UdtSocket_send : public QThread {
    Q_OBJECT
public:
    UdtSocket_send(UdtSocket_private& socket);

    void setState(UdtSocketState newState);
    void configureHandshake(const HandshakePacket& hsPacket, const PacketID& sendPacketID, bool resetSequence, unsigned mtu);
    void sendMessage(ByteSlice content, QDeadlineTimer expireTime);
    void packetReceived(const Packet& udtPacket, const QElapsedTimer& timeReceived);
    void queueDisconnect();
    void resetReceiveTimer();
    void setCongestionWindow(unsigned pkt);
    void setPacketSendPeriod(std::chrono::milliseconds snd);
    void setRTOperiod(std::chrono::milliseconds rto);
    qint64 bytesToWrite() const;
    bool waitForPacketSent(const QDeadlineTimer& timeout) const;
    bool flush();

private slots:
    void SNDevent();
    void EXPevent();

private:
    static constexpr std::chrono::milliseconds MIN_EXP_INTERVAL{ 300 };                             // factors into the minimum time we will wait before requesting packet resends
    static constexpr std::chrono::milliseconds MIN_CONNECTION_TIMEOUT{ std::chrono::seconds{ 5 } }; // we will wait at minimum this time before dropping an inactive connection

    enum class SendState
    {
        Closed,      // Connection is closed
        Idle,        // Connection is open and waiting
        Sending,     // Recently sent a packet
        Waiting,     // Waiting for the peer to process a packet
        ProcessDrop, // Dropping a lost packet
        Shutdown,    // Connection has been recently closed and is listening for packet-resend requests
    };

    struct MessageEntry {
        ByteSlice content;
        QElapsedTimer sendTime;
        QDeadlineTimer expireTime;
        bool fullySent{ false };

        inline MessageEntry(const ByteSlice& c) : content(c) {
            sendTime.start();
            expireTime.setRemainingTime(-1);
        }
    };
    typedef QSharedPointer<MessageEntry> MessageEntryPointer;
    typedef QList<MessageEntryPointer> MessageEntryList;

    struct ReceivedPacket {
        Packet udtPacket;
        QElapsedTimer timeReceived;

        inline ReceivedPacket() {}
        inline ReceivedPacket(const Packet& p, const QElapsedTimer& t);
    };
    typedef std::list<ReceivedPacket> ReceivedPacketList;

    struct SendPacketEntry {
        DataPacket packet;
        QElapsedTimer sendTime;
        QDeadlineTimer expireTime;
    };
    typedef QSharedPointer<SendPacketEntry> SendPacketEntryPointer;
    typedef std::map<PacketID, SendPacketEntryPointer, WrappedSequenceLess<PacketID>> SendPacketEntryMap;

    typedef std::set<PacketID, WrappedSequenceLess<PacketID>> PacketIDSet;

private:  // called exclusively within our private thread
    void startupInit();
    virtual void run() override;
    bool processEvent(QMutexLocker& eventGuard);
    bool processDataMsg(bool isFirst);
    SendState reevalSendState() const;
    bool processSendLoss();
    bool processSendExpire();
    void processExpEvent();
    void resetEXP();
    void sendDataPacket(SendPacketEntryPointer dataPacket, bool isResend);
    void ingestAck(const ACKPacket& ackPacket, const QElapsedTimer& timeReceived);
    void ingestNak(const NAKPacket& nakPacket, const QElapsedTimer& timeReceived);
    void ingestCongestion(const Packet& udtPacket, const QElapsedTimer& timeReceived);
    bool assertValidSentPktID(const char* pktType, const PacketID& packetID);

private:
    UdtSocket_private& _socket;  // private interface pointing back at the UdtSocket

    // this is a condition-based thread.  All the variables in this block can only be accessed while holding _eventMutex
    mutable QMutex _eventMutex;
    QWaitCondition _eventCondition;
    UdtSocketState _socketState;
    bool           _flagRecentReceivedPacket{ false }; // has a packet been recently received from our peer?
    bool           _flagRecentEXPevent{ false };       // has the EXP timer fired recently?
    bool           _flagRecentSNDevent{ false };       // has the SND timer fired recently?
    bool           _flagSendDisconnect{ false };       // are we being asked to send a Disconnect packet?
    MessageEntryList _pendingMessages;                 // the list of messages queued but not yet sent
    ReceivedPacketList _receivedPacketList;            // list of packets we have not yet processed

    mutable QMutex _sendMutex;             // held while we're assembling packets.  Sometimes interacts with _eventMutex, if you're grabbing both then GRAB THIS FIRST (i.e. avoid philosopher's dilemma)
    mutable QWaitCondition _sendCondition; // triggers when a packet has finished processing or a packet has been sent

    // While the send thread is running these variables only to be accessed by that thread (can be initialized carefully by other threads)
    unsigned       _mtu{ 1500 };
    bool           _isDatagram{ true };
	SendState      _sendState{SendState::Closed}; // current sender state
    SendPacketEntryMap  _sendPktPend;     // list of packets that have been sent but not yet acknowledged
	PacketID       _sendPacketID;         // the current packet sequence number
    MessageEntryPointer _msgPartialSend;  // when a message can only partially fit in a packet, this is the remainder
    SequenceNumber _messageSequence;      // the current message sequence number
    unsigned       _expCount{ 1 };        // number of continuous EXP timeouts.
	QElapsedTimer  _lastReceiveTime;      // the last time we've heard something from the remote system
	PacketID       _lastAckPacketID;      // largest packetID we've received an ACK from
    SequenceNumber _sentAck2;             // largest ACK2 packet we've sent
    PacketIDSet    _sendLossList;         // loss list
	unsigned       _flowWindowSize{ 16 }; // negotiated maximum number of unacknowledged packets (in packets)

    // These variables may be set/adjusted by congestion control and therefore are controlled by QAtomicInteger
    QAtomicInteger<unsigned> _sndPeriod;      // delay between sending packets (in milliseconds)
    QAtomicInteger<unsigned> _rtoPeriod;      // override of EXP timer calculations (in milliseconds)
    QAtomicInteger<unsigned> _congestWindow;  // size of the current congestion window (in packets)

	// timers
	QTimer _SNDtimer;              // if a packet is recently sent, this timer fires when SND completes
	QTimer _EXPtimer;              // Fires when we haven't heard from the peer in a while
	QDeadlineTimer _ACK2SentTimer; // if an ACK2 packet has recently sent, wait SYN before sending another one

private:
    Q_DISABLE_COPY(UdtSocket_send)
};

}  // namespace udt4

#include "UdtSocket_send.inl"
#endif /* hifi_udt4_UdtSocket_send_h */