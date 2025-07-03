//
// Overte OpenXR Plugin
//
// Copyright 2024 Lubosz Sarnecki
// Copyright 2024 Overte e.V.
//
// SPDX-License-Identifier: Apache-2.0
//

#include "OpenXrContext.h"
#include <QLoggingCategory>
#include <QString>
#include <QGuiApplication>

#include <sstream>

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

static bool loadXrFunction(XrInstance instance, const char* name, PFN_xrVoidFunction* out) {
    auto result = xrGetInstanceProcAddr(instance, name, out);

    if (result != XR_SUCCESS) {
        qCCritical(xr_context_cat) << "Failed to load OpenXR function '" << name << "'";
        return false;
    }

    return true;
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
    auto myApp = static_cast<QGuiApplication*>(qApp);
    if (myApp->platformName() == "wayland") {
        auto msg = QString::fromUtf8("The OpenXR plugin does not support Wayland yet! Use the QT_QPA_PLATFORM=xcb environment variable to force Overte to launch with X11.");
        qCCritical(xr_context_cat) << msg;
        return false;
    }

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
    bool userPresenceSupported = false;
    bool odysseyControllerSupported = false;
    bool handTrackingSupported = false;

    qCInfo(xr_context_cat, "Runtime supports %d extensions:", count);
    for (uint32_t i = 0; i < count; i++) {
        qCInfo(xr_context_cat, "%s v%d", properties[i].extensionName, properties[i].extensionVersion);
        if (strcmp(XR_KHR_OPENGL_ENABLE_EXTENSION_NAME, properties[i].extensionName) == 0) {
            openglSupported = true;
        } else if (strcmp(XR_EXT_USER_PRESENCE_EXTENSION_NAME, properties[i].extensionName) == 0) {
            userPresenceSupported = true;
        } else if (strcmp(XR_EXT_SAMSUNG_ODYSSEY_CONTROLLER_EXTENSION_NAME, properties[i].extensionName) == 0) {
            odysseyControllerSupported = true;
        } else if (strcmp(XR_EXT_HAND_TRACKING_EXTENSION_NAME, properties[i].extensionName) == 0) {
            handTrackingSupported = true;
        }
    }

    if (!openglSupported) {
        qCCritical(xr_context_cat, "Runtime does not support OpenGL!");
        return false;
    }

    std::vector<const char*> enabled = {XR_KHR_OPENGL_ENABLE_EXTENSION_NAME};

    if (userPresenceSupported) {
        enabled.push_back(XR_EXT_USER_PRESENCE_EXTENSION_NAME);
    }

    if (odysseyControllerSupported) {
        enabled.push_back(XR_EXT_SAMSUNG_ODYSSEY_CONTROLLER_EXTENSION_NAME);
    }

    if (handTrackingSupported) {
        enabled.push_back(XR_EXT_HAND_TRACKING_EXTENSION_NAME);
        _handTrackingSupported = true;
    }

    XrInstanceCreateInfo info = {
        .type = XR_TYPE_INSTANCE_CREATE_INFO,
        .applicationInfo = {
            .applicationName = "Overte",
            .applicationVersion = 1,
            .engineName = "Overte",
            .engineVersion = 0,
            .apiVersion = XR_API_VERSION_1_0,
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

    if (!loadXrFunction(_instance, "xrGetOpenGLGraphicsRequirementsKHR", (PFN_xrVoidFunction*)&pfnGetOpenGLGraphicsRequirementsKHR)) {
        qCCritical(xr_context_cat) << "Failed to get OpenGL graphics requirements function!";
        return false;
    }

    xrStringToPath(_instance, "/user/hand/left", &_handPaths[0]);
    xrStringToPath(_instance, "/user/hand/right", &_handPaths[1]);

    xrStringToPath(_instance, "/interaction_profiles/htc/vive_controller", &_viveControllerPath);

    if (userPresenceSupported) {
        XrSystemUserPresencePropertiesEXT presenceProps = {XR_TYPE_SYSTEM_USER_PRESENCE_PROPERTIES_EXT};
        XrSystemProperties sysProps = {XR_TYPE_SYSTEM_PROPERTIES, &presenceProps};
        result = xrGetSystemProperties(_instance, _systemId, &sysProps);
        if (xrCheck(XR_NULL_HANDLE, result, "Couldn't get system properties")) {
            _userPresenceAvailable = presenceProps.supportsUserPresence;
        }
    }

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
        .next = nullptr,
    };

    XrSystemHandTrackingPropertiesEXT handTrackingProps = {
        .type = XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_EXT,
        .next = props.next,
    };

    if (_handTrackingSupported) {
        props.next = &handTrackingProps;
    }

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

    auto next = reinterpret_cast<const XrExtensionProperties*>(props.next);
    while (next) {
        if (next->type == XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_EXT) {
            auto ext = reinterpret_cast<const XrSystemHandTrackingPropertiesEXT*>(next);
            _handTrackingSupported = ext->supportsHandTracking;

            xrGetInstanceProcAddr(
                _instance,
                "xrCreateHandTrackerEXT",
                reinterpret_cast<PFN_xrVoidFunction*>(&xrCreateHandTrackerEXT)
            );

            xrGetInstanceProcAddr(
                _instance,
                "xrDestroyHandTrackerEXT",
                reinterpret_cast<PFN_xrVoidFunction*>(&xrDestroyHandTrackerEXT)
            );

            xrGetInstanceProcAddr(
                _instance,
                "xrLocateHandJointsEXT",
                reinterpret_cast<PFN_xrVoidFunction*>(&xrLocateHandJointsEXT)
            );
        }

        next = reinterpret_cast<const XrExtensionProperties*>(next->next);
    }

    // don't start up hand tracking stuff if it's force disabled
    if (qApp->arguments().contains("--xrNoHandTracking")) {
        _handTrackingSupported = false;
    }

    return true;
}

bool OpenXrContext::initGraphics() {
    XrGraphicsRequirementsOpenGLKHR requirements = { .type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR };
    XrResult result = pfnGetOpenGLGraphicsRequirementsKHR(_instance, _systemId, &requirements);
    return xrCheck(_instance, result, "Failed to get OpenGL graphics requirements!");
}

bool OpenXrContext::requestExitSession() {
    if (_session == XR_NULL_HANDLE) { return true; }

    XrResult result = xrRequestExitSession(_session);
    return xrCheck(_instance, result, "Failed to request exit session!");
}

bool OpenXrContext::initSession() {
    if (_session != XR_NULL_HANDLE) { return true; }

    XrSessionCreateInfo info = {
        .type = XR_TYPE_SESSION_CREATE_INFO,
        .next = nullptr,
        .systemId = _systemId,
    };

#if defined(Q_OS_LINUX)
    // if (wayland) {
    // blah blah...
    // info.next = &wlBinding;
    // } else
    XrGraphicsBindingOpenGLXlibKHR xlibBinding = {
        .type = XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR,
        .xDisplay = XOpenDisplay(nullptr),
        .glxDrawable = glXGetCurrentDrawable(),
        .glxContext = glXGetCurrentContext(),
    };

    info.next = &xlibBinding;
#elif defined(Q_OS_WIN)
    XrGraphicsBindingOpenGLWin32KHR binding = {
        .type = XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR,
        .hDC = wglGetCurrentDC(),
        .hGLRC = wglGetCurrentContext(),
    };

    info.next = &binding;
#else
  #error "Unsupported platform"
#endif

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
            _isValid = true;
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
            _session = XR_NULL_HANDLE;
            _isValid = false;
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
                const auto& instanceLossPending = *reinterpret_cast<XrEventDataInstanceLossPending*>(&event);
                qCCritical(xr_context_cat, "Instance loss pending at %lu! Destroying instance.", instanceLossPending.lossTime);
                _shouldQuit = true;
                _isValid = false;
                continue;
            }
            case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
                const auto& sessionStateChanged = *reinterpret_cast<XrEventDataSessionStateChanged*>(&event);
                if (!updateSessionState(sessionStateChanged.state)) {
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

                    _stickEmulation = false;
                    if (_viveControllerPath != XR_NULL_PATH && state.interactionProfile == _viveControllerPath) {
                        _stickEmulation = true;
                    }

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
            case XR_TYPE_EVENT_DATA_USER_PRESENCE_CHANGED_EXT: {
                const auto& eventdata = *reinterpret_cast<XrEventDataUserPresenceChangedEXT*>(&event);
                _hmdMounted = eventdata.isUserPresent;
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
        _isValid = false;
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
