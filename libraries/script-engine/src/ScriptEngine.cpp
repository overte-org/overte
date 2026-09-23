//
//  ScriptEngine.cpp
//  libraries/script-engine/src
//
//  Created by Brad Hefta-Gaub on 12/14/13.
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "ScriptEngine.h"

#include "ScriptEngineLogging.h"
#include "ScriptValue.h"
#include "v8/ScriptEngineV8.h"

ScriptEnginePointer newScriptEngine(ScriptManager* manager) {
    return std::make_shared<ScriptEngineV8>(manager);
}

ScriptValue makeScopedHandlerObject(const ScriptValue& scopeOrCallback, const ScriptValue& methodOrName) {
    auto engine = scopeOrCallback.engine();
    if (!engine) {
        return scopeOrCallback;
    }
    ScriptValue scope;
    ScriptValue callback = scopeOrCallback;
    if (scopeOrCallback.isObject()) {
        if (methodOrName.isString()) {
            scope = scopeOrCallback;
            callback = scope.property(methodOrName.toString());
        } else if (methodOrName.isFunction()) {
            scope = scopeOrCallback;
            callback = methodOrName;
        } else if (!methodOrName.isValid()) {
            // instantiate from an existing scoped handler object
            if (scopeOrCallback.property("callback").isFunction()) {
                scope = scopeOrCallback.property("scope");
                callback = scopeOrCallback.property("callback");
            }
        }
    }
    auto handler = engine->newObject();
    handler.setProperty("scope", scope);
    handler.setProperty("callback", callback);
    return handler;
}

ScriptValue callScopedHandlerObject(const ScriptValue& handler, const ScriptValue& err, const ScriptValue& result) {
    return handler.property("callback").call(handler.property("scope"), ScriptValueList({ err, result }));
}

bool ScriptEngine::IS_THREADSAFE_INVOCATION(const QString& method) {
    QThread* thread = this->thread();
    if (QThread::currentThread() == thread) {
        return true;
    }
    qCCritical(scriptengine) << QString("Scripting::%1 @ %2 -- ignoring thread-unsafe call from %3")
                                    .arg(method)
                                    .arg(thread ? thread->objectName() : "(!thread)")
                                    .arg(QThread::currentThread()->objectName());
    qCDebug(scriptengine) << "(please resolve on the calling side by using invokeMethod, executeOnScriptThread, etc.)";
    Q_ASSERT(false);
    return false;
}

inline QDebug operator<<(QDebug dbg, const ScriptEngine::LogLevel& logLevel) {
    switch (logLevel) {
        case ScriptEngine::LogLevel::Log:
            dbg << "[LOG]";
            break;
        case ScriptEngine::LogLevel::Trace:
            dbg << "[TRC]";
            break;
        case ScriptEngine::LogLevel::Debug:
            dbg << "[DBG]";
            break;
        case ScriptEngine::LogLevel::Info:
            dbg << "[INF]";
            break;
        case ScriptEngine::LogLevel::Warning:
            dbg << "[WRN]";
            break;
        case ScriptEngine::LogLevel::Error:
            dbg << "[ERR]";
            break;
    }
    return dbg;
}

void ScriptEngine::log(const LogLevel logLevel, const QString& message, const QString& fileName, int lineNumber) {
    if (_logSignal) {
        emit scriptMessage(logLevel, message, fileName, lineNumber);
    }
    if (_logQtLogger) {
        qCDebug(_loggingCategory) << logLevel << message;
    }
}
