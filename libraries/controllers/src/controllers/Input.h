//
//  Created by Bradley Austin Davis on 2015/10/18
//  (based on UserInputMapper inner class created by Sam Gateau on 4/27/15)
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once
#ifndef hifi_controllers_Input_h
#define hifi_controllers_Input_h

#include <GLMHelpers.h>

namespace controller {

enum class HmdAvatarAlignmentType {
    Eyes = 0,  // align the user's eyes with the avatars eyes
    Head       // align the user's head with the avatars head
};

struct InputCalibrationData {
    glm::mat4 sensorToWorldMat;    // sensor to world
    glm::mat4 avatarMat;           // avatar to world
    glm::mat4 hmdSensorMat;        // hmd pos and orientation in sensor space
    glm::mat4 defaultCenterEyeMat; // default pose for the center of the eyes in sensor space.
    glm::mat4 defaultHeadMat;      // default pose for head joint in sensor space
    glm::mat4 defaultSpine2;       // default pose for spine2 joint in sensor space
    glm::mat4 defaultHips;         // default pose for hips joint in sensor space
    glm::mat4 defaultLeftFoot;     // default pose for leftFoot joint in sensor space
    glm::mat4 defaultRightFoot;    // default pose for rightFoot joint in sensor space
    glm::mat4 defaultRightArm;     // default pose for rightArm joint in sensor space
    glm::mat4 defaultLeftArm;      // default pose for leftArm joint in sensor space
    glm::mat4 defaultRightHand;    // default pose for rightHand joint in sensor space
    glm::mat4 defaultLeftHand;     // default pose for leftHand joint in sensor space
    HmdAvatarAlignmentType hmdAvatarAlignmentType;
};

enum class ChannelType {
    UNKNOWN = 0,
    BUTTON,
    AXIS,
    POSE,
    RUMBLE,
    INVALID = 0x7
};

// Input is the unique identifier to find a n input channel of a particular device
// Devices are responsible for registering to the UserInputMapper so their input channels can be used and mapped
// to the Action channels
struct Input {
    union {
        uint32_t id{ 0 }; // by default Input is 0 meaning invalid
        struct {
            uint16_t device; // Up to 64K possible devices
            uint16_t channel : 12 ; // 2^12 possible channel per Device
            uint16_t type : 3; // 2 bits to store the Type directly in the ID
            uint16_t padding : 1; // 2 bits to store the Type directly in the ID
        };
    };

    bool isValid() const { return (id != INVALID_INPUT.id); }

    uint16_t getDevice() const { return device; }
    uint16_t getChannel() const { return channel; }
    uint32_t getID() const { return id; }
    ChannelType getType() const { return (ChannelType) type; }

    bool isButton() const { return getType() == ChannelType::BUTTON; }
    bool isAxis() const { return getType() == ChannelType::AXIS; }
    bool isPose() const { return getType() == ChannelType::POSE; }

    // WORKAROUND: the explicit initializer here avoids a bug in GCC-4.8.2 (but not found in 4.9.2)
    // where the default initializer (a C++-11ism) for the union data above is not applied.
    explicit Input() {}
    explicit Input(uint32_t id) : id(id) {}
    explicit Input(uint16_t device, uint16_t channel, ChannelType type, QString displayValue = QString())
	: device(device), channel(channel), type(uint16_t(type)), displayValue(displayValue), padding(0) {}
    Input(const Input& src) : id(src.id) {}
    Input& operator = (const Input& src) { id = src.id; return (*this); }
    bool operator ==(const Input& right) const { return INVALID_INPUT.id != id && INVALID_INPUT.id != right.id && id == right.id; }
    bool operator !=(const Input& right) const { return !(*this == right); }
    bool operator < (const Input& src) const { return id < src.id; }

    static const Input INVALID_INPUT;
    static const uint16_t INVALID_DEVICE;
    static const uint16_t INVALID_CHANNEL;
    static const uint16_t INVALID_TYPE;

    using NamedPair = QPair<Input, QString>;
    using NamedVector = QVector<NamedPair>;

    static const Input& invalidInput();

    QString displayValue;
};

}

#endif
