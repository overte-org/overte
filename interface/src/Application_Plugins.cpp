//
//  Application_Plugins.cpp
//  interface/src
//
//  Split from Application.cpp by HifiExperiments on 3/30/24
//  Created by Andrzej Kapolka on 5/10/13.
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors.
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "Application.h"

#include <QtCore/QCommandLineParser>

#include <input-plugins/InputPlugin.h>
#include <display-plugins/DisplayPlugin.h>
#include <display-plugins/hmd/HmdDisplayPlugin.h>
#include <OffscreenUi.h>
#include <plugins/PluginManager.h>
#include <plugins/PluginUtils.h>
#include <scripting/HMDScriptingInterface.h>
#include <UserActivityLogger.h>

#include "AudioClient.h"
#include "InterfaceLogging.h"
#include "Menu.h"

static const int INTERVAL_TO_CHECK_HMD_WORN_STATUS = 500;  // milliseconds
static const QString ACTIVE_DISPLAY_PLUGIN_SETTING_NAME = "activeDisplayPlugin";
static const QString DESKTOP_DISPLAY_PLUGIN_NAME = "Desktop";

// Statically provided display and input plugins
extern DisplayPluginList getDisplayPlugins();
extern InputPluginList getInputPlugins();
extern void saveInputPluginSettings(const InputPluginList& plugins);

void Application::initializePluginManager(const QCommandLineParser& parser) {
    DependencyManager::set<PluginManager>();
    auto pluginManager = PluginManager::getInstance();

    // To avoid any confusion: the getInputPlugins and getDisplayPlugins are not the ones
    // from PluginManager, but functions exported by input-plugins/InputPlugin.cpp and
    // display-plugins/DisplayPlugin.cpp.
    //
    // These functions provide the plugin manager with static default plugins.
    pluginManager->setInputPluginProvider([] { return getInputPlugins(); });
    pluginManager->setDisplayPluginProvider([] { return getDisplayPlugins(); });
    pluginManager->setInputPluginSettingsPersister([](const InputPluginList& plugins) { saveInputPluginSettings(plugins); });

    // This must be a member function -- PluginManager must exist, and for that
    // QApplication must exist, or it can't find the plugin path, as QCoreApplication:applicationDirPath
    // won't work yet.

    if (parser.isSet("display")) {
        auto preferredDisplays = parser.value("display").split(',', Qt::SkipEmptyParts);
        qInfo() << "Setting prefered display plugins:" << preferredDisplays;
        PluginManager::getInstance()->setPreferredDisplayPlugins(preferredDisplays);
    }

    if (parser.isSet("disableDisplayPlugins")) {
        auto disabledDisplays = parser.value("disableDisplayPlugins").split(',', Qt::SkipEmptyParts);
        qInfo() << "Disabling following display plugins:"  << disabledDisplays;
        PluginManager::getInstance()->disableDisplays(disabledDisplays);
    }

    if (parser.isSet("disableInputPlugins")) {
        auto disabledInputs = parser.value("disableInputPlugins").split(',', Qt::SkipEmptyParts);
        qInfo() << "Disabling following input plugins:" << disabledInputs;
        PluginManager::getInstance()->disableInputs(disabledInputs);
    }
}

void Application::shutdownPlugins() {}

void Application::initializeDisplayPlugins() {
    const auto& displayPlugins = PluginManager::getInstance()->getDisplayPlugins();
    Setting::Handle<QString> activeDisplayPluginSetting { ACTIVE_DISPLAY_PLUGIN_SETTING_NAME, displayPlugins.at(0)->getName() };
    auto lastActiveDisplayPluginName = activeDisplayPluginSetting.get();

    auto defaultDisplayPlugin = displayPlugins.at(0);
    // One time initialization code
    DisplayPluginPointer targetDisplayPlugin;
    for(const auto& displayPlugin : displayPlugins) {
        displayPlugin->setContext(_graphicsEngine->getGPUContext());
        if (displayPlugin->getName() == lastActiveDisplayPluginName) {
            targetDisplayPlugin = displayPlugin;
        }

        if (!_autoSwitchDisplayModeSupportedHMDPlugin) {
            if (displayPlugin->isHmd() && displayPlugin->getSupportsAutoSwitch()) {
                _autoSwitchDisplayModeSupportedHMDPlugin = displayPlugin;
                _autoSwitchDisplayModeSupportedHMDPluginName = _autoSwitchDisplayModeSupportedHMDPlugin->getName();
                _previousHMDWornStatus = _autoSwitchDisplayModeSupportedHMDPlugin->isDisplayVisible() && _autoSwitchDisplayModeSupportedHMDPlugin->isActive();
            }
        }

        QObject::connect(displayPlugin.get(), &DisplayPlugin::recommendedFramebufferSizeChanged,
            [this](const QSize& size) { resizeGL(); });
        QObject::connect(displayPlugin.get(), &DisplayPlugin::resetSensorsRequested, this, &Application::requestReset);

        if (displayPlugin->isHmd()) {
            auto hmdDisplayPlugin = dynamic_cast<HmdDisplayPlugin*>(displayPlugin.get());
            QObject::connect(hmdDisplayPlugin, &HmdDisplayPlugin::hmdMountedChanged,
                DependencyManager::get<HMDScriptingInterface>().data(), &HMDScriptingInterface::mountedChanged);
            QObject::connect(hmdDisplayPlugin, &HmdDisplayPlugin::hmdVisibleChanged, this, &Application::hmdVisibleChanged);
        }
    }

    // The default display plugin needs to be activated first, otherwise the display plugin thread
    // may be launched by an external plugin, which is bad
    setDisplayPlugin(defaultDisplayPlugin);

    // Now set the desired plugin if it's not the same as the default plugin
    if (targetDisplayPlugin && (targetDisplayPlugin != defaultDisplayPlugin)) {
        setDisplayPlugin(targetDisplayPlugin);
    }

    if (_autoSwitchDisplayModeSupportedHMDPlugin) {
        if (getActiveDisplayPlugin() != _autoSwitchDisplayModeSupportedHMDPlugin && !_autoSwitchDisplayModeSupportedHMDPlugin->isSessionActive()) {
            startHMDStandBySession();
        }
        // Poll periodically to check whether the user has worn HMD or not. Switch Display mode accordingly.
        // If the user wears HMD then switch to VR mode. If the user removes HMD then switch to Desktop mode.
        QTimer* autoSwitchDisplayModeTimer = new QTimer(this);
        connect(autoSwitchDisplayModeTimer, SIGNAL(timeout()), this, SLOT(switchDisplayMode()));
        autoSwitchDisplayModeTimer->start(INTERVAL_TO_CHECK_HMD_WORN_STATUS);
    }

    // Submit a default frame to render until the engine starts up
    updateRenderArgs(0.0f);
}

DisplayPluginPointer Application::getActiveDisplayPlugin() const {
    if (QThread::currentThread() != thread()) {
        std::unique_lock<std::mutex> lock(_displayPluginLock);
        return _displayPlugin;
    }

    if (!_aboutToQuit && !_displayPlugin) {
        const_cast<Application*>(this)->updateDisplayMode();
        Q_ASSERT(_displayPlugin);
    }
    return _displayPlugin;
}

void Application::setActiveDisplayPlugin(const QString& pluginName) {
    DisplayPluginPointer newDisplayPlugin;
    for (const DisplayPluginPointer& displayPlugin : PluginManager::getInstance()->getDisplayPlugins()) {
        QString name = displayPlugin->getName();
        if (pluginName == name) {
            newDisplayPlugin = displayPlugin;
            break;
        }
    }

    if (newDisplayPlugin) {
        setDisplayPlugin(newDisplayPlugin);
    }
}

glm::uvec2 Application::getUiSize() const {
    static const uint MIN_SIZE = 1;
    glm::uvec2 result(MIN_SIZE);
    if (_displayPlugin) {
        result = getActiveDisplayPlugin()->getRecommendedUiSize();
    }
    return result;
}

QRect Application::getRecommendedHUDRect() const {
    auto uiSize = getUiSize();
    QRect result(0, 0, uiSize.x, uiSize.y);
    if (_displayPlugin) {
        result = getActiveDisplayPlugin()->getRecommendedHUDRect();
    }
    return result;
}

glm::vec2 Application::getDeviceSize() const {
    static const int MIN_SIZE = 1;
    glm::vec2 result(MIN_SIZE);
    if (_displayPlugin) {
        result = getActiveDisplayPlugin()->getRecommendedRenderSize();
    }
    return result;
}

bool Application::isThrottleRendering() const {
    if (_displayPlugin) {
        return getActiveDisplayPlugin()->isThrottled();
    }
    return false;
}

float Application::getTargetRenderFrameRate() const {
    return getActiveDisplayPlugin()->getTargetFrameRate();
}

bool Application::hasRiftControllers() {
    return PluginUtils::isOculusTouchControllerAvailable();
}

bool Application::hasViveControllers() {
    return PluginUtils::isViveControllerAvailable();
}

bool Application::isHMDMode() const {
    return getActiveDisplayPlugin()->isHmd();
}

mat4 Application::getHMDSensorPose() const {
    if (isHMDMode()) {
        return getActiveDisplayPlugin()->getHeadPose();
    }
    return mat4();
}

mat4 Application::getEyeOffset(int eye) const {
    // FIXME invert?
    return getActiveDisplayPlugin()->getEyeToHeadTransform((Eye)eye);
}

mat4 Application::getEyeProjection(int eye) const {
    QMutexLocker viewLocker(&_viewMutex);
    if (isHMDMode()) {
        return getActiveDisplayPlugin()->getEyeProjection((Eye)eye, _viewFrustum.getProjection());
    }
    return _viewFrustum.getProjection();
}

// resentSensors() is a bit of vestigial feature. It used to be used for Oculus DK2 to recenter the view around
// the current head orientation.  With the introduction of "room scale" tracking we no longer need that particular
// feature.  However, we still use this to reset face trackers, eye trackers, audio and to optionally re-load the avatar
// rig and animations from scratch.
void Application::resetSensors(bool andReload) {
    _overlayConductor.centerUI();
    getActiveDisplayPlugin()->resetSensors();
    getMyAvatar()->reset(true, andReload);
    QMetaObject::invokeMethod(DependencyManager::get<AudioClient>().data(), "reset", Qt::QueuedConnection);
}

void Application::updateDisplayMode() {
    // Unsafe to call this method from anything but the main thread
    if (QThread::currentThread() != thread()) {
        qFatal("Attempted to switch display plugins from a non-main thread");
    }

    // Once time initialization code that depends on the UI being available
    const auto& displayPlugins = getDisplayPlugins();

    // Default to the first item on the list, in case none of the menu items match

    DisplayPluginPointer newDisplayPlugin = displayPlugins.at(0);
    auto menu = getPrimaryMenu();
    if (menu) {
        for (const auto& displayPlugin : PluginManager::getInstance()->getDisplayPlugins()) {
            QString name = displayPlugin->getName();
            QAction* action = menu->getActionForOption(name);
            // Menu might have been removed if the display plugin lost
            if (!action) {
                continue;
            }
            if (action->isChecked()) {
                newDisplayPlugin = displayPlugin;
                break;
            }
        }
    }

    if (newDisplayPlugin == _displayPlugin) {
        return;
    }

    setDisplayPlugin(newDisplayPlugin);
}

void Application::switchDisplayMode() {
    if (!_autoSwitchDisplayModeSupportedHMDPlugin) {
        return;
    }

    bool currentHMDWornStatus = _autoSwitchDisplayModeSupportedHMDPlugin->isDisplayVisible();
    if (currentHMDWornStatus != _previousHMDWornStatus) {
        // Switch to respective mode as soon as currentHMDWornStatus changes
        if (currentHMDWornStatus) {
            qCDebug(interfaceapp) << "Switching from Desktop to HMD mode";
            endHMDSession();
            setActiveDisplayPlugin(_autoSwitchDisplayModeSupportedHMDPluginName);
        } else {
            qCDebug(interfaceapp) << "Switching from HMD to desktop mode";
            setActiveDisplayPlugin(DESKTOP_DISPLAY_PLUGIN_NAME);
            startHMDStandBySession();
        }
    }
    _previousHMDWornStatus = currentHMDWornStatus;
}

void Application::setDisplayPlugin(DisplayPluginPointer newDisplayPlugin) {
    if (newDisplayPlugin == _displayPlugin) {
        return;
    }

    // FIXME don't have the application directly set the state of the UI,
    // instead emit a signal that the display plugin is changing and let
    // the desktop lock itself.  Reduces coupling between the UI and display
    // plugins
    auto offscreenUi = getOffscreenUI();
    auto desktop = offscreenUi ? offscreenUi->getDesktop() : nullptr;
    auto menu = Menu::getInstance();

    // Make the switch atomic from the perspective of other threads
    {
        std::unique_lock<std::mutex> lock(_displayPluginLock);
        bool wasRepositionLocked = false;
        if (desktop) {
            // Tell the desktop to no reposition (which requires plugin info), until we have set the new plugin, below.
            wasRepositionLocked = desktop->property("repositionLocked").toBool();
            desktop->setProperty("repositionLocked", true);
        }

        if (_displayPlugin) {
            disconnect(_displayPlugin.get(), &DisplayPlugin::presented, this, &Application::onPresent);
            _displayPlugin->deactivate();
        }

        auto oldDisplayPlugin = _displayPlugin;
        bool active = newDisplayPlugin->activate();

        if (!active) {
            const DisplayPluginList& displayPlugins = PluginManager::getInstance()->getDisplayPlugins();

            // If the new plugin fails to activate, fallback to last display
            qWarning() << "Failed to activate display: " << newDisplayPlugin->getName();
            newDisplayPlugin = oldDisplayPlugin;

            if (newDisplayPlugin) {
                qWarning() << "Falling back to last display: " << newDisplayPlugin->getName();
                active = newDisplayPlugin->activate();
            }

            // If there is no last display, or
            // If the last display fails to activate, fallback to desktop
            if (!active) {
                newDisplayPlugin = displayPlugins.at(0);
                qWarning() << "Falling back to display: " << newDisplayPlugin->getName();
                active = newDisplayPlugin->activate();
            }

            if (!active) {
                qFatal("Failed to activate fallback plugin");
            }
        }

        if (offscreenUi) {
            offscreenUi->resize(fromGlm(newDisplayPlugin->getRecommendedUiSize()));
        }
        getApplicationCompositor().setDisplayPlugin(newDisplayPlugin);
        _displayPlugin = newDisplayPlugin;
        connect(_displayPlugin.get(), &DisplayPlugin::presented, this, &Application::onPresent, Qt::DirectConnection);
        if (desktop) {
            desktop->setProperty("repositionLocked", wasRepositionLocked);
        }

        RefreshRateManager& refreshRateManager = getRefreshRateManager();
        refreshRateManager.setRefreshRateOperator(OpenGLDisplayPlugin::getRefreshRateOperator());
        bool isHmd = newDisplayPlugin->isHmd();
        RefreshRateManager::UXMode uxMode = isHmd ? RefreshRateManager::UXMode::VR :
            RefreshRateManager::UXMode::DESKTOP;

        refreshRateManager.setUXMode(uxMode);
    }

    bool isHmd = _displayPlugin->isHmd();
    qCDebug(interfaceapp) << "Entering into" << (isHmd ? "HMD" : "Desktop") << "Mode";

    // Only log/emit after a successful change
    UserActivityLogger::getInstance().logAction("changed_display_mode", {
        { "previous_display_mode", _displayPlugin ? _displayPlugin->getName() : "" },
        { "display_mode", newDisplayPlugin ? newDisplayPlugin->getName() : "" },
        { "hmd", isHmd }
    });
    emit activeDisplayPluginChanged();

    // reset the avatar, to set head and hand palms back to a reasonable default pose.
    getMyAvatar()->reset(false);

    // switch to first person if entering hmd and setting is checked
    if (menu) {
        QAction* action = menu->getActionForOption(newDisplayPlugin->getName());
        if (action) {
            action->setChecked(true);
        }

        if (isHmd && menu->isOptionChecked(MenuOption::FirstPersonHMD)) {
            menu->setIsOptionChecked(MenuOption::FirstPersonLookAt, true);
            cameraMenuChanged();
        }

        // Remove the selfie camera options from menu if in HMD mode
        auto selfieAction = menu->getActionForOption(MenuOption::SelfieCamera);
        selfieAction->setVisible(!isHmd);
    }

    Q_ASSERT_X(_displayPlugin, "Application::updateDisplayMode", "could not find an activated display plugin");
}

void Application::startHMDStandBySession() {
    _autoSwitchDisplayModeSupportedHMDPlugin->startStandBySession();
}

void Application::endHMDSession() {
    _autoSwitchDisplayModeSupportedHMDPlugin->endSession();
}
