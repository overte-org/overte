//
// Overte OpenXR Plugin
//
// Copyright 2024 Lubosz Sarnecki
//
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "controllers/Pose.h"

#include <openxr/openxr.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#define HAND_COUNT 2

constexpr XrPosef XR_INDENTITY_POSE = {
    .orientation = { .x = 0, .y = 0, .z = 0, .w = 1.0 },
    .position = { .x = 0, .y = 0, .z = 0 },
};

constexpr XrViewConfigurationType XR_VIEW_CONFIG_TYPE = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

class OpenXrContext {
public:
    XrInstance _instance = XR_NULL_HANDLE;
    XrSession _session = XR_NULL_HANDLE;
    XrSystemId _systemId = XR_NULL_SYSTEM_ID;

    XrSpace _stageSpace = XR_NULL_HANDLE;
    XrSpace _viewSpace = XR_NULL_HANDLE;
    XrPath _handPaths[HAND_COUNT];

    controller::Pose _lastHeadPose;
    XrTime _lastPredictedDisplayTime;
    // TODO: Enable C++17 and use std::optional
    bool _lastPredictedDisplayTimeInitialized = false;

    bool _shouldQuit = false;
    bool _shouldRunFrameCycle = false;

    bool _isSupported = false;

    QString _systemName;
    bool _isSessionRunning = false;

private:
    XrSessionState _lastSessionState = XR_SESSION_STATE_UNKNOWN;

public:
    OpenXrContext();
    ~OpenXrContext();

    bool initPostGraphics();
    bool beginFrame();
    bool pollEvents();
    bool requestExitSession();
    void reset();

private:
    bool initPreGraphics();
    bool initInstance();
    bool initSystem();
    bool initGraphics();
    bool initSession();
    bool initSpaces();

    bool updateSessionState(XrSessionState newState);
};

inline static glm::vec3 xrVecToGlm(const XrVector3f& v) {
    return glm::vec3(v.x, v.y, v.z);
}

inline static glm::quat xrQuatToGlm(const XrQuaternionf& q) {
    return glm::quat(q.w, q.x, q.y, q.z);
}

bool xrCheck(XrInstance instance, XrResult result, const char* message);