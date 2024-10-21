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

#include <RenderCommonTask.h>
#include <ScriptEngineCast.h>

#include "LightingModel.h"
#include <QScreen>
#include "ScreenName.h"

#include <procedural/Procedural.h>

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
        _renderMethod = _renderMethodSetting.get();
        _shadowsEnabled = _shadowsEnabledSetting.get();
        _hazeEnabled = _hazeEnabledSetting.get();
        _bloomEnabled = _bloomEnabledSetting.get();
        _ambientOcclusionEnabled = _ambientOcclusionEnabledSetting.get();
        _proceduralMaterialsEnabled = _proceduralMaterialsEnabledSetting.get();
        _antialiasingMode = static_cast<AntialiasingConfig::Mode>(_antialiasingModeSetting.get());
        _viewportResolutionScale = _viewportResolutionScaleSetting.get();
        _fullScreenScreen = _fullScreenScreenSetting.get();
    });

    // If full screen screen is not initialized, or set to an invalid value,
    // set to the first screen.
    auto screens = getScreens();
    if (std::find(screens.begin(), screens.end(), _fullScreenScreen) == screens.end()) {
        setFullScreenScreen(screens.first());
    }


    forceRenderMethod((RenderMethod)_renderMethod);
    forceShadowsEnabled(_shadowsEnabled);
    forceHazeEnabled(_hazeEnabled);
    forceBloomEnabled(_bloomEnabled);
    forceAmbientOcclusionEnabled(_ambientOcclusionEnabled);
    forceProceduralMaterialsEnabled(_proceduralMaterialsEnabled);
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

void recursivelyUpdateMirrorRenderMethods(const QString& parentTaskName, int renderMethod, int depth) {
    if (depth == RenderMirrorTask::MAX_MIRROR_DEPTH) {
        return;
    }

    for (size_t mirrorIndex = 0; mirrorIndex < RenderMirrorTask::MAX_MIRRORS_PER_LEVEL; mirrorIndex++) {
        std::string mirrorTaskString = parentTaskName.toStdString() + ".RenderMirrorView" + std::to_string(mirrorIndex) + "Depth" + std::to_string(depth) + ".DeferredForwardSwitch";
        auto mirrorConfig = dynamic_cast<render::SwitchConfig*>(qApp->getRenderEngine()->getConfiguration()->getConfig(QString::fromStdString(mirrorTaskString)));
        if (mirrorConfig) {
            mirrorConfig->setBranch((int)renderMethod);
            recursivelyUpdateMirrorRenderMethods(QString::fromStdString(mirrorTaskString) + (renderMethod == 1 ? ".RenderForwardTask" : ".RenderShadowsAndDeferredTask.RenderDeferredTask"),
                renderMethod, depth + 1);
        }
    }
}

void RenderScriptingInterface::forceRenderMethod(RenderMethod renderMethod) {
    _renderSettingLock.withWriteLock([&] {
        _renderMethod = (int)renderMethod;
        _renderMethodSetting.set((int)renderMethod);

        QString configName = "RenderMainView.DeferredForwardSwitch";
        auto config = dynamic_cast<render::SwitchConfig*>(qApp->getRenderEngine()->getConfiguration()->getConfig(configName));
        if (config) {
            config->setBranch((int)renderMethod);

            recursivelyUpdateMirrorRenderMethods(configName + (renderMethod == RenderMethod::FORWARD ? ".RenderForwardTask" : ".RenderShadowsAndDeferredTask.RenderDeferredTask"),
                (int)renderMethod, 0);
        }
    });
}

QStringList RenderScriptingInterface::getRenderMethodNames() const {
    static const QStringList refrenderMethodNames = { "DEFERRED", "FORWARD" };
    return refrenderMethodNames;
}

void recursivelyUpdateLightingModel(const QString& parentTaskName, std::function<void(MakeLightingModelConfig *)> updateLambda, int depth = -1) {
    if (depth == -1) {
        auto secondaryLightingModelConfig = qApp->getRenderEngine()->getConfiguration()->getConfig<MakeLightingModel>("RenderSecondView.LightingModel");
        if (secondaryLightingModelConfig) {
            updateLambda(secondaryLightingModelConfig);
        }

        auto mainLightingModelConfig = qApp->getRenderEngine()->getConfiguration()->getConfig<MakeLightingModel>("RenderMainView.LightingModel");
        if (mainLightingModelConfig) {
            updateLambda(mainLightingModelConfig);
        }

        recursivelyUpdateLightingModel("RenderMainView", updateLambda, depth + 1);
    } else if (depth == RenderMirrorTask::MAX_MIRROR_DEPTH) {
        return;
    }

    for (size_t mirrorIndex = 0; mirrorIndex < RenderMirrorTask::MAX_MIRRORS_PER_LEVEL; mirrorIndex++) {
        std::string mirrorTaskString = parentTaskName.toStdString() + ".RenderMirrorView" + std::to_string(mirrorIndex) + "Depth" + std::to_string(depth);
        auto lightingModelConfig = qApp->getRenderEngine()->getConfiguration()->getConfig<MakeLightingModel>(mirrorTaskString + ".LightingModel");
        if (lightingModelConfig) {
            updateLambda(lightingModelConfig);
            recursivelyUpdateLightingModel(QString::fromStdString(mirrorTaskString), updateLambda, depth + 1);
        }
    }
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

        Menu::getInstance()->setIsOptionChecked(MenuOption::Shadows, enabled);

        recursivelyUpdateLightingModel("", [enabled] (MakeLightingModelConfig *config) { config->setShadow(enabled); });
    });
}

bool RenderScriptingInterface::getHazeEnabled() const {
    return _hazeEnabled;
}

void RenderScriptingInterface::setHazeEnabled(bool enabled) {
    if (_hazeEnabled != enabled) {
        forceHazeEnabled(enabled);
        emit settingsChanged();
    }
}

void RenderScriptingInterface::forceHazeEnabled(bool enabled) {
    _renderSettingLock.withWriteLock([&] {
        _hazeEnabled = (enabled);
        _hazeEnabledSetting.set(enabled);

        recursivelyUpdateLightingModel("", [enabled] (MakeLightingModelConfig *config) { config->setHaze(enabled); });
    });
}

bool RenderScriptingInterface::getBloomEnabled() const {
    return _bloomEnabled;
}

void RenderScriptingInterface::setBloomEnabled(bool enabled) {
    if (_bloomEnabled != enabled) {
        forceBloomEnabled(enabled);
        emit settingsChanged();
    }
}

void RenderScriptingInterface::forceBloomEnabled(bool enabled) {
    _renderSettingLock.withWriteLock([&] {
        _bloomEnabled = (enabled);
        _bloomEnabledSetting.set(enabled);

        recursivelyUpdateLightingModel("", [enabled] (MakeLightingModelConfig *config) { config->setBloom(enabled); });
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

        Menu::getInstance()->setIsOptionChecked(MenuOption::AmbientOcclusion, enabled);

        recursivelyUpdateLightingModel("", [enabled] (MakeLightingModelConfig *config) { config->setAmbientOcclusion(enabled); });
    });
}

bool RenderScriptingInterface::getProceduralMaterialsEnabled() const {
    return _proceduralMaterialsEnabled;
}

void RenderScriptingInterface::setProceduralMaterialsEnabled(bool enabled) {
    if (_proceduralMaterialsEnabled != enabled) {
        forceProceduralMaterialsEnabled(enabled);
        emit settingsChanged();
    }
}

void RenderScriptingInterface::forceProceduralMaterialsEnabled(bool enabled) {
    _renderSettingLock.withWriteLock([&] {
        _proceduralMaterialsEnabled = (enabled);
        _proceduralMaterialsEnabledSetting.set(enabled);

        Menu::getInstance()->setIsOptionChecked(MenuOption::MaterialProceduralShaders, enabled);
        Procedural::enableProceduralShaders = enabled;
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

void recursivelyUpdateAntialiasingMode(const QString& parentTaskName, AntialiasingConfig::Mode mode, int depth = -1) {
    if (depth == -1) {
        auto secondViewJitterCamConfig = qApp->getRenderEngine()->getConfiguration()->getConfig<JitterSample>("RenderSecondView.JitterCam");
        auto secondViewAntialiasingConfig = qApp->getRenderEngine()->getConfiguration()->getConfig<Antialiasing>("RenderSecondView.Antialiasing");
        if (secondViewJitterCamConfig && secondViewAntialiasingConfig) {
            setAntialiasingModeForView(mode, secondViewJitterCamConfig, secondViewAntialiasingConfig);
        }

        auto mainViewJitterCamConfig = qApp->getRenderEngine()->getConfiguration()->getConfig<JitterSample>("RenderMainView.JitterCam");
        auto mainViewAntialiasingConfig = qApp->getRenderEngine()->getConfiguration()->getConfig<Antialiasing>("RenderMainView.Antialiasing");
        if (mainViewJitterCamConfig && mainViewAntialiasingConfig) {
            setAntialiasingModeForView( mode, mainViewJitterCamConfig, mainViewAntialiasingConfig);
        }

        recursivelyUpdateAntialiasingMode("RenderMainView", mode, depth + 1);
    } else if (depth == RenderMirrorTask::MAX_MIRROR_DEPTH) {
        return;
    }

    for (size_t mirrorIndex = 0; mirrorIndex < RenderMirrorTask::MAX_MIRRORS_PER_LEVEL; mirrorIndex++) {
        std::string mirrorTaskString = parentTaskName.toStdString() + ".RenderMirrorView" + std::to_string(mirrorIndex) + "Depth" + std::to_string(depth);
        auto jitterCamConfig = qApp->getRenderEngine()->getConfiguration()->getConfig<JitterSample>(mirrorTaskString + ".JitterCam");
        auto antialiasingConfig = qApp->getRenderEngine()->getConfiguration()->getConfig<Antialiasing>(mirrorTaskString + ".Antialiasing");
        if (jitterCamConfig && antialiasingConfig) {
            setAntialiasingModeForView(mode, jitterCamConfig, antialiasingConfig);
            recursivelyUpdateAntialiasingMode(QString::fromStdString(mirrorTaskString), mode, depth + 1);
        }
    }
}

void RenderScriptingInterface::forceAntialiasingMode(AntialiasingConfig::Mode mode) {
    if ((int)mode < 0 || mode >= AntialiasingConfig::Mode::MODE_COUNT) {
        mode = AntialiasingConfig::Mode::NONE;
    }

    _renderSettingLock.withWriteLock([&] {
        _antialiasingMode = mode;
        _antialiasingModeSetting.set(_antialiasingMode);

        recursivelyUpdateAntialiasingMode("", _antialiasingMode);
    });
}

void RenderScriptingInterface::setVerticalFieldOfView(float fieldOfView) {
    if (qApp->getFieldOfView() != fieldOfView) {
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

float RenderScriptingInterface::getViewportResolutionScale() const {
    return _viewportResolutionScale;
}

void RenderScriptingInterface::setViewportResolutionScale(float scale) {
    if (_viewportResolutionScale != scale) {
        forceViewportResolutionScale(scale);
        emit settingsChanged();
    }
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
