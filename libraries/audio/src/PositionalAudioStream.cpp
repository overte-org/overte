//
//  PositionalAudioStream.cpp
//  libraries/audio/src
//
//  Created by Stephen Birarda on 6/5/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "PositionalAudioStream.h"
#include "SharedUtil.h"

#include <cstring>

#include <QtCore/QDataStream>
#include <QtCore/QLoggingCategory>

#include <LogHandler.h>
#include <Node.h>
#include <PacketHeaders.h>
#include <UUID.h>

PositionalAudioStream::PositionalAudioStream(PositionalAudioStream::Type type, bool isStereo, int numStaticJitterFrames) :
    InboundAudioStream(isStereo ? AudioConstants::STEREO : AudioConstants::MONO,
                       AudioConstants::NETWORK_FRAME_SAMPLES_PER_CHANNEL,
                       AUDIOMIXER_INBOUND_RING_BUFFER_FRAME_CAPACITY,
                       numStaticJitterFrames),
    _type(type),
    _position(0.0f, 0.0f, 0.0f),
    _orientation(0.0f, 0.0f, 0.0f, 0.0f),
    _shouldLoopbackForNode(false),
    _isStereo(isStereo),
    _ignorePenumbra(false),
    _lastPopOutputTrailingLoudness(0.0f),
    _lastPopOutputLoudness(0.0f),
    _quietestTrailingFrameLoudness(std::numeric_limits<float>::max()),
    _quietestFrameLoudness(0.0f),
    _frameCounter(0) {}

void PositionalAudioStream::resetStats() {
    _lastPopOutputTrailingLoudness = 0.0f;
    _lastPopOutputLoudness = 0.0f;
}

void PositionalAudioStream::updateLastPopOutputLoudnessAndTrailingLoudness() {
    _lastPopOutputLoudness = _ringBuffer.getFrameLoudness(_lastPopOutput);

    const int TRAILING_MUTE_THRESHOLD_FRAMES = 400;
    const int TRAILING_LOUDNESS_FRAMES = 200;
    const float CURRENT_FRAME_RATIO = 1.0f / TRAILING_LOUDNESS_FRAMES;
    const float PREVIOUS_FRAMES_RATIO = 1.0f - CURRENT_FRAME_RATIO;
    const float LOUDNESS_EPSILON = 0.000001f;

    if (_lastPopOutputLoudness >= _lastPopOutputTrailingLoudness) {
        _lastPopOutputTrailingLoudness = _lastPopOutputLoudness;
    } else {
        _lastPopOutputTrailingLoudness = (_lastPopOutputTrailingLoudness * PREVIOUS_FRAMES_RATIO) + (CURRENT_FRAME_RATIO * _lastPopOutputLoudness);

        if (_lastPopOutputTrailingLoudness < LOUDNESS_EPSILON) {
            _lastPopOutputTrailingLoudness = 0;
        }
    }
    if (_frameCounter++ == TRAILING_MUTE_THRESHOLD_FRAMES) {
        _quietestFrameLoudness = _quietestTrailingFrameLoudness;
        _frameCounter = 0;
        _quietestTrailingFrameLoudness = std::numeric_limits<float>::max();

    }
    if (_lastPopOutputLoudness < _quietestTrailingFrameLoudness) {
        _quietestTrailingFrameLoudness = _lastPopOutputLoudness;
    }
}

int PositionalAudioStream::parsePositionalData(const QByteArray& positionalByteArray) {
    QDataStream packetStream(positionalByteArray);

    packetStream.readRawData(reinterpret_cast<char*>(&_position), sizeof(_position));
    packetStream.readRawData(reinterpret_cast<char*>(&_orientation), sizeof(_orientation));
    packetStream.readRawData(reinterpret_cast<char*>(&_avatarBoundingBoxCorner), sizeof(_avatarBoundingBoxCorner));
    packetStream.readRawData(reinterpret_cast<char*>(&_avatarBoundingBoxScale), sizeof(_avatarBoundingBoxScale));

    if (_avatarBoundingBoxCorner != _ignoreBox.getCorner()) {
        // if the ignore box corner changes, we need to re-calculate the ignore box
        calculateIgnoreBox();
    }

    // if this node sent us a NaN for first float in orientation then don't consider this good audio and bail
    if (glm::isnan(_orientation.x)) {
        // NOTE: why would we reset the ring buffer here?
        _ringBuffer.reset();
        return 0;
    }

    return packetStream.device()->pos();
}

AudioStreamStats PositionalAudioStream::getAudioStreamStats() const {
    AudioStreamStats streamStats = InboundAudioStream::getAudioStreamStats();
    streamStats._streamType = _type;
    return streamStats;
}

void PositionalAudioStream::calculateIgnoreBox() {
    if (_avatarBoundingBoxScale != glm::vec3(0)) {
        auto scale = _avatarBoundingBoxScale;

        // enforce a minimum scale
        static const glm::vec3 MIN_IGNORE_BOX_SCALE = glm::vec3(0.3f, 1.3f, 0.3f);
        if (glm::any(glm::lessThan(scale, MIN_IGNORE_BOX_SCALE))) {
            scale = MIN_IGNORE_BOX_SCALE;
        }

        // (this is arbitrary number determined empirically for comfort)
        const float IGNORE_BOX_SCALE_FACTOR = 2.4f;
        scale *= IGNORE_BOX_SCALE_FACTOR;

        // create the box (we use a box for the zone for convenience)
        _ignoreBox.setBox(_avatarBoundingBoxCorner, scale);
    }
}

void PositionalAudioStream::enableIgnoreBox() {
    // re-calculate the ignore box using the latest values
    calculateIgnoreBox();

    _isIgnoreBoxEnabled = true;
}
