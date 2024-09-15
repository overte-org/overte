//
//  OctreeQueryNode.h
//  libraries/octree/src
//
//  Created by Brad Hefta-Gaub on 12/4/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_OctreeQueryNode_h
#define hifi_OctreeQueryNode_h

#include <iostream>

#include <qqueue.h>

#include "OctreeConstants.h"
#include "OctreeElementBag.h"
#include "OctreePacketData.h"
#include "OctreeQuery.h"
#include "OctreeSceneStats.h"
#include "SentPacketHistory.h"

class OctreeSendThread;
class OctreeServer;

class OctreeQueryNode : public OctreeQuery {
    Q_OBJECT
public:
    OctreeQueryNode() = default;
    virtual ~OctreeQueryNode() = default;

    void init(); // called after creation to set up some virtual items
    virtual PacketType getMyPacketType() const = 0;

    void resetOctreePacket(bool list);  // resets octree packet to after "V" header

    void writeToPacket(const unsigned char* buffer, unsigned int bytes, bool list); // writes to end of packet

    NLPacket& getPacket() const { return *_octreePacket; }
    NLPacketList& getPacketList() const { return *_octreePacketList; }
    bool isPacketWaiting() const { return _octreePacketWaiting; }

    bool packetIsDuplicate() const;
    bool shouldSuppressDuplicatePacket();

    unsigned int getAvailable() const { return _octreePacket->bytesAvailableForWrite(); }

    OctreeElementExtraEncodeData extraEncodeData;

    const ConicalViewFrustums& getCurrentViews() const { return _currentConicalViews; }

    // These are not classic setters because they are calculating and maintaining state
    // which is set asynchronously through the network receive
    bool updateCurrentViewFrustum();

    bool getViewSent() const { return _viewSent; }
    void setViewSent(bool viewSent);

    bool getViewFrustumChanging() const { return _viewFrustumChanging; }
    bool getViewFrustumJustStoppedChanging() const { return _viewFrustumJustStoppedChanging; }

    bool hasLodChanged() const { return _lodChanged; }

    OctreeSceneStats stats;

    unsigned int getlastOctreePacketLength() const { return _lastOctreePacketLength; }
    bool getLastOctreePacketWasList() const { return _lastOctreePacketWasList; };
    int getDuplicatePacketCount() const { return _duplicatePacketCount; }

    void nodeKilled();
    bool isShuttingDown() const { return _isShuttingDown; }

    void octreePacketSent() { packetSent(*_octreePacket); }
    void octreePacketListSent() { packetListSent(*_octreePacketList); }
    void packetSent(const NLPacket& packet);
    void packetListSent(const NLPacketList& packetList);

    OCTREE_PACKET_SEQUENCE getSequenceNumber() const { return _sequenceNumber; }

    void parseNackPacket(ReceivedMessage& message);
    bool hasNextNackedPacket() const;
    const NLPacket* getNextNackedPacket();

    // call only from OctreeSendThread for the given node
    bool haveJSONParametersChanged();

    bool shouldForceFullScene() const { return _shouldForceFullScene; }
    void setShouldForceFullScene(bool shouldForceFullScene) { _shouldForceFullScene = shouldForceFullScene; }

    void updatePacketSequenceNumber() const;

private:
    bool _viewSent { false };
    std::unique_ptr<NLPacket> _octreePacket;
    std::unique_ptr<NLPacketList> _octreePacketList;
    bool _octreePacketWaiting;

    unsigned int _lastOctreePacketLength { 0 };
    bool _lastOctreePacketWasList { false };
    int _duplicatePacketCount { 0 };
    quint64 _firstSuppressedPacket { usecTimestampNow() };

    ConicalViewFrustums _currentConicalViews;
    bool _viewFrustumChanging { false };
    bool _viewFrustumJustStoppedChanging { true };

    // watch for LOD changes
    int _lastClientBoundaryLevelAdjust { 0 };
    float _lastClientOctreeSizeScale { DEFAULT_OCTREE_SIZE_SCALE };
    bool _lodChanged { false };
    bool _lodInitialized { false };

    OCTREE_PACKET_SEQUENCE _sequenceNumber { 0 };

    bool _isShuttingDown { false };

    SentPacketHistory _sentPacketHistory;
    QQueue<OCTREE_PACKET_SEQUENCE> _nackedSequenceNumbers;

    std::array<char, udt::MAX_PACKET_SIZE> _lastOctreePayload;
    QByteArray _lastOctreePayloadListData;

    QJsonObject _lastCheckJSONParameters;

    bool _shouldForceFullScene { false };
};

#endif // hifi_OctreeQueryNode_h
