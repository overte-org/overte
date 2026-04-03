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

#if defined(Q_OS_LINUX)
#include <QOpenGLContext>
#include <QtPlatformHeaders/QGLXNativeContext>
#endif

#if defined(HAVE_VULKAN)
#include <QMessageBox>
#endif

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
#if defined(HAVE_VULKAN)
    _isSupported = false;
    qCCritical(xr_context_cat, "OpenXR is not supported on the Vulkan backend yet.");
    QMessageBox::critical(nullptr, "OpenXR", "OpenXR is not supported on the Vulkan backend yet.");
#else
    _isSupported = initPreGraphics();
    if (!_isSupported) {
        qCWarning(xr_context_cat, "OpenXR is not supported.");
    }
#endif
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
#if defined(HAVE_VULKAN)
    // VKTODO
    return false;
#else
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
    bool palmPoseSupported = false;
    bool MNDX_xdevSpaceSupported = false;
    bool HTCX_viveTrackerInteractionSupported = false;
    bool MNDX_eglEnableSupported = false;

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
        } else if (strcmp(XR_MNDX_XDEV_SPACE_EXTENSION_NAME, properties[i].extensionName) == 0) {
            MNDX_xdevSpaceSupported = true;
        } else if (strcmp(XR_HTCX_VIVE_TRACKER_INTERACTION_EXTENSION_NAME, properties[i].extensionName) == 0) {
            HTCX_viveTrackerInteractionSupported = true;
        } else if (strcmp(XR_EXT_PALM_POSE_EXTENSION_NAME, properties[i].extensionName) == 0) {
            palmPoseSupported = true;
#if defined(XR_USE_PLATFORM_EGL)
        } else if (strcmp(XR_MNDX_EGL_ENABLE_EXTENSION_NAME, properties[i].extensionName) == 0) {
            MNDX_eglEnableSupported = true;
#endif
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

    if (MNDX_xdevSpaceSupported) {
        enabled.push_back(XR_MNDX_XDEV_SPACE_EXTENSION_NAME);
        _MNDX_xdevSpaceSupported = true;
    }

    if (HTCX_viveTrackerInteractionSupported) {
        enabled.push_back(XR_HTCX_VIVE_TRACKER_INTERACTION_EXTENSION_NAME);
        _HTCX_viveTrackerInteractionSupported = true;
    }

    if (palmPoseSupported) {
        enabled.push_back(XR_EXT_PALM_POSE_EXTENSION_NAME);
        _palmPoseSupported = true;
    }

#if defined(XR_USE_PLATFORM_EGL)
    if (MNDX_eglEnableSupported) {
        enabled.push_back(XR_MNDX_EGL_ENABLE_EXTENSION_NAME);
        _MNDX_eglEnableSupported = true;
    }
#endif

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
#endif
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

    XrSystemXDevSpacePropertiesMNDX xdevProps = {
        .type =XR_TYPE_SYSTEM_XDEV_SPACE_PROPERTIES_MNDX,
        .next = props.next,
    };

    if (_MNDX_xdevSpaceSupported) {
        props.next = &xdevProps;
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

            if (!_handTrackingSupported) {
                next = reinterpret_cast<const XrExtensionProperties*>(next->next);
                continue;
            }

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

        if (next->type == XR_TYPE_SYSTEM_XDEV_SPACE_PROPERTIES_MNDX) {
            auto ext = reinterpret_cast<const XrSystemXDevSpacePropertiesMNDX*>(next);
            _MNDX_xdevSpaceSupported = ext->supportsXDevSpace;

            if (!_MNDX_xdevSpaceSupported) {
                next = reinterpret_cast<const XrExtensionProperties*>(next->next);
                continue;
            }

            xrGetInstanceProcAddr(
                _instance,
                "xrCreateXDevListMNDX",
                reinterpret_cast<PFN_xrVoidFunction*>(&xrCreateXDevListMNDX)
            );

            xrGetInstanceProcAddr(
                _instance,
                "xrGetXDevListGenerationNumberMNDX",
                reinterpret_cast<PFN_xrVoidFunction*>(&xrGetXDevListGenerationNumberMNDX)
            );

            xrGetInstanceProcAddr(
                _instance,
                "xrEnumerateXDevsMNDX",
                reinterpret_cast<PFN_xrVoidFunction*>(&xrEnumerateXDevsMNDX)
            );

            xrGetInstanceProcAddr(
                _instance,
                "xrGetXDevPropertiesMNDX",
                reinterpret_cast<PFN_xrVoidFunction*>(&xrGetXDevPropertiesMNDX)
            );

            xrGetInstanceProcAddr(
                _instance,
                "xrDestroyXDevListMNDX",
                reinterpret_cast<PFN_xrVoidFunction*>(&xrDestroyXDevListMNDX)
            );

            xrGetInstanceProcAddr(
                _instance,
                "xrCreateXDevSpaceMNDX",
                reinterpret_cast<PFN_xrVoidFunction*>(&xrCreateXDevSpaceMNDX)
            );
        }

        next = reinterpret_cast<const XrExtensionProperties*>(next->next);
    }

    // don't start up hand tracking stuff if it's force disabled
    if (qApp->arguments().contains("--xrNoHandTracking")) {
        _handTrackingSupported = false;
    }

    if (qApp->arguments().contains("--xrNoBodyTracking")) {
        _MNDX_xdevSpaceSupported = false;
        _HTCX_viveTrackerInteractionSupported = false;
    }

    if (qApp->arguments().contains("--xrNoPalmPose")) {
        _palmPoseSupported = false;
    }

    // disable the MNDX tracker extension if they're both available
    if (_HTCX_viveTrackerInteractionSupported) {
        _MNDX_xdevSpaceSupported = false;

        xrGetInstanceProcAddr(
            _instance,
            "xrEnumerateViveTrackerPathsHTCX",
            reinterpret_cast<PFN_xrVoidFunction*>(&xrEnumerateViveTrackerPathsHTCX)
        );
    }

    return true;
}

bool OpenXrContext::initGraphics() {
#if defined(HAVE_VULKAN)
    // VKTODO
    return false;
#else
    XrGraphicsRequirementsOpenGLKHR requirements = { .type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR };
    XrResult result = pfnGetOpenGLGraphicsRequirementsKHR(_instance, _systemId, &requirements);
    return xrCheck(_instance, result, "Failed to get OpenGL graphics requirements!");
#endif
}

bool OpenXrContext::requestExitSession() {
    if (_session == XR_NULL_HANDLE) { return true; }

    XrResult result = xrRequestExitSession(_session);
    return xrCheck(_instance, result, "Failed to request exit session!");
}

bool OpenXrContext::initSession() {
#if defined(HAVE_VULKAN)
    // VKTODO
    return false;
#else
    if (_session != XR_NULL_HANDLE) { return true; }

    XrSessionCreateInfo info = {
        .type = XR_TYPE_SESSION_CREATE_INFO,
        .next = nullptr,
        .systemId = _systemId,
    };

    bool eglBindingAvailable = false;

#if defined(XR_USE_PLATFORM_EGL)
    XrGraphicsBindingEGLMNDX eglBinding = {
        .type = XR_TYPE_GRAPHICS_BINDING_EGL_MNDX,
        .next = nullptr,
    };

    // try egl first since it should work on any platform
    // do-while so we can break out early
    do {
        if (!_MNDX_eglEnableSupported) { break; }

        auto eglContext = eglGetCurrentContext();
        auto eglDisplay = eglGetCurrentDisplay();

        if (!eglContext || !eglDisplay) { break; }

        auto attribs = std::to_array<EGLint>({
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
#if defined(Q_OS_ANDROID)
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
#else
            EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT | EGL_OPENGL_ES3_BIT,
#endif
            EGL_NONE // terminator
        });

        EGLConfig eglConfig;
        EGLint configCount = 0;
        if (
            !eglChooseConfig(eglDisplay, attribs.data(), &eglConfig, 1, &configCount) ||
            configCount != 1 ||
            !eglConfig
        ) {
            qCWarning(xr_context_cat, "Failed to get EGL config");
            break;
        }

        eglBinding.getProcAddress = eglGetProcAddress;
        eglBinding.display = eglDisplay;
        eglBinding.config = eglConfig;
        eglBinding.context = eglContext;
        info.next = &eglBinding;

        eglBindingAvailable = true;
    } while(0);
#endif

#if defined(Q_OS_ANDROID)
    // ANDROID TODO: This is untested and will need changes in
    // OpenXrDisplayPlugin to use the OpenGLES structs instead
    if (!eglBindingAvailable) {
        auto eglContext = eglGetCurrentContext();
        auto eglDisplay = eglGetCurrentDisplay();

        auto attribs = std::to_array<EGLint>({
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_NONE // terminator
        });

        EGLConfig eglConfig;
        EGLint configCount = 0;
        if (
            !eglChooseConfig(eglDisplay, attribs.data(), &eglConfig, 1, &configCount) ||
            configCount != 1 ||
            !eglConfig
        ) {
            qCWarning(xr_context_cat, "Failed to get EGL config");
            return false;
        }

        XrGraphicsBindingOpenGLESAndroidKHR androidBinding = {
            .type = XR_TYPE_GRAPHICS_BINDING_OPENGL_ES_ANDROID_KHR,
            .next = nullptr,
            .display = eglDisplay,
            .config = eglConfig,
            .context = eglContext,
        };

        info.next = &androidBinding;
    }
#elif defined(Q_OS_LINUX)
    if (!eglBindingAvailable) {
        auto* xDisplay = XOpenDisplay(nullptr);
        int fbConfigCount = 0;
        auto* fbConfigs = glXGetFBConfigs(xDisplay, 0, &fbConfigCount);

        XrGraphicsBindingOpenGLXlibKHR xlibBinding = {
            .type = XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR,
            .xDisplay = xDisplay,

            // not actually used anywhere but monado now
            // requires these to be non-null (in-line with the spec)
            .visualid = 1,
            .glxFBConfig = fbConfigs[0],

            .glxDrawable = glXGetCurrentDrawable(),
            .glxContext = glXGetCurrentContext(),
        };

        // HACK: Is this a compiler bug? How come adding this check fixes
        // the XR_ERROR_GRAPHICS_DEVICE_INVALID (glxContext is null) error??
        // Putting glxContext into a separate variable and checking that *doesn't*
        // work, but checking it after xlibBinding has been initialised with it *does*?
        if (!xlibBinding.glxContext) {
            qCCritical(xr_context_cat, "glXContext is null");
            return false;
        }

        info.next = &xlibBinding;
    }
#elif defined(Q_OS_WIN)
    if (!eglBindingAvailable) {
        XrGraphicsBindingOpenGLWin32KHR binding = {
            .type = XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR,
            .hDC = wglGetCurrentDC(),
            .hGLRC = wglGetCurrentContext(),
        };

        info.next = &binding;
    }
#else
    #error "Unsupported platform"
#endif

    XrResult result = xrCreateSession(_instance, &info, &_session);
    return xrCheck(_instance, result, "Failed to create session");
#endif
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

                    _vivePoseHack[i] = _viveControllerPath != XR_NULL_PATH && state.interactionProfile == _viveControllerPath;

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
