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

// TODO: Make a config UI
static const QString XR_CONFIGURATION_LAYOUT = QString("");

void OpenXrInputPlugin::calibrate() {
}

bool OpenXrInputPlugin::uncalibrate() {
    return true;
}

bool OpenXrInputPlugin::isSupported() const {
    return _context->_isSupported;
}

void OpenXrInputPlugin::setConfigurationSettings(const QJsonObject configurationSettings) {
}

QJsonObject OpenXrInputPlugin::configurationSettings() {
    return QJsonObject();
}

QString OpenXrInputPlugin::configurationLayout() {
    return XR_CONFIGURATION_LAYOUT;
}

bool OpenXrInputPlugin::activate() {
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
    if (_context->_shouldQuit) {
        deactivate();
        return;
    }

    auto userInputMapper = DependencyManager::get<controller::UserInputMapper>();
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
}

void OpenXrInputPlugin::saveSettings() const {
}

OpenXrInputPlugin::InputDevice::InputDevice(std::shared_ptr<OpenXrContext> c) : controller::InputDevice("OpenXR") {
    _context = c;
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

    auto path = (index == 0) ? "hand_haptic_left" : "hand_haptic_right";

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
        makePair(RIGHT_HAND, "RightHand"),

        // INPUT FIXME: Actions.Translate.{X,Z} work
        // perfectly but Standard.LY is unreliable
        makePair(LX, "WalkX"),
        makePair(LY, "WalkY"),

        makePair(LT, "LeftInteract"),
        makePair(RT, "RightInteract"),
        makePair(LT_CLICK, "LeftInteractClick"),
        makePair(RT_CLICK, "RightInteractClick"),

        makePair(LEFT_GRIP, "LeftGrip"),
        makePair(RIGHT_GRIP, "RightGrip"),

        // INPUT TODO: horrific hack that breaks depending on handedness
        // because the input system is in dire need of a refactor,
        // it was (mostly) designed with raw inputs in mind which makes
        // it extremely difficult to map onto openxr actions
        makePair(LEFT_PRIMARY_THUMB, "ToggleTablet"),
        makePair(RIGHT_PRIMARY_THUMB, "Jump"),
        makePair(DD, "Sprint"),
        makePair(RX, "Turn"),
        makePair(RY, "Teleport"),
        makePair(LS_TOUCH, "CycleCamera"),
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

    std::map<std::string, std::pair<std::string, XrActionType>> actionTypes = {
        {"tablet",             {"Toggle Tablet", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"teleport",           {"Teleport", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"cycle_camera",       {"Cycle Camera", XR_ACTION_TYPE_BOOLEAN_INPUT}},

        {"interact_left",      {"Left Interact", XR_ACTION_TYPE_FLOAT_INPUT}},
        {"interact_right",     {"Right Interact", XR_ACTION_TYPE_FLOAT_INPUT}},

        {"grip_left",          {"Left Grip", XR_ACTION_TYPE_FLOAT_INPUT}},
        {"grip_right",         {"Right Grip", XR_ACTION_TYPE_FLOAT_INPUT}},

        {"turn_left",          {"Turn Left", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"turn_right",         {"Turn Right", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"walk",               {"Walk", XR_ACTION_TYPE_VECTOR2F_INPUT}},
        {"sprint",             {"Sprint", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"jump",               {"Jump", XR_ACTION_TYPE_BOOLEAN_INPUT}},

        // in case the runtime doesn't support dpad emulation
        {"stick_left",         {"Left Stick (Fallback)", XR_ACTION_TYPE_VECTOR2F_INPUT}},
        {"stick_right",        {"Right Stick (Fallback)", XR_ACTION_TYPE_VECTOR2F_INPUT}},
        {"stick_click_left",   {"Left Stick Click (Fallback)", XR_ACTION_TYPE_BOOLEAN_INPUT}},
        {"stick_click_right",  {"Right Stick Click (Fallback)", XR_ACTION_TYPE_BOOLEAN_INPUT}},

        {"hand_pose_left",     {"Left Hand Pose", XR_ACTION_TYPE_POSE_INPUT}},
        {"hand_pose_right",    {"Right Hand Pose", XR_ACTION_TYPE_POSE_INPUT}},
        {"hand_haptic_left",   {"Left Hand Haptic", XR_ACTION_TYPE_VIBRATION_OUTPUT}},
        {"hand_haptic_right",  {"Right Hand Haptic", XR_ACTION_TYPE_VIBRATION_OUTPUT}},
    };

    // palm pose is nice but monado doesn't support it yet
    // (disable it for now because i can't test what palm
    //  pose looks like and if it'll break something)
    //auto hand_pose_name = (_context->_palmPoseSupported) ? "/palm_ext/pose" : "/grip/pose";
    auto hand_pose_name = "/grip/pose";

    // TODO: set up the openxr dpad bindings modifier (looks complicated)
    std::map<std::string, std::map<std::string, std::string>> actionSuggestions = {
        {"/interaction_profiles/khr/simple_controller", {
            {"tablet",            "/user/hand/left/input/menu/click"},
            {"teleport",          "/user/hand/right/input/menu/click"},
            {"interact_left",     "/user/hand/left/input/select/click"},
            {"interact_right",    "/user/hand/right/input/select/click"},
            {"hand_pose_left",    std::string("/user/hand/left/input") + hand_pose_name},
            {"hand_pose_right",   std::string("/user/hand/right/input") + hand_pose_name},
            {"hand_haptic_left",  "/user/hand/left/output/haptic"},
            {"hand_haptic_right", "/user/hand/right/output/haptic"},
        }},
        {"/interaction_profiles/htc/vive_controller", {
            {"tablet",            "/user/hand/left/input/menu/click"},
            //{"teleport",          "/user/hand/right/input/trackpad/dpad_up"},
            //{"cycle_camera",      "/user/hand/right/input/trackpad/dpad_down"},
            {"interact_left",     "/user/hand/left/input/trigger/value"},
            {"interact_right",    "/user/hand/right/input/trigger/value"},
            {"grip_left",         "/user/hand/left/input/squeeze/click"},
            {"grip_right",        "/user/hand/right/input/squeeze/click"},
            {"jump",              "/user/hand/right/input/menu/click"},
            {"walk",              "/user/hand/left/input/trackpad"},
            //{"turn_left",         "/user/hand/right/input/trackpad/dpad_left"},
            //{"turn_right",        "/user/hand/right/input/trackpad/dpad_right"},
            {"stick_left",        "/user/hand/left/input/trackpad"},
            {"stick_right",       "/user/hand/right/input/trackpad"},
            {"stick_click_left",  "/user/hand/left/input/trackpad/click"},
            {"stick_click_right", "/user/hand/right/input/trackpad/click"},
            {"hand_pose_left",    std::string("/user/hand/left/input") + hand_pose_name},
            {"hand_pose_right",   std::string("/user/hand/right/input") + hand_pose_name},
            {"hand_haptic_left",  "/user/hand/left/output/haptic"},
            {"hand_haptic_right", "/user/hand/right/output/haptic"},
        }},
        {"/interaction_profiles/oculus/touch_controller", {
            {"tablet",            "/user/hand/left/input/y/click"},
            {"interact_left",     "/user/hand/left/input/trigger/value"},
            {"interact_right",    "/user/hand/right/input/trigger/value"},
            {"grip_left",         "/user/hand/left/input/squeeze/value"},
            {"grip_right",        "/user/hand/right/input/squeeze/value"},
            {"jump",              "/user/hand/right/input/b/click"},
            {"walk",              "/user/hand/left/input/thumbstick"},
            {"stick_left",        "/user/hand/left/input/thumbstick"},
            {"stick_right",       "/user/hand/right/input/thumbstick"},
            {"stick_click_left",  "/user/hand/left/input/thumbstick/click"},
            {"stick_click_right", "/user/hand/right/input/thumbstick/click"},
            {"hand_pose_left",    std::string("/user/hand/left/input") + hand_pose_name},
            {"hand_pose_right",   std::string("/user/hand/right/input") + hand_pose_name},
            {"hand_haptic_left",  "/user/hand/left/output/haptic"},
            {"hand_haptic_right", "/user/hand/right/output/haptic"},
        }},
        {"/interaction_profiles/microsoft/motion_controller", {
            {"tablet",            "/user/hand/left/input/menu/click"},
            {"interact_left",     "/user/hand/left/input/trigger/value"},
            {"interact_right",    "/user/hand/right/input/trigger/value"},
            {"grip_left",         "/user/hand/left/input/squeeze/click"},
            {"grip_right",        "/user/hand/right/input/squeeze/click"},
            {"jump",              "/user/hand/right/input/menu/click"},
            {"walk",              "/user/hand/left/input/thumbstick"},
            {"stick_left",        "/user/hand/left/input/thumbstick"},
            {"stick_right",       "/user/hand/right/input/thumbstick"},
            {"stick_click_left",  "/user/hand/left/input/thumbstick/click"},
            {"stick_click_right", "/user/hand/right/input/thumbstick/click"},
            {"hand_pose_left",    std::string("/user/hand/left/input") + hand_pose_name},
            {"hand_pose_right",   std::string("/user/hand/right/input") + hand_pose_name},
            {"hand_haptic_left",  "/user/hand/left/output/haptic"},
            {"hand_haptic_right", "/user/hand/right/output/haptic"},
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
        auto hand_path = (i == 0) ? "hand_pose_left" : "hand_pose_right";
        XrSpaceLocation handLocation = _actions.at(hand_path)->getPose();
        bool locationValid = (handLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0;
        if (locationValid) {
            vec3 translation = xrVecToGlm(handLocation.pose.position);
            quat rotation = xrQuatToGlm(handLocation.pose.orientation);
            auto pose = controller::Pose(translation, rotation);
            glm::mat4 handOffset = i == 0 ? glm::toMat4(leftRotationOffset) : glm::toMat4(rightRotationOffset);

            // offset constants taken from OpenComposite
            // and tweaked to fit my hands as best i could
            glm::mat4 posOffset(1.0f);
            posOffset *= glm::translate(glm::vec3(handOffset[0]) * (i == 0 ? 0.04f : -0.04f));
            posOffset *= glm::translate(glm::vec3(handOffset[1]) * -0.16f);
            posOffset *= glm::translate(glm::vec3(handOffset[2]) * -0.04f);
            _poseStateMap[i == 0 ? controller::LEFT_HAND : controller::RIGHT_HAND] =
                pose.postTransform(posOffset).postTransform(handOffset).transform(sensorToAvatar);
        }
    }

    glm::mat4 defaultHeadOffset;
    float eyeZOffset = 0.16f;
    if (inputCalibrationData.hmdAvatarAlignmentType == controller::HmdAvatarAlignmentType::Eyes) {
        // align the eyes of the user with the eyes of the avatar
        defaultHeadOffset = Matrices::Y_180 * (glm::inverse(inputCalibrationData.defaultCenterEyeMat) * inputCalibrationData.defaultHeadMat);

        // dont double up on eye offset
        eyeZOffset = 0.0f;
    } else {
        defaultHeadOffset = createMatFromQuatAndPos(-DEFAULT_AVATAR_HEAD_ROT, -DEFAULT_AVATAR_HEAD_TO_MIDDLE_EYE_OFFSET);
    }

    // try to account for weirdness with HMD view being inside the root of the head bone
    auto headCorrectionA = glm::translate(glm::vec3(0.0f, 0.16f, eyeZOffset));
    auto headCorrectionB = glm::translate(glm::vec3(0.0f, -0.2f, 0.0f));
    _poseStateMap[controller::HEAD] = _context->_lastHeadPose.postTransform(headCorrectionA).postTransform(defaultHeadOffset).postTransform(headCorrectionB).transform(sensorToAvatar);

    std::vector<std::pair<std::string, controller::StandardAxisChannel>> floatsToUpdate = {
        {"interact_left", controller::LT},
        {"interact_right", controller::RT},
        {"grip_left", controller::LEFT_GRIP},
        {"grip_right", controller::RIGHT_GRIP},
    };

    for (const auto& [name, channel] : floatsToUpdate) {
        auto action = _actions.at(name)->getFloat();
        if (action.isActive) {
            _axisStateMap[channel].value = action.currentState;
        }
    }

    std::vector<std::tuple<std::string, controller::StandardAxisChannel, controller::StandardAxisChannel>> axesToUpdate = {
        //{"stick_left", controller::LX, controller::LY},
        //{"stick_right", controller::RX, controller::RY},
        {"walk", controller::LX, controller::LY},
    };

    for (const auto& [name, x_channel, y_channel] : axesToUpdate) {
        auto action = _actions.at(name)->getVector2f();
        if (action.isActive) {
            _axisStateMap[x_channel].value = action.currentState.x;
            _axisStateMap[y_channel].value = -action.currentState.y;
        }
    }

    // INPUT TODO: more hacks
    {
        auto turn_left = _actions.at("turn_left")->getBool();
        if (turn_left.isActive && turn_left.currentState) {
            _axisStateMap[controller::RX].value -= 1.0f;
        }

        auto turn_right = _actions.at("turn_right")->getBool();
        if (turn_right.isActive && turn_right.currentState) {
            _axisStateMap[controller::RX].value += 1.0f;
        }

        // INPUT TODO: the teleport script is hardcoded to use the LY/RY axes
        auto teleport = _actions.at("teleport")->getBool();
        if (teleport.isActive && teleport.currentState) {
            _axisStateMap[controller::RY].value += 1.0f;
        }
    }

    // don't double up on the stick values between the proper actions and fallback sticks
    _axisStateMap[controller::LX].value = std::clamp(_axisStateMap[controller::LX].value, -1.0f, 1.0f);
    _axisStateMap[controller::LY].value = std::clamp(_axisStateMap[controller::LY].value, -1.0f, 1.0f);
    _axisStateMap[controller::RX].value = std::clamp(_axisStateMap[controller::RX].value, -1.0f, 1.0f);
    _axisStateMap[controller::RY].value = std::clamp(_axisStateMap[controller::RY].value, -1.0f, 1.0f);

    std::vector<std::pair<std::string, controller::StandardButtonChannel>> buttonsToUpdate = {
        {"tablet", controller::LEFT_PRIMARY_THUMB},
        {"jump", controller::RIGHT_PRIMARY_THUMB},
        {"cycle_camera", controller::LS_TOUCH},
        {"sprint", controller::DD},
    };

    for (const auto& [name, channel] : buttonsToUpdate) {
        auto action = _actions.at(name)->getBool();
        if (action.isActive && action.currentState) {
            _buttonPressedMap.insert(channel);
        }
    }

    // INPUT TODO: it's really not necessary to expose "interact click" bindings,
    // but the engine expects there to be click buttons to work properly
    {
        auto left = _actions.at("interact_left")->getFloat();
        if (left.isActive && left.currentState == 1.0f) {
            _buttonPressedMap.insert(controller::LT_CLICK);
        }

        auto right = _actions.at("interact_right")->getFloat();
        if (right.isActive && right.currentState == 1.0f) {
            _buttonPressedMap.insert(controller::RT_CLICK);
        }
    }

    // emulate dpad if the dpad extension isn't available
    emulateStickDPad();
}

void OpenXrInputPlugin::InputDevice::emulateStickDPad() {
    auto right_stick = _actions.at("stick_right")->getVector2f();

    auto left_stick_click = _actions.at("stick_click_left")->getBool();
    auto right_stick_click = _actions.at("stick_click_right")->getBool();

    auto left_needs_click = _context->_dpadNeedsClick && left_stick_click.isActive;
    auto right_needs_click = _context->_dpadNeedsClick && left_stick_click.isActive;

    auto left_clicked = left_needs_click ? left_stick_click.currentState : true;
    auto right_clicked = right_needs_click ? right_stick_click.currentState : true;

    if (right_stick.isActive && right_clicked) {
        // camera switch (right stick down)
        if (
            right_stick.currentState.y < -0.6f
            && right_stick.currentState.x > -0.4f
            && right_stick.currentState.x < 0.4f
        ) {
            _buttonPressedMap.insert(controller::LS_TOUCH);
        }

        // snap turn left
        if (
            right_stick.currentState.x < -0.6f
            && right_stick.currentState.y > -0.4f
            && right_stick.currentState.y < 0.4f
        ) {
            _axisStateMap[controller::RX].value = -1.0f;
            _axisStateMap[controller::RY].value = 0.0f;
        }

        // snap turn right
        if (
            right_stick.currentState.x > 0.6f
            && right_stick.currentState.y > -0.4f
            && right_stick.currentState.y < 0.4f
        ) {
            _axisStateMap[controller::RX].value = 1.0f;
            _axisStateMap[controller::RY].value = 0.0f;
        }

        // teleport
        if (
            right_stick.currentState.y > 0.6f
            && right_stick.currentState.x > -0.4f
            && right_stick.currentState.x < 0.4f
        ) {
            _axisStateMap[controller::RX].value = 0.0f;
            _axisStateMap[controller::RY].value = -1.0f;
        }
    }

    // set stick inputs to zero if they're not
    // clicked in or too close to the center
    if (
        !right_clicked
        || (right_stick.currentState.x > -0.6f
            && right_stick.currentState.x < 0.6f
            && right_stick.currentState.y > -0.6f
            && right_stick.currentState.y < 0.6f)
    ) {
        _axisStateMap[controller::RX].value = 0.0f;
        _axisStateMap[controller::RY].value = 0.0f;
    }

    // don't walk unless the trackpad is clicked in
    if (!left_clicked) {
        _axisStateMap[controller::LX].value = 0.0f;
        _axisStateMap[controller::LY].value = 0.0f;
    }
}
