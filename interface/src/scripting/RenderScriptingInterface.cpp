//
//  Created by Sam Gondelman on 5/16/19
//  Copyright 2013-2019 High Fidelity, Inc.
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//
#include "RenderScriptingInterface.h"

#include <ScriptEngineCast.h>

#include "LightingModel.h"
#include <QScreen>
#include "ScreenName.h"

STATIC_SCRIPT_TYPES_INITIALIZER((+[](ScriptManager* manager){
    auto scriptEngine = manager->engine().get();

    scriptRegisterMetaType<RenderScriptingInterface::RenderMethod, scriptValueFromEnumClass<RenderScriptingInterface::RenderMethod>, scriptValueToEnumClass<RenderScriptingInterface::RenderMethod> >(scriptEngine, "RenderMethod");
    scriptRegisterMetaType<AntialiasingConfig::Mode, scriptValueFromEnumClass<AntialiasingConfig::Mode>, scriptValueToEnumClass<AntialiasingConfig::Mode> >(scriptEngine, "Mode");
}));

STATIC_SCRIPT_INITIALIZER(+[](ScriptManager* manager){
    auto scriptEngine = manager->engine().get();

    scriptEngine->registerEnum("Render.RenderMethod",QMetaEnum::fromType<RenderScriptingInterface::RenderMethod>());
    scriptEngine->registerEnum("AntialiasingMode",QMetaEnum::fromType<AntialiasingConfig::Mode>());
});

RenderScriptingInterface* RenderScriptingInterface::getInstance() {
    static RenderScriptingInterface sharedInstance;
    return &sharedInstance;
}

std::once_flag RenderScriptingInterface::registry_flag;

RenderScriptingInterface::RenderScriptingInterface() {
    std::call_once(registry_flag, [] {
        qmlRegisterType<RenderScriptingInterface>("RenderEnums", 1, 0, "RenderEnums");
    });
}


void RenderScriptingInterface::loadSettings() {
    _renderSettingLock.withReadLock([&] {
        _renderMethod = (_renderMethodSetting.get());
        _shadowsEnabled = (_shadowsEnabledSetting.get());
        _ambientOcclusionEnabled = (_ambientOcclusionEnabledSetting.get());
        //_antialiasingMode = (_antialiasingModeSetting.get());
        _antialiasingMode = static_cast<AntialiasingConfig::Mode>(_antialiasingModeSetting.get());
        _viewportResolutionScale = (_viewportResolutionScaleSetting.get());
        _fullScreenScreen = (_fullScreenScreenSetting.get());
    });

    // If full screen screen is not initialized, or set to an invalid value,
    // set to the first screen.
    auto screens = getScreens();
    if (std::find(screens.begin(), screens.end(), _fullScreenScreen) == screens.end()) {
        setFullScreenScreen(screens.first());
    }


    forceRenderMethod((RenderMethod)_renderMethod);
    forceShadowsEnabled(_shadowsEnabled);
    forceAmbientOcclusionEnabled(_ambientOcclusionEnabled);
    forceAntialiasingMode(_antialiasingMode);
    forceViewportResolutionScale(_viewportResolutionScale);
}

RenderScriptingInterface::RenderMethod RenderScriptingInterface::getRenderMethod() const {
    return (RenderMethod) _renderMethod;
}

void RenderScriptingInterface::setRenderMethod(RenderMethod renderMethod) {
    if (isValidRenderMethod(renderMethod) && (_renderMethod != (int) renderMethod)) {
        forceRenderMethod(renderMethod);
        emit settingsChanged();
    }
}
void RenderScriptingInterface::forceRenderMethod(RenderMethod renderMethod) {
    _renderSettingLock.withWriteLock([&] {
        _renderMethod = (int)renderMethod;
        _renderMethodSetting.set((int)renderMethod);

        auto config = dynamic_cast<render::SwitchConfig*>(qApp->getRenderEngine()->getConfiguration()->getConfig("RenderMainView.DeferredForwardSwitch"));
        if (config) {
            config->setBranch((int)renderMethod);
        }
    });
}

QStringList RenderScriptingInterface::getRenderMethodNames() const {
    static const QStringList refrenderMethodNames = { "DEFERRED", "FORWARD" };
    return refrenderMethodNames;
}

bool RenderScriptingInterface::getShadowsEnabled() const {
    return _shadowsEnabled;
}

void RenderScriptingInterface::setShadowsEnabled(bool enabled) {
    if (_shadowsEnabled != enabled) {
        forceShadowsEnabled(enabled);
        emit settingsChanged();
    }
}

void RenderScriptingInterface::forceShadowsEnabled(bool enabled) {
    _renderSettingLock.withWriteLock([&] {
        _shadowsEnabled = (enabled);
        _shadowsEnabledSetting.set(enabled);

        auto renderConfig = qApp->getRenderEngine()->getConfiguration();
        assert(renderConfig);
        auto lightingModelConfig = renderConfig->getConfig<MakeLightingModel>("RenderMainView.LightingModel");
        if (lightingModelConfig) {
            Menu::getInstance()->setIsOptionChecked(MenuOption::Shadows, enabled);
            lightingModelConfig->setShadow(enabled);
        }
        auto secondaryLightingModelConfig = renderConfig->getConfig<MakeLightingModel>("RenderSecondView.LightingModel");
        if (secondaryLightingModelConfig) {
            Menu::getInstance()->setIsOptionChecked(MenuOption::Shadows, enabled);
            secondaryLightingModelConfig->setShadow(enabled);
        }
    });
}

bool RenderScriptingInterface::getAmbientOcclusionEnabled() const {
    return _ambientOcclusionEnabled;
}

void RenderScriptingInterface::setAmbientOcclusionEnabled(bool enabled) {
    if (_ambientOcclusionEnabled != enabled) {
        forceAmbientOcclusionEnabled(enabled);
        emit settingsChanged();
    }
}

void RenderScriptingInterface::forceAmbientOcclusionEnabled(bool enabled) {
    _renderSettingLock.withWriteLock([&] {
        _ambientOcclusionEnabled = (enabled);
        _ambientOcclusionEnabledSetting.set(enabled);

        auto lightingModelConfig = qApp->getRenderEngine()->getConfiguration()->getConfig<MakeLightingModel>("RenderMainView.LightingModel");
        if (lightingModelConfig) {
            Menu::getInstance()->setIsOptionChecked(MenuOption::AmbientOcclusion, enabled);
            lightingModelConfig->setAmbientOcclusion(enabled);
        }
    });
}

AntialiasingConfig::Mode RenderScriptingInterface::getAntialiasingMode() const {
    return _antialiasingMode;
}

void RenderScriptingInterface::setAntialiasingMode(AntialiasingConfig::Mode mode) {
    if (_antialiasingMode != mode) {
        forceAntialiasingMode(mode);
        emit settingsChanged();
    }
}

void setAntialiasingModeForView(AntialiasingConfig::Mode mode, JitterSampleConfig *jitterCamConfig, AntialiasingConfig *antialiasingConfig) {
    switch (mode) {
        case AntialiasingConfig::Mode::NONE:
            jitterCamConfig->none();
            antialiasingConfig->blend = 1;
            antialiasingConfig->setDebugFXAA(false);
            break;
        case AntialiasingConfig::Mode::TAA:
            jitterCamConfig->play();
            antialiasingConfig->blend = 0.25;
            antialiasingConfig->setDebugFXAA(false);
            break;
        case AntialiasingConfig::Mode::FXAA:
            jitterCamConfig->none();
            antialiasingConfig->blend = 0.25;
            antialiasingConfig->setDebugFXAA(true);
            break;
        default:
            jitterCamConfig->none();
            antialiasingConfig->blend = 1;
            antialiasingConfig->setDebugFXAA(false);
            break;
    }
}

void RenderScriptingInterface::forceAntialiasingMode(AntialiasingConfig::Mode mode) {
    _renderSettingLock.withWriteLock([&] {
        _antialiasingMode = mode;

        auto mainViewJitterCamConfig = qApp->getRenderEngine()->getConfiguration()->getConfig<JitterSample>("RenderMainView.JitterCam");
        auto mainViewAntialiasingConfig = qApp->getRenderEngine()->getConfiguration()->getConfig<Antialiasing>("RenderMainView.Antialiasing");
        auto secondViewJitterCamConfig = qApp->getRenderEngine()->getConfiguration()->getConfig<JitterSample>("RenderSecondView.JitterCam");
        auto secondViewAntialiasingConfig = qApp->getRenderEngine()->getConfiguration()->getConfig<Antialiasing>("RenderSecondView.Antialiasing");
        if (mode != AntialiasingConfig::Mode::NONE
                && mode != AntialiasingConfig::Mode::TAA
                && mode != AntialiasingConfig::Mode::FXAA) {
            _antialiasingMode = AntialiasingConfig::Mode::NONE;
        }
        if (mainViewJitterCamConfig && mainViewAntialiasingConfig) {
            setAntialiasingModeForView( mode, mainViewJitterCamConfig, mainViewAntialiasingConfig);
        }
        if (secondViewJitterCamConfig && secondViewAntialiasingConfig) {
            setAntialiasingModeForView( mode, secondViewJitterCamConfig, secondViewAntialiasingConfig);
        }

        _antialiasingModeSetting.set(_antialiasingMode);
    });
}


float RenderScriptingInterface::getViewportResolutionScale() const {
    return _viewportResolutionScale;
}

void RenderScriptingInterface::setViewportResolutionScale(float scale) {
    if (_viewportResolutionScale != scale) {
        forceViewportResolutionScale(scale);
        emit settingsChanged();
    }
}

void RenderScriptingInterface::setVerticalFieldOfView(float fieldOfView) {
    if (getViewportResolutionScale() != fieldOfView) {
        qApp->setFieldOfView(fieldOfView);
        emit settingsChanged();
    }
}

QStringList RenderScriptingInterface::getScreens() const {
    QStringList screens;

    for(QScreen *screen : qApp->screens()) {
        screens << ScreenName::getNameForScreen(screen);
    }

    return screens;
}

bool RenderScriptingInterface::setFullScreenScreen(QString name) {
    auto screens = getScreens();

    if (std::find(screens.begin(), screens.end(), name) == screens.end()) {
        // Screens can come and go and don't have a stable opaque ID, so we
        // go by model here. For multiple screens with the same model we get names
        // that include a serial number, so it works.
        return false;
    }

    _renderSettingLock.withWriteLock([&] {
        _fullScreenScreen = name;
        _fullScreenScreenSetting.set(name);
    });

    emit settingsChanged();

    return true;
}

QString RenderScriptingInterface::getFullScreenScreen() const {
    return _fullScreenScreen;
}


void RenderScriptingInterface::forceViewportResolutionScale(float scale) {
    // just not negative values or zero
    if (scale <= 0.f) {
        return;
    }
    _renderSettingLock.withWriteLock([&] {
        _viewportResolutionScale = (scale);
        _viewportResolutionScaleSetting.set(scale);

        auto renderConfig = qApp->getRenderEngine()->getConfiguration();
        assert(renderConfig);
        auto deferredView = renderConfig->getConfig("RenderMainView.RenderDeferredTask");
        // mainView can be null if we're rendering in forward mode
        if (deferredView) {
            deferredView->setProperty("resolutionScale", _viewportResolutionScale);
        }
        auto forwardView = renderConfig->getConfig("RenderMainView.RenderForwardTask");
        // mainView can be null if we're rendering in forward mode
        if (forwardView) {
            forwardView->setProperty("resolutionScale", _viewportResolutionScale);
        }
    });
}
