//
// Overte OpenXR Plugin
//
// Copyright 2024 Lubosz Sarnecki
// Copyright 2024 Overte e.V.
//
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "plugins/InputPlugin.h"
#include "controllers/InputDevice.h"
#include "OpenXrContext.h"

#define HAND_COUNT 2

class OpenXrInputPlugin : public InputPlugin {
    Q_OBJECT
public:
    OpenXrInputPlugin(std::shared_ptr<OpenXrContext> c);
    bool isSupported() const override;
    const QString getName() const override { return "OpenXR"; }

    bool isHandController() const override { return true; }
    bool configurable() override { return true; }

    QString configurationLayout() override;
    void setConfigurationSettings(const QJsonObject configurationSettings) override;
    QJsonObject configurationSettings() override;
    void calibrate() override;
    bool uncalibrate() override;
    bool isHeadController() const override { return true; }

    bool activate() override;
    void deactivate() override;

    QString getDeviceName() override { return _context.get()->_systemName; }

    void pluginFocusOutEvent() override { _inputDevice->focusOutEvent(); }

    void pluginUpdate(float deltaTime, const controller::InputCalibrationData& inputCalibrationData) override;

    virtual void saveSettings() const override;
    virtual void loadSettings() override;

private:
    class Action {
    public:
        Action(std::shared_ptr<OpenXrContext> c, XrActionType type, const std::string& path) {
            _context = c;
            _path = path;
            _type = type;
        }

        bool init(XrActionSet actionSet);
        std::vector<XrActionSuggestedBinding> getBindings();
        XrActionStateFloat getFloat(uint32_t handId);
        XrActionStateBoolean getBool(uint32_t handId);
        XrSpaceLocation getPose(uint32_t handId);
        bool applyHaptic(uint32_t handId, XrDuration duration, float frequency, float amplitude);

    private:
        bool createPoseSpaces();
        XrAction _action = XR_NULL_HANDLE;
        std::shared_ptr<OpenXrContext> _context;
        std::string _path;
        XrActionType _type;
        XrSpace _poseSpaces[HAND_COUNT] = { XR_NULL_HANDLE, XR_NULL_HANDLE };
    };

    class InputDevice : public controller::InputDevice {
    public:
        InputDevice(std::shared_ptr<OpenXrContext> c);

    private:
        controller::Input::NamedVector getAvailableInputs() const override;
        QString getDefaultMappingConfig() const override;
        void update(float deltaTime, const controller::InputCalibrationData& inputCalibrationData) override;
        void focusOutEvent() override;
        bool triggerHapticPulse(float strength, float duration, uint16_t index) override;

        void partitionTouchpad(int sButton, int xAxis, int yAxis, int centerPseudoButton, int xPseudoButton, int yPseudoButton);

        mutable std::recursive_mutex _lock;
        template <typename F>
        void withLock(F&& f) {
            std::unique_lock<std::recursive_mutex> locker(_lock);
            f();
        }

        friend class OpenXrInputPlugin;

        uint32_t _trackedControllers = 0;
        XrActionSet _actionSet;
        std::map<std::string, std::shared_ptr<Action>> _actions;
        std::shared_ptr<OpenXrContext> _context;
        bool _actionsInitialized = false;

        bool initActions();
        bool initBindings(const std::string& profileName, const std::vector<std::string>& actionsToBind);
    };

    bool _registeredWithInputMapper = false;
    std::shared_ptr<OpenXrContext> _context;
    std::shared_ptr<InputDevice> _inputDevice;
};
