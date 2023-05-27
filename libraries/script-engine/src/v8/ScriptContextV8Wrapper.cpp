//
//  ScriptContextV8Wrapper.cpp
//  libraries/script-engine/src/v8
//
//  Created by Heather Anderson on 5/22/21.
//  Modified for V8 by dr Karol Suprynowicz on 2022/10/08
//  Copyright 2021 Vircadia contributors.
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "ScriptContextV8Wrapper.h"

#include "ScriptEngineV8.h"
#include "ScriptValueV8Wrapper.h"
#include "ScriptEngineLoggingV8.h"

ScriptContextV8Wrapper::ScriptContextV8Wrapper(ScriptEngineV8* engine, const v8::Local<v8::Context> context, ScriptContextPointer parent) :
    _functionCallbackInfo(nullptr), _propertyCallbackInfo(nullptr), _engine(engine),
    _context(engine->getIsolate(), context), _parentContext(parent) {
}

ScriptContextV8Wrapper::ScriptContextV8Wrapper(ScriptEngineV8* engine, const v8::FunctionCallbackInfo<v8::Value> *functionCallbackInfo,
                                               const v8::Local<v8::Context> context, ScriptContextPointer parent) :
    _functionCallbackInfo(functionCallbackInfo), _propertyCallbackInfo(nullptr), _engine(engine),
    _context(engine->getIsolate(), context), _parentContext(parent)  {
}

ScriptContextV8Wrapper::ScriptContextV8Wrapper(ScriptEngineV8* engine, const v8::PropertyCallbackInfo<v8::Value> *propertyCallbackInfo,
                                               const v8::Local<v8::Context> context, ScriptContextPointer parent) :
    _functionCallbackInfo(nullptr), _propertyCallbackInfo(propertyCallbackInfo), _engine(engine),
    _context(engine->getIsolate(), context), _parentContext(parent)   {
}

ScriptContextV8Wrapper::~ScriptContextV8Wrapper() noexcept {
    //V8TODO: what if destructor happens during shutdown and V8 isolate is already disposed of?
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    _context.Reset();
}

ScriptContextV8Wrapper* ScriptContextV8Wrapper::unwrap(ScriptContext* val) {
    if (!val) {
        return nullptr;
    }

    return dynamic_cast<ScriptContextV8Wrapper*>(val);
}

v8::Local<v8::Context> ScriptContextV8Wrapper::toV8Value() const {
    v8::EscapableHandleScope handleScope(_engine->getIsolate());
    return handleScope.Escape(_context.Get(_engine->getIsolate()));
}

int ScriptContextV8Wrapper::argumentCount() const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_context.Get(isolate));
    if (_functionCallbackInfo) {
        return _functionCallbackInfo->Length();
    } else if (_propertyCallbackInfo) {
        return 0;
    } else {
        return Q_METAMETHOD_INVOKE_MAX_ARGS;
    }
}

ScriptValue ScriptContextV8Wrapper::argument(int index) const {
    if (_functionCallbackInfo) {
        auto isolate = _engine->getIsolate();
        v8::Locker locker(isolate);
        v8::Isolate::Scope isolateScope(isolate);
        v8::HandleScope handleScope(isolate);
        v8::Context::Scope contextScope(_context.Get(isolate));
        v8::Local<v8::Value> result = (*_functionCallbackInfo)[index];
        if (index < _functionCallbackInfo->kArgsLength) {
            return ScriptValue(new ScriptValueV8Wrapper(_engine, V8ScriptValue(_engine, result)));
        } else {
            return _engine->undefinedValue();
        }
    } else if (_propertyCallbackInfo) {
        return _engine->undefinedValue();
    } else {
        return _engine->undefinedValue();
    }
}

QStringList ScriptContextV8Wrapper::backtrace() const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_context.Get(isolate));
    v8::Local<v8::StackTrace> stackTrace = v8::StackTrace::CurrentStackTrace(isolate, 40);
    QStringList backTrace;
    for (int i = 0; i < stackTrace->GetFrameCount(); i++) {
        v8::Local<v8::StackFrame> stackFrame = stackTrace->GetFrame(isolate, i);
        backTrace.append(QString(*v8::String::Utf8Value(isolate, stackFrame->GetScriptNameOrSourceURL())) +
                        QString(" ") +
                        QString(*v8::String::Utf8Value(isolate, stackFrame->GetFunctionName())) +
                        QString(":") +
                        QStringLiteral("%1").arg(stackFrame->GetLineNumber())
        );
    }
    return backTrace;
}

ScriptValue ScriptContextV8Wrapper::callee() const {
    Q_ASSERT(false);
    //V8TODO
    //Can this be done with CurrentStackTrace?
    //V8ScriptValue result = _context->callee();
    return _engine->undefinedValue();
}

ScriptEnginePointer ScriptContextV8Wrapper::engine() const {
    return _engine->shared_from_this();
}

ScriptFunctionContextPointer ScriptContextV8Wrapper::functionContext() const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_context.Get(isolate));
    auto scriptFunctionContextPointer = std::make_shared<ScriptFunctionContextV8Wrapper>(_engine, _context.Get(_engine->getIsolate()));
    return scriptFunctionContextPointer;
}

ScriptContextPointer ScriptContextV8Wrapper::parentContext() const {
    return _parentContext;
}

ScriptValue ScriptContextV8Wrapper::thisObject() const {
    if (_functionCallbackInfo) {
        auto isolate = _engine->getIsolate();
        v8::Locker locker(isolate);
        v8::Isolate::Scope isolateScope(isolate);
        v8::HandleScope handleScope(isolate);
        v8::Context::Scope contextScope(_context.Get(isolate));
        v8::Local<v8::Value> result = _functionCallbackInfo->This();
        return ScriptValue(new ScriptValueV8Wrapper(_engine, V8ScriptValue(_engine, result)));
    } else if (_propertyCallbackInfo) {
        auto isolate = _engine->getIsolate();
        v8::Locker locker(isolate);
        v8::Isolate::Scope isolateScope(isolate);
        v8::HandleScope handleScope(isolate);
        v8::Context::Scope contextScope(_context.Get(isolate));
        v8::Local<v8::Value> result = _propertyCallbackInfo->This();
        return ScriptValue(new ScriptValueV8Wrapper(_engine, V8ScriptValue(_engine, result)));
    } else {
        return _engine->undefinedValue();
    }
}

ScriptValue ScriptContextV8Wrapper::throwError(const QString& text) {
    auto isolate = _engine->getIsolate();
    // V8TODO: I have no idea how to do this yet when it happens on another thread
    if(isolate->IsCurrent()) {
        v8::Locker locker(isolate);
        v8::Isolate::Scope isolateScope(isolate);
        v8::HandleScope handleScope(isolate);
        v8::Context::Scope contextScope(_context.Get(isolate));
        V8ScriptValue result(_engine,
                             _engine->getIsolate()->ThrowError(
                                 v8::String::NewFromUtf8(_engine->getIsolate(), text.toStdString().c_str()).ToLocalChecked()));
        return ScriptValue(new ScriptValueV8Wrapper(_engine, std::move(result)));
    } else {
        qCWarning(scriptengine_v8) << "throwError on a different thread not implemented yet, error value: " << text;
        return ScriptValue();
    }
}

ScriptValue ScriptContextV8Wrapper::throwValue(const ScriptValue& value) {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_context.Get(isolate));
    ScriptValueV8Wrapper* unwrapped = ScriptValueV8Wrapper::unwrap(value);
    if (!unwrapped) {
        return _engine->undefinedValue();
    }
    V8ScriptValue result(_engine, _engine->getIsolate()->ThrowException(unwrapped->toV8Value().constGet()));
    return ScriptValue(new ScriptValueV8Wrapper(_engine, std::move(result)));
}

ScriptFunctionContextV8Wrapper::ScriptFunctionContextV8Wrapper(ScriptEngineV8* engine, const v8::Local<v8::Context> context) : _engine(engine) {
    v8::Locker locker(engine->getIsolate());
    v8::Isolate::Scope isolateScope(engine->getIsolate());
    v8::HandleScope handleScope(engine->getIsolate());
    _context.Reset(engine->getIsolate(), context);
}

ScriptFunctionContextV8Wrapper::~ScriptFunctionContextV8Wrapper() {
    //V8TODO: what if destructor happens during shutdown and V8 isolate is already disposed of?
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    _context.Reset();
}

QString ScriptFunctionContextV8Wrapper::fileName() const {
    // It's not exactly like in QtScript, because there's no such context object in V8, let's return the current one for now
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_context.Get(isolate));
    v8::Local<v8::String> name = v8::StackTrace::CurrentScriptNameOrSourceURL(_engine->getIsolate());
    v8::String::Utf8Value nameUTF(_engine->getIsolate(), name);
    return QString(*nameUTF);
}

QString ScriptFunctionContextV8Wrapper::functionName() const {
    // It's not exactly like in QtScript, because there's no such context object in V8, let's return the current one for now
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_context.Get(isolate));
    v8::Local<v8::StackTrace> stackTrace = v8::StackTrace::CurrentStackTrace(_engine->getIsolate(), 1);
    v8::Local<v8::StackFrame> stackFrame = stackTrace->GetFrame(_engine->getIsolate(), 0);
    v8::Local<v8::String> name = stackFrame->GetFunctionName();
    v8::String::Utf8Value nameUTF(_engine->getIsolate(), name);
    return QString(*nameUTF);
}

ScriptFunctionContext::FunctionType ScriptFunctionContextV8Wrapper::functionType() const {
    //V8TODO: This is only used in debugPrint. Should we remove it?
    //return static_cast<ScriptFunctionContext::FunctionType>(_value.functionType());
    return ScriptFunctionContext::FunctionType::ScriptFunction;
}

int ScriptFunctionContextV8Wrapper::lineNumber() const {
    // It's not exactly like in QtScript, because there's no such context object in V8, let's return the current one for now
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_context.Get(isolate));
    v8::Local<v8::StackTrace> stackTrace = v8::StackTrace::CurrentStackTrace(_engine->getIsolate(), 1);
    v8::Local<v8::StackFrame> stackFrame = stackTrace->GetFrame(_engine->getIsolate(), 0);
    return stackFrame->GetLineNumber();
}
