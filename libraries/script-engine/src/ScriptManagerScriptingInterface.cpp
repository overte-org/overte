//
//  ScriptManagerScriptingInterface.h
//  libraries/script-engine/src
//
//  Created by Dale Glass on 24/02/2023.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "ScriptManager.h"
#include "ScriptManagerScriptingInterface.h"
#include "ScriptEngine.h"
#include <QMetaType>



 ScriptManagerScriptingInterface::ScriptManagerScriptingInterface(ScriptManager *parent): QObject(parent), _manager(parent) {

    qRegisterMetaType<ScriptException>();
    qRegisterMetaType<ScriptEngineException>();
    qRegisterMetaType<ScriptRuntimeException>();

    qRegisterMetaType<std::shared_ptr<ScriptException>>();
    qRegisterMetaType<std::shared_ptr<ScriptEngineException>>();
    qRegisterMetaType<std::shared_ptr<ScriptRuntimeException>>();


    connect(_manager, &ScriptManager::scriptLoaded, this, &ScriptManagerScriptingInterface::scriptLoaded);
    connect(_manager, &ScriptManager::errorLoadingScript, this, &ScriptManagerScriptingInterface::errorLoadingScript);
    connect(_manager, &ScriptManager::update, this, &ScriptManagerScriptingInterface::update);
    connect(_manager, &ScriptManager::scriptEnding, this, &ScriptManagerScriptingInterface::scriptEnding);
    connect(_manager, &ScriptManager::finished, this, &ScriptManagerScriptingInterface::finished);
    connect(_manager, &ScriptManager::printedMessage, this, &ScriptManagerScriptingInterface::printedMessage);
    connect(_manager, &ScriptManager::errorMessage, this, &ScriptManagerScriptingInterface::errorMessage);
    connect(_manager, &ScriptManager::warningMessage, this, &ScriptManagerScriptingInterface::warningMessage);
    connect(_manager, &ScriptManager::infoMessage, this, &ScriptManagerScriptingInterface::infoMessage);
    connect(_manager, &ScriptManager::runningStateChanged, this, &ScriptManagerScriptingInterface::runningStateChanged);
    connect(_manager, &ScriptManager::clearDebugWindow, this, &ScriptManagerScriptingInterface::clearDebugWindow);
    connect(_manager, &ScriptManager::loadScript, this, &ScriptManagerScriptingInterface::loadScript);
    connect(_manager, &ScriptManager::doneRunning, this, &ScriptManagerScriptingInterface::doneRunning);
    connect(_manager, &ScriptManager::entityScriptDetailsUpdated, this, &ScriptManagerScriptingInterface::entityScriptDetailsUpdated);
    connect(_manager, &ScriptManager::entityScriptPreloadFinished, this, &ScriptManagerScriptingInterface::entityScriptPreloadFinished);
    connect(_manager, &ScriptManager::unhandledException, this, &ScriptManagerScriptingInterface::scriptManagerException);
}

void ScriptManagerScriptingInterface::scriptManagerException(std::shared_ptr<ScriptException> exception) {
    // V8TODO: What should we actually handle here?


    // emit unhandledException(exception.thrownValue);
}

QVariantMap ScriptManagerScriptingInterface::getMemoryUsageStatistics() {
    auto statistics = _manager->engine()->getMemoryUsageStatistics();
    QVariantMap map;
    map.insert("totalHeapSize", QVariant((qulonglong)(statistics.totalHeapSize)));
    map.insert("usedHeapSize", QVariant((qulonglong)(statistics.usedHeapSize)));
    map.insert("totalAvailableSize", QVariant((qulonglong)(statistics.totalAvailableSize)));
    map.insert("totalGlobalHandlesSize", QVariant((qulonglong)(statistics.totalGlobalHandlesSize)));
    map.insert("usedGlobalHandlesSize", QVariant((qulonglong)(statistics.usedGlobalHandlesSize)));
#ifdef OVERTE_V8_HANDLE_COUNTERS
    map.insert("scriptValueCount", QVariant((qulonglong)(statistics.scriptValueCount)));
    map.insert("scriptValueProxyCount", QVariant((qulonglong)(statistics.scriptValueProxyCount)));
#endif
    return map;
}

ScriptValue ScriptManagerScriptingInterface::createGarbageCollectorDebuggingObject() {
    //auto value = _manager->engine()->newQObject(new TestQObject, ScriptEngine::ScriptOwnership);
    return _manager->engine()->newQObject(new TestQObject, ScriptEngine::ScriptOwnership);
    //return _manager->engine()->newValue(1);
}
