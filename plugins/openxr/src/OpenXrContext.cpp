//
// Overte OpenXR Plugin
//
// Copyright 2024 Lubosz Sarnecki
//
// SPDX-License-Identifier: Apache-2.0
//

#include "OpenXrContext.h"
#include <qloggingcategory.h>

#include <sstream>

#include <GL/glx.h>

#define XR_USE_PLATFORM_XLIB
#define XR_USE_GRAPHICS_API_OPENGL
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

Q_DECLARE_LOGGING_CATEGORY(xr_context_cat)
Q_LOGGING_CATEGORY(xr_context_cat, "openxr.context")

// Checks XrResult, returns false on errors and logs the error as qCritical.
bool xrCheck(XrInstance instance, XrResult result, const char* message) {
    if (XR_SUCCEEDED(result))
        return true;

    char errorName[XR_MAX_RESULT_STRING_SIZE];
    if (instance != XR_NULL_HANDLE) {
        xrResultToString(instance, result, errorName);
    } else {
        sprintf(errorName, "%d", result);
    }

    qCCritical(xr_context_cat, "%s: %s", errorName, message);

    return false;
}

// Extension functions must be loaded with xrGetInstanceProcAddr
static PFN_xrGetOpenGLGraphicsRequirementsKHR pfnGetOpenGLGraphicsRequirementsKHR = nullptr;
static bool initFunctionPointers(XrInstance instance) {
    XrResult result = xrGetInstanceProcAddr(instance, "xrGetOpenGLGraphicsRequirementsKHR",
                                            (PFN_xrVoidFunction*)&pfnGetOpenGLGraphicsRequirementsKHR);
    return xrCheck(instance, result, "Failed to get OpenGL graphics requirements function!");
}

OpenXrContext::OpenXrContext() {
    _isSupported = initPreGraphics();
    if (!_isSupported) {
        qCWarning(xr_context_cat, "OpenXR is not supported.");
    }
}

OpenXrContext::~OpenXrContext() {
    if (_instance == XR_NULL_HANDLE) {
        return;
    }
    XrResult res = xrDestroyInstance(_instance);
    if (res != XR_SUCCESS) {
        qCCritical(xr_context_cat, "Failed to destroy OpenXR instance");
    }
    qCDebug(xr_context_cat, "Destroyed instance.");
}

bool OpenXrContext::initInstance() {
    uint32_t count = 0;
    XrResult result = xrEnumerateInstanceExtensionProperties(nullptr, 0, &count, nullptr);

    // Since this is the first OpenXR call we do, check here if RUNTIME_UNAVAILABLE is returned.
    if (result == XR_ERROR_RUNTIME_UNAVAILABLE) {
        qCCritical(xr_context_cat, "XR_ERROR_RUNTIME_UNAVAILABLE: Is XR_RUNTIME_JSON set correctly?");
        return false;
    }

    if (!xrCheck(XR_NULL_HANDLE, result, "Failed to enumerate number of extensions."))
        return false;

    std::vector<XrExtensionProperties> properties;
    for (uint32_t i = 0; i < count; i++) {
        XrExtensionProperties props = { .type = XR_TYPE_EXTENSION_PROPERTIES };
        properties.push_back(props);
    }

    result = xrEnumerateInstanceExtensionProperties(nullptr, count, &count, properties.data());
    if (!xrCheck(XR_NULL_HANDLE, result, "Failed to enumerate extensions."))
        return false;

    bool openglSupported = false;

    qCInfo(xr_context_cat, "Runtime supports %d extensions:", count);
    for (uint32_t i = 0; i < count; i++) {
        qCInfo(xr_context_cat, "%s v%d", properties[i].extensionName, properties[i].extensionVersion);
        if (strcmp(XR_KHR_OPENGL_ENABLE_EXTENSION_NAME, properties[i].extensionName) == 0) {
            openglSupported = true;
        }
    }

    if (!openglSupported) {
        qCCritical(xr_context_cat, "Runtime does not support OpenGL!");
        return false;
    }

    std::vector<const char*> enabled = { XR_KHR_OPENGL_ENABLE_EXTENSION_NAME };

    XrInstanceCreateInfo info = {
      .type = XR_TYPE_INSTANCE_CREATE_INFO,
      .applicationInfo = {
          .applicationName = "overte",
          .applicationVersion = 1,
          .engineName = "overte",
          .engineVersion = 0,
          .apiVersion = XR_CURRENT_API_VERSION,
      },
      .enabledExtensionCount = (uint32_t)enabled.size(),
      .enabledExtensionNames = enabled.data(),
  };

    result = xrCreateInstance(&info, &_instance);

    if (result == XR_ERROR_RUNTIME_FAILURE) {
        qCCritical(xr_context_cat, "XR_ERROR_RUNTIME_FAILURE: Is the OpenXR runtime up and running?");
        return false;
    }

    if (!xrCheck(XR_NULL_HANDLE, result, "Failed to create OpenXR instance."))
        return false;

    if (!initFunctionPointers(_instance))
        return false;

    xrStringToPath(_instance, "/user/hand/left", &_handPaths[0]);
    xrStringToPath(_instance, "/user/hand/right", &_handPaths[1]);

    return true;
}

bool OpenXrContext::initSystem() {
    XrSystemGetInfo info = {
        .type = XR_TYPE_SYSTEM_GET_INFO,
        .formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY,
    };

    XrResult result = xrGetSystem(_instance, &info, &_systemId);
    if (!xrCheck(_instance, result, "Failed to get system for HMD form factor."))
        return false;

    XrSystemProperties props = {
        .type = XR_TYPE_SYSTEM_PROPERTIES,
    };

    result = xrGetSystemProperties(_instance, _systemId, &props);
    if (!xrCheck(_instance, result, "Failed to get System properties"))
        return false;

    _systemName = QString::fromUtf8(props.systemName);

    qCInfo(xr_context_cat, "System name         : %s", props.systemName);
    qCInfo(xr_context_cat, "Max layers          : %d", props.graphicsProperties.maxLayerCount);
    qCInfo(xr_context_cat, "Max swapchain size  : %dx%d", props.graphicsProperties.maxSwapchainImageHeight,
           props.graphicsProperties.maxSwapchainImageWidth);
    qCInfo(xr_context_cat, "Orientation Tracking: %d", props.trackingProperties.orientationTracking);
    qCInfo(xr_context_cat, "Position Tracking   : %d", props.trackingProperties.positionTracking);

    return true;
}

bool OpenXrContext::initGraphics() {
    XrGraphicsRequirementsOpenGLKHR requirements = { .type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR };
    XrResult result = pfnGetOpenGLGraphicsRequirementsKHR(_instance, _systemId, &requirements);
    return xrCheck(_instance, result, "Failed to get OpenGL graphics requirements!");
}

bool OpenXrContext::requestExitSession() {
    XrResult result = xrRequestExitSession(_session);
    return xrCheck(_instance, result, "Failed to request exit session!");
}

bool OpenXrContext::initSession() {
    // TODO: Make cross platform
    XrGraphicsBindingOpenGLXlibKHR binding = {
        .type = XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR,
        .xDisplay = XOpenDisplay(nullptr),
        .glxDrawable = glXGetCurrentDrawable(),
        .glxContext = glXGetCurrentContext(),
    };

    XrSessionCreateInfo info = {
        .type = XR_TYPE_SESSION_CREATE_INFO,
        .next = &binding,
        .systemId = _systemId,
    };

    XrResult result = xrCreateSession(_instance, &info, &_session);
    return xrCheck(_instance, result, "Failed to create session");
}

bool OpenXrContext::initSpaces() {
    // TODO: Do xrEnumerateReferenceSpaces before assuming stage space is available.
    XrReferenceSpaceCreateInfo stageSpaceInfo = {
        .type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO,
        .referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE,
        .poseInReferenceSpace = XR_INDENTITY_POSE,
    };

    XrResult result = xrCreateReferenceSpace(_session, &stageSpaceInfo, &_stageSpace);
    if (!xrCheck(_instance, result, "Failed to create stage space!"))
        return false;

    XrReferenceSpaceCreateInfo viewSpaceInfo = {
        .type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO,
        .referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW,
        .poseInReferenceSpace = XR_INDENTITY_POSE,
    };

    result = xrCreateReferenceSpace(_session, &viewSpaceInfo, &_viewSpace);
    return xrCheck(_instance, result, "Failed to create view space!");
}

#define ENUM_TO_STR(r) \
    case r:            \
        return #r

static std::string xrSessionStateStr(XrSessionState state) {
    switch (state) {
        ENUM_TO_STR(XR_SESSION_STATE_UNKNOWN);
        ENUM_TO_STR(XR_SESSION_STATE_IDLE);
        ENUM_TO_STR(XR_SESSION_STATE_READY);
        ENUM_TO_STR(XR_SESSION_STATE_SYNCHRONIZED);
        ENUM_TO_STR(XR_SESSION_STATE_VISIBLE);
        ENUM_TO_STR(XR_SESSION_STATE_FOCUSED);
        ENUM_TO_STR(XR_SESSION_STATE_STOPPING);
        ENUM_TO_STR(XR_SESSION_STATE_LOSS_PENDING);
        ENUM_TO_STR(XR_SESSION_STATE_EXITING);
        default: {
            std::ostringstream ss;
            ss << "UNKNOWN STATE " << state;
            return ss.str();
        }
    }
}

// Called before restarting a new session
void OpenXrContext::reset() {
    _shouldQuit = false;
    _lastSessionState = XR_SESSION_STATE_UNKNOWN;
}

bool OpenXrContext::updateSessionState(XrSessionState newState) {
    qCDebug(xr_context_cat, "Session state changed %s -> %s", xrSessionStateStr(_lastSessionState).c_str(),
            xrSessionStateStr(newState).c_str());
    _lastSessionState = newState;

    switch (newState) {
        // Don't run frame cycle but keep polling events
        case XR_SESSION_STATE_IDLE:
        case XR_SESSION_STATE_UNKNOWN: {
            _shouldRunFrameCycle = false;
            break;
        }

        // Run frame cycle and poll events
        case XR_SESSION_STATE_FOCUSED:
        case XR_SESSION_STATE_SYNCHRONIZED:
        case XR_SESSION_STATE_VISIBLE: {
            _shouldRunFrameCycle = true;
            break;
        }

        // Begin the session
        case XR_SESSION_STATE_READY: {
            if (!_isSessionRunning) {
                XrSessionBeginInfo session_begin_info = {
                    .type = XR_TYPE_SESSION_BEGIN_INFO,
                    .primaryViewConfigurationType = XR_VIEW_CONFIG_TYPE,
                };
                XrResult result = xrBeginSession(_session, &session_begin_info);
                if (!xrCheck(_instance, result, "Failed to begin session!"))
                    return false;
                qCDebug(xr_context_cat, "Session started!");
                _isSessionRunning = true;
            }
            _shouldRunFrameCycle = true;
            break;
        }

        // End the session, don't render, but keep polling for events
        case XR_SESSION_STATE_STOPPING: {
            if (_isSessionRunning) {
                XrResult result = xrEndSession(_session);
                if (!xrCheck(_instance, result, "Failed to end session!"))
                    return false;
                _isSessionRunning = false;
            }
            _shouldRunFrameCycle = false;
            break;
        }

        // Destroy session, skip run frame cycle, quit
        case XR_SESSION_STATE_LOSS_PENDING:
        case XR_SESSION_STATE_EXITING: {
            XrResult result = xrDestroySession(_session);
            if (!xrCheck(_instance, result, "Failed to destroy session!"))
                return false;
            _shouldQuit = true;
            _shouldRunFrameCycle = false;
            qCDebug(xr_context_cat, "Destroyed session");
            break;
        }
        default:
            qCWarning(xr_context_cat, "Unhandled session state: %d", newState);
    }

    return true;
}

bool OpenXrContext::pollEvents() {
    XrEventDataBuffer event = { .type = XR_TYPE_EVENT_DATA_BUFFER };
    XrResult result = xrPollEvent(_instance, &event);
    while (result == XR_SUCCESS) {
        switch (event.type) {
            case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: {
                XrEventDataInstanceLossPending* instanceLossPending = (XrEventDataInstanceLossPending*)&event;
                qCCritical(xr_context_cat, "Instance loss pending at %lu! Destroying instance.", instanceLossPending->lossTime);
                _shouldQuit = true;
                continue;
            }
            case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
                XrEventDataSessionStateChanged* sessionStateChanged = (XrEventDataSessionStateChanged*)&event;
                if (!updateSessionState(sessionStateChanged->state)) {
                    return false;
                }
                break;
            }
            case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED: {
                for (int i = 0; i < HAND_COUNT; i++) {
                    XrInteractionProfileState state = { .type = XR_TYPE_INTERACTION_PROFILE_STATE };
                    XrResult res = xrGetCurrentInteractionProfile(_session, _handPaths[i], &state);
                    if (!xrCheck(_instance, res, "Failed to get interaction profile"))
                        continue;

                    uint32_t bufferCountOutput;
                    char profilePath[XR_MAX_PATH_LENGTH];
                    res = xrPathToString(_instance, state.interactionProfile, XR_MAX_PATH_LENGTH, &bufferCountOutput,
                                         profilePath);
                    if (!xrCheck(_instance, res, "Failed to get interaction profile path."))
                        continue;

                    qCInfo(xr_context_cat, "Controller %d: Interaction profile changed to '%s'", i, profilePath);
                }
                break;
            }
            default:
                qCWarning(xr_context_cat, "Unhandled event type %d", event.type);
        }

        event.type = XR_TYPE_EVENT_DATA_BUFFER;
        result = xrPollEvent(_instance, &event);
    }

    if (result != XR_EVENT_UNAVAILABLE) {
        qCCritical(xr_context_cat, "Failed to poll events!");
        return false;
    }

    return true;
}

bool OpenXrContext::beginFrame() {
    XrFrameBeginInfo info = { .type = XR_TYPE_FRAME_BEGIN_INFO };
    XrResult result = xrBeginFrame(_session, &info);
    return xrCheck(_instance, result, "failed to begin frame!");
}

bool OpenXrContext::initPreGraphics() {
    if (!initInstance()) {
        return false;
    }

    if (!initSystem()) {
        return false;
    }

    return true;
}

bool OpenXrContext::initPostGraphics() {
    if (!initGraphics()) {
        return false;
    }

    if (!initSession()) {
        return false;
    }

    if (!initSpaces()) {
        return false;
    }

    return true;
}
