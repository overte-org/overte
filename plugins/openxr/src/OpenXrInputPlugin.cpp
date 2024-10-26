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

OpenXrInputPlugin::InputDevice::InputDevice(std::shared_ptr<OpenXrContext> c) : controller::InputDevice("Vive") {
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

    if (!_actions.at("/output/haptic")->applyHaptic(index, xrDuration, XR_FREQUENCY_UNSPECIFIED, 0.5f * strength)) {
        qCCritical(xr_input_cat, "Failed to apply haptic feedback!");
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

    QString name = QString::fromStdString(_path);
    name.replace("/input/", "");
    name.replace("/", "-");
    strcpy(info.actionName, name.toUtf8().data());
    name.replace("-", " ");
    strcpy(info.localizedActionName, name.toUtf8().data());

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

const std::vector<std::string> HAND_PATHS = { "left", "right" };

std::vector<XrActionSuggestedBinding> OpenXrInputPlugin::Action::getBindings() {
    assert(_action != XR_NULL_HANDLE);

    std::vector<XrActionSuggestedBinding> bindings;
    for (uint32_t i = 0; i < HAND_COUNT; i++) {
        XrPath path;
        std::string pathString = "/user/hand/" + HAND_PATHS[i] + _path;
        xrStringToPath(_context->_instance, pathString.c_str(), &path);
        XrActionSuggestedBinding binding = { .action = _action, .binding = path };
        bindings.push_back(binding);
    }
    return bindings;
}

XrActionStateFloat OpenXrInputPlugin::Action::getFloat(uint32_t handId) {
    XrActionStateFloat state = {
        .type = XR_TYPE_ACTION_STATE_FLOAT,
    };

    XrActionStateGetInfo info = {
        .type = XR_TYPE_ACTION_STATE_GET_INFO,
        .action = _action,
        .subactionPath = _context->_handPaths[handId],
    };

    XrResult result = xrGetActionStateFloat(_context->_session, &info, &state);
    xrCheck(_context->_instance, result, "Failed to get float state!");

    return state;
}

XrActionStateBoolean OpenXrInputPlugin::Action::getBool(uint32_t handId) {
    XrActionStateBoolean state = {
        .type = XR_TYPE_ACTION_STATE_BOOLEAN,
    };

    XrActionStateGetInfo info = {
        .type = XR_TYPE_ACTION_STATE_GET_INFO,
        .action = _action,
        .subactionPath = _context->_handPaths[handId],
    };

    XrResult result = xrGetActionStateBoolean(_context->_session, &info, &state);
    xrCheck(_context->_instance, result, "Failed to get float state!");

    return state;
}

XrSpaceLocation OpenXrInputPlugin::Action::getPose(uint32_t handId) {
    XrActionStatePose state = {
        .type = XR_TYPE_ACTION_STATE_POSE,
    };
    XrActionStateGetInfo info = {
        .type = XR_TYPE_ACTION_STATE_GET_INFO,
        .action = _action,
        .subactionPath = _context->_handPaths[handId],
    };

    XrResult result = xrGetActionStatePose(_context->_session, &info, &state);
    xrCheck(_context->_instance, result, "failed to get pose value!");

    XrSpaceLocation location = {
        .type = XR_TYPE_SPACE_LOCATION,
    };

    if (_context->_lastPredictedDisplayTime.has_value()) {
        result = xrLocateSpace(_poseSpaces[handId], _context->_stageSpace, _context->_lastPredictedDisplayTime.value(), &location);
        xrCheck(_context->_instance, result, "Failed to locate hand space!");
    }

    return location;
}

bool OpenXrInputPlugin::Action::applyHaptic(uint32_t handId, XrDuration duration, float frequency, float amplitude) {
    XrHapticVibration vibration = {
        .type = XR_TYPE_HAPTIC_VIBRATION,
        .duration = duration,
        .frequency = frequency,
        .amplitude = amplitude,
    };

    XrHapticActionInfo haptic_action_info = {
        .type = XR_TYPE_HAPTIC_ACTION_INFO,
        .action = _action,
        .subactionPath = _context->_handPaths[handId],
    };
    XrResult result = xrApplyHapticFeedback(_context->_session, &haptic_action_info, (const XrHapticBaseHeader*)&vibration);

    return xrCheck(_context->_instance, result, "Failed to apply haptic feedback!");
}

bool OpenXrInputPlugin::Action::createPoseSpaces() {
    assert(_action != XR_NULL_HANDLE);

    for (int hand = 0; hand < HAND_COUNT; hand++) {
        XrActionSpaceCreateInfo info = {
            .type = XR_TYPE_ACTION_SPACE_CREATE_INFO,
            .action = _action,
            .subactionPath = _context->_handPaths[hand],
            .poseInActionSpace = XR_INDENTITY_POSE,
        };

        XrResult result = xrCreateActionSpace(_context->_session, &info, &_poseSpaces[hand]);
        if (!xrCheck(_context->_instance, result, "Failed to create hand pose space"))
            return false;
    }

    return true;
}

bool OpenXrInputPlugin::InputDevice::initBindings(const std::string& profileName,
                                                  const std::vector<std::string>& actionsToBind) {
    XrPath profilePath;
    XrResult result = xrStringToPath(_context->_instance, profileName.c_str(), &profilePath);
    if (!xrCheck(_context->_instance, result, "Failed to get interaction profile"))
        return false;

    std::vector<XrActionSuggestedBinding> bindings;
    for (const std::string& path : actionsToBind) {
        if (!_actions.contains(path)) {
            qCWarning(xr_input_cat, "%s has unbound input %s", profileName.c_str(), path.c_str());
            continue;
        }
        std::vector<XrActionSuggestedBinding> actionBindings = _actions.at(path)->getBindings();
        bindings.insert(std::end(bindings), std::begin(actionBindings), std::end(actionBindings));
    }

    const XrInteractionProfileSuggestedBinding suggestedBinding = {
        .type = XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING,
        .interactionProfile = profilePath,
        .countSuggestedBindings = (uint32_t)bindings.size(),
        .suggestedBindings = bindings.data(),
    };

    result = xrSuggestInteractionProfileBindings(_context->_instance, &suggestedBinding);

    return xrCheck(_context->_instance, result, "Failed to suggest bindings");
}

controller::Input::NamedVector OpenXrInputPlugin::InputDevice::getAvailableInputs() const {
    using namespace controller;

    // clang-format off
    QVector<Input::NamedPair> availableInputs{
        makePair(HEAD, "Head"),
        makePair(LEFT_HAND, "LeftHand"),
        makePair(RIGHT_HAND, "RightHand"),

        // Trackpad analogs
        makePair(LX, "LX"),
        makePair(LY, "LY"),
        makePair(RX, "RX"),
        makePair(RY, "RY"),

        // capacitive touch on the touch pad
        makePair(LS_TOUCH, "LSTouch"),
        makePair(RS_TOUCH, "RSTouch"),

        // touch pad press
        makePair(LS, "LS"),
        makePair(RS, "RS"),
        // Differentiate where we are in the touch pad click
        makePair(LS_CENTER, "LSCenter"),
        makePair(LS_X, "LSX"),
        makePair(LS_Y, "LSY"),
        makePair(RS_CENTER, "RSCenter"),
        makePair(RS_X, "RSX"),
        makePair(RS_Y, "RSY"),

        // triggers
        makePair(LT, "LT"),
        makePair(RT, "RT"),
        // Trigger clicks
        makePair(LT_CLICK, "LTClick"),
        makePair(RT_CLICK, "RTClick"),

        // low profile side grip button.
        makePair(LEFT_GRIP, "LeftGrip"),
        makePair(RIGHT_GRIP, "RightGrip"),

        makePair(LEFT_PRIMARY_THUMB, "LeftPrimaryThumb"),
        makePair(RIGHT_PRIMARY_THUMB, "RightPrimaryThumb"),
        makePair(LEFT_SECONDARY_THUMB, "LeftSecondaryThumb"),
        makePair(RIGHT_SECONDARY_THUMB, "RightSecondaryThumb"),

        makePair(LEFT_SECONDARY_THUMB, "LeftApplicationMenu"),
        makePair(RIGHT_SECONDARY_THUMB, "RightApplicationMenu"),
    };
    // clang-format on

    return availableInputs;
}

QString OpenXrInputPlugin::InputDevice::getDefaultMappingConfig() const {
    // FIXME: for some reason vive works but openxr_generic breaks the vertical trackpad?
    return PathUtils::resourcesPath() + "/controllers/vive.json";
}

bool OpenXrInputPlugin::InputDevice::initActions() {
    if (_actionsInitialized)
        return true;

    assert(_context->_session != XR_NULL_HANDLE);

    XrInstance instance = _context->_instance;

    XrActionSetCreateInfo actionSetInfo = {
        .type = XR_TYPE_ACTION_SET_CREATE_INFO,
        .actionSetName = "action_set",
        .localizedActionSetName = "Action Set",
        .priority = 0,
    };
    XrResult result = xrCreateActionSet(instance, &actionSetInfo, &_actionSet);
    if (!xrCheck(instance, result, "Failed to create action set."))
        return false;

    // clang-format off
    std::map<std::string, XrActionType> actionsToInit = {
        { "/output/haptic", XR_ACTION_TYPE_VIBRATION_OUTPUT },
        { "/input/grip/pose", XR_ACTION_TYPE_POSE_INPUT },

        // click but pretend it's a float for the trigger actions
        { "/input/select/click", XR_ACTION_TYPE_FLOAT_INPUT },
        { "/input/menu/click", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { "/input/system/click", XR_ACTION_TYPE_BOOLEAN_INPUT },

        { "/input/trackpad/x", XR_ACTION_TYPE_FLOAT_INPUT },
        { "/input/trackpad/y", XR_ACTION_TYPE_FLOAT_INPUT },
        { "/input/trackpad/touch", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { "/input/trackpad/click", XR_ACTION_TYPE_BOOLEAN_INPUT },

        { "/input/thumbstick/x", XR_ACTION_TYPE_FLOAT_INPUT },
        { "/input/thumbstick/y", XR_ACTION_TYPE_FLOAT_INPUT },
        { "/input/thumbstick/touch", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { "/input/thumbstick/click", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { "/input/a/click", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { "/input/b/click", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { "/input/a/touch", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { "/input/b/touch", XR_ACTION_TYPE_BOOLEAN_INPUT },

        { "/input/squeeze/click", XR_ACTION_TYPE_FLOAT_INPUT },
        { "/input/trigger/value", XR_ACTION_TYPE_FLOAT_INPUT },
        { "/input/trigger/click", XR_ACTION_TYPE_BOOLEAN_INPUT },
        { "/input/trigger/touch", XR_ACTION_TYPE_BOOLEAN_INPUT },
    };
    // clang-format on

    for (const auto& [path, type] : actionsToInit) {
        std::shared_ptr<Action> action = std::make_shared<Action>(_context, type, path);
        if (!action->init(_actionSet)) {
            qCCritical(xr_input_cat, "Creating action %s failed!", path.c_str());
        } else {
            _actions.emplace(path, action);
        }
    }

    // Khronos Simple Controller
    std::vector<std::string> simpleBindings = {
        "/input/select/click",
        "/input/menu/click",
        "/input/grip/pose",
        "/output/haptic",
    };

    if (!initBindings("/interaction_profiles/khr/simple_controller", simpleBindings)) {
        qCCritical(xr_input_cat, "Failed to init bindings for khr/simple_controller");
    }

    // HTC Vive
    std::vector<std::string> viveBindings = {
        "/input/system/click",
        "/input/squeeze/click",
        "/input/menu/click",
        "/input/trigger/click",
        "/input/trigger/value",
        "/input/trackpad/x",
        "/input/trackpad/y",
        "/input/trackpad/click",
        "/input/trackpad/touch",
        "/input/grip/pose",
        "/output/haptic",
    };

    if (!initBindings("/interaction_profiles/htc/vive_controller", viveBindings)) {
        qCCritical(xr_input_cat, "Failed to init bindings for htc/vive_controller");
    }

    // Valve Index Controller
    // clang-format off
    std::vector<std::string> indexBindings = {
        "/input/grip/pose",
        "/input/thumbstick/x",
        "/input/thumbstick/y",
        "/input/thumbstick/touch",
        "/input/thumbstick/click",
        "/input/a/click",
        "/input/a/touch",
        "/input/b/click",
        "/input/b/touch",
        "/input/trigger/value",
        "/input/trigger/click",
        "/input/trigger/touch",
        "/output/haptic",
        "/input/system/click",
    };
    // clang-format on

    if (!initBindings("/interaction_profiles/valve/index_controller", indexBindings)) {
        qCCritical(xr_input_cat, "Failed to init bindings for valve/index_controller");
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

    if (!initActions()) {
        qCCritical(xr_input_cat, "Could not initialize actions!");
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
        XrSpaceLocation handLocation = _actions.at("/input/grip/pose")->getPose(i);
        bool locationValid = (handLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0;
        if (locationValid) {
            vec3 translation = xrVecToGlm(handLocation.pose.position);
            quat rotation = xrQuatToGlm(handLocation.pose.orientation);
            auto pose = controller::Pose(translation, rotation);
            glm::mat4 handOffset = i == 0 ? glm::toMat4(leftRotationOffset) : glm::toMat4(rightRotationOffset);

            // offset constants taken from OpenComposite
            // (apparently the openvr plugin uses similar constants?)
            glm::mat4 posOffset(1.0f);
            posOffset *= glm::translate(glm::vec3(handOffset[0]) * (i == 0 ? 0.04f : -0.04f));
            posOffset *= glm::translate(glm::vec3(handOffset[1]) * -0.15f);
            posOffset *= glm::translate(glm::vec3(handOffset[2]) * 0.02f);
            _poseStateMap[i == 0 ? controller::LEFT_HAND : controller::RIGHT_HAND] =
                pose.postTransform(posOffset).postTransform(handOffset).transform(sensorToAvatar);
        }
    }

    glm::mat4 defaultHeadOffset = createMatFromQuatAndPos(-DEFAULT_AVATAR_HEAD_ROT, -DEFAULT_AVATAR_HEAD_TO_MIDDLE_EYE_OFFSET);
    _poseStateMap[controller::HEAD] = _context->_lastHeadPose.postTransform(defaultHeadOffset).transform(sensorToAvatar);

    std::vector<std::pair<controller::StandardAxisChannel, std::string>> axesToUpdate[2] = {
        {
            { controller::LT, "/input/trigger/value" },
            { controller::LT, "/input/select/click" },
            { controller::LEFT_GRIP, "/input/squeeze/click" },
            { controller::LX, "/input/trackpad/x" },
            { controller::LY, "/input/trackpad/y" },
            { controller::LX, "/input/thumbstick/x" },
            { controller::LY, "/input/thumbstick/y" },
        },
        {
            { controller::RT, "/input/trigger/value" },
            { controller::RT, "/input/select/click" },
            { controller::RIGHT_GRIP, "/input/squeeze/click" },
            { controller::RX, "/input/trackpad/x" },
            { controller::RY, "/input/trackpad/y" },
            { controller::RX, "/input/thumbstick/x" },
            { controller::RY, "/input/thumbstick/y" },
        },
    };

    for (uint32_t i = 0; i < HAND_COUNT; i++) {
        for (const auto& [channel, path] : axesToUpdate[i]) {
            auto action = _actions.at(path)->getFloat(i);
            if (action.isActive) {
                _axisStateMap[channel].value = action.currentState;
            }
        }
    }

    std::vector<std::pair<controller::StandardButtonChannel, std::string>> buttonsToUpdate[2] = {
        {
            { controller::LS, "/input/trackpad/click" },
            { controller::LS_TOUCH, "/input/trackpad/touch" },
            { controller::LS, "/input/thumbstick/click" },
            { controller::LS_TOUCH, "/input/thumbstick/touch" },
            { controller::LT_CLICK, "/input/trigger/click" },
            { controller::LEFT_PRIMARY_THUMB, "/input/a/click" },
            { controller::LEFT_PRIMARY_THUMB, "/input/system/click" },
            { controller::LEFT_SECONDARY_THUMB, "/input/b/click" },
            { controller::LEFT_SECONDARY_THUMB, "/input/menu/click" },
        },
        {
            { controller::RS, "/input/trackpad/click" },
            { controller::RS_TOUCH, "/input/trackpad/touch" },
            { controller::RS, "/input/thumbstick/click" },
            { controller::RS_TOUCH, "/input/thumbstick/touch" },
            { controller::RT_CLICK, "/input/trigger/click" },
            { controller::RIGHT_PRIMARY_THUMB, "/input/a/click" },
            { controller::RIGHT_PRIMARY_THUMB, "/input/system/click" },
            { controller::RIGHT_SECONDARY_THUMB, "/input/b/click" },
            { controller::RIGHT_SECONDARY_THUMB, "/input/menu/click" },
        },
    };

    for (uint32_t i = 0; i < HAND_COUNT; i++) {
        for (const auto& [channel, path] : buttonsToUpdate[i]) {
            auto action = _actions.at(path)->getBool(i);
            if (action.isActive && action.currentState) {
                _buttonPressedMap.insert(channel);
            }
        }
    }

    // TODO: better alternative to having every controller emulate a vive one
    partitionTouchpad(controller::LS, controller::LX, controller::LY, controller::LS_CENTER, controller::LS_X, controller::LS_Y);
    partitionTouchpad(controller::RS, controller::RX, controller::RY, controller::RS_CENTER, controller::RS_X, controller::RS_Y);
}

// copied from openvr/ViveControllerManager
void OpenXrInputPlugin::InputDevice::partitionTouchpad(int sButton, int xAxis, int yAxis, int centerPseudoButton, int xPseudoButton, int yPseudoButton) {
    // Populate the L/RS_CENTER/OUTER pseudo buttons, corresponding to a partition of the L/RS space based on the X/Y values.
    const float CENTER_DEADBAND = 0.6f;
    const float DIAGONAL_DIVIDE_IN_RADIANS = PI / 4.0f;
    if (_buttonPressedMap.find(sButton) != _buttonPressedMap.end()) {
        float absX = abs(_axisStateMap[xAxis].value);
        float absY = abs(_axisStateMap[yAxis].value);
        glm::vec2 cartesianQuadrantI(absX, absY);
        float angle = glm::atan(cartesianQuadrantI.y / cartesianQuadrantI.x);
        float radius = glm::length(cartesianQuadrantI);
        bool isCenter = radius < CENTER_DEADBAND;
        _buttonPressedMap.insert(isCenter ? centerPseudoButton : ((angle < DIAGONAL_DIVIDE_IN_RADIANS) ? xPseudoButton :yPseudoButton));
    }
}
