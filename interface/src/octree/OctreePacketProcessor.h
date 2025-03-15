//
//  OctreePacketProcessor.h
//  interface/src/octree
//
//  Created by Brad Hefta-Gaub on 8/12/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_OctreePacketProcessor_h
#define hifi_OctreePacketProcessor_h

#include <QtCore/QSharedPointer>

#include <ReceivedPacketProcessor.h>
#include <ReceivedMessage.h>

#include "OctreeSceneStats.h"
#include "SafeLanding.h"

/// Handles processing of incoming voxel packets for the interface application. As with other ReceivedPacketProcessor classes
/// the user is responsible for reading inbound packets and adding them to the processing queue by calling queueReceivedPacket()
class OctreePacketProcessor : public ReceivedPacketProcessor {
    Q_OBJECT
public:
    OctreePacketProcessor();
    ~OctreePacketProcessor();

    NodeToOctreeSceneStats* getOctreeSceneStats() { return &_octreeServerSceneStats; }
    std::atomic<uint32_t>& getFullSceneReceivedCounter() { return _fullSceneReceivedCounter; }

    void startSafeLanding();
    void updateSafeLanding();
    void stopSafeLanding();
    void resetSafeLanding();
    bool safeLandingIsActive() const;
    bool safeLandingIsComplete() const;

    float domainLoadingProgress() const { return _safeLanding->loadingProgressPercentage(); }

signals:
    void packetVersionMismatch();

protected:
    virtual void processPacket(QSharedPointer<ReceivedMessage> message, SharedNodePointer sendingNode) override;

private slots:
    void handleOctreePacket(QSharedPointer<ReceivedMessage> message, SharedNodePointer senderNode);

private:
    int processOctreeStats(ReceivedMessage& message, SharedNodePointer sendingNode);
    void trackIncomingOctreePacket(ReceivedMessage& message, SharedNodePointer sendingNode, bool wasStatsPacket);

    NodeToOctreeSceneStats _octreeServerSceneStats;
    std::atomic<uint32_t> _fullSceneReceivedCounter { 0 };  // how many times have we received a full-scene octree stats packet

    OCTREE_PACKET_SEQUENCE _safeLandingSequenceStart { SafeLanding::INVALID_SEQUENCE };
    std::unique_ptr<SafeLanding> _safeLanding;
};
#endif  // hifi_OctreePacketProcessor_h
