//
//  Created by Bradley Austin Davis on 2019/05/14
//  Copyright 2013-2019 High Fidelity, Inc.
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//


#include "PerformanceScriptingInterface.h"

#include <ScriptEngineCast.h>

#include "../Application.h"

STATIC_SCRIPT_TYPES_INITIALIZER((+[](ScriptManager* manager){
    auto scriptEngine = manager->engine().get();

    scriptRegisterMetaType<PerformanceScriptingInterface::PerformancePreset, scriptValueFromEnumClass<PerformanceScriptingInterface::PerformancePreset>, scriptValueToEnumClass<PerformanceScriptingInterface::PerformancePreset> >(scriptEngine, "PerformancePreset");
    scriptRegisterMetaType<PerformanceScriptingInterface::RefreshRateProfile, scriptValueFromEnumClass<PerformanceScriptingInterface::RefreshRateProfile>, scriptValueToEnumClass<PerformanceScriptingInterface::RefreshRateProfile> >(scriptEngine, "RefreshRateProfile");
}));

STATIC_SCRIPT_INITIALIZER(+[](ScriptManager* manager){
    auto scriptEngine = manager->engine().get();

    scriptEngine->registerEnum("Performance.PerformancePreset",QMetaEnum::fromType<PerformanceScriptingInterface::PerformancePreset>());
    scriptEngine->registerEnum("Performance.RefreshRateProfile",QMetaEnum::fromType<PerformanceScriptingInterface::RefreshRateProfile>());
});

std::once_flag PerformanceScriptingInterface::registry_flag;

PerformanceScriptingInterface::PerformanceScriptingInterface() {
    std::call_once(registry_flag, [] {
        qmlRegisterType<PerformanceScriptingInterface>("PerformanceEnums", 1, 0, "PerformanceEnums");
        //qRegisterMetaType<PerformanceScriptingInterface::PerformancePreset>("PerformanceScriptingInterface::PerformancePreset");
        //qRegisterMetaType<PerformanceScriptingInterface::RefreshRateProfile>("PerformanceScriptingInterface::RefreshRateProfile");
    });
}

void PerformanceScriptingInterface::setPerformancePreset(PerformancePreset performancePreset) {
    qApp->getPerformanceManager().setPerformancePreset((PerformanceManager::PerformancePreset)performancePreset);
    emit settingsChanged();
}

PerformanceScriptingInterface::PerformancePreset PerformanceScriptingInterface::getPerformancePreset() const {
    return (PerformanceScriptingInterface::PerformancePreset)qApp->getPerformanceManager().getPerformancePreset();
}

QStringList PerformanceScriptingInterface::getPerformancePresetNames() const {
    static const QStringList performancePresetNames = { "UNKNOWN", "LOW_POWER", "LOW", "MID", "HIGH" };
    return performancePresetNames;
}

void PerformanceScriptingInterface::setRefreshRateProfile(RefreshRateProfile refreshRateProfile) {
    qApp->getRefreshRateManager().setRefreshRateProfile((RefreshRateManager::RefreshRateProfile)refreshRateProfile);
    emit settingsChanged();
}

PerformanceScriptingInterface::RefreshRateProfile PerformanceScriptingInterface::getRefreshRateProfile() const {
    return (PerformanceScriptingInterface::RefreshRateProfile)qApp->getRefreshRateManager().getRefreshRateProfile();
}

QStringList PerformanceScriptingInterface::getRefreshRateProfileNames() const {
    static const QStringList refreshRateProfileNames = { "ECO", "INTERACTIVE", "REALTIME" };
    return refreshRateProfileNames;
}

int PerformanceScriptingInterface::getActiveRefreshRate() const {
    return qApp->getRefreshRateManager().getActiveRefreshRate();
}

RefreshRateManager::UXMode PerformanceScriptingInterface::getUXMode() const {
    return qApp->getRefreshRateManager().getUXMode();
}

RefreshRateManager::RefreshRateRegime PerformanceScriptingInterface::getRefreshRateRegime() const {
    return qApp->getRefreshRateManager().getRefreshRateRegime();
}
