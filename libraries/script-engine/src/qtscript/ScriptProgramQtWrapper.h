//
//  ScriptProgramQtWrapper.h
//  libraries/script-engine/src/qtscript
//
//  Created by Heather Anderson on 5/21/21.
//  Copyright 2021 Vircadia contributors.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

/// @addtogroup ScriptEngine
/// @{

#ifndef hifi_ScriptProgramQtWrapper_h
#define hifi_ScriptProgramQtWrapper_h

#include <QtCore/QPointer>
#include <QtScript/QScriptProgram>

#include "../ScriptProgram.h"
#include "ScriptEngineQtScript.h"

/// [QtScript] Implements ScriptProgram for QtScript and translates calls for QScriptProgram
class ScriptProgramQtWrapper final : public ScriptProgram {
public: // construction
    inline ScriptProgramQtWrapper(ScriptEngineQtScript* engine, const QScriptProgram& value) :
        _engine(engine), _value(value) {}
    inline ScriptProgramQtWrapper(ScriptEngineQtScript* engine, QScriptProgram&& value) :
        _engine(engine), _value(std::move(value)) {}
    static ScriptProgramQtWrapper* unwrap(ScriptProgramPointer val);
    inline const QScriptProgram& toQtValue() const { return _value; }

public: // ScriptProgram implementation
    virtual ScriptSyntaxCheckResultPointer checkSyntax() const override;
    virtual QString fileName() const override;
    virtual QString sourceCode() const override;

private: // storage
    QPointer<ScriptEngineQtScript> _engine;
    QScriptProgram _value;
};

class ScriptSyntaxCheckResultQtWrapper final : public ScriptSyntaxCheckResult {
public: // construction
    inline ScriptSyntaxCheckResultQtWrapper(QScriptSyntaxCheckResult&& value) :
        _value(std::move(value)) {}

public: // ScriptSyntaxCheckResult implementation
    virtual int errorColumnNumber() const override;
    virtual int errorLineNumber() const override;
    virtual QString errorMessage() const override;
    virtual State state() const override;

private: // storage
    QScriptSyntaxCheckResult _value;
};

#endif  // hifi_ScriptValueQtWrapper_h

/// @}