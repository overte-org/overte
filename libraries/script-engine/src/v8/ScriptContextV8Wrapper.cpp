//
//  ScriptContextV8Wrapper.cpp
//  libraries/script-engine/src/v8
//
//  Created by Heather Anderson on 5/22/21.
//  Modified for V8 by dr Karol Suprynowicz on 2022/10/08
//  Copyright 2021 Vircadia contributors.
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ScriptContextV8Wrapper.h"

#include "ScriptEngineV8.h"
#include "ScriptValueV8Wrapper.h"

/*ScriptContextV8Wrapper::ScriptContextV8Wrapper(ScriptEngineV8* engine, const v8::Local<v8::Context> context) : _functionCallbackInfo(nullptr), _propertyCallbackInfo(nullptr), _engine(engine) {
    _context.Reset(_engine->getIsolate(), context);
}*/

ScriptContextV8Wrapper::ScriptContextV8Wrapper(ScriptEngineV8* engine) : _functionCallbackInfo(nullptr), _propertyCallbackInfo(nullptr), _engine(engine) {
}

ScriptContextV8Wrapper::ScriptContextV8Wrapper(ScriptEngineV8* engine, const v8::FunctionCallbackInfo<v8::Value> *functionCallbackInfo) : _functionCallbackInfo(functionCallbackInfo), _propertyCallbackInfo(nullptr), _engine(engine)  {
}

ScriptContextV8Wrapper::ScriptContextV8Wrapper(ScriptEngineV8* engine, const v8::PropertyCallbackInfo<v8::Value> *propertyCallbackInfo) : _functionCallbackInfo(nullptr), _propertyCallbackInfo(propertyCallbackInfo), _engine(engine)  {
}

ScriptContextV8Wrapper* ScriptContextV8Wrapper::unwrap(ScriptContext* val) {
    if (!val) {
        return nullptr;
    }

    return dynamic_cast<ScriptContextV8Wrapper*>(val);
}

v8::Local<v8::Context> ScriptContextV8Wrapper::toV8Value() const {
    v8::EscapableHandleScope handleScope(_engine->getIsolate());
    return handleScope.Escape(_engine->getContext());
}

int ScriptContextV8Wrapper::argumentCount() const {
    /*auto isolate = _engine->getIsolate();
    Q_ASSERT(isolate->IsCurrent());
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());*/
    //Q_ASSERT(_functionCallbackInfo);A
    // V8TODO
    if (_functionCallbackInfo) {
        return _functionCallbackInfo->Length();
    } else if (_propertyCallbackInfo) {
        return 0;
    } else {
        return Q_METAMETHOD_INVOKE_MAX_ARGS;
    }
    // This was wrong, in function registration it seems to be used as maximum number od arguments instead?
    // Is it also used for something else?
    //return _functionCallbackInfo->Length();
}

ScriptValue ScriptContextV8Wrapper::argument(int index) const {
    if (_functionCallbackInfo) {
        auto isolate = _engine->getIsolate();
        Q_ASSERT(isolate->IsCurrent());
        v8::HandleScope handleScope(isolate);
        v8::Context::Scope contextScope(_engine->getContext());
        v8::Local<v8::Value> result = (*_functionCallbackInfo)[index];
        if (index < _functionCallbackInfo->kArgsLength) {
            return ScriptValue(new ScriptValueV8Wrapper(_engine, V8ScriptValue(_engine->getIsolate(), result)));
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
    Q_ASSERT(isolate->IsCurrent());
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    v8::Local<v8::StackTrace> stackTrace = v8::StackTrace::CurrentStackTrace(isolate, 40);
    QStringList backTrace;
    //V8TODO nicer formatting
    for (int i = 0; i < stackTrace->GetFrameCount(); i++) {
        v8::Local<v8::StackFrame> stackFrame = stackTrace->GetFrame(isolate, i);
        backTrace.append(QString(*v8::String::Utf8Value(isolate, stackFrame->GetScriptNameOrSourceURL())) +
            QString(" ") +
            QString(*v8::String::Utf8Value(isolate, stackFrame->GetFunctionName())) +
            QString(":") +
            QString(stackFrame->GetLineNumber())
        );
    }
    return backTrace;
}

ScriptValue ScriptContextV8Wrapper::callee() const {
    Q_ASSERT(false);
    //V8TODO
    //Can this be done with CurrentStackTrace?
    //V8ScriptValue result = _context->callee();
    //return ScriptValue(new ScriptValueV8Wrapper(_engine, std::move(result)));
    return _engine->undefinedValue();
}

ScriptEnginePointer ScriptContextV8Wrapper::engine() const {
    return _engine->shared_from_this();
}

ScriptFunctionContextPointer ScriptContextV8Wrapper::functionContext() const {
    return std::make_shared<ScriptFunctionContextV8Wrapper>(_engine->getContext());
}

ScriptContextPointer ScriptContextV8Wrapper::parentContext() const {
    //V8TODO
    Q_ASSERT(false);
    //V8ScriptContext* result = _context->parentContext();
    //return result ? std::make_shared<ScriptContextV8Wrapper>(_engine, result) : ScriptContextPointer();
    return ScriptContextPointer();
}

ScriptValue ScriptContextV8Wrapper::thisObject() const {
    if (_functionCallbackInfo) {
        auto isolate = _engine->getIsolate();
        Q_ASSERT(isolate->IsCurrent());
        v8::HandleScope handleScope(isolate);
        v8::Context::Scope contextScope(_engine->getContext());
        v8::Local<v8::Value> result = _functionCallbackInfo->This();
        return ScriptValue(new ScriptValueV8Wrapper(_engine, V8ScriptValue(_engine->getIsolate(), result)));
    } else if (_propertyCallbackInfo) {
        auto isolate = _engine->getIsolate();
        Q_ASSERT(isolate->IsCurrent());
        v8::HandleScope handleScope(isolate);
        v8::Context::Scope contextScope(_engine->getContext());
        v8::Local<v8::Value> result = _propertyCallbackInfo->This();
        return ScriptValue(new ScriptValueV8Wrapper(_engine, V8ScriptValue(_engine->getIsolate(), result)));
    } else {
        return _engine->undefinedValue();
    }
}

ScriptValue ScriptContextV8Wrapper::throwError(const QString& text) {
    auto isolate = _engine->getIsolate();
    Q_ASSERT(isolate->IsCurrent());
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    V8ScriptValue result(_engine->getIsolate(), _engine->getIsolate()->ThrowError(v8::String::NewFromUtf8(_engine->getIsolate(), text.toStdString().c_str()).ToLocalChecked()));
    return ScriptValue(new ScriptValueV8Wrapper(_engine, std::move(result)));
}

ScriptValue ScriptContextV8Wrapper::throwValue(const ScriptValue& value) {
    auto isolate = _engine->getIsolate();
    Q_ASSERT(isolate->IsCurrent());
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    ScriptValueV8Wrapper* unwrapped = ScriptValueV8Wrapper::unwrap(value);
    if (!unwrapped) {
        return _engine->undefinedValue();
    }
    V8ScriptValue result(_engine->getIsolate(), _engine->getIsolate()->ThrowException(unwrapped->toV8Value().constGet()));
    return ScriptValue(new ScriptValueV8Wrapper(_engine, std::move(result)));
}


QString ScriptFunctionContextV8Wrapper::fileName() const {
    //V8TODO
    //return _value.fileName();
    return QString("");
}

QString ScriptFunctionContextV8Wrapper::functionName() const {
    //V8TODO
    //return _value.functionName();
    return QString("");
}

ScriptFunctionContext::FunctionType ScriptFunctionContextV8Wrapper::functionType() const {
    //V8TODO
    //return static_cast<ScriptFunctionContext::FunctionType>(_value.functionType());
    return ScriptFunctionContext::FunctionType::ScriptFunction;
}

int ScriptFunctionContextV8Wrapper::lineNumber() const {
    //V8TODO
    //return _value.lineNumber();
    return 0;
}
