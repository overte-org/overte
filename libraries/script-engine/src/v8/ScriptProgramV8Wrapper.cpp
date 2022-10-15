//
//  ScriptProgramV8Wrapper.cpp
//  libraries/script-engine/src/v8
//
//  Created by Heather Anderson on 8/24/21.
//  Modified for V8 by dr Karol Suprynowicz on 2022/10/08
//  Copyright 2021 Vircadia contributors.
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ScriptProgramV8Wrapper.h"

#include "ScriptEngineV8.h"
#include "ScriptValueV8Wrapper.h"

ScriptProgramV8Wrapper* ScriptProgramV8Wrapper::unwrap(ScriptProgramPointer val) {
    if (!val) {
        return nullptr;
    }

    return dynamic_cast<ScriptProgramV8Wrapper*>(val.get());
}

ScriptSyntaxCheckResultPointer ScriptProgramV8Wrapper::checkSyntax() {
    if (!_isCompiled) {
        compile();
    }
    return std::make_shared<ScriptSyntaxCheckResultV8Wrapper>(_compileResult);
}

bool ScriptProgramV8Wrapper::compile() {
    auto isolate = _engine->getIsolate();
    v8::HandleScope handleScope(isolate);
    auto context = _engine->getContext();
    int errorColumnNumber = 0;
    int errorLineNumber = 0;
    QString errorMessage = "";
    QString errorBacktrace = "";
    ScriptSyntaxCheckResult::State state;
    v8::TryCatch tryCatch(isolate);
    v8::ScriptOrigin scriptOrigin(isolate, v8::String::NewFromUtf8(isolate, _url.toStdString().c_str()).ToLocalChecked());
    v8::Local<v8::Script> script;
    if (v8::Script::Compile(context, v8::String::NewFromUtf8(isolate, _source.toStdString().c_str()).ToLocalChecked(), &scriptOrigin).ToLocal(&script)) {
        qDebug() << "Script compilation succesful: " << _url;
        _compileResult = ScriptSyntaxCheckResultV8Wrapper(ScriptSyntaxCheckResult::Valid);
        _value = V8ScriptProgram(isolate, script);
        return true;
    }
    qDebug() << "Script compilation failed: " << _url;
    v8::String::Utf8Value utf8Value(isolate, tryCatch.Exception());
    errorMessage = QString(*utf8Value);
    v8::Local<v8::Message> exceptionMessage = tryCatch.Message();
    if (!exceptionMessage.IsEmpty()) {
        errorLineNumber = exceptionMessage->GetLineNumber(context).FromJust();
        errorColumnNumber = exceptionMessage->GetStartColumn(context).FromJust();
        v8::Local<v8::Value> backtraceV8String;
        if (tryCatch.StackTrace(context).ToLocal(&backtraceV8String) && backtraceV8String->IsString() &&
                v8::Local<v8::String>::Cast(backtraceV8String)->Length() > 0) {
            v8::String::Utf8Value backtraceUtf8Value(isolate, backtraceV8String);
            errorBacktrace = *backtraceUtf8Value;
        }
    }
    //V8TODO
    _compileResult = ScriptSyntaxCheckResultV8Wrapper(ScriptSyntaxCheckResult::Error, errorColumnNumber, errorLineNumber, errorMessage, errorBacktrace);
    return false;
}
