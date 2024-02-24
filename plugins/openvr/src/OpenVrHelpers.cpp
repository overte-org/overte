//
//  Created by Bradley Austin Davis on 2015/11/01
//  Copyright 2015 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "OpenVrHelpers.h"

#include <atomic>
#include <mutex>

#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <QtCore/QThread>
#include <QtCore/QLoggingCategory>
#include <QtCore/QProcessEnvironment>
#include <QtGui/QInputMethodEvent>
#include <QtQuick/QQuickWindow>

#include <OffscreenUi.h>
#include <controllers/Pose.h>
#include <NumericalConstants.h>
#include <ui-plugins/PluginContainer.h>
#include <plugins/PluginManager.h>

#include <ui/Menu.h>
#include "../../interface/src/Menu.h"

Q_DECLARE_LOGGING_CATEGORY(displayplugins)
Q_LOGGING_CATEGORY(displayplugins, "hifi.plugins.display")

using Mutex = std::mutex;
using Lock = std::unique_lock<Mutex>;

static int refCount { 0 };
static Mutex mutex;
static vr::IVRSystem* activeHmd { nullptr };
static bool _openVrQuitRequested { false };
static bool _headInHeadset { false };

bool isHeadInHeadset() {
    return _headInHeadset;
}

bool openVrQuitRequested() {
    return _openVrQuitRequested;
}

static const uint32_t RELEASE_OPENVR_HMD_DELAY_MS = 5000;

bool isOculusPresent() {
    bool result = false;
#ifdef Q_OS_WIN
    // Only check for Oculus presence if Oculus plugin is enabled
    if (PluginManager::getInstance()->getEnableOculusPluginSetting()) {
        qDebug() << "isOculusPresent: Oculus plugin enabled by a setting";
        HANDLE oculusServiceEvent = ::OpenEventW(SYNCHRONIZE, FALSE, L"OculusHMDConnected");
        // The existence of the service indicates a running Oculus runtime
        if (oculusServiceEvent) {
            // A signaled event indicates a connected HMD
            if (WAIT_OBJECT_0 == ::WaitForSingleObject(oculusServiceEvent, 0)) {
                result = true;
            }
            ::CloseHandle(oculusServiceEvent);
        }
    } else {
        qDebug() << "isOculusPresent: Oculus plugin disabled by a setting";
    }
#endif
    return result;
}

bool oculusViaOpenVR() {
    static const QString DEBUG_FLAG("HIFI_DEBUG_OPENVR");
    static bool enableDebugOpenVR = QProcessEnvironment::systemEnvironment().contains(DEBUG_FLAG);
    return enableDebugOpenVR && isOculusPresent() && vr::VR_IsHmdPresent();
}

std::string getOpenVrDeviceName() {
    auto system = acquireOpenVrSystem();
    std::string trackingSystemName = "";
    if (system) {
        uint32_t HmdTrackingIndex = 0;
        uint32_t bufferLength = system->GetStringTrackedDeviceProperty(HmdTrackingIndex, vr::Prop_TrackingSystemName_String, NULL, 0, NULL);
        if (bufferLength > 0) {
            char* stringBuffer = new char[bufferLength];
            system->GetStringTrackedDeviceProperty(HmdTrackingIndex, vr::Prop_ManufacturerName_String, stringBuffer, bufferLength, NULL);
            trackingSystemName = stringBuffer;
            delete[] stringBuffer;
        }
    }
    return trackingSystemName;
}

bool openVrSupported() {
    static const QString DEBUG_FLAG("HIFI_DEBUG_OPENVR");
    static bool enableDebugOpenVR = QProcessEnvironment::systemEnvironment().contains(DEBUG_FLAG);
    return (enableDebugOpenVR || !isOculusPresent()) && vr::VR_IsHmdPresent();
}

QString getVrSettingString(const char* section, const char* setting) {
    QString result;
    static const uint32_t BUFFER_SIZE = 1024;
    static char BUFFER[BUFFER_SIZE];
    vr::IVRSettings * vrSettings = vr::VRSettings();
    if (vrSettings) {
        vr::EVRSettingsError error = vr::VRSettingsError_None;
        vrSettings->GetString(vr::k_pch_audio_Section, setting, BUFFER, BUFFER_SIZE, &error);
        if (error == vr::VRSettingsError_None) {
            result = BUFFER;
        }
    }
    return result;
}

bool isHMDInErrorState = false;

vr::IVRSystem* acquireOpenVrSystem() {
    bool hmdPresent = vr::VR_IsHmdPresent();
    if (hmdPresent && !isHMDInErrorState) {
        Lock lock(mutex);
        if (!activeHmd) {
            #if DEV_BUILD
                qCDebug(displayplugins) << "OpenVR: No vr::IVRSystem instance active, building";
            #endif
            vr::EVRInitError eError = vr::VRInitError_None;
            activeHmd = vr::VR_Init(&eError, vr::VRApplication_Scene);

            #if DEV_BUILD
                qCDebug(displayplugins) << "OpenVR display: HMD is " << activeHmd << " error is " << eError;
            #endif

            if (eError == vr::VRInitError_Init_HmdNotFound) {
                isHMDInErrorState = true;
                activeHmd = nullptr;
                #if DEV_BUILD
                    qCDebug(displayplugins) << "OpenVR: No HMD connected, setting nullptr!";
                #endif
            }
        }
        if (activeHmd) {
            #if DEV_BUILD
                qCDebug(displayplugins) << "OpenVR: incrementing refcount";
            #endif
            ++refCount;
        }
    } else {
        #if DEV_BUILD
            qCDebug(displayplugins) << "OpenVR: no hmd present";
        #endif
    }
    return activeHmd;
}

void releaseOpenVrSystem() {
    if (activeHmd) {
        Lock lock(mutex);
        #if DEV_BUILD
            qCDebug(displayplugins) << "OpenVR: decrementing refcount";
        #endif
        --refCount;
        if (0 == refCount) {
            #if DEV_BUILD
                qCDebug(displayplugins) << "OpenVR: zero refcount, deallocate VR system";
            #endif

            // HACK: workaround openvr crash, call submit with an invalid texture, right before VR_Shutdown.
            const void* INVALID_GL_TEXTURE_HANDLE = (void*)(uintptr_t)-1;
            vr::Texture_t vrTexture{ (void*)INVALID_GL_TEXTURE_HANDLE, vr::TextureType_OpenGL, vr::ColorSpace_Auto };
            static vr::VRTextureBounds_t OPENVR_TEXTURE_BOUNDS_LEFT{ 0, 0, 0.5f, 1 };
            static vr::VRTextureBounds_t OPENVR_TEXTURE_BOUNDS_RIGHT{ 0.5f, 0, 1, 1 };

            auto compositor = vr::VRCompositor();
            if (compositor) {
                compositor->Submit(vr::Eye_Left, &vrTexture, &OPENVR_TEXTURE_BOUNDS_LEFT);
                compositor->Submit(vr::Eye_Right, &vrTexture, &OPENVR_TEXTURE_BOUNDS_RIGHT);
            }

            vr::VR_Shutdown();
            _openVrQuitRequested = false;
            activeHmd = nullptr;
        }
    }
}

static char textArray[8192];

static QMetaObject::Connection _focusConnection, _focusTextConnection, _overlayMenuConnection;
extern bool _openVrDisplayActive;
static vr::IVROverlay* _overlay { nullptr };
static QObject* _keyboardFocusObject { nullptr };
static QString _existingText;
static Qt::InputMethodHints _currentHints;
extern PoseData _nextSimPoseData;
static bool _keyboardShown { false };
static bool _overlayRevealed { false };
static const uint32_t SHOW_KEYBOARD_DELAY_MS = 400;

void updateFromOpenVrKeyboardInput() {
    auto chars = _overlay->GetKeyboardText(textArray, 8192);
    auto newText = QString(QByteArray(textArray, chars));
    _keyboardFocusObject->setProperty("text", newText);
    //// TODO modify the new text to match the possible input hints:
    //// ImhDigitsOnly  ImhFormattedNumbersOnly  ImhUppercaseOnly  ImhLowercaseOnly
    //// ImhDialableCharactersOnly ImhEmailCharactersOnly  ImhUrlCharactersOnly  ImhLatinOnly
    //QInputMethodEvent event(_existingText, QList<QInputMethodEvent::Attribute>());
    //event.setCommitString(newText, 0, _existingText.size());
    //qApp->sendEvent(_keyboardFocusObject, &event);
}

void finishOpenVrKeyboardInput() {
    auto offscreenUI = DependencyManager::get<OffscreenUi>();
    updateFromOpenVrKeyboardInput();
    // Simulate an enter press on the top level window to trigger the action
    if (0 == (_currentHints & Qt::ImhMultiLine) && offscreenUI) {
        auto keyPress = QKeyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::KeyboardModifiers(), QString("\n"));
        auto keyRelease = QKeyEvent(QEvent::KeyRelease, Qt::Key_Return, Qt::KeyboardModifiers());
        qApp->sendEvent(offscreenUI->getWindow(), &keyPress);
        qApp->sendEvent(offscreenUI->getWindow(), &keyRelease);
    }
}

static const QString DEBUG_FLAG("HIFI_DISABLE_STEAM_VR_KEYBOARD");
bool disableSteamVrKeyboard = QProcessEnvironment::systemEnvironment().contains(DEBUG_FLAG);

void enableOpenVrKeyboard(PluginContainer* container) {
    if (disableSteamVrKeyboard) {
        return;
    }
    _overlay = vr::VROverlay();

    auto menu = container->getPrimaryMenu();
    auto action = menu->getActionForOption(MenuOption::Overlays);

    // When the overlays are revealed, suppress the keyboard from appearing on text focus for a tenth of a second.
    _overlayMenuConnection = QObject::connect(action, &QAction::triggered, [action] {
        if (action->isChecked()) {
            _overlayRevealed = true;
            const int KEYBOARD_DELAY_MS = 100;
            QTimer::singleShot(KEYBOARD_DELAY_MS, [&] { _overlayRevealed = false; });
        }
    });
}


void disableOpenVrKeyboard() {
    if (disableSteamVrKeyboard) {
        return;
    }
    QObject::disconnect(_overlayMenuConnection);
    QObject::disconnect(_focusTextConnection);
    QObject::disconnect(_focusConnection);
}

bool isOpenVrKeyboardShown() {
    return _keyboardShown;
}


void handleOpenVrEvents() {
    if (!activeHmd) {
        return;
    }
    Lock lock(mutex);
    if (!activeHmd) {
        return;
    }

    vr::VREvent_t event;
    while (activeHmd->PollNextEvent(&event, sizeof(event))) {
        switch (event.eventType) {
            case vr::VREvent_Quit:
                _openVrQuitRequested = true;
                activeHmd->AcknowledgeQuit_Exiting();
                break;

            case vr::VREvent_KeyboardCharInput:
                // Make the focused field match the keyboard results, inclusive of combining characters and such.
                updateFromOpenVrKeyboardInput();
                break;

            case vr::VREvent_KeyboardDone:
                finishOpenVrKeyboardInput();

            // FALL THROUGH
            case vr::VREvent_KeyboardClosed:
                _keyboardFocusObject = nullptr;
                _keyboardShown = false;
                if (auto offscreenUI = DependencyManager::get<OffscreenUi>()) {
                    offscreenUI->unfocusWindows();
                }
                break;

            default:
                break;
        }
        if (event.data.controller.button == vr::k_EButton_ProximitySensor) {
            vr::VRControllerState_t controllerState = vr::VRControllerState_t();
            if (activeHmd->GetControllerState(vr::k_unTrackedDeviceIndex_Hmd, &controllerState, sizeof(vr::VRControllerState_t))) {
                ulong promitySensorFlag = (1UL << ((int)vr::k_EButton_ProximitySensor));
                _headInHeadset = (controllerState.ulButtonPressed & promitySensorFlag) == promitySensorFlag;
            }
        }

        #if DEV_BUILD
            //qDebug() << "OpenVR: Event " << activeHmd->GetEventTypeNameFromEnum((vr::EVREventType)event.eventType) << "(" << event.eventType << ")";
            // FIXME: Reinstate the line above and remove the following lines once the problem with excessive occurrences of 
            // VREvent_ActionBindingReloaded events is fixed in SteamVR for Linux. 
            // https://github.com/ValveSoftware/SteamVR-for-Linux/issues/307
            #ifdef Q_OS_LINUX
                if (event.eventType != vr::VREvent_ActionBindingReloaded) {
                    qDebug() << "OpenVR: Event " << activeHmd->GetEventTypeNameFromEnum((vr::EVREventType)event.eventType) << "(" << event.eventType << ")";
                };
            #else
                qDebug() << "OpenVR: Event " << activeHmd->GetEventTypeNameFromEnum((vr::EVREventType)event.eventType) << "(" << event.eventType << ")";
            #endif
        #endif
    }

}

controller::Pose openVrControllerPoseToHandPose(bool isLeftHand, const mat4& mat, const vec3& linearVelocity, const vec3& angularVelocity) {
    // When the sensor-to-world rotation is identity the coordinate axes look like this:
    //
    //                       user
    //                      forward
    //                        -z
    //                         |
    //                        y|      user
    //      y                  o----x right
    //       o-----x         user
    //       |                up
    //       |
    //       z
    //
    //     Rift

    // From ABOVE the hand canonical axes looks like this:
    //
    //      | | | |          y        | | | |
    //      | | | |          |        | | | |
    //      |     |          |        |     |
    //      |left | /  x---- +      \ |right|
    //      |     _/          z      \_     |
    //       |   |                     |   |
    //       |   |                     |   |
    //

    // So when the user is in Rift space facing the -zAxis with hands outstretched and palms down
    // the rotation to align the Touch axes with those of the hands is:
    //
    //    touchToHand = halfTurnAboutY * quaterTurnAboutX

    // Due to how the Touch controllers fit into the palm there is an offset that is different for each hand.
    // You can think of this offset as the inverse of the measured rotation when the hands are posed, such that
    // the combination (measurement * offset) is identity at this orientation.
    //
    //    Qoffset = glm::inverse(deltaRotation when hand is posed fingers forward, palm down)
    //
    // An approximate offset for the Touch can be obtained by inspection:
    //
    //    Qoffset = glm::inverse(glm::angleAxis(sign * PI/2.0f, zAxis) * glm::angleAxis(PI/4.0f, xAxis))
    //
    // So the full equation is:
    //
    //    Q = combinedMeasurement * touchToHand
    //
    //    Q = (deltaQ * QOffset) * (yFlip * quarterTurnAboutX)
    //
    //    Q = (deltaQ * inverse(deltaQForAlignedHand)) * (yFlip * quarterTurnAboutX)
    static const glm::quat yFlip = glm::angleAxis(PI, Vectors::UNIT_Y);
    static const glm::quat quarterX = glm::angleAxis(PI_OVER_TWO, Vectors::UNIT_X);
    static const glm::quat touchToHand = yFlip * quarterX;

    static const glm::quat leftQuarterZ = glm::angleAxis(-PI_OVER_TWO, Vectors::UNIT_Z);
    static const glm::quat rightQuarterZ = glm::angleAxis(PI_OVER_TWO, Vectors::UNIT_Z);
    static const glm::quat eighthX = glm::angleAxis(PI / 4.0f, Vectors::UNIT_X);

    static const glm::quat leftRotationOffset = glm::inverse(leftQuarterZ * eighthX) * touchToHand;
    static const glm::quat rightRotationOffset = glm::inverse(rightQuarterZ * eighthX) * touchToHand;

    // this needs to match the leftBasePosition in tutorial/viveControllerConfiguration.js:21
    static const float CONTROLLER_LATERAL_OFFSET = 0.0381f;
    static const float CONTROLLER_VERTICAL_OFFSET = 0.0495f;
    static const float CONTROLLER_FORWARD_OFFSET = 0.1371f;
    static const glm::vec3 CONTROLLER_OFFSET(CONTROLLER_LATERAL_OFFSET, CONTROLLER_VERTICAL_OFFSET, CONTROLLER_FORWARD_OFFSET);

    static const glm::vec3 leftTranslationOffset = glm::vec3(-1.0f, 1.0f, 1.0f) * CONTROLLER_OFFSET;
    static const glm::vec3 rightTranslationOffset = CONTROLLER_OFFSET;

    auto translationOffset = (isLeftHand ? leftTranslationOffset : rightTranslationOffset);
    auto rotationOffset = (isLeftHand ? leftRotationOffset : rightRotationOffset);

    glm::vec3 position = extractTranslation(mat);
    glm::quat rotation = glm::normalize(glm::quat_cast(mat));

    position += rotation * translationOffset;
    rotation = rotation * rotationOffset;

    // transform into avatar frame
    auto result = controller::Pose(position, rotation);
    // handle change in velocity due to translationOffset
    result.velocity = linearVelocity + glm::cross(angularVelocity, position - extractTranslation(mat));
    result.angularVelocity = angularVelocity;
    return result;
}

#define FAILED_MIN_SPEC_OVERLAY_NAME "FailedMinSpecOverlay"
#define FAILED_MIN_SPEC_OVERLAY_FRIENDLY_NAME "Minimum specifications for SteamVR not met"
#define FAILED_MIN_SPEC_UPDATE_INTERVAL_MS 10
#define FAILED_MIN_SPEC_AUTO_QUIT_INTERVAL_MS (MSECS_PER_SECOND * 30)
#define MIN_CORES_SPEC 3

void showMinSpecWarning() {
    auto vrSystem = acquireOpenVrSystem();
    auto vrOverlay = vr::VROverlay();
    if (!vrOverlay) {
        qFatal("Unable to initialize SteamVR overlay manager");
    }
    auto vrChaperone = vr::VRChaperone();
    if (!vrChaperone) {
        qFatal("Unable to initialize SteamVR chaperone");
    }
    auto vrCompositor = vr::VRCompositor();
    if (!vrCompositor) {
        qFatal("Unable to initialize SteamVR compositor");
    }

    vr::VROverlayHandle_t minSpecFailedOverlay = 0;
    if (vr::VROverlayError_None != vrOverlay->CreateOverlay(FAILED_MIN_SPEC_OVERLAY_NAME, FAILED_MIN_SPEC_OVERLAY_FRIENDLY_NAME, &minSpecFailedOverlay)) {
        qFatal("Unable to create overlay");
    }

#ifdef Q_OS_LINUX
    QFile cmdlineFile("/proc/self/cmdline");
    if (!cmdlineFile.open(QIODevice::ReadOnly)) {
        qFatal("Unable to open /proc/self/cmdline");
    }

    auto contents = cmdlineFile.readAll();
    auto arguments = contents.split('\0');
    arguments.pop_back();  // Last element is empty.
    cmdlineFile.close();

    int __argc = arguments.count();
    char** __argv = new char* [__argc];
    for (int i = 0; i < __argc; i++) {
        __argv[i] = arguments[i].data();
    }
#endif

    QCoreApplication miniApp(__argc, __argv);

#ifdef Q_OS_LINUX
    QObject::connect(&miniApp, &QCoreApplication::destroyed, [=] {
        delete[] __argv;
    });
#endif

    vrChaperone->ResetZeroPose(vrCompositor->GetTrackingSpace());
    QString imagePath = PathUtils::resourcesPath() + "/images/steam-min-spec-failed.png";
    vrOverlay->SetOverlayFromFile(minSpecFailedOverlay, imagePath.toLocal8Bit().toStdString().c_str());
    vrOverlay->SetOverlayWidthInMeters(minSpecFailedOverlay, 1.4f);
    vrOverlay->SetOverlayInputMethod(minSpecFailedOverlay, vr::VROverlayInputMethod_Mouse);
    vrOverlay->ShowOverlay(minSpecFailedOverlay);

    QTimer* timer = new QTimer(&miniApp);
    timer->setInterval(FAILED_MIN_SPEC_UPDATE_INTERVAL_MS); // Qt::CoarseTimer acceptable, we don't need this to be frame rate accurate
    QObject::connect(timer, &QTimer::timeout, [&] {
        vr::TrackedDevicePose_t vrPoses[vr::k_unMaxTrackedDeviceCount];
        vrSystem->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseSeated, 0, vrPoses, vr::k_unMaxTrackedDeviceCount);
        auto headPose = toGlm(vrPoses[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking);
        auto overlayPose = toOpenVr(headPose * glm::translate(glm::mat4(), vec3(0, 0, -1)));
        vrOverlay->SetOverlayTransformAbsolute(minSpecFailedOverlay, vr::TrackingUniverseSeated, &overlayPose);

        vr::VREvent_t event;
        while (vrSystem->PollNextEvent(&event, sizeof(event))) {
            switch (event.eventType) {
                case vr::VREvent_Quit:
                    vrSystem->AcknowledgeQuit_Exiting();
                    QCoreApplication::quit();
                    break;

                case vr::VREvent_ButtonPress:
                    // Quit on any button press except for 'putting on the headset'
                    if (event.data.controller.button != vr::k_EButton_ProximitySensor) {
                        QCoreApplication::quit();
                    }
                    break;

                default:
                    break;
            }
        }

    });
    timer->start();

    QTimer::singleShot(FAILED_MIN_SPEC_AUTO_QUIT_INTERVAL_MS, &miniApp, &QCoreApplication::quit);
    miniApp.exec();
}


bool checkMinSpecImpl() {
    // If OpenVR isn't supported, we have no min spec, so pass
    if (!openVrSupported()) {
        return true;
    }

    // If we have at least MIN_CORES_SPEC cores, pass
    auto coreCount = QThread::idealThreadCount();
    if (coreCount >= MIN_CORES_SPEC) {
        return true;
    }

    // Even if we have too few cores... if the compositor is using async reprojection, pass
    auto system = acquireOpenVrSystem();
    auto compositor = vr::VRCompositor();
    if (system && compositor) {
        vr::Compositor_FrameTiming timing;
        memset(&timing, 0, sizeof(timing));
        timing.m_nSize = sizeof(vr::Compositor_FrameTiming);
        compositor->GetFrameTiming(&timing);
        releaseOpenVrSystem();
        if (timing.m_nReprojectionFlags & VRCompositor_ReprojectionAsync) {
            return true;
        }
    }

    // We're using OpenVR and we don't have enough cores...
    showMinSpecWarning();

    return false;
}

extern "C" {
#if defined(Q_OS_WIN32)
    __declspec(dllexport) int __stdcall CheckMinSpec() {
#else
    __attribute__((visibility("default"))) int CheckMinSpec() {
#endif
        return checkMinSpecImpl() ? 1 : 0;
    }

}
