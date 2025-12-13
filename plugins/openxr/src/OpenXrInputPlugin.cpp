//
// Overte OpenXR Plugin
//
// Copyright 2024 Lubosz Sarnecki
// Copyright 2024 Overte e.V.
//
// SPDX-License-Identifier: Apache-2.0
//

#include <glm/ext.hpp>
#include <QJsonArray>
#include <tuple>

#include "OpenXrInputPlugin.h"

#include "AvatarConstants.h"
#include "PathUtils.h"

#include "SettingHandle.h"
#include "controllers/UserInputMapper.h"

Q_DECLARE_LOGGING_CATEGORY(xr_input_cat)
Q_LOGGING_CATEGORY(xr_input_cat, "openxr.input")

enum ExtraButtonChannel {
    LEFT_HAS_PRIMARY = controller::StandardButtonChannel::NUM_STANDARD_BUTTONS,
    LEFT_HAS_TRACKPAD,
    LEFT_HAS_THUMBSTICK,
    LEFT_HAS_CAPACITIVE_TOUCH,

    RIGHT_HAS_PRIMARY,
    RIGHT_HAS_TRACKPAD,
    RIGHT_HAS_THUMBSTICK,
    RIGHT_HAS_CAPACITIVE_TOUCH,

    LEFT_TRACKPAD_TOUCH,
    RIGHT_TRACKPAD_TOUCH,
    LT_TOUCH,
    RT_TOUCH,
};

enum ExtraAxisChannel {
    LEFT_TRACKPAD_X = controller::StandardAxisChannel::NUM_STANDARD_AXES,
    LEFT_TRACKPAD_Y,
    LEFT_TRACKPAD_PRESS,
    RIGHT_TRACKPAD_X,
    RIGHT_TRACKPAD_Y,
    RIGHT_TRACKPAD_PRESS,
};

static const std::unordered_map<controller::StandardPoseChannel, QString> poseChannelToString = {
    {controller::HIPS, "Hips"},
    {controller::LEFT_LEG, "LeftLeg"},
    {controller::LEFT_FOOT, "LeftFoot"},
    {controller::RIGHT_LEG, "RightLeg"},
    {controller::RIGHT_FOOT, "RightFoot"},
    {controller::SPINE2, "Spine2"},
    {controller::HEAD, "Head"},
    {controller::RIGHT_ARM, "RightArm"},
    {controller::RIGHT_HAND, "RightHand"},
    {controller::LEFT_ARM, "LeftArm"},
    {controller::LEFT_HAND, "LeftHand"},
};

static const std::unordered_map<QString, controller::StandardPoseChannel> stringToPoseChannel = {
    {"Hips", controller::HIPS},
    {"LeftLeg", controller::LEFT_LEG},
    {"LeftFoot", controller::LEFT_FOOT},
    {"RightLeg", controller::RIGHT_LEG},
    {"RightFoot", controller::RIGHT_FOOT},
    {"Spine2", controller::SPINE2},
    {"Head", controller::HEAD},
    {"RightArm", controller::RIGHT_ARM},
    {"RightHand", controller::RIGHT_HAND},
    {"LeftArm", controller::LEFT_ARM},
    {"LeftHand", controller::LEFT_HAND},
};

OpenXrInputPlugin::OpenXrInputPlugin(std::shared_ptr<OpenXrContext> c) {
    _context = c;
    _inputDevice = std::make_shared<InputDevice>(_context);
}

static const QString XR_CONFIGURATION_LAYOUT = QString("OpenXrConfiguration.qml");

static glm::mat4 defaultPoseOffset(const controller::InputCalibrationData& data, controller::StandardPoseChannel channel) {
    switch (channel) {
        case controller::SPINE2:
            return data.defaultSpine2;

        case controller::HIPS:
            return data.defaultHips;

        case controller::LEFT_FOOT:
            return data.defaultLeftFoot;

        case controller::RIGHT_FOOT:
            return data.defaultRightFoot;

        default:
            return glm::mat4();
    }
}

void OpenXrInputPlugin::guessXDevRoles(std::unordered_map<XrXDevIdMNDX, XDevTracker>& tracker_map) {
    std::vector<std::tuple<XrXDevIdMNDX, glm::vec3, controller::Pose>> tracker_list;

    for (auto [id, tracker] : tracker_map) {
        XrResult result = XR_SUCCESS;
        XrSpaceLocation stageSpace = { XR_TYPE_SPACE_LOCATION };
        XrSpaceLocation localSpace = { XR_TYPE_SPACE_LOCATION };
        XrSpaceLocation headSpace = { XR_TYPE_SPACE_LOCATION };
        result = xrLocateSpace(tracker.space, _context->_stageSpace, _context->_lastPredictedDisplayTime.value(), &stageSpace);
        xrCheck(_context->_instance, result, "guessXDevRoles: tracker stage space fail");
        result = xrLocateSpace(tracker.space, _context->_viewSpace, _context->_lastPredictedDisplayTime.value(), &localSpace);
        xrCheck(_context->_instance, result, "guessXDevRoles: tracker local space fail");
        result = xrLocateSpace(_context->_viewSpace, _context->_stageSpace, _context->_lastPredictedDisplayTime.value(), &headSpace);
        xrCheck(_context->_instance, result, "guessXDevRoles: head space fail");

        // the tracker's position, relative horizontally to the headset
        // and vertically to the floor, normalized by the headset height
        // so we can check relative height rather than absolute meters
        auto position = xrVecToGlm(localSpace.pose.position);
        position.y = stageSpace.pose.position.y / headSpace.pose.position.y;
        tracker_list.push_back({
            id,
            position,
            controller::Pose(xrVecToGlm(stageSpace.pose.position), xrQuatToGlm(stageSpace.pose.orientation)),
        });
    }

    for (auto& tracker : tracker_list) {
        using namespace controller;

        auto position = std::get<1>(tracker);
        auto id = std::get<0>(tracker);
        auto& state = tracker_map[id];

        // TODO: our input system only really expects 7-point tracking,
        // (i.e. head, hands, chest, hips, and feet), but we can expand
        // it later to support more joints if there's demand for it

        if (position.y < 0.2f) {
            state.pose_channel = position.x < 0.0f ? LEFT_FOOT : RIGHT_FOOT;
        }

        if (position.y > 0.4f && position.y < 0.65f) {
            state.pose_channel = HIPS;
        }

        if (position.y > 0.65f && position.y < 0.9f) {
            state.pose_channel = SPINE2;
        }

        qCDebug(xr_input_cat) <<
            id <<
            ":" <<
            position.y <<
            (state.pose_channel.has_value() ? poseChannelToString.at(state.pose_channel.value()) : "None");
    }
}

void OpenXrInputPlugin::calibrate() {
    qCDebug(xr_input_cat) << "OpenXrInputPlugin::calibrate";

    if (_context->_MNDX_xdevSpaceSupported) {
        guessXDevRoles(_inputDevice->_xdev);
    }
    _inputDevice->_trackerCalibrations.clear();
    _inputDevice->_wantsCalibrate = true;
}

bool OpenXrInputPlugin::uncalibrate() {
    qCDebug(xr_input_cat) << "OpenXrInputPlugin::uncalibrate";

    for (auto [_, tracker] : _inputDevice->_xdev) {
        tracker.pose_channel = {};
    }

    _inputDevice->_trackerCalibrations.clear();
    _inputDevice->_wantsCalibrate = false;

    return true;
}

bool OpenXrInputPlugin::isSupported() const {
    return _context->_isValid && _context->_isSupported;
}

void OpenXrInputPlugin::setConfigurationSettings(const QJsonObject configurationSettings) {
    const auto& calibration = configurationSettings["tracker_calibration"].toObject();
    _inputDevice->_hapticsEnabled = configurationSettings["enable_haptics"].toBool(true);

    // grr qt5 doesnt support destructured iterating like std::map
    foreach (const auto& key, calibration.keys()) {
        const auto& value = calibration.value(key).toObject();

        // some kind of invalid channel name, don't read it
        if (!stringToPoseChannel.contains(key)) { continue; }

        // broken setting, don't read it
        if (!value.contains("translation") || !value.contains("rotation")) { continue; }

        auto translationArray = value.value("translation").toArray();
        auto rotationArray = value.value("rotation").toArray();

        auto channel = stringToPoseChannel.at(key);
        auto translation = vec3(translationArray[0].toDouble(), translationArray[1].toDouble(), translationArray[2].toDouble());
        auto rotation = quat(rotationArray[0].toDouble(), rotationArray[1].toDouble(), rotationArray[2].toDouble(), rotationArray[3].toDouble());

        _inputDevice->_trackerCalibrations[channel] = controller::Pose(translation, rotation);
    }
}

QJsonObject OpenXrInputPlugin::configurationSettings() {
    QJsonObject calibration;

    for (const auto& [channel, pose] : _inputDevice->_trackerCalibrations) {
        const auto& translation = pose.getTranslation();
        const auto& rotation = pose.getRotation();
        auto object = QJsonObject {
            {"translation", QJsonArray { translation.x, translation.y, translation.z }},
            {"rotation", QJsonArray { rotation.x, rotation.y, rotation.z, rotation.w }},
        };

        calibration[poseChannelToString.at(channel)] = object;
    }

    QJsonObject configurationSettings;
    configurationSettings["tracker_calibration"] = calibration;
    configurationSettings["enable_haptics"] = _inputDevice->_hapticsEnabled;
    return configurationSettings;
}

QString OpenXrInputPlugin::configurationLayout() {
    return XR_CONFIGURATION_LAYOUT;
}

bool OpenXrInputPlugin::activate() {
    if (!_context->_isValid) { return false; }

    InputPlugin::activate();

    loadSettings();

    // register with UserInputMapper
    auto userInputMapper = DependencyManager::get<controller::UserInputMapper>();
    userInputMapper->registerDevice(_inputDevice);
    _registeredWithInputMapper = true;

    return true;
}

void OpenXrInputPlugin::deactivate() {
    InputPlugin::deactivate();

    _inputDevice->_poseStateMap.clear();

    // unregister with UserInputMapper
    auto userInputMapper = DependencyManager::get<controller::UserInputMapper>();
    userInputMapper->removeDevice(_inputDevice->_deviceID);
    _registeredWithInputMapper = false;

    saveSettings();
}

void OpenXrInputPlugin::pluginUpdate(float deltaTime, const controller::InputCalibrationData& inputCalibrationData) {
    if (_context->_shouldQuit || !_context->_isValid) {
        deactivate();
        return;
    }

    auto userInputMapper = DependencyManager::get<controller::UserInputMapper>();

    if (_registeredWithInputMapper && !_context->_isDisplayActive) {
        userInputMapper->removeDevice(_inputDevice->_deviceID);
        _registeredWithInputMapper = false;
        _inputDevice->_poseStateMap.clear();
        return;
    } else if (!_registeredWithInputMapper && _context->_isDisplayActive) {
        userInputMapper->registerDevice(_inputDevice);
        _registeredWithInputMapper = true;
    }

    if (!_registeredWithInputMapper) { return; }

    userInputMapper->withLock([&, this]() { _inputDevice->update(deltaTime, inputCalibrationData); });

    if (_inputDevice->_trackedControllers == 0 && _registeredWithInputMapper) {
        userInputMapper->removeDevice(_inputDevice->_deviceID);
        _registeredWithInputMapper = false;
        _inputDevice->_poseStateMap.clear();
    }

    if (!_registeredWithInputMapper && _inputDevice->_trackedControllers > 0) {
        userInputMapper->registerDevice(_inputDevice);
        _registeredWithInputMapper = true;
    }
}

void OpenXrInputPlugin::loadSettings() {
    Settings settings;
    settings.beginGroup(getName());

    settings.endGroup();
}

void OpenXrInputPlugin::saveSettings() const {
    Settings settings;
    settings.beginGroup(getName());

    settings.endGroup();
}

OpenXrInputPlugin::InputDevice::InputDevice(std::shared_ptr<OpenXrContext> c) : controller::InputDevice("OpenXR") {
    _context = c;

    qCInfo(xr_input_cat) << "Hand tracking supported:" << _context->_handTrackingSupported;
}

void OpenXrInputPlugin::InputDevice::focusOutEvent() {
    _axisStateMap.clear();
    _buttonPressedMap.clear();
};

bool OpenXrInputPlugin::InputDevice::triggerHapticPulse(float strength, float duration, uint16_t index) {
    if (index > 2 || !_hapticsEnabled) {
        return false;
    }

    std::unique_lock<std::recursive_mutex> locker(_lock);

    using namespace std::chrono;
    nanoseconds durationNs = duration_cast<nanoseconds>(milliseconds(static_cast<int>(duration)));
    XrDuration xrDuration = durationNs.count();

    auto path = (index == 0) ? "left_haptic" : "right_haptic";

    // FIXME: sometimes something bugs out and hammers this,
    // and the controller vibrates really loudly until another
    // haptic pulse is triggered
    if (!_actions.at(path)->applyHaptic(xrDuration, 50, 0.5f * strength)) {
        qCCritical(xr_input_cat) << "Failed to apply haptic feedback!";
    }

    return true;
}

bool OpenXrInputPlugin::Action::init(XrActionSet actionSet) {
    XrInstance instance = _context->_instance;
    XrActionCreateInfo info = {
        .type = XR_TYPE_ACTION_CREATE_INFO,
        .actionType = _type,
        .countSubactionPaths = HAND_COUNT,
        .subactionPaths = _context->_handPaths,
    };

    strncpy(info.actionName, _id.c_str(), XR_MAX_ACTION_NAME_SIZE - 1);
    strncpy(info.localizedActionName, _friendlyName.c_str(), XR_MAX_LOCALIZED_ACTION_NAME_SIZE - 1);

    XrResult result = xrCreateAction(actionSet, &info, &_action);
    if (!xrCheck(instance, result, "Failed to create action"))
        return false;

    // Pose actions need spaces
    if (_type == XR_ACTION_TYPE_POSE_INPUT) {
        if (!createPoseSpaces()) {
            return false;
        }
    }

    return true;
}

std::vector<XrActionSuggestedBinding> OpenXrInputPlugin::Action::getBindings() {
    assert(_action != XR_NULL_HANDLE);

    std::vector<XrActionSuggestedBinding> bindings;
    for (uint32_t i = 0; i < HAND_COUNT; i++) {
        XrPath path;
        xrStringToPath(_context->_instance, _id.c_str(), &path);
        XrActionSuggestedBinding binding = { .action = _action, .binding = path };
        bindings.push_back(binding);
    }
    return bindings;
}

XrActionStateFloat OpenXrInputPlugin::Action::getFloat() {
    XrActionStateFloat state = {
        .type = XR_TYPE_ACTION_STATE_FLOAT,
    };

    XrActionStateGetInfo info = {
        .type = XR_TYPE_ACTION_STATE_GET_INFO,
        .action = _action,
    };

    XrResult result = xrGetActionStateFloat(_context->_session, &info, &state);
    xrCheck(_context->_instance, result, "Failed to get float state!");

    return state;
}

XrActionStateVector2f OpenXrInputPlugin::Action::getVector2f() {
    XrActionStateVector2f state = {
        .type = XR_TYPE_ACTION_STATE_VECTOR2F,
    };

    XrActionStateGetInfo info = {
        .type = XR_TYPE_ACTION_STATE_GET_INFO,
        .action = _action,
    };

    XrResult result = xrGetActionStateVector2f(_context->_session, &info, &state);
    xrCheck(_context->_instance, result, "Failed to get vector2 state!");

    return state;
}

XrActionStateBoolean OpenXrInputPlugin::Action::getBool() {
    XrActionStateBoolean state = {
        .type = XR_TYPE_ACTION_STATE_BOOLEAN,
    };

    XrActionStateGetInfo info = {
        .type = XR_TYPE_ACTION_STATE_GET_INFO,
        .action = _action,
    };

    XrResult result = xrGetActionStateBoolean(_context->_session, &info, &state);
    xrCheck(_context->_instance, result, "Failed to get float state!");

    return state;
}

XrSpaceLocation OpenXrInputPlugin::Action::getPose() {
    XrActionStatePose state = {
        .type = XR_TYPE_ACTION_STATE_POSE,
    };
    XrActionStateGetInfo info = {
        .type = XR_TYPE_ACTION_STATE_GET_INFO,
        .action = _action,
    };

    XrResult result = xrGetActionStatePose(_context->_session, &info, &state);
    xrCheck(_context->_instance, result, "failed to get pose value!");

    XrSpaceLocation location = {
        .type = XR_TYPE_SPACE_LOCATION,
    };

    if (_context->_lastPredictedDisplayTime.has_value()) {
        result = xrLocateSpace(_poseSpace, _context->_stageSpace, _context->_lastPredictedDisplayTime.value(), &location);
        xrCheck(_context->_instance, result, "Failed to locate hand space!");
    }

    return location;
}

bool OpenXrInputPlugin::Action::isPoseActive() {
    XrActionStatePose state = {
        .type = XR_TYPE_ACTION_STATE_POSE,
    };
    XrActionStateGetInfo info = {
        .type = XR_TYPE_ACTION_STATE_GET_INFO,
        .action = _action,
    };

    XrResult result = xrGetActionStatePose(_context->_session, &info, &state);
    xrCheck(_context->_instance, result, "failed to get pose value!");

    return state.isActive;
}

bool OpenXrInputPlugin::Action::applyHaptic(XrDuration duration, float frequency, float amplitude) {
    XrHapticVibration vibration = {
        .type = XR_TYPE_HAPTIC_VIBRATION,
        .duration = duration,
        .frequency = frequency,
        .amplitude = amplitude,
    };

    XrHapticActionInfo haptic_action_info = {
        .type = XR_TYPE_HAPTIC_ACTION_INFO,
        .action = _action,
    };
    XrResult result = xrApplyHapticFeedback(_context->_session, &haptic_action_info, (const XrHapticBaseHeader*)&vibration);

    return xrCheck(_context->_instance, result, "Failed to apply haptic feedback!");
}

bool OpenXrInputPlugin::Action::createPoseSpaces() {
    assert(_action != XR_NULL_HANDLE);

    XrActionSpaceCreateInfo info = {
        .type = XR_TYPE_ACTION_SPACE_CREATE_INFO,
        .action = _action,
        .poseInActionSpace = XR_INDENTITY_POSE,
    };

    XrResult result = xrCreateActionSpace(_context->_session, &info, &_poseSpace);
    if (!xrCheck(_context->_instance, result, "Failed to create hand pose space"))
        return false;

    return true;
}

bool OpenXrInputPlugin::InputDevice::initBindings(const std::string& profileName,
                                                  const std::map<std::string, std::string>& actionsToBind) {
    XrPath profilePath;
    XrResult result = xrStringToPath(_context->_instance, profileName.c_str(), &profilePath);
    if (!xrCheck(_context->_instance, result, "Failed to get interaction profile"))
        return false;

    std::vector<XrActionSuggestedBinding> suggestions;
    for (const auto& [actionName, inputPathRaw] : actionsToBind) {
        // don't set up the palm poses if they're not supported
        if (
            (actionName == "left_palm_pose" || actionName == "right_palm_pose") &&
            !_context->_palmPoseSupported
        ) {
            continue;
        }

        if (!_actions.contains(actionName)) {
            qCCritical(xr_input_cat) << "Unknown action name" << actionName.c_str();
            return false;
        }

        XrActionSuggestedBinding bind = {
            .action = _actions[actionName]->_action,
        };
        xrStringToPath(_context->_instance, inputPathRaw.c_str(), &bind.binding);
        suggestions.emplace(suggestions.end(), bind);
    }

    const XrInteractionProfileSuggestedBinding suggestedBinding = {
        .type = XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING,
        .interactionProfile = profilePath,
        .countSuggestedBindings = (uint32_t)suggestions.size(),
        .suggestedBindings = suggestions.data(),
    };

    result = xrSuggestInteractionProfileBindings(_context->_instance, &suggestedBinding);

    return xrCheck(_context->_instance, result, "Failed to suggest bindings");
}

controller::Input::NamedVector OpenXrInputPlugin::InputDevice::getAvailableInputs() const {
    using namespace controller;

    QVector<Input::NamedPair> availableInputs = {
        // Controller metainfo
        // TODO: Is there an existing "meta" system or are things like
        // Application.InHMD just virtual buttons too?
        makePair((StandardButtonChannel)LEFT_HAS_PRIMARY, "LeftHasPrimary"),
        makePair((StandardButtonChannel)LEFT_HAS_TRACKPAD, "LeftHasTrackpad"),
        makePair((StandardButtonChannel)LEFT_HAS_THUMBSTICK, "LeftHasThumbstick"),
        makePair((StandardButtonChannel)LEFT_HAS_CAPACITIVE_TOUCH, "LeftHasCapacitiveTouch"),

        makePair((StandardButtonChannel)RIGHT_HAS_PRIMARY, "RightHasPrimary"),
        makePair((StandardButtonChannel)RIGHT_HAS_TRACKPAD, "RightHasTrackpad"),
        makePair((StandardButtonChannel)RIGHT_HAS_THUMBSTICK, "RightHasThumbstick"),
        makePair((StandardButtonChannel)RIGHT_HAS_CAPACITIVE_TOUCH, "RightHasCapacitiveTouch"),

        makePair(HEAD, "Head"),

        makePair(LEFT_HAND, "LeftHand"),
        makePair(LS, "LS"),
        makePair(LS_TOUCH, "LSTouch"),
        makePair(LX, "LX"),
        makePair(LY, "LY"),
        makePair(LT, "LT"),
        makePair(LT_CLICK, "LTClick"),
        makePair((StandardButtonChannel)LT_TOUCH, "LTTouch"),
        makePair(LEFT_GRIP, "LeftGrip"),
        makePair(LEFT_PRIMARY_THUMB, "LeftPrimary"),
        makePair(LEFT_SECONDARY_THUMB, "LeftSecondary"),
        makePair(LEFT_PRIMARY_THUMB_TOUCH, "LeftPrimaryTouch"),
        makePair(LEFT_SECONDARY_THUMB_TOUCH, "LeftSecondaryTouch"),
        makePair((StandardButtonChannel)LEFT_TRACKPAD_TOUCH, "LeftTrackpadTouch"),
        makePair((StandardAxisChannel)LEFT_TRACKPAD_PRESS, "LeftTrackpadClick"),
        makePair((StandardAxisChannel)LEFT_TRACKPAD_X, "LeftTrackpadX"),
        makePair((StandardAxisChannel)LEFT_TRACKPAD_Y, "LeftTrackpadY"),
        makePair(LEFT_INDEX_POINT, "LeftIndexPoint"),
        makePair(LEFT_THUMB_UP, "LeftThumbUp"),

        makePair(RIGHT_HAND, "RightHand"),
        makePair(RS, "RS"),
        makePair(RS_TOUCH, "RSTouch"),
        makePair(RX, "RX"),
        makePair(RY, "RY"),
        makePair(RT, "RT"),
        makePair(RT_CLICK, "RTClick"),
        makePair((StandardButtonChannel)RT_TOUCH, "RTTouch"),
        makePair(RIGHT_GRIP, "RightGrip"),
        makePair(RIGHT_PRIMARY_THUMB, "RightPrimary"),
        makePair(RIGHT_SECONDARY_THUMB, "RightSecondary"),
        makePair(RIGHT_PRIMARY_THUMB_TOUCH, "RightPrimaryTouch"),
        makePair(RIGHT_SECONDARY_THUMB_TOUCH, "RightSecondaryTouch"),
        makePair((StandardButtonChannel)RIGHT_TRACKPAD_TOUCH, "RightTrackpadTouch"),
        makePair((StandardAxisChannel)RIGHT_TRACKPAD_PRESS, "RightTrackpadClick"),
        makePair((StandardAxisChannel)RIGHT_TRACKPAD_X, "RightTrackpadX"),
        makePair((StandardAxisChannel)RIGHT_TRACKPAD_Y, "RightTrackpadY"),
        makePair(RIGHT_INDEX_POINT, "RightIndexPoint"),
        makePair(RIGHT_THUMB_UP, "RightThumbUp"),

        // hand tracking
        makePair(LEFT_HAND_THUMB1, "LeftHandThumb1"),
        makePair(LEFT_HAND_THUMB2, "LeftHandThumb2"),
        makePair(LEFT_HAND_THUMB3, "LeftHandThumb3"),
        makePair(LEFT_HAND_THUMB4, "LeftHandThumb4"),
        makePair(LEFT_HAND_INDEX1, "LeftHandIndex1"),
        makePair(LEFT_HAND_INDEX2, "LeftHandIndex2"),
        makePair(LEFT_HAND_INDEX3, "LeftHandIndex3"),
        makePair(LEFT_HAND_INDEX4, "LeftHandIndex4"),
        makePair(LEFT_HAND_MIDDLE1, "LeftHandMiddle1"),
        makePair(LEFT_HAND_MIDDLE2, "LeftHandMiddle2"),
        makePair(LEFT_HAND_MIDDLE3, "LeftHandMiddle3"),
        makePair(LEFT_HAND_MIDDLE4, "LeftHandMiddle4"),
        makePair(LEFT_HAND_RING1, "LeftHandRing1"),
        makePair(LEFT_HAND_RING2, "LeftHandRing2"),
        makePair(LEFT_HAND_RING3, "LeftHandRing3"),
        makePair(LEFT_HAND_RING4, "LeftHandRing4"),
        makePair(LEFT_HAND_PINKY1, "LeftHandPinky1"),
        makePair(LEFT_HAND_PINKY2, "LeftHandPinky2"),
        makePair(LEFT_HAND_PINKY3, "LeftHandPinky3"),
        makePair(LEFT_HAND_PINKY4, "LeftHandPinky4"),

        makePair(RIGHT_HAND_THUMB1, "RightHandThumb1"),
        makePair(RIGHT_HAND_THUMB2, "RightHandThumb2"),
        makePair(RIGHT_HAND_THUMB3, "RightHandThumb3"),
        makePair(RIGHT_HAND_THUMB4, "RightHandThumb4"),
        makePair(RIGHT_HAND_INDEX1, "RightHandIndex1"),
        makePair(RIGHT_HAND_INDEX2, "RightHandIndex2"),
        makePair(RIGHT_HAND_INDEX3, "RightHandIndex3"),
        makePair(RIGHT_HAND_INDEX4, "RightHandIndex4"),
        makePair(RIGHT_HAND_MIDDLE1, "RightHandMiddle1"),
        makePair(RIGHT_HAND_MIDDLE2, "RightHandMiddle2"),
        makePair(RIGHT_HAND_MIDDLE3, "RightHandMiddle3"),
        makePair(RIGHT_HAND_MIDDLE4, "RightHandMiddle4"),
        makePair(RIGHT_HAND_RING1, "RightHandRing1"),
        makePair(RIGHT_HAND_RING2, "RightHandRing2"),
        makePair(RIGHT_HAND_RING3, "RightHandRing3"),
        makePair(RIGHT_HAND_RING4, "RightHandRing4"),
        makePair(RIGHT_HAND_PINKY1, "RightHandPinky1"),
        makePair(RIGHT_HAND_PINKY2, "RightHandPinky2"),
        makePair(RIGHT_HAND_PINKY3, "RightHandPinky3"),
        makePair(RIGHT_HAND_PINKY4, "RightHandPinky4"),

        // body tracking
        makePair(LEFT_FOOT, "LeftFoot"),
        makePair(RIGHT_FOOT, "RightFoot"),
        makePair(HIPS, "Hips"),
        makePair(SPINE2, "Spine2"),
    };

    return availableInputs;
}

QString OpenXrInputPlugin::InputDevice::getDefaultMappingConfig() const {
    return PathUtils::resourcesPath() + "/controllers/openxr.json";
}

bool OpenXrInputPlugin::InputDevice::initActions() {
    if (_actionsInitialized)
        return true;

    assert(_context->_session != XR_NULL_HANDLE);

    XrInstance instance = _context->_instance;

    XrActionSetCreateInfo actionSetInfo = {
        .type = XR_TYPE_ACTION_SET_CREATE_INFO,
        .actionSetName = "overte_virtual_controller",
        .localizedActionSetName = "Overte Virtual Controller",
        .priority = 0,
    };
    XrResult result = xrCreateActionSet(instance, &actionSetInfo, &_actionSet);
    if (!xrCheck(instance, result, "Failed to create action set."))
        return false;

    std::map<std::string, std::pair<std::string, XrActionType>> actionTypes = {
        {"left_primary_click",     {"Left Primary", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"left_primary_touch",     {"Left Primary Touch", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"left_secondary_click",   {"Left Secondary", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"left_secondary_touch",   {"Left Secondary Touch", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"left_squeeze_value",     {"Left Squeeze", XR_ACTION_TYPE_FLOAT_INPUT}},
        {"left_trigger_value",     {"Left Trigger", XR_ACTION_TYPE_FLOAT_INPUT}},
        {"left_trigger_touch",     {"Left Trigger Touch", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"left_thumbstick",        {"Left Thumbstick", XR_ACTION_TYPE_VECTOR2F_INPUT}},
        {"left_thumbstick_click",  {"Left Thumbstick Click", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"left_thumbstick_touch",  {"Left Thumbstick Touch", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"left_trackpad",          {"Left Trackpad", XR_ACTION_TYPE_VECTOR2F_INPUT}},
        {"left_trackpad_touch",    {"Left Trackpad Touch", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"left_trackpad_click",    {"Left Trackpad Click", XR_ACTION_TYPE_FLOAT_INPUT}},
        {"left_grip_pose",         {"Left Grip Pose", XR_ACTION_TYPE_POSE_INPUT}},
        {"left_palm_pose",         {"Left Palm Pose", XR_ACTION_TYPE_POSE_INPUT}},
        {"left_haptic",            {"Left Hand Haptic", XR_ACTION_TYPE_VIBRATION_OUTPUT}},

        {"right_primary_click",    {"Right Primary", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"right_primary_touch",    {"Right Primary Touch", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"right_secondary_click",  {"Right Secondary", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"right_secondary_touch",  {"Right Secondary Touch", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"right_squeeze_value",    {"Right Squeeze", XR_ACTION_TYPE_FLOAT_INPUT}},
        {"right_trigger_value",    {"Right Trigger", XR_ACTION_TYPE_FLOAT_INPUT}},
        {"right_trigger_touch",    {"Right Trigger Touch", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"right_thumbstick",       {"Right Thumbstick", XR_ACTION_TYPE_VECTOR2F_INPUT}},
        {"right_thumbstick_click", {"Right Thumbstick Click", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"right_thumbstick_touch", {"Right Thumbstick Touch", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"right_trackpad",         {"Right Trackpad", XR_ACTION_TYPE_VECTOR2F_INPUT}},
        {"right_trackpad_touch",   {"Right Trackpad Touch", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"right_trackpad_click",   {"Right Trackpad Click", XR_ACTION_TYPE_FLOAT_INPUT}},
        {"right_grip_pose",        {"Right Grip Pose", XR_ACTION_TYPE_POSE_INPUT}},
        {"right_palm_pose",        {"Right Palm Pose", XR_ACTION_TYPE_POSE_INPUT}},
        {"right_haptic",           {"Right Hand Haptic", XR_ACTION_TYPE_VIBRATION_OUTPUT}},

        {"hips_pose",              {"Hips Pose", XR_ACTION_TYPE_POSE_INPUT}},
        {"chest_pose",             {"Chest Pose", XR_ACTION_TYPE_POSE_INPUT}},
        {"left_foot_pose",         {"Left Foot Pose", XR_ACTION_TYPE_POSE_INPUT}},
        {"right_foot_pose",        {"Right Foot Pose", XR_ACTION_TYPE_POSE_INPUT}},
    };

    std::string hand_left = "/user/hand/left/input";
    std::string hand_right = "/user/hand/right/input";

    std::map<std::string, std::map<std::string, std::string>> actionSuggestions = {
        // not really usable, bare minimum
        {"/interaction_profiles/khr/simple_controller", {
            {"left_secondary_click",   hand_left  + "/menu/click"},
            {"left_trigger_value",     hand_left  + "/select/click"},
            {"left_grip_pose",         hand_left  + "/grip/pose"},
            {"left_palm_pose",         hand_left  + "/palm_ext/pose"},
            {"left_haptic",            "/user/hand/left/output/haptic"},

            {"right_secondary_click",  hand_right + "/menu/click"},
            {"right_trigger_value",    hand_right + "/select/click"},
            {"right_grip_pose",        hand_right + "/grip/pose"},
            {"right_palm_pose",        hand_right + "/palm_ext/pose"},
            {"right_haptic",           "/user/hand/right/output/haptic"},
        }},
        {"/interaction_profiles/htc/vive_controller", {
            {"left_secondary_click",   hand_left  + "/menu/click"},
            {"left_squeeze_value",     hand_left  + "/squeeze/click"},
            {"left_trigger_value",     hand_left  + "/trigger/value"},
            {"left_trackpad",          hand_left  + "/trackpad"},
            {"left_trackpad_click",    hand_left  + "/trackpad/click"},
            {"left_trackpad_touch",    hand_left  + "/trackpad/touch"},
            {"left_grip_pose",         hand_left  + "/grip/pose"},
            {"left_palm_pose",         hand_left  + "/palm_ext/pose"},
            {"left_haptic",            "/user/hand/left/output/haptic"},

            {"right_secondary_click",  hand_right + "/menu/click"},
            {"right_squeeze_value",    hand_right + "/squeeze/click"},
            {"right_trigger_value",    hand_right + "/trigger/value"},
            {"right_trackpad",         hand_right + "/trackpad"},
            {"right_trackpad_click",   hand_right + "/trackpad/click"},
            {"right_trackpad_touch",   hand_right + "/trackpad/touch"},
            {"right_grip_pose",        hand_right + "/grip/pose"},
            {"right_palm_pose",        hand_right + "/palm_ext/pose"},
            {"right_haptic",           "/user/hand/right/output/haptic"},
        }},
        {"/interaction_profiles/oculus/touch_controller", {
            {"left_primary_click",     hand_left  + "/x/click"},
            {"left_primary_touch",     hand_left  + "/x/touch"},
            {"left_secondary_click",   hand_left  + "/y/click"},
            {"left_secondary_touch",   hand_left  + "/y/touch"},
            {"left_squeeze_value",     hand_left  + "/squeeze/value"},
            {"left_trigger_value",     hand_left  + "/trigger/value"},
            {"left_trigger_touch",     hand_left  + "/trigger/touch"},
            {"left_thumbstick",        hand_left  + "/thumbstick"},
            {"left_thumbstick_click",  hand_left  + "/thumbstick/click"},
            {"left_thumbstick_touch",  hand_left  + "/thumbstick/touch"},
            {"left_grip_pose",         hand_left  + "/grip/pose"},
            {"left_palm_pose",         hand_left  + "/palm_ext/pose"},
            {"left_haptic",            "/user/hand/left/output/haptic"},

            {"right_primary_click",    hand_right + "/a/click"},
            {"right_primary_touch",    hand_right + "/a/touch"},
            {"right_secondary_click",  hand_right + "/b/click"},
            {"right_secondary_touch",  hand_right + "/b/touch"},
            {"right_squeeze_value",    hand_right + "/squeeze/value"},
            {"right_trigger_value",    hand_right + "/trigger/value"},
            {"right_trigger_touch",    hand_right + "/trigger/touch"},
            {"right_thumbstick",       hand_right + "/thumbstick"},
            {"right_thumbstick_click", hand_right + "/thumbstick/click"},
            {"right_thumbstick_touch", hand_right + "/thumbstick/touch"},
            {"right_grip_pose",        hand_right + "/grip/pose"},
            {"right_palm_pose",        hand_right + "/palm_ext/pose"},
            {"right_haptic",           "/user/hand/right/output/haptic"},
        }},
        {"/interaction_profiles/microsoft/motion_controller", {
            {"left_secondary_click",   hand_left  + "/menu/click"},
            {"left_squeeze_value",     hand_left  + "/squeeze/click"},
            {"left_trigger_value",     hand_left  + "/trigger/value"},
            {"left_thumbstick",        hand_left  + "/thumbstick"},
            {"left_thumbstick_click",  hand_left  + "/trackpad/click"},
            {"left_trackpad",          hand_left  + "/trackpad"},
            {"left_trackpad_click",    hand_left  + "/trackpad/click"},
            {"left_trackpad_touch",    hand_left  + "/trackpad/touch"},
            {"left_grip_pose",         hand_left  + "/grip/pose"},
            {"left_palm_pose",         hand_left  + "/palm_ext/pose"},
            {"left_haptic",            "/user/hand/left/output/haptic"},

            {"right_secondary_click",  hand_right + "/menu/click"},
            {"right_squeeze_value",    hand_right + "/squeeze/click"},
            {"right_trigger_value",    hand_right + "/trigger/value"},
            {"right_thumbstick",       hand_right + "/thumbstick"},
            {"right_thumbstick_click", hand_right + "/trackpad/click"},
            {"right_trackpad",         hand_right + "/trackpad"},
            {"right_trackpad_click",   hand_right + "/trackpad/click"},
            {"right_trackpad_touch",   hand_right + "/trackpad/touch"},
            {"right_grip_pose",        hand_right + "/grip/pose"},
            {"right_palm_pose",        hand_right + "/palm_ext/pose"},
            {"right_haptic",           "/user/hand/right/output/haptic"},
        }},
        {"/interaction_profiles/samsung/odyssey_controller", {
            {"left_secondary_click",   hand_left  + "/menu/click"},
            {"left_squeeze_value",     hand_left  + "/squeeze/click"},
            {"left_trigger_value",     hand_left  + "/trigger/value"},
            {"left_thumbstick",        hand_left  + "/thumbstick"},
            {"left_thumbstick_click",  hand_left  + "/trackpad/click"},
            {"left_trackpad",          hand_left  + "/trackpad"},
            {"left_trackpad_click",    hand_left  + "/trackpad/click"},
            {"left_trackpad_touch",    hand_left  + "/trackpad/touch"},
            {"left_grip_pose",         hand_left  + "/grip/pose"},
            {"left_palm_pose",         hand_left  + "/palm_ext/pose"},
            {"left_haptic",            "/user/hand/left/output/haptic"},

            {"right_secondary_click",  hand_right + "/menu/click"},
            {"right_squeeze_value",    hand_right + "/squeeze/click"},
            {"right_trigger_value",    hand_right + "/trigger/value"},
            {"right_thumbstick",       hand_right + "/thumbstick"},
            {"right_thumbstick_click", hand_right + "/trackpad/click"},
            {"right_trackpad",         hand_right + "/trackpad"},
            {"right_trackpad_click",   hand_right + "/trackpad/click"},
            {"right_trackpad_touch",   hand_right + "/trackpad/touch"},
            {"right_grip_pose",        hand_right + "/grip/pose"},
            {"right_palm_pose",        hand_right + "/palm_ext/pose"},
            {"right_haptic",           "/user/hand/right/output/haptic"},
        }},
        {"/interaction_profiles/valve/index_controller", {
            {"left_primary_click",     hand_left  + "/a/click"},
            {"left_primary_touch",     hand_left  + "/a/touch"},
            {"left_secondary_click",   hand_left  + "/b/click"},
            {"left_secondary_touch",   hand_left  + "/b/touch"},
            {"left_squeeze_value",     hand_left  + "/squeeze/force"},
            {"left_trigger_value",     hand_left  + "/trigger/value"},
            {"left_trigger_touch",     hand_left  + "/trigger/touch"},
            {"left_thumbstick",        hand_left  + "/thumbstick"},
            {"left_thumbstick_click",  hand_left  + "/thumbstick/click"},
            {"left_thumbstick_touch",  hand_left  + "/thumbstick/touch"},
            {"left_trackpad",          hand_left  + "/trackpad"},
            {"left_trackpad_click",    hand_left  + "/trackpad/force"},
            {"left_trackpad_touch",    hand_left  + "/trackpad/touch"},
            {"left_grip_pose",         hand_left  + "/grip/pose"},
            {"left_palm_pose",         hand_left  + "/palm_ext/pose"},
            {"left_haptic",            "/user/hand/left/output/haptic"},

            {"right_primary_click",    hand_right + "/a/click"},
            {"right_primary_touch",    hand_right + "/a/touch"},
            {"right_secondary_click",  hand_right + "/b/click"},
            {"right_secondary_touch",  hand_right + "/b/touch"},
            {"right_squeeze_value",    hand_right + "/squeeze/force"},
            {"right_trigger_value",    hand_right + "/trigger/value"},
            {"right_trigger_touch",    hand_right + "/trigger/touch"},
            {"right_thumbstick",       hand_right + "/thumbstick"},
            {"right_thumbstick_click", hand_right + "/thumbstick/click"},
            {"right_thumbstick_touch", hand_right + "/thumbstick/touch"},
            {"right_trackpad",         hand_right + "/trackpad"},
            {"right_trackpad_click",   hand_right + "/trackpad/force"},
            {"right_trackpad_touch",   hand_right + "/trackpad/touch"},
            {"right_grip_pose",        hand_right + "/grip/pose"},
            {"right_palm_pose",        hand_right + "/palm_ext/pose"},
            {"right_haptic",           "/user/hand/right/output/haptic"},
        }},
    };

    if (_context->_HTCX_viveTrackerInteractionSupported) {
        actionSuggestions.insert(
            {"/interaction_profiles/htc/vive_tracker_htcx", {
                {"hips_pose",       "/usr/vive_tracker_htcx/role/waist/input/grip/pose"},
                {"chest_pose",      "/usr/vive_tracker_htcx/role/chest/input/grip/pose"},
                {"left_foot_pose",  "/usr/vive_tracker_htcx/role/left_foot/input/grip/pose"},
                {"right_foot_pose", "/usr/vive_tracker_htcx/role/right_foot/input/grip/pose"},
            }}
        );
    }

    for (const auto& [id, args] : actionTypes) {
        auto friendlyName = args.first;
        auto xr_type = args.second;
        std::shared_ptr<Action> action = std::make_shared<Action>(_context, id, friendlyName, xr_type);
        if (!action->init(_actionSet)) {
            qCCritical(xr_input_cat) << "Creating action " << id.c_str() << " failed!";
        } else {
            _actions.emplace(id, action);
        }
    }

    for (const auto& [profile, input] : actionSuggestions) {
        if (!initBindings(profile, input)) {
            qCWarning(xr_input_cat) << "Failed to suggest actions for " << profile.c_str();
        }
    }

    XrSessionActionSetsAttachInfo attachInfo = {
        .type = XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO,
        .countActionSets = 1,
        .actionSets = &_actionSet,
    };
    result = xrAttachSessionActionSets(_context->_session, &attachInfo);
    if (!xrCheck(_context->_instance, result, "Failed to attach action set"))
        return false;

    if (_context->_handTrackingSupported) {
        XrHandTrackerCreateInfoEXT createInfo = {
            .type = XR_TYPE_HAND_TRACKER_CREATE_INFO_EXT,
            .next = nullptr,
            .handJointSet = XR_HAND_JOINT_SET_DEFAULT_EXT,
        };

        createInfo.hand = XR_HAND_LEFT_EXT;
        xrCheck(_context->_instance, _context->xrCreateHandTrackerEXT(_context->_session, &createInfo, &_handTracker[0]), "Failed to create left hand tracker");

        createInfo.hand = XR_HAND_RIGHT_EXT;
        xrCheck(_context->_instance, _context->xrCreateHandTrackerEXT(_context->_session, &createInfo, &_handTracker[1]), "Failed to create right hand tracker");
    }

    if (_context->_MNDX_xdevSpaceSupported) {
        _xdev.clear();

        XrXDevListMNDX xdevList;
        std::vector<XrXDevIdMNDX> xdevIDs(MAX_TRACKER_COUNT);
        uint32_t xdevIDsCount = 0;

        XrCreateXDevListInfoMNDX createInfo = {.type = XR_TYPE_CREATE_XDEV_LIST_INFO_MNDX};

        _context->xrCreateXDevListMNDX(_context->_session, &createInfo, &xdevList);
        _context->xrEnumerateXDevsMNDX(xdevList, MAX_TRACKER_COUNT, &xdevIDsCount, xdevIDs.data());

        // shrink the list to the number of xdevs we actually received
        xdevIDs.resize(xdevIDsCount);

        for (const auto id : xdevIDs) {
            XrGetXDevInfoMNDX info = {.type = XR_TYPE_GET_XDEV_INFO_MNDX};
            XrXDevPropertiesMNDX properties = {.type = XR_TYPE_XDEV_PROPERTIES_MNDX};

            info.id = id;
            _context->xrGetXDevPropertiesMNDX(xdevList, &info, &properties);

            qCDebug(xr_input_cat, "XDev %lx \"%s\"", id, properties.name);

            if (std::string(properties.name).find("Tracker") == std::string::npos) {
                // it's probably a headset or a controller, discard
                continue;
            }

            XDevTracker tracker;
            tracker.properties = properties;

            tracker.pose_channel = {};

            XrCreateXDevSpaceInfoMNDX createSpaceInfo = {
                .type = XR_TYPE_CREATE_XDEV_SPACE_INFO_MNDX,
                .next = nullptr,
                .xdevList = xdevList,
                .id = id,
                .offset = { {0, 0, 0, 1}, {} },
            };

            _context->xrCreateXDevSpaceMNDX(_context->_session, &createSpaceInfo, &tracker.space);

            _xdev.insert({id, tracker});
        }
    }

    _actionsInitialized = true;

    return true;
}

void OpenXrInputPlugin::InputDevice::update(float deltaTime, const controller::InputCalibrationData& inputCalibrationData) {
    using namespace controller;

    _poseStateMap.clear();
    _buttonPressedMap.clear();
    _trackedControllers = 2;

    if (_context->_session == XR_NULL_HANDLE) {
        return;
    }

    if (!_actionsInitialized && !initActions()) {
        qCCritical(xr_input_cat) << "Could not initialize actions!";
        return;
    }

    const XrActiveActionSet active_actionset = {
        .actionSet = _actionSet,
    };

    XrActionsSyncInfo syncInfo = {
        .type = XR_TYPE_ACTIONS_SYNC_INFO,
        .countActiveActionSets = 1,
        .activeActionSets = &active_actionset,
    };

    XrInstance instance = _context->_instance;
    XrSession session = _context->_session;

    XrResult result = xrSyncActions(session, &syncInfo);
    xrCheck(instance, result, "failed to sync actions!");

    glm::mat4 sensorToAvatar = glm::inverse(inputCalibrationData.avatarMat) * inputCalibrationData.sensorToWorldMat;

    static const glm::quat yFlip = glm::angleAxis(PI, Vectors::UNIT_Y);
    static const glm::quat quarterX = glm::angleAxis(PI_OVER_TWO, Vectors::UNIT_X);
    static const glm::quat touchToHand = yFlip * quarterX;

    static const glm::quat leftQuarterZ = glm::angleAxis(-PI_OVER_TWO, Vectors::UNIT_Z);
    static const glm::quat rightQuarterZ = glm::angleAxis(PI_OVER_TWO, Vectors::UNIT_Z);
    static const glm::quat eighthX = glm::angleAxis(PI / 4.0f, Vectors::UNIT_X);

    static const glm::quat leftRotationOffset = glm::inverse(leftQuarterZ * eighthX) * touchToHand;
    static const glm::quat rightRotationOffset = glm::inverse(rightQuarterZ * eighthX) * touchToHand;

    for (int i = 0; i < HAND_COUNT; i++) {
        auto palm_path = (i == 0) ? "left_palm_pose" : "right_palm_pose";
        auto grip_path = (i == 0) ? "left_grip_pose" : "right_grip_pose";

        bool usingPalm = false;
        XrSpaceLocation handLocation;

        // use the palm pose if it's supported
        if (_context->_palmPoseSupported && _actions.at(palm_path)->isPoseActive()) {
            handLocation = _actions.at(palm_path)->getPose();
            usingPalm = true;
        } else {
            handLocation = _actions.at(grip_path)->getPose();
        }

        bool locationValid = (handLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0;
        if (locationValid) {
            vec3 translation = xrVecToGlm(handLocation.pose.position);
            quat rotation = xrQuatToGlm(handLocation.pose.orientation);
            auto pose = Pose(translation, rotation);
            glm::mat4 handOffset = i == 0 ? glm::toMat4(leftRotationOffset) : glm::toMat4(rightRotationOffset);

            glm::mat4 posOffset(1.0f);

            if (usingPalm) {
                posOffset *= glm::translate(Vectors::UNIT_Z * 0.06f);
                posOffset *= glm::rotate(PI / 4.0f, Vectors::UNIT_X);
            } else {
                // Vive controllers have bugged poses that aren't in the grip or aim position,
                // they're always at the top near the tracking ring. This is how all runtimes
                // that currently support them behave, and it's probably here to stay.
                if (_context->_vivePoseHack[i]) {
                    posOffset *= glm::translate(glm::vec3(handOffset[0]) * (i == 0 ? 0.1f : -0.1f));
                    posOffset *= glm::translate(glm::vec3(handOffset[1]) * -0.16f);
                    posOffset *= glm::translate(glm::vec3(handOffset[2]) * -0.02f);
                } else {
                    posOffset *= glm::translate(glm::vec3(handOffset[0]) * (i == 0 ? -0.08f : 0.08f));
                    posOffset *= glm::translate(glm::vec3(handOffset[1]) * -0.10f);
                    posOffset *= glm::translate(glm::vec3(handOffset[2]) * -0.01f);
                    posOffset *= glm::rotate(PI / -8.0f, Vectors::UNIT_X);
                }
            }

            _poseStateMap[i == 0 ? LEFT_HAND : RIGHT_HAND] =
                pose.postTransform(posOffset).postTransform(handOffset).transform(sensorToAvatar);
        }
    }

    glm::mat4 defaultHeadOffset;
    float eyeZOffset = 0.16f;
    if (inputCalibrationData.hmdAvatarAlignmentType == HmdAvatarAlignmentType::Eyes) {
        // align the eyes of the user with the eyes of the avatar
        defaultHeadOffset = Matrices::Y_180 * (glm::inverse(inputCalibrationData.defaultCenterEyeMat) * inputCalibrationData.defaultHeadMat);

        // don't double up on eye offset
        eyeZOffset = 0.0f;
    } else {
        defaultHeadOffset = createMatFromQuatAndPos(-DEFAULT_AVATAR_HEAD_ROT, -DEFAULT_AVATAR_HEAD_TO_MIDDLE_EYE_OFFSET);
    }

    // try to account for weirdness with HMD view being inside the root of the head bone
    auto headCorrectionA = glm::translate(glm::vec3(0.0f, 0.16f, eyeZOffset));
    auto headCorrectionB = glm::translate(glm::vec3(0.0f, -0.2f, 0.0f));
    _poseStateMap[HEAD] = _context->_lastHeadPose.postTransform(headCorrectionA).postTransform(defaultHeadOffset).postTransform(headCorrectionB).transform(sensorToAvatar);

    std::vector<std::pair<std::string, StandardAxisChannel>> floatsToUpdate = {
        {"left_trigger_value", LT},
        {"left_squeeze_value", LEFT_GRIP},
        {"left_trackpad_click", (StandardAxisChannel)LEFT_TRACKPAD_PRESS},

        {"right_trigger_value", RT},
        {"right_squeeze_value", RIGHT_GRIP},
        {"right_trackpad_click", (StandardAxisChannel)RIGHT_TRACKPAD_PRESS},
    };

    for (const auto& [name, channel] : floatsToUpdate) {
        auto action = _actions.at(name)->getFloat();
        if (action.isActive) {
            _axisStateMap[channel].value = action.currentState;
        }
    }

    // Vive and Index controllers have a physical trigger click, but most
    // software only checks for /trigger/value ≈ 1, even on those controllers.
    // The virtual "trigger click" button is still required internally
    // for laser interactiom to work properly, so we emulate it here.
    {
        auto left_trigger = _actions.at("left_trigger_value")->getFloat();
        auto right_trigger = _actions.at("right_trigger_value")->getFloat();

        // TODO: Customisable click threshold?
        if (left_trigger.isActive && left_trigger.currentState >= 0.95f) {
            _buttonPressedMap.insert(LT_CLICK);
        }

        if (right_trigger.isActive && right_trigger.currentState >= 0.95f) {
            _buttonPressedMap.insert(RT_CLICK);
        }
    }

    std::vector<std::tuple<std::string, StandardAxisChannel, StandardAxisChannel>> axesToUpdate = {
        {"left_thumbstick", LX, LY},
        {"left_trackpad", (StandardAxisChannel)LEFT_TRACKPAD_X, (StandardAxisChannel)LEFT_TRACKPAD_Y},
        {"right_thumbstick", RX, RY},
        {"right_trackpad", (StandardAxisChannel)RIGHT_TRACKPAD_X, (StandardAxisChannel)RIGHT_TRACKPAD_Y},
    };

    for (const auto& [name, x_channel, y_channel] : axesToUpdate) {
        auto action = _actions.at(name)->getVector2f();
        if (action.isActive) {
            _axisStateMap[x_channel].value = action.currentState.x;
            _axisStateMap[y_channel].value = -action.currentState.y;
        }
    }

    std::vector<std::pair<std::string, StandardButtonChannel>> buttonsToUpdate = {
        {"left_primary_click", LEFT_PRIMARY_THUMB},
        {"left_primary_touch", LEFT_PRIMARY_THUMB_TOUCH},
        {"left_secondary_click", LEFT_SECONDARY_THUMB},
        {"left_secondary_touch", LEFT_SECONDARY_THUMB_TOUCH},
        {"left_thumbstick_click", LS},
        {"left_thumbstick_touch", LS_TOUCH},
        {"left_trigger_touch", (StandardButtonChannel)LT_TOUCH},
        {"left_trackpad_touch", (StandardButtonChannel)LEFT_TRACKPAD_TOUCH},

        {"right_primary_click", RIGHT_PRIMARY_THUMB},
        {"right_primary_touch", RIGHT_PRIMARY_THUMB_TOUCH},
        {"right_secondary_click", RIGHT_SECONDARY_THUMB},
        {"right_secondary_touch", RIGHT_SECONDARY_THUMB_TOUCH},
        {"right_thumbstick_click", RS},
        {"right_thumbstick_touch", RS_TOUCH},
        {"right_trigger_touch", (StandardButtonChannel)RT_TOUCH},
        {"right_trackpad_touch", (StandardButtonChannel)RIGHT_TRACKPAD_TOUCH},
    };

    for (const auto& [name, channel] : buttonsToUpdate) {
        auto action = _actions.at(name)->getBool();
        if (action.isActive && action.currentState) {
            _buttonPressedMap.insert(channel);
        }
    }

    setupControllerFlags();

    // TODO: Fix up and finish the HTCX_vive_tracker_interaction support
    if (_context->_HTCX_viveTrackerInteractionSupported) {
        updateBodyFromViveTrackers(sensorToAvatar);
    } else if (_context->_MNDX_xdevSpaceSupported) {
        updateBodyFromXDevSpaces(sensorToAvatar);
    }

    if (_wantsCalibrate) {
        calibratePucks(inputCalibrationData);
    }

    for (int i = 0; i < HAND_COUNT; i++) {
        getHandTrackingInputs(i, sensorToAvatar);
    }
}

void OpenXrInputPlugin::InputDevice::setupControllerFlags() {
    using namespace controller;

    auto left_primary = _actions.at("left_primary_click")->getBool();
    auto left_primary_touch = _actions.at("left_primary_touch")->getBool();
    auto left_thumbstick = _actions.at("left_thumbstick")->getVector2f();
    auto left_trackpad = _actions.at("left_trackpad")->getVector2f();

    auto right_primary = _actions.at("right_primary_click")->getBool();
    auto right_primary_touch = _actions.at("right_primary_touch")->getBool();
    auto right_thumbstick = _actions.at("right_thumbstick")->getVector2f();
    auto right_trackpad = _actions.at("right_trackpad")->getVector2f();

    if (left_primary.isActive) { _buttonPressedMap.insert(LEFT_HAS_PRIMARY); }
    if (left_trackpad.isActive) { _buttonPressedMap.insert(LEFT_HAS_TRACKPAD); }
    if (left_thumbstick.isActive) { _buttonPressedMap.insert(LEFT_HAS_THUMBSTICK); }

    // Every controller in the OpenXR spec that has
    // /input/a/touch also has /input/trigger/touch,
    // so we can safely use that for hand pose simulation
    if (left_primary_touch.isActive) { _buttonPressedMap.insert(LEFT_HAS_CAPACITIVE_TOUCH); }

    if (right_primary.isActive) { _buttonPressedMap.insert(RIGHT_HAS_PRIMARY); }
    if (right_trackpad.isActive) { _buttonPressedMap.insert(RIGHT_HAS_TRACKPAD); }
    if (right_thumbstick.isActive) { _buttonPressedMap.insert(RIGHT_HAS_THUMBSTICK); }
    if (right_primary_touch.isActive) { _buttonPressedMap.insert(RIGHT_HAS_CAPACITIVE_TOUCH); }
}

void OpenXrInputPlugin::InputDevice::getHandTrackingInputs(int i, const mat4& sensorToAvatar) {
    using namespace controller;

    // simple index/thumb/rest tracking from button inputs
    if (_buttonPressedMap.contains(LEFT_HAS_CAPACITIVE_TOUCH)) {
        auto left_trigger_touch = _actions.at("left_trigger_touch")->getBool().currentState;

        if (!left_trigger_touch) { _buttonPressedMap.insert(LEFT_INDEX_POINT); }

        auto left_primary_touch = _actions.at("left_primary_touch")->getBool().currentState;
        auto left_secondary_touch = _actions.at("left_secondary_touch")->getBool().currentState;
        auto left_thumbstick_touch = _actions.at("left_thumbstick_touch")->getBool().currentState;
        auto left_trackpad_touch = _actions.at("left_trackpad_touch")->getBool().currentState;

        if (
            !left_primary_touch &&
            !left_secondary_touch &&
            !left_thumbstick_touch &&
            !left_trackpad_touch
        ) {
            _buttonPressedMap.insert(LEFT_THUMB_UP);
        }
    }

    if (_buttonPressedMap.contains(RIGHT_HAS_CAPACITIVE_TOUCH)) {
        auto right_trigger_touch = _actions.at("right_trigger_touch")->getBool().currentState;

        if (!right_trigger_touch) { _buttonPressedMap.insert(RIGHT_INDEX_POINT); }

        auto right_primary_touch = _actions.at("right_primary_touch")->getBool().currentState;
        auto right_secondary_touch = _actions.at("right_secondary_touch")->getBool().currentState;
        auto right_thumbstick_touch = _actions.at("right_thumbstick_touch")->getBool().currentState;
        auto right_trackpad_touch = _actions.at("right_trackpad_touch")->getBool().currentState;

        if (
            !right_primary_touch &&
            !right_secondary_touch &&
            !right_thumbstick_touch &&
            !right_trackpad_touch
        ) {
            _buttonPressedMap.insert(RIGHT_THUMB_UP);
        }
    }

    // if real hand tracking isn't available,
    // don't do any more than the capacitive touch tracking
    if (_context->_handTrackingSupported) {
        return;
    }

    if (_handTracker[i] == XR_NULL_HANDLE) { return; }
    if (!_context->_lastPredictedDisplayTime.has_value()) { return; }

    XrHandJointLocationEXT joints[XR_HAND_JOINT_COUNT_EXT];
    XrHandJointLocationsEXT locations = {
        .type = XR_TYPE_HAND_JOINT_LOCATIONS_EXT,
        .jointCount = XR_HAND_JOINT_COUNT_EXT,
        .jointLocations = joints,
    };

    XrHandJointsLocateInfoEXT locateInfo = {
        .type = XR_TYPE_HAND_JOINTS_LOCATE_INFO_EXT,
        .baseSpace = _context->_stageSpace,
        .time = _context->_lastPredictedDisplayTime.value(),
    };

    _context->xrLocateHandJointsEXT(_handTracker[i], &locateInfo, &locations);

    if (!locations.isActive) { return; }

    // Handles coordinate space conversion:
    //
    //   OpenXR
    // * Y-up relative to the fingernail
    // * Z flowing down into the finger and wrist
    // * Finger bone chain ends at the metacarpals and wrist
    // * Has finger tip bones
    // * Has a palm bone in the center of the middle metacarpal
    // * Can report joints as they are positioned on the user's real hand
    //
    //   Mixamo
    // * Z facing away from the palm
    // * Y flowing out of the finger
    // * Finger bone chain ends at the knuckles, except for the thumb, which has a metacarpal
    // * Thumb tip has an extra bone
    //
    auto xrJointToGlm = [&](int joint) -> Pose {
        auto position = xrVecToGlm(joints[joint].pose.position);
        auto rotation = xrQuatToGlm(joints[joint].pose.orientation);

        // rotate the thumb bones from thumbnail-relative to palm-relative, 90°
        if (joint >= XR_HAND_JOINT_THUMB_METACARPAL_EXT && joint <= XR_HAND_JOINT_THUMB_TIP_EXT) {
            // ccw on the right hand, cw on the left hand
            rotation = glm::rotate(rotation, glm::radians(i == 0 ? 90.0f : -90.0f), Vectors::UNIT_Z);
        }

        // rotate 90° clockwise on X, then rotate 180° on Z
        rotation = glm::rotate(rotation, glm::radians(90.0f), Vectors::UNIT_X);
        rotation = glm::rotate(rotation, glm::radians(180.0f), Vectors::UNIT_Z);

        return Pose(position, rotation).transform(sensorToAvatar);
    };

    _poseStateMap[i == 0 ? LEFT_HAND : RIGHT_HAND] = xrJointToGlm(XR_HAND_JOINT_WRIST_EXT);

    _poseStateMap[i == 0 ? LEFT_HAND_THUMB1 : RIGHT_HAND_THUMB1] = xrJointToGlm(XR_HAND_JOINT_THUMB_METACARPAL_EXT);
    _poseStateMap[i == 0 ? LEFT_HAND_THUMB2 : RIGHT_HAND_THUMB2] = xrJointToGlm(XR_HAND_JOINT_THUMB_PROXIMAL_EXT);
    _poseStateMap[i == 0 ? LEFT_HAND_THUMB3 : RIGHT_HAND_THUMB3] = xrJointToGlm(XR_HAND_JOINT_THUMB_DISTAL_EXT);

    _poseStateMap[i == 0 ? LEFT_HAND_INDEX1 : RIGHT_HAND_INDEX1] = xrJointToGlm(XR_HAND_JOINT_INDEX_PROXIMAL_EXT);
    _poseStateMap[i == 0 ? LEFT_HAND_INDEX2 : RIGHT_HAND_INDEX2] = xrJointToGlm(XR_HAND_JOINT_INDEX_INTERMEDIATE_EXT);
    _poseStateMap[i == 0 ? LEFT_HAND_INDEX3 : RIGHT_HAND_INDEX3] = xrJointToGlm(XR_HAND_JOINT_INDEX_DISTAL_EXT);

    _poseStateMap[i == 0 ? LEFT_HAND_MIDDLE1 : RIGHT_HAND_MIDDLE1] = xrJointToGlm(XR_HAND_JOINT_MIDDLE_PROXIMAL_EXT);
    _poseStateMap[i == 0 ? LEFT_HAND_MIDDLE2 : RIGHT_HAND_MIDDLE2] = xrJointToGlm(XR_HAND_JOINT_MIDDLE_INTERMEDIATE_EXT);
    _poseStateMap[i == 0 ? LEFT_HAND_MIDDLE3 : RIGHT_HAND_MIDDLE3] = xrJointToGlm(XR_HAND_JOINT_MIDDLE_DISTAL_EXT);

    _poseStateMap[i == 0 ? LEFT_HAND_RING1 : RIGHT_HAND_RING1] = xrJointToGlm(XR_HAND_JOINT_RING_PROXIMAL_EXT);
    _poseStateMap[i == 0 ? LEFT_HAND_RING2 : RIGHT_HAND_RING2] = xrJointToGlm(XR_HAND_JOINT_RING_INTERMEDIATE_EXT);
    _poseStateMap[i == 0 ? LEFT_HAND_RING3 : RIGHT_HAND_RING3] = xrJointToGlm(XR_HAND_JOINT_RING_DISTAL_EXT);

    _poseStateMap[i == 0 ? LEFT_HAND_PINKY1 : RIGHT_HAND_PINKY1] = xrJointToGlm(XR_HAND_JOINT_LITTLE_PROXIMAL_EXT);
    _poseStateMap[i == 0 ? LEFT_HAND_PINKY2 : RIGHT_HAND_PINKY2] = xrJointToGlm(XR_HAND_JOINT_LITTLE_INTERMEDIATE_EXT);
    _poseStateMap[i == 0 ? LEFT_HAND_PINKY3 : RIGHT_HAND_PINKY3] = xrJointToGlm(XR_HAND_JOINT_LITTLE_DISTAL_EXT);
}

void OpenXrInputPlugin::InputDevice::calibratePucks(const controller::InputCalibrationData& data) {
    using namespace controller;

    static const std::array posesToCalibrate = {
        LEFT_FOOT, RIGHT_FOOT, HIPS, SPINE2,
    };

    for (auto channel : posesToCalibrate) {
        // not connected, don't calibrate
        if (!_poseStateMap[channel].isValid()) { continue; }

        // get the heading of the headset for the forward direction
        auto heading = glm::eulerAngles(_context->_lastHeadPose.getRotation()).y;
        auto headAngle = glm::inverse(quat(vec3(0.0f, heading, 0.0f)));

        //auto position = headAngle * _poseStateMap[channel].getTranslation();
        auto rotation = headAngle * _poseStateMap[channel].getRotation();
        auto offset = defaultPoseOffset(data, channel);

        _trackerCalibrations[channel] = Pose(
            glm::inverse(rotation) * vec3(0.0f, 0.0f, channel == HIPS || channel == SPINE2 ? 0.2f : 0.1f),
            glm::inverse(rotation) * quat(offset)
        );
    }

    _wantsCalibrate = false;
}

void OpenXrInputPlugin::InputDevice::updateBodyFromViveTrackers(const mat4& sensorToAvatar) {
    using namespace controller;

    auto handlePose = [&](std::string action, StandardPoseChannel channel) {
        XrSpaceLocation location = _actions.at(action)->getPose();
        bool locationValid = (location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0;
        if (locationValid) {
            vec3 translation = xrVecToGlm(location.pose.position);
            quat rotation = xrQuatToGlm(location.pose.orientation);
            auto pose = controller::Pose(translation, rotation);

            if (_wantsCalibrate) {
                // use the raw sensor pose for calibration
                _poseStateMap[channel] = pose;
            } else {
                if (_trackerCalibrations.contains(channel)) {
                    _poseStateMap[channel] = pose.postTransform(_trackerCalibrations[channel].getMatrix()).transform(sensorToAvatar);
                } else {
                    _poseStateMap[channel] = pose.transform(sensorToAvatar);
                }
            }
        }
    };

    handlePose("hips_pose", HIPS);
    handlePose("chest_pose", SPINE2);
    handlePose("left_foot_pose", LEFT_FOOT);
    handlePose("right_foot_pose", RIGHT_FOOT);
}

void OpenXrInputPlugin::InputDevice::updateBodyFromXDevSpaces(const mat4& sensorToAvatar) {
    using namespace controller;

    for (const auto& [id, xdev] : _xdev) {
        XrSpaceLocation location = {.type = XR_TYPE_SPACE_LOCATION};
        if (_context->_lastPredictedDisplayTime.has_value()) {
            XrResult result = xrLocateSpace(xdev.space, _context->_stageSpace, _context->_lastPredictedDisplayTime.value(), &location);
            xrCheck(_context->_instance, result, "Failed to locate xdev space!");
        }

        bool locationValid = (location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0;

        if (locationValid && xdev.pose_channel.has_value()) {
            vec3 translation = xrVecToGlm(location.pose.position);
            quat rotation = xrQuatToGlm(location.pose.orientation);
            auto pose = controller::Pose(translation, rotation);

            auto channel = xdev.pose_channel.value();

            if (_wantsCalibrate) {
                // use the raw sensor pose for calibration
                _poseStateMap[channel] = pose;
            } else {
                if (_trackerCalibrations.contains(channel)) {
                    _poseStateMap[channel] = pose.postTransform(_trackerCalibrations[channel].getMatrix()).transform(sensorToAvatar);
                } else {
                    _poseStateMap[channel] = pose.transform(sensorToAvatar);
                }
            }
        }
    }
}
