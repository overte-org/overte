//
// Overte OpenXR Plugin
//
// Copyright 2024 Lubosz Sarnecki
// Copyright 2024 Overte e.V.
//
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <optional>

#include <openxr/openxr.h>

#include "gpu/gl/GLBackend.h"

#if defined(Q_OS_LINUX)
    #define XR_USE_PLATFORM_XLIB
    #include <GL/glx.h>
    // Unsorted from glx.h conflicts with qdir.h
    #undef Unsorted
    // MappingPointer from X11 conflicts with one from controllers/Forward.h
    #undef MappingPointer
#elif defined(Q_OS_WIN)
    #define XR_USE_PLATFORM_WIN32
    #include <Unknwn.h>
    #include <Windows.h>
#else
    #error "Unimplemented platform"
#endif


#define XR_USE_GRAPHICS_API_OPENGL
#include <openxr/openxr_platform.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "controllers/Pose.h"

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
    std::optional<XrTime> _lastPredictedDisplayTime;

    bool _isValid = true; // set to false when the context is lost
    bool _shouldQuit = false;
    bool _shouldRunFrameCycle = false;
    bool _isDisplayActive = false;

    bool _isSupported = false;

    QString _systemName;
    bool _isSessionRunning = false;

    // hack for vive controllers
    bool _stickEmulation = false;

    // only supported by a few runtimes, but lets us
    // emulate OpenVR's headset proximity sensor system
    bool _userPresenceAvailable = false;

    // whether the headset is on, using XR_EXT_user_presence
    bool _hmdMounted = true;

    bool _handTrackingSupported = false;

    PFN_xrCreateHandTrackerEXT xrCreateHandTrackerEXT = nullptr;
    PFN_xrLocateHandJointsEXT xrLocateHandJointsEXT = nullptr;
    PFN_xrDestroyHandTrackerEXT xrDestroyHandTrackerEXT = nullptr;

private:
    XrSessionState _lastSessionState = XR_SESSION_STATE_UNKNOWN;

    XrPath _viveControllerPath = XR_NULL_PATH;

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
