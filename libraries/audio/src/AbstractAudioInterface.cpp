//
//  Created by Bradley Austin Davis on 2015/11/18
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "AbstractAudioInterface.h"

#include <Node.h>
#include <NodeType.h>
#include <DependencyManager.h>
#include <NodeList.h>
#include <NLPacket.h>
#include <Transform.h>

#include "AudioConstants.h"

void AbstractAudioInterface::emitAudioPacket(const void* audioData, size_t bytes, quint16& sequenceNumber, bool isStereo,
                                             const Transform& transform, glm::vec3 avatarBoundingBoxCorner, glm::vec3 avatarBoundingBoxScale,
                                             PacketType packetType, QString codecName) {
    static std::mutex _mutex;
    using Locker = std::unique_lock<std::mutex>;
    auto nodeList = DependencyManager::get<NodeList>();
    SharedNodePointer audioMixer = nodeList->soloNodeOfType(NodeType::AudioMixer);
    if (audioMixer && audioMixer->getActiveSocket()) {
        Locker lock(_mutex);
        auto audioPacket = NLPacket::create(packetType);

        // write sequence number
        auto sequence = sequenceNumber++;
        audioPacket->writePrimitive(sequence);

        // write the codec
        audioPacket->writeString(codecName);

        if (packetType == PacketType::SilentAudioFrame) {
            // pack num silent samples
            quint16 numSilentSamples = isStereo ?
                AudioConstants::NETWORK_FRAME_SAMPLES_STEREO :
                AudioConstants::NETWORK_FRAME_SAMPLES_PER_CHANNEL;
            audioPacket->writePrimitive(numSilentSamples);
        } else {
            // set the mono/stereo byte
            quint8 channelFlag = isStereo ? 1 : 0;
            audioPacket->writePrimitive(channelFlag);
        }

        // at this point we'd better be sending the mixer a valid position, or it won't consider us for mixing
        assert(!isNaN(transform.getTranslation()));

        // pack the three float positions
        audioPacket->writePrimitive(transform.getTranslation());
        // pack the orientation
        audioPacket->writePrimitive(transform.getRotation());

        audioPacket->writePrimitive(avatarBoundingBoxCorner);
        audioPacket->writePrimitive(avatarBoundingBoxScale);


        if (audioPacket->getType() != PacketType::SilentAudioFrame) {
            // audio samples have already been packed (written to networkAudioSamples)
            int leadingBytes = audioPacket->getPayloadSize();
            audioPacket->setPayloadSize(leadingBytes + bytes);
            memcpy(audioPacket->getPayload() + leadingBytes, audioData, bytes);
        }
        nodeList->flagTimeForConnectionStep(LimitedNodeList::ConnectionStep::SendAudioPacket);
        nodeList->sendUnreliablePacket(*audioPacket, *audioMixer);
    }
}
