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

ScriptContextV8Wrapper::ScriptContextV8Wrapper(ScriptEngineV8* engine, const v8::Local<v8::Context> context) : _engine(engine) {
    _context.Reset(_engine->getIsolate(), _engine->getConstContext());
}

ScriptContextV8Wrapper::ScriptContextV8Wrapper(ScriptEngineV8* engine, const v8::Local<v8::Context> context, std::shared_ptr<v8::FunctionCallbackInfo<v8::Value>> functionCallbackInfo) : _context(engine->getIsolate(), engine->getConstContext()), _functionCallbackInfo(functionCallbackInfo), _engine(engine)  {
    _context.Reset(_engine->getIsolate(), _engine->getConstContext());
}

ScriptContextV8Wrapper* ScriptContextV8Wrapper::unwrap(ScriptContext* val) {
    if (!val) {
        return nullptr;
    }

    return dynamic_cast<ScriptContextV8Wrapper*>(val);
}

v8::Local<v8::Context> ScriptContextV8Wrapper::toV8Value() const {
    return _context.Get(_engine->getIsolate());
}

int ScriptContextV8Wrapper::argumentCount() const {
    Q_ASSERT(_functionCallbackInfo);
    return _functionCallbackInfo->Length();
}

ScriptValue ScriptContextV8Wrapper::argument(int index) const {
    Q_ASSERT(_functionCallbackInfo);
    v8::Local<v8::Value> result = (*_functionCallbackInfo)[index];
    //V8ScriptValue result = _context->argument(index);
    return ScriptValue(new ScriptValueV8Wrapper(_engine, V8ScriptValue(_engine->getIsolate(), result)));
}

QStringList ScriptContextV8Wrapper::backtrace() const {
    auto isolate = _engine->getIsolate();
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
    //V8TODO
    //Can this be done with CurrentStackTrace?
    //V8ScriptValue result = _context->callee();
    //return ScriptValue(new ScriptValueV8Wrapper(_engine, std::move(result)));
    return ScriptValue();
}

ScriptEnginePointer ScriptContextV8Wrapper::engine() const {
    return _engine->shared_from_this();
}

ScriptFunctionContextPointer ScriptContextV8Wrapper::functionContext() const {
    return std::make_shared<ScriptFunctionContextV8Wrapper>(_context.Get(_engine->getIsolate()));
}

ScriptContextPointer ScriptContextV8Wrapper::parentContext() const {
    //V8TODO
    //V8ScriptContext* result = _context->parentContext();
    //return result ? std::make_shared<ScriptContextV8Wrapper>(_engine, result) : ScriptContextPointer();
    return ScriptContextPointer();
}

ScriptValue ScriptContextV8Wrapper::thisObject() const {
    Q_ASSERT(_functionCallbackInfo);
    v8::Local<v8::Value> result = _functionCallbackInfo->This();
    return ScriptValue(new ScriptValueV8Wrapper(_engine, V8ScriptValue(_engine->getIsolate(), result)));
    return ScriptValue();
}

ScriptValue ScriptContextV8Wrapper::throwError(const QString& text) {
    V8ScriptValue result(_engine->getIsolate(), _engine->getIsolate()->ThrowError(v8::String::NewFromUtf8(_engine->getIsolate(), text.toStdString().c_str()).ToLocalChecked()));
    return ScriptValue(new ScriptValueV8Wrapper(_engine, std::move(result)));
}

ScriptValue ScriptContextV8Wrapper::throwValue(const ScriptValue& value) {
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
