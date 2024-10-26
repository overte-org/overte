//
//  HelperScriptEngine.h
//  libraries/script-engine/src/HelperScriptEngine.h
//
//  Created by dr Karol Suprynowicz on 2024/04/28.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "HelperScriptEngine.h"

HelperScriptEngine::HelperScriptEngine() {
    std::lock_guard<std::mutex> lock(_scriptEngineLock);
    _scriptEngine = newScriptEngine();
    _scriptEngineThread.reset(new QThread());
    _scriptEngine->setThread(_scriptEngineThread.get());
    _scriptEngineThread->start();
}

HelperScriptEngine::~HelperScriptEngine() {
    std::lock_guard<std::mutex> lock(_scriptEngineLock);
    if (_scriptEngine) {
        if (_scriptEngineThread) {
            _scriptEngineThread->quit();
            _scriptEngineThread->wait();
        }
        _scriptEngine.reset();
    }
}
