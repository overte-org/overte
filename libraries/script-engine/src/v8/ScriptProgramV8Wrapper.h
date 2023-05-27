//
//  ScriptProgramV8Wrapper.h
//  libraries/script-engine/src/v8
//
//  Created by Heather Anderson on 5/21/21.
//  Modified for V8 by dr Karol Suprynowicz on 2022/10/08
//  Copyright 2021 Vircadia contributors.
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

/// @addtogroup ScriptEngine
/// @{

#ifndef hifi_ScriptProgramV8Wrapper_h
#define hifi_ScriptProgramV8Wrapper_h

#include <QtCore/QPointer>

#include "../ScriptProgram.h"
#include "ScriptEngineV8.h"
#include "V8Types.h"


// V8TODO: This class is likely unnecessary, and it'd be enough
// to just use a non-abstract version of ScriptSyntaxCheckResult instead.
class ScriptSyntaxCheckResultV8Wrapper final : public ScriptSyntaxCheckResult {
public: // construction
    inline ScriptSyntaxCheckResultV8Wrapper() : _errorColumnNumber(0), _errorLineNumber(0), _errorMessage("Not compiled"), _state(ScriptSyntaxCheckResult::Error) {}
    inline ScriptSyntaxCheckResultV8Wrapper(State state, int columnNumber = 0, int lineNumber = 0, const QString &message = QString(""), const QString &errorBacktrace = QString("")) :
        _errorColumnNumber(columnNumber), _errorLineNumber(lineNumber), _errorMessage(message), _state(state) {}

public: // ScriptSyntaxCheckResult implementation
    virtual int errorColumnNumber() const override {return _errorColumnNumber;}
    virtual int errorLineNumber() const override {return _errorLineNumber;}
    virtual QString errorMessage() const override {return _errorMessage;}
    virtual QString errorBacktrace() const override {return _errorBacktrace;}
    virtual State state() const override {return _state;}

private: // storage
    int _errorColumnNumber;
    int _errorLineNumber;
    QString _errorMessage;
    QString _errorBacktrace;
    State _state;
};

/// [V8] Implements ScriptProgram for V8 and translates calls for V8ScriptProgram
class ScriptProgramV8Wrapper final : public ScriptProgram {
public: // construction
    inline ScriptProgramV8Wrapper(ScriptEngineV8* engine, QString source, QString url) :
    _engine(engine), _source(source), _url(url), _value(_engine) {
        auto isolate = _engine->getIsolate();
        v8::Locker locker(isolate);
        v8::Isolate::Scope isolateScope(isolate);
        v8::HandleScope handleScope(isolate);
        v8::Context::Scope contextScope(_engine->getContext());
        _value = V8ScriptProgram(engine, v8::Local<v8::Script>());
    }
    static ScriptProgramV8Wrapper* unwrap(ScriptProgramPointer val);
    bool compile();
    inline const V8ScriptProgram& toV8Value() const { return _value; }

public: // ScriptProgram implementation
    virtual ScriptSyntaxCheckResultPointer checkSyntax() override;
    virtual QString fileName() const override {return _url;}
    virtual QString sourceCode() const override {return _source;}

private: // storage
    ScriptEngineV8 *_engine;
    QString _source;
    QString _url;
    V8ScriptProgram _value;
    bool _isCompiled = false;
    ScriptSyntaxCheckResultV8Wrapper _compileResult;
};

#endif  // hifi_ScriptValueV8Wrapper_h

/// @}
