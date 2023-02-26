//
//  ScriptManagerScriptingInterface.h
//  libraries/script-engine/src
//
//  Created by Dale Glass on 24/02/2023.
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ScriptManager.h"
#include "ScriptManagerScriptingInterface.h"


 ScriptManagerScriptingInterface::ScriptManagerScriptingInterface(ScriptManager *parent): QObject(parent), _manager(parent) {
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
        connect(_manager, &ScriptManager::unhandledException, this, &ScriptManagerScriptingInterface::unhandledException);
    }