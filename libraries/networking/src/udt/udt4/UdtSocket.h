//
//  UdtSocket.h
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-05-28.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_udt4_UdtSocket_h
#define hifi_udt4_UdtSocket_h

#include "ByteSlice.h"
#include <chrono>
#include <map>
#include "Packet.h"
#include "PacketID.h"
#include <set>
#include <QtCore/QIODevice>
#include <QtCore/QAtomicInteger>
#include <QtCore/QDeadlineTimer>
#include <QtCore/QElapsedTimer>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QHostInfo>
#include <QtCore/QMutex>
#include <QtCore/QReadWriteLock>
#include <QtCore/QSharedPointer>
#include <QtCore/QThread>
#include <QtCore/QWaitCondition>
#include <QtCore/QTimer>
#include <QtNetwork/QUdpSocket>
#include "UdtSocket_cc.h"
#include "UdtSocket_recv.h"
#include "UdtSocket_send.h"

namespace udt4 {

class HandshakePacket;
class UdtMultiplexer;
class UdtSocket;
typedef QSharedPointer<UdtMultiplexer> UdtMultiplexerPointer;
typedef QSharedPointer<UdtSocket> UdtSocketPointer;

enum
{
    UDT_VERSION = 4,
};

enum class UdtSocketState
{
    Init,           // connection is closed
    HostLookup,     // resolving the requested hostname to an IP address
    LookupFailure,  // could not resolve the requested hostname
    Error,          // an error occurred establishing the connection
    Rendezvous,     // attempting to create a rendezvous connection
    Connecting,     // attempting to connect to a server
    HalfConnected,  // connection is technically established, can receive but we want to confirm the MTU before sending anything
    Connected,      // connection is established
    HalfClosed,     // connection is being closed (by either end), keeping receiver open for packet retransmits
    Closed,         // connection has been closed
    Refused,        // connection rejected by remote host
    Corrupted,      // peer behaved in an improper manner
    Timeout,        // connection failed due to peer timeout
};

// private interface for interactions within a UdtSocket
class UdtSocket_private {
public:
    virtual void getRTT(std::chrono::microseconds& rtt, std::chrono::microseconds& rttVariance) const = 0;
    virtual void applyRTT(std::chrono::microseconds rtt) = 0;
    virtual void getReceiveRates(unsigned& recvSpeed, unsigned& bandwidth) const = 0;
    virtual void applyReceiveRates(unsigned deliveryRate, unsigned bandwidth) = 0;
    virtual unsigned getMaxBandwidth() const = 0;
    virtual void sendPacket(const Packet& udtPacket) = 0;
    virtual void receivedMessage(const ByteSlice& message) = 0;
    virtual unsigned getMaxFlowWinSize() const = 0;
    virtual QString localAddressDebugString() const = 0;
    virtual void ingestErrorPacket(const Packet& udtPacket) = 0;
    virtual void requestShutdown(UdtSocketState toState, QString error) = 0;
    virtual void setCongestionWindow(unsigned pkt) = 0;
    virtual void setPacketSendPeriod(std::chrono::milliseconds snd) = 0;
    virtual void setACKperiod(std::chrono::milliseconds ack) = 0;
    virtual void setACKinterval(unsigned ack) = 0;
    virtual void setRTOperiod(std::chrono::milliseconds rto) = 0;
    virtual UdtSocket_CongestionControl& getCongestionControl() = 0;
};

/*
udtSocket encapsulates a UDT socket between a local and remote address pair, as
defined by the UDT specification.  udtSocket implements the net.Conn interface
so that it can be used anywhere that a stream-oriented network connection
(like TCP) would be used.
*/
class UdtSocket : public QIODevice, public QEnableSharedFromThis<UdtSocket>, protected UdtSocket_private {
    Q_OBJECT
public:
    enum TimeStuff
    {
        SECOND = 1000,
    };

    static constexpr std::chrono::milliseconds CONNECT_TIMEOUT{ std::chrono::seconds{ 3 } };             // how long should a client -> server connection take, from start to connected?
    static constexpr std::chrono::milliseconds RENDEZVOUS_CONNECT_TIMEOUT{ std::chrono::seconds{ 30 } }; // how long should a client <-> client connection take, from start to connected?
    static constexpr std::chrono::milliseconds CONNECT_RETRY{ 250 };                                     // if we haven't received a response to a packet in this amount of time, resend it
    static constexpr std::chrono::milliseconds LINGER_TIMEOUT{ std::chrono::seconds{ 180 } };            // after a connection is closed, wait this long for any potential packet resend requests
    static constexpr std::chrono::milliseconds SYN{ 10 };                                                // SYN is used as a timing duration in a number of places in UDT

    static constexpr unsigned MTU_DROP_INTERVAL{ 3 };                     // if we're negotiating a connection and have sent this many retries, drop the MTU
    static constexpr unsigned MTU_DROP_INCREMENT{ 10 };                   // if we're negotiating a connection and think we may not be getting through, drop the MTU by this much
    static constexpr unsigned MTU_MINIMUM{ 1280 };                        // we should not drop the MTU below this point on our own

public:
    explicit UdtSocket(QObject* parent = nullptr);
    virtual ~UdtSocket();

public: // from QUdpSocket
    bool isInDatagramMode() const;
    bool hasPendingDatagrams() const;
    qint64 pendingDatagramSize() const;
    ByteSlice receiveDatagram();
    qint64 readDatagram(char* data, qint64 maxlen);
    qint64 writeDatagram(const char* data, qint64 len, const QDeadlineTimer& timeout = QDeadlineTimer(QDeadlineTimer::Forever));
    inline qint64 writeDatagram(const QByteArray& datagram);
    inline qint64 writeDatagram(const ByteSlice& datagram);

public:  // from QAbstractSocket
    inline void abort();
    virtual void connectToHost(const QString& hostName, quint16 port, quint16 localPort = 0, bool datagramMode = true);
    virtual void connectToHost(const QHostAddress& address, quint16 port, quint16 localPort = 0, bool datagramMode = true);
    virtual void rendezvousToHost(const QString& hostName, quint16 port, quint16 localPort = 0, bool datagramMode = true);
    virtual void rendezvousToHost(const QHostAddress& address, quint16 port, quint16 localPort = 0, bool datagramMode = true);
    virtual void disconnectFromHost();
    inline QString errorString() const;
    bool flush();
    bool isValid() const;
    QHostAddress localAddress() const;
    quint16 localPort() const;
    inline const QHostAddress& peerAddress() const;
    inline quint16 peerPort() const;
    qint64 readBufferSize() const;
    virtual void setReadBufferSize(qint64 size);
    virtual void setSocketOption(QAbstractSocket::SocketOption option, const QVariant& value);
    virtual QVariant socketOption(QAbstractSocket::SocketOption option);
    inline UdtSocketState state() const;
    virtual bool waitForConnected(int msecs = 30000);
    virtual bool waitForDisconnected(int msecs = 30000);

    virtual qint64 bytesAvailable() const override;
    virtual qint64 bytesToWrite() const override;
    virtual void close() override;
    virtual bool isSequential() const override;
    virtual bool waitForBytesWritten(int msecs = 30000) override;
    virtual bool waitForReadyRead(int msecs = 30000) override;

signals: // from QAbstractSocket
    void connected();
    void disconnected();
    void hostFound();
    void stateChanged(UdtSocketState socketState);
    void customMessageReceived(Packet udtPacket, QElapsedTimer now);
signals:  // private
    void handshakeReceived(HandshakePacket hsPacket, QElapsedTimer now, QPrivateSignal);
    void shutdownRequested(UdtSocketState toState, QString error, QPrivateSignal);

protected: // from QAbstractSocket
    virtual qint64 readData(char* data, qint64 maxSize) override;
    virtual qint64 readLineData(char* data, qint64 maxlen) override;
    virtual qint64 writeData(const char* data, qint64 size) override;

public: // internal implementation
    bool readHandshake(const HandshakePacket& hsPacket, const QHostAddress& peerAddress, uint peerPort);
    static UdtSocketPointer newServerSocket(UdtMultiplexerPointer multiplexer, const HandshakePacket& hsPacket,
        const QHostAddress& peerAddress, uint peerPort);
    void readPacket(const Packet& udtPacket, const QHostAddress& peerAddress, uint peerPort);
    inline void setLocalSocketID(quint32 socketID);

private: // UdtSocket_private implementation
    virtual void getRTT(std::chrono::microseconds& rtt, std::chrono::microseconds& rttVariance) const;
    virtual void applyRTT(std::chrono::microseconds rtt);
    virtual unsigned getMaxBandwidth() const;
    virtual void getReceiveRates(unsigned& recvSpeed, unsigned& bandwidth) const;
    virtual void applyReceiveRates(unsigned deliveryRate, unsigned bandwidth);
    virtual void sendPacket(const Packet& udtPacket);
    virtual void receivedMessage(const ByteSlice& message);
    virtual unsigned getMaxFlowWinSize() const;
    virtual void ingestErrorPacket(const Packet& udtPacket);
    virtual void requestShutdown(UdtSocketState toState, QString error);
    virtual void setCongestionWindow(unsigned pkt);
    virtual void setPacketSendPeriod(std::chrono::milliseconds snd);
    virtual void setACKperiod(std::chrono::milliseconds ack);
    virtual void setACKinterval(unsigned ack);
    virtual void setRTOperiod(std::chrono::milliseconds rto);
    virtual UdtSocket_CongestionControl& getCongestionControl();

protected:
    virtual bool checkValidHandshake(const HandshakePacket& hsPacket, const QHostAddress& peerAddress, uint peerPort);

private slots:
    void onRendezvousHandshake();
    void onConnectionRetry();
    void onConnectionTimeout();
    void onLingerTimeout();
    void onHandshakeReceived(HandshakePacket hsPacket, QElapsedTimer now);
    void onShutdownRequested(UdtSocketState toState, QString error);

private:
    void setState(UdtSocketState newState);
    void createOffAxisSocket();
    bool preConnect(const QHostAddress& address, quint16 port, quint16 localPort);
    unsigned getCurrentPathMtu() const;
    void startNameConnect(const QString& hostName, quint16 port, quint16 localPort);
    void onNameResolved(QHostInfo info, quint16 port, quint16 localPort);
    void startConnect(const QHostAddress& address, quint16 port, quint16 localPort);
    bool processHandshake(const HandshakePacket& hsPacket);
    void sendHandshake(HandshakePacket::RequestType requestType, bool mtuDiscovery);
    bool initServerSocket(UdtMultiplexerPointer multiplexer, const HandshakePacket& hsPacket, const QHostAddress& peerAddress, uint peerPort);
    static QString addressDebugString(const QHostAddress& address, quint16 port, quint32 socketID);
    virtual QString localAddressDebugString() const;
    QString remoteAddressDebugString() const;

private:
    enum class SocketRole
    {
        Unknown,
        Client,
        Server,
        Rendezvous
    };

private:
    // these variables are used during specific _sockState values and invalid on all other values
    int _hostLookupID{ 0 };   // HostLookup: when doing a host lookup, the ID of the active request
    QTimer _connTimeout;      // Connecting/Rendezvous: fires when connection attempt times out
    QTimer _connRetry;        // Connecting/Rendezvous: fires when connection attempt to be retried
	QTimer _lingerTimer;      // CloseLinger: after disconnection, fires once our linger timer runs out

	// this data not changed after the socket is initialized and/or handshaked
    QElapsedTimer _createTime;           // the time this socket was created
    QString _errorString;                // a string describing the most recent error on this socket
    quint32 _farSocketID{ 0 };           // the remote's socketID
    quint32 _synCookie{ 0 };             // the remote's SYN cookie
    unsigned _connectionRetriesBeforeMTU{ 0 }; // the number of connection retry attempts we've made before adjusting the MTU
    PacketID _initialPacketSequence;     // initial packet sequence to start the connection with
    bool _isDatagram{ true };            // if true then we're sending and receiving datagrams, otherwise we're a streaming socket
    QThread _monitorThread;              // thread to monitor the state of the overall connection
    UdtMultiplexerPointer _multiplexer;  // the multiplexer that handles this socket
    QUdpSocket _offAxisUdpSocket;        // a "connected" udp socket we only use for detecting MTU path (as otherwise the system won't tell us about ICMP packets)
    QHostAddress _remoteAddr;            // the remote address
    quint16 _remotePort{ 0 };            // the remote port number
    quint32 _socketID{ 0 };                         // our socketID identifying this stream to the multiplexer
    SocketRole _socketRole{ SocketRole::Unknown };  // what role do we play in the channel negotiation?

    mutable QMutex _sockStateProtect;
    QWaitCondition _sockStateCondition;
    UdtSocketState _sockState{ UdtSocketState::Init };  // socket state - used mostly during handshakes

    QAtomicInteger<unsigned> _mtu;  // the negotiated maximum packet size
	unsigned _maxFlowWinSize{32};   // receiver: maximum unacknowledged packet count (minimum = 32)
//	ByteSlice _currPartialRead;     // stream connections: currently reading message (for partial reads). Owned by client caller (Read)
//	QDeadlineTimer _readDeadline;   // if set, then calls to Read() will return "timeout" after this time
//	bool _readDeadlinePassed;       // if set, then calls to Read() will return "timeout"
//	QDeadlineTimer _writeDeadline;  // if set, then calls to Write() will return "timeout" after this time
//	bool _writeDeadlinePassed;      // if set, then calls to Write() will return "timeout"

	mutable QReadWriteLock _rttProtect;          // lock must be held before referencing rtt/rttVar
	std::chrono::microseconds _rtt{ 0 };         // receiver: estimated roundtrip time. (in microseconds)
	std::chrono::microseconds _rttVariance{ 0 }; // receiver: roundtrip variance. (in microseconds)

	mutable QReadWriteLock _receiveRateProtect;  // lock must be held before referencing deliveryRate/bandwidth
	unsigned _deliveryRate{ 16 };                // delivery rate reported from peer (packets/sec)
	unsigned _bandwidth{ 1 };                    // bandwidth reported from peer (packets/sec)
/*
	// channels
	messageIn     chan []byte          // inbound messages. Sender is goReceiveEvent->ingestData, Receiver is client caller (Read)
	messageOut    chan sendMessage     // outbound messages. Sender is client caller (Write), Receiver is goSendEvent. Closed when socket is closed
	recvEvent     chan recvPktEvent    // receiver: ingest the specified packet. Sender is readPacket, receiver is goReceiveEvent
	sendPacket    chan packet.Packet   // packets to send out on the wire (once goManageConnection is running)
*/

    UdtSocket_send              _send;       // the "outgoing" side of this UDT connection
    UdtSocket_receive           _recv;       // the "incoming" side of this UDT connection
    UdtSocket_CongestionControl _congestion; // connecting glue to the chosen congestion control algorighm

	// timers
/*
	recv *udtSocketRecv // reference to receiving side of this socket
	cong *udtSocketCc   // reference to contestion control
*/
	// performance metrics
	//quint64 PktSent        // number of sent data packets, including retransmissions
	//quint64 PktRecv        // number of received packets
	//uint    PktSndLoss     // number of lost packets (sender side)
	//uint    PktRcvLoss     // number of lost packets (receiver side)
	//uint    PktRetrans     // number of retransmitted packets
	//uint    PktSentACK     // number of sent ACK packets
	//uint    PktRecvACK     // number of received ACK packets
	//uint    PktSentNAK     // number of sent NAK packets
	//uint    PktRecvNAK     // number of received NAK packets
	//double  MbpsSendRate   // sending rate in Mb/s
	//double  MbpsRecvRate   // receiving rate in Mb/s
	//time.Duration SndDuration // busy sending time (i.e., idle time exclusive)

	// instant measurements
	//time.Duration PktSndPeriod        // packet sending period
	//uint          PktFlowWindow       // flow window size, in number of packets
	//uint          PktCongestionWindow // congestion window size, in number of packets
	//uint          PktFlightSize       // number of packets on flight
	//time.Duration MsRTT               // RTT
	//double        MbpsBandwidth       // estimated bandwidth, in Mb/s
	//uint          ByteAvailSndBuf     // available UDT sender buffer size
	//uint          ByteAvailRcvBuf     // available UDT receiver buffer size

private:
    Q_DISABLE_COPY(UdtSocket)
};

template <class T>
typename std::map<PacketID, T, WrappedSequenceLess<PacketID>>::iterator findFirstEntry(std::map<PacketID, T, WrappedSequenceLess<PacketID>>& map, const PacketID& key, const PacketID& limit);

template <class T>
typename std::map<PacketID, T, WrappedSequenceLess<PacketID>>::const_iterator findFirstEntry(const std::map<PacketID, T, WrappedSequenceLess<PacketID>>& map, const PacketID& key, const PacketID& limit);

std::set<PacketID, WrappedSequenceLess<PacketID>>::iterator findFirstEntry(std::set<PacketID, WrappedSequenceLess<PacketID>>& set, const PacketID& key, const PacketID& limit);

std::set<PacketID, WrappedSequenceLess<PacketID>>::const_iterator findFirstEntry(const std::set<PacketID, WrappedSequenceLess<PacketID>>& set, const PacketID& key, const PacketID& limit);

} // namespace udt4
#include "UdtSocket.inl"
#endif /* hifi_udt4_UdtSocket_h */