//
// Overte OpenXR Plugin
//
// Copyright 2024 Lubosz Sarnecki
// Copyright 2024 Overte e.V.
//
// SPDX-License-Identifier: Apache-2.0
//

#include <glm/ext.hpp>

#include "OpenXrInputPlugin.h"

#include "AvatarConstants.h"
#include "PathUtils.h"

#include "controllers/UserInputMapper.h"

Q_DECLARE_LOGGING_CATEGORY(xr_input_cat)
Q_LOGGING_CATEGORY(xr_input_cat, "openxr.input")

OpenXrInputPlugin::OpenXrInputPlugin(std::shared_ptr<OpenXrContext> c) {
    _context = c;
    _inputDevice = std::make_shared<InputDevice>(_context);
}

// TODO: Config options
static const QString XR_CONFIGURATION_LAYOUT = QString("");

// TODO: full-body-tracking
void OpenXrInputPlugin::calibrate() {
}

// TODO: full-body-tracking
bool OpenXrInputPlugin::uncalibrate() {
    return true;
}

bool OpenXrInputPlugin::isSupported() const {
    return _context->_isValid && _context->_isSupported;
}

// TODO: Config options
void OpenXrInputPlugin::setConfigurationSettings(const QJsonObject configurationSettings) {
}

// TODO: Config options
QJsonObject OpenXrInputPlugin::configurationSettings() {
    return QJsonObject();
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

// TODO: Config options
void OpenXrInputPlugin::loadSettings() {
}

// TODO: Config options
void OpenXrInputPlugin::saveSettings() const {
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
    if (index > 2) {
        return false;
    }

    std::unique_lock<std::recursive_mutex> locker(_lock);

    // TODO: Haptic values in overte are always strengh 1.0 and duration only 13.0 or 16.0. So it's not really used.
    // The duration does not seem to map to a time unit. 16ms seems quite short for a haptic vibration.
    // Let's assume the duration is in 10 milliseconds.
    // Let's also assume strength 1.0 is the middle value, which is 0.5 in OpenXR.
    using namespace std::chrono;
    nanoseconds durationNs = duration_cast<nanoseconds>(milliseconds(static_cast<int>(duration * 10.0f)));
    XrDuration xrDuration = durationNs.count();

    auto path = (index == 0) ? "left_haptic" : "right_haptic";

    // FIXME: sometimes something bugs out and hammers this,
    // and the controller vibrates really loudly until another
    // haptic pulse is triggered
    // The OpenVR plugin has a lock protecting these
    if (!_actions.at(path)->applyHaptic(xrDuration, XR_FREQUENCY_UNSPECIFIED, 0.5f * strength)) {
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

    QVector<Input::NamedPair> availableInputs{
        makePair(HEAD, "Head"),

        makePair(LEFT_HAND, "LeftHand"),
        makePair(LS, "LS"),
        makePair(LS_TOUCH, "LSTouch"),
        makePair(LX, "LX"),
        makePair(LY, "LY"),
        makePair(LT, "LT"),
        makePair(LT_CLICK, "LTClick"),
        makePair(LEFT_GRIP, "LeftGrip"),
        makePair(LEFT_PRIMARY_THUMB, "LeftPrimaryThumb"),
        makePair(LEFT_SECONDARY_THUMB, "LeftSecondaryThumb"),

        makePair(RIGHT_HAND, "RightHand"),
        makePair(RS, "RS"),
        makePair(RS_TOUCH, "RSTouch"),
        makePair(RX, "RX"),
        makePair(RY, "RY"),
        makePair(RT, "RT"),
        makePair(RT_CLICK, "RTClick"),
        makePair(RIGHT_GRIP, "RightGrip"),
        makePair(RIGHT_PRIMARY_THUMB, "RightPrimaryThumb"),
        makePair(RIGHT_SECONDARY_THUMB, "RightSecondaryThumb"),

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
        .actionSetName = "overte",
        .localizedActionSetName = "Overte",
        .priority = 0,
    };
    XrResult result = xrCreateActionSet(instance, &actionSetInfo, &_actionSet);
    if (!xrCheck(instance, result, "Failed to create action set."))
        return false;

    // NOTE: The "squeeze" actions have quite a high deadzone in the controller config.
    // A lot of our controller scripts currently only check for (squeeze > 0),
    // which means controllers like the Index ones will be way too sensitive.
    std::map<std::string, std::pair<std::string, XrActionType>> actionTypes = {
        {"left_primary_click",     {"Left Primary", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"left_secondary_click",   {"Left Secondary (Tablet)", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"left_squeeze_value",     {"Left Squeeze", XR_ACTION_TYPE_FLOAT_INPUT}},
        {"left_trigger_value",     {"Left Trigger", XR_ACTION_TYPE_FLOAT_INPUT}},
        {"left_trigger_click",     {"Left Trigger Click", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"left_thumbstick",        {"Left Thumbstick", XR_ACTION_TYPE_VECTOR2F_INPUT}},
        {"left_thumbstick_click",  {"Left Thumbstick Click", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"left_thumbstick_touch",  {"Left Thumbstick Touch", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"left_pose",              {"Left Hand Pose", XR_ACTION_TYPE_POSE_INPUT}},
        {"left_haptic",            {"Left Hand Haptic", XR_ACTION_TYPE_VIBRATION_OUTPUT}},

        {"right_primary_click",    {"Right Primary", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"right_secondary_click",  {"Right Secondary (Jump)", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"right_squeeze_value",    {"Right Squeeze", XR_ACTION_TYPE_FLOAT_INPUT}},
        {"right_trigger_value",    {"Right Trigger", XR_ACTION_TYPE_FLOAT_INPUT}},
        {"right_trigger_click",    {"Right Trigger Click", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"right_thumbstick",       {"Right Thumbstick", XR_ACTION_TYPE_VECTOR2F_INPUT}},
        {"right_thumbstick_click", {"Right Thumbstick Click", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"right_thumbstick_touch", {"Right Thumbstick Touch", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"right_pose",             {"Right Hand Pose", XR_ACTION_TYPE_POSE_INPUT}},
        {"right_haptic",           {"Right Hand Haptic", XR_ACTION_TYPE_VIBRATION_OUTPUT}},
    };

    std::string hand_left = "/user/hand/left/input";
    std::string hand_right = "/user/hand/right/input";

    std::map<std::string, std::map<std::string, std::string>> actionSuggestions = {
        // not really usable, bare minimum
        {"/interaction_profiles/khr/simple_controller", {
            {"left_secondary_click",   hand_left  + "/menu/click"},
            {"left_trigger_value",     hand_left  + "/select/click"},
            {"left_pose",              hand_left  + "/grip/pose"},
            {"left_haptic",            "/user/hand/left/output/haptic"},

            {"right_secondary_click",  hand_right + "/menu/click"},
            {"right_trigger_value",    hand_right + "/select/click"},
            {"right_pose",             hand_right + "/grip/pose"},
            {"right_haptic",           "/user/hand/right/output/haptic"},
        }},
        {"/interaction_profiles/htc/vive_controller", {
            {"left_secondary_click",   hand_left  + "/menu/click"},
            {"left_squeeze_value",     hand_left  + "/squeeze/click"},
            {"left_trigger_value",     hand_left  + "/trigger/value"},
            {"left_trigger_click",     hand_left  + "/trigger/click"},
            {"left_thumbstick",        hand_left  + "/trackpad"},
            {"left_thumbstick_click",  hand_left  + "/trackpad/click"},
            {"left_thumbstick_touch",  hand_left  + "/trackpad/touch"},
            {"left_pose",              hand_left  + "/grip/pose"},
            {"left_haptic",            "/user/hand/left/output/haptic"},

            {"right_secondary_click",  hand_right + "/menu/click"},
            {"right_squeeze_value",    hand_right + "/squeeze/click"},
            {"right_trigger_value",    hand_right + "/trigger/value"},
            {"right_trigger_click",    hand_right + "/trigger/click"},
            {"right_thumbstick",       hand_right + "/trackpad"},
            {"right_thumbstick_click", hand_right + "/trackpad/click"},
            {"right_thumbstick_touch", hand_right + "/trackpad/touch"},
            {"right_pose",             hand_right + "/grip/pose"},
            {"right_haptic",           "/user/hand/right/output/haptic"},
        }},
        {"/interaction_profiles/oculus/touch_controller", {
            {"left_primary_click",     hand_left  + "/x/click"},
            {"left_secondary_click",   hand_left  + "/y/click"},
            {"left_squeeze_value",     hand_left  + "/squeeze/value"},
            {"left_trigger_value",     hand_left  + "/trigger/value"},
            {"left_thumbstick",        hand_left  + "/thumbstick"},
            {"left_thumbstick_click",  hand_left  + "/thumbstick/click"},
            {"left_thumbstick_touch",  hand_left  + "/thumbstick/touch"},
            {"left_pose",              hand_left  + "/grip/pose"},
            {"left_haptic",            "/user/hand/left/output/haptic"},

            {"right_primary_click",    hand_right  + "/a/click"},
            {"right_secondary_click",  hand_right  + "/b/click"},
            {"right_squeeze_value",    hand_right  + "/squeeze/value"},
            {"right_trigger_value",    hand_right  + "/trigger/value"},
            {"right_thumbstick",       hand_right  + "/thumbstick"},
            {"right_thumbstick_click", hand_right  + "/thumbstick/click"},
            {"right_thumbstick_touch", hand_right  + "/thumbstick/touch"},
            {"right_pose",             hand_right  + "/grip/pose"},
            {"right_haptic",           "/user/hand/right/output/haptic"},
        }},
        {"/interaction_profiles/microsoft/motion_controller", {
            {"left_secondary_click",   hand_left  + "/menu/click"},
            {"left_squeeze_value",     hand_left  + "/squeeze/click"},
            {"left_trigger_value",     hand_left  + "/trigger/value"},
            {"left_thumbstick",        hand_left  + "/thumbstick"},
            {"left_thumbstick_click",  hand_left  + "/trackpad/click"},
            {"left_thumbstick_touch",  hand_left  + "/trackpad/touch"},
            {"left_pose",              hand_left  + "/grip/pose"},
            {"left_haptic",            "/user/hand/left/output/haptic"},

            {"right_secondary_click",   hand_right  + "/menu/click"},
            {"right_squeeze_value",     hand_right  + "/squeeze/click"},
            {"right_trigger_value",     hand_right  + "/trigger/value"},
            {"right_thumbstick",        hand_right  + "/thumbstick"},
            {"right_thumbstick_click",  hand_right  + "/trackpad/click"},
            {"right_thumbstick_touch",  hand_right  + "/trackpad/touch"},
            {"right_pose",              hand_right  + "/grip/pose"},
            {"right_haptic",            "/user/hand/right/output/haptic"},
        }},
        {"/interaction_profiles/samsung/odyssey_controller", {
            {"left_secondary_click",   hand_left  + "/menu/click"},
            {"left_squeeze_value",     hand_left  + "/squeeze/click"},
            {"left_trigger_value",     hand_left  + "/trigger/value"},
            {"left_thumbstick",        hand_left  + "/thumbstick"},
            {"left_thumbstick_click",  hand_left  + "/trackpad/click"},
            {"left_thumbstick_touch",  hand_left  + "/trackpad/touch"},
            {"left_pose",              hand_left  + "/grip/pose"},
            {"left_haptic",            "/user/hand/left/output/haptic"},

            {"right_secondary_click",   hand_right  + "/menu/click"},
            {"right_squeeze_value",     hand_right  + "/squeeze/click"},
            {"right_trigger_value",     hand_right  + "/trigger/value"},
            {"right_thumbstick",        hand_right  + "/thumbstick"},
            {"right_thumbstick_click",  hand_right  + "/trackpad/click"},
            {"right_thumbstick_touch",  hand_right  + "/trackpad/touch"},
            {"right_pose",              hand_right  + "/grip/pose"},
            {"right_haptic",            "/user/hand/right/output/haptic"},
        }},
        {"/interaction_profiles/valve/index_controller", {
            {"left_primary_click",     hand_left  + "/a/click"},
            {"left_secondary_click",   hand_left  + "/b/click"},
            {"left_squeeze_value",     hand_left  + "/squeeze/force"},
            {"left_trigger_value",     hand_left  + "/trigger/value"},
            {"left_trigger_click",     hand_left  + "/trigger/click"},
            {"left_thumbstick",        hand_left  + "/thumbstick"},
            {"left_thumbstick_click",  hand_left  + "/thumbstick/click"},
            {"left_thumbstick_touch",  hand_left  + "/thumbstick/touch"},
            {"left_pose",              hand_left  + "/grip/pose"},
            {"left_haptic",            "/user/hand/left/output/haptic"},

            {"right_primary_click",    hand_right  + "/a/click"},
            {"right_secondary_click",  hand_right  + "/b/click"},
            {"right_squeeze_value",    hand_right  + "/squeeze/force"},
            {"right_trigger_value",    hand_right  + "/trigger/value"},
            {"right_trigger_click",    hand_right  + "/trigger/click"},
            {"right_thumbstick",       hand_right  + "/thumbstick"},
            {"right_thumbstick_click", hand_right  + "/thumbstick/click"},
            {"right_thumbstick_touch", hand_right  + "/thumbstick/touch"},
            {"right_pose",             hand_right  + "/grip/pose"},
            {"right_haptic",           "/user/hand/right/output/haptic"},
        }},
    };

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

    _actionsInitialized = true;

    return true;
}

void OpenXrInputPlugin::InputDevice::update(float deltaTime, const controller::InputCalibrationData& inputCalibrationData) {
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
        auto hand_path = (i == 0) ? "left_pose" : "right_pose";
        XrSpaceLocation handLocation = _actions.at(hand_path)->getPose();
        bool locationValid = (handLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0;
        if (locationValid) {
            vec3 translation = xrVecToGlm(handLocation.pose.position);
            quat rotation = xrQuatToGlm(handLocation.pose.orientation);
            auto pose = controller::Pose(translation, rotation);
            glm::mat4 handOffset = i == 0 ? glm::toMat4(leftRotationOffset) : glm::toMat4(rightRotationOffset);

            glm::mat4 posOffset(1.0f);

            // vive controllers have bugged poses that aren't in the grip or aim position,
            // they're always at the top near the tracking ring
            if (_context->_stickEmulation) {
                posOffset *= glm::translate(glm::vec3(handOffset[0]) * (i == 0 ? 0.1f : -0.1f));
                posOffset *= glm::translate(glm::vec3(handOffset[1]) * -0.16f);
                posOffset *= glm::translate(glm::vec3(handOffset[2]) * -0.02f);
            } else {
                posOffset *= glm::translate(glm::vec3(handOffset[0]) * (i == 0 ? -0.07f : 0.07f));
                posOffset *= glm::translate(glm::vec3(handOffset[1]) * -0.10f);
                posOffset *= glm::translate(glm::vec3(handOffset[2]) * -0.01f);
            }

            _poseStateMap[i == 0 ? controller::LEFT_HAND : controller::RIGHT_HAND] =
                pose.postTransform(posOffset).postTransform(handOffset).transform(sensorToAvatar);
        }
    }

    glm::mat4 defaultHeadOffset;
    float eyeZOffset = 0.16f;
    if (inputCalibrationData.hmdAvatarAlignmentType == controller::HmdAvatarAlignmentType::Eyes) {
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
    _poseStateMap[controller::HEAD] = _context->_lastHeadPose.postTransform(headCorrectionA).postTransform(defaultHeadOffset).postTransform(headCorrectionB).transform(sensorToAvatar);

    std::vector<std::pair<std::string, controller::StandardAxisChannel>> floatsToUpdate = {
        {"left_trigger_value", controller::LT},
        {"left_squeeze_value", controller::LEFT_GRIP},

        {"right_trigger_value", controller::RT},
        {"right_squeeze_value", controller::RIGHT_GRIP},
    };

    for (const auto& [name, channel] : floatsToUpdate) {
        auto action = _actions.at(name)->getFloat();
        if (action.isActive) {
            _axisStateMap[channel].value = action.currentState;
        }
    }

    // emulate stick clicks for controllers that don't have them
    {
        const auto& left_trigger = _actions.at("left_trigger_value")->getFloat();
        const auto& left_click = _actions.at("left_trigger_click")->getBool();

        const auto& right_trigger = _actions.at("right_trigger_value")->getFloat();
        const auto& right_click = _actions.at("right_trigger_click")->getBool();

        if (!left_click.isActive && (left_trigger.isActive && left_trigger.currentState > 0.9f)) {
            _buttonPressedMap.insert(controller::LT_CLICK);
        }

        if (!right_click.isActive && (right_trigger.isActive && right_trigger.currentState > 0.9f)) {
            _buttonPressedMap.insert(controller::RT_CLICK);
        }
    }

    std::vector<std::tuple<std::string, controller::StandardAxisChannel, controller::StandardAxisChannel>> axesToUpdate = {
        {"left_thumbstick", controller::LX, controller::LY},
        {"right_thumbstick", controller::RX, controller::RY},
    };

    for (const auto& [name, x_channel, y_channel] : axesToUpdate) {
        auto action = _actions.at(name)->getVector2f();
        if (action.isActive) {
            _axisStateMap[x_channel].value = action.currentState.x;
            _axisStateMap[y_channel].value = -action.currentState.y;
        }
    }

    std::vector<std::pair<std::string, controller::StandardButtonChannel>> buttonsToUpdate = {
        {"left_primary_click", controller::LEFT_PRIMARY_THUMB},
        {"left_secondary_click", controller::LEFT_SECONDARY_THUMB},
        {"left_trigger_click", controller::LT_CLICK},
        {"left_thumbstick_click", controller::LS},
        {"left_thumbstick_touch", controller::LS_TOUCH},

        {"right_primary_click", controller::RIGHT_PRIMARY_THUMB},
        {"right_secondary_click", controller::RIGHT_SECONDARY_THUMB},
        {"right_trigger_click", controller::RT_CLICK},
        {"right_thumbstick_click", controller::RS},
        {"right_thumbstick_touch", controller::RS_TOUCH},
    };

    for (const auto& [name, channel] : buttonsToUpdate) {
        auto action = _actions.at(name)->getBool();
        if (action.isActive && action.currentState) {
            _buttonPressedMap.insert(channel);
        }
    }

    // emulate primary button for controllers with only one physical button,
    // but not on vive controllers because we have special behavior there already
    if (!_context->_stickEmulation) {
        const auto& left_click = _actions.at("left_thumbstick_click")->getBool();
        const auto& right_click = _actions.at("right_thumbstick_click")->getBool();
        const auto& left_primary = _actions.at("left_primary_click")->getBool();
        const auto& right_primary = _actions.at("right_primary_click")->getBool();

        if (!left_primary.isActive && left_click.currentState) {
            _buttonPressedMap.insert(controller::LEFT_PRIMARY_THUMB);
        }

        if (!right_primary.isActive && right_click.currentState) {
            _buttonPressedMap.insert(controller::RIGHT_PRIMARY_THUMB);
        }
    }

    awfulRightStickHackForBrokenScripts();

    if (_context->_stickEmulation) {
        emulateStickFromTrackpad();
    }

    if (_context->_handTrackingSupported) {
        for (int i = 0; i < HAND_COUNT; i++) {
            getHandTrackingInputs(i, sensorToAvatar);
        }
    }
}

void OpenXrInputPlugin::InputDevice::emulateStickFromTrackpad() {
    auto left_stick = _actions.at("left_thumbstick")->getVector2f().currentState;
    auto right_stick = _actions.at("right_thumbstick")->getVector2f().currentState;
    auto left_click = _actions.at("left_thumbstick_click")->getBool().currentState;
    auto right_click = _actions.at("right_thumbstick_click")->getBool().currentState;

    // set the axes to zero if the trackpad isn't clicked in
    if (!right_click) {
        _axisStateMap[controller::RX].value = 0.0f;
        _axisStateMap[controller::RY].value = 0.0f;
    }

    if (!left_click) {
        _axisStateMap[controller::LX].value = 0.0f;
        _axisStateMap[controller::LY].value = 0.0f;
    }

    // "primary" button on trackpad center
    if (
        left_click &&
        left_stick.x > -0.4f &&
        left_stick.x < 0.4f &&
        left_stick.y > -0.4f &&
        left_stick.y < 0.4f
    ) {
        _buttonPressedMap.insert(controller::LEFT_PRIMARY_THUMB);
    }

    if (
        right_click &&
        right_stick.x > -0.4f &&
        right_stick.x < 0.4f &&
        right_stick.y > -0.4f &&
        right_stick.y < 0.4f
    ) {
        _buttonPressedMap.insert(controller::RIGHT_PRIMARY_THUMB);
    }
}

// FIXME: the vr controller scripts are horribly broken and don't work properly,
// this emulates a segmented vive trackpad to get teleport and snap turning behaving
void OpenXrInputPlugin::InputDevice::awfulRightStickHackForBrokenScripts() {
    auto stick = _actions.at("right_thumbstick")->getVector2f().currentState;

    _axisStateMap[controller::RX].value = 0.0f;
    _axisStateMap[controller::RY].value = 0.0f;

    if (stick.x < -0.6f && stick.y > -0.4f && stick.y < 0.4f) {
        _axisStateMap[controller::RX].value = -1.0f;
    }

    if (stick.x > 0.6f && stick.y > -0.4f && stick.y < 0.4f) {
        _axisStateMap[controller::RX].value = 1.0f;
    }

    if (stick.y > 0.6f && stick.x > -0.4f && stick.x < 0.4f) {
        _axisStateMap[controller::RY].value = -1.0f;
    }

    if (stick.y < -0.6f && stick.x > -0.4f && stick.x < 0.4f) {
        _axisStateMap[controller::RY].value = 1.0f;
    }
}

void OpenXrInputPlugin::InputDevice::getHandTrackingInputs(int i, const mat4& sensorToAvatar) {
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
    auto xrJointToGlm = [&](int joint) -> controller::Pose {
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

        return controller::Pose(position, rotation).transform(sensorToAvatar);
    };

    _poseStateMap[i == 0 ? controller::LEFT_HAND : controller::RIGHT_HAND] = xrJointToGlm(XR_HAND_JOINT_WRIST_EXT);

    _poseStateMap[i == 0 ? controller::LEFT_HAND_THUMB1 : controller::RIGHT_HAND_THUMB1] = xrJointToGlm(XR_HAND_JOINT_THUMB_METACARPAL_EXT);
    _poseStateMap[i == 0 ? controller::LEFT_HAND_THUMB2 : controller::RIGHT_HAND_THUMB2] = xrJointToGlm(XR_HAND_JOINT_THUMB_PROXIMAL_EXT);
    _poseStateMap[i == 0 ? controller::LEFT_HAND_THUMB3 : controller::RIGHT_HAND_THUMB3] = xrJointToGlm(XR_HAND_JOINT_THUMB_DISTAL_EXT);

    _poseStateMap[i == 0 ? controller::LEFT_HAND_INDEX1 : controller::RIGHT_HAND_INDEX1] = xrJointToGlm(XR_HAND_JOINT_INDEX_PROXIMAL_EXT);
    _poseStateMap[i == 0 ? controller::LEFT_HAND_INDEX2 : controller::RIGHT_HAND_INDEX2] = xrJointToGlm(XR_HAND_JOINT_INDEX_INTERMEDIATE_EXT);
    _poseStateMap[i == 0 ? controller::LEFT_HAND_INDEX3 : controller::RIGHT_HAND_INDEX3] = xrJointToGlm(XR_HAND_JOINT_INDEX_DISTAL_EXT);

    _poseStateMap[i == 0 ? controller::LEFT_HAND_MIDDLE1 : controller::RIGHT_HAND_MIDDLE1] = xrJointToGlm(XR_HAND_JOINT_MIDDLE_PROXIMAL_EXT);
    _poseStateMap[i == 0 ? controller::LEFT_HAND_MIDDLE2 : controller::RIGHT_HAND_MIDDLE2] = xrJointToGlm(XR_HAND_JOINT_MIDDLE_INTERMEDIATE_EXT);
    _poseStateMap[i == 0 ? controller::LEFT_HAND_MIDDLE3 : controller::RIGHT_HAND_MIDDLE3] = xrJointToGlm(XR_HAND_JOINT_MIDDLE_DISTAL_EXT);

    _poseStateMap[i == 0 ? controller::LEFT_HAND_RING1 : controller::RIGHT_HAND_RING1] = xrJointToGlm(XR_HAND_JOINT_RING_PROXIMAL_EXT);
    _poseStateMap[i == 0 ? controller::LEFT_HAND_RING2 : controller::RIGHT_HAND_RING2] = xrJointToGlm(XR_HAND_JOINT_RING_INTERMEDIATE_EXT);
    _poseStateMap[i == 0 ? controller::LEFT_HAND_RING3 : controller::RIGHT_HAND_RING3] = xrJointToGlm(XR_HAND_JOINT_RING_DISTAL_EXT);

    _poseStateMap[i == 0 ? controller::LEFT_HAND_PINKY1 : controller::RIGHT_HAND_PINKY1] = xrJointToGlm(XR_HAND_JOINT_LITTLE_PROXIMAL_EXT);
    _poseStateMap[i == 0 ? controller::LEFT_HAND_PINKY2 : controller::RIGHT_HAND_PINKY2] = xrJointToGlm(XR_HAND_JOINT_LITTLE_INTERMEDIATE_EXT);
    _poseStateMap[i == 0 ? controller::LEFT_HAND_PINKY3 : controller::RIGHT_HAND_PINKY3] = xrJointToGlm(XR_HAND_JOINT_LITTLE_DISTAL_EXT);
}
