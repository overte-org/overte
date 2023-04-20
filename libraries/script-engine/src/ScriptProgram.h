//
//  ScriptProgram.h
//  libraries/script-engine/src
//
//  Created by Heather Anderson on 5/2/21.
//  Copyright 2021 Vircadia contributors.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

/// @addtogroup ScriptEngine
/// @{

#ifndef hifi_ScriptProgram_h
#define hifi_ScriptProgram_h

#include <memory>

class ScriptProgram;
class ScriptSyntaxCheckResult;
using ScriptProgramPointer = std::shared_ptr<ScriptProgram>;
using ScriptSyntaxCheckResultPointer = std::shared_ptr<ScriptSyntaxCheckResult>;

/// [ScriptInterface] Provides an engine-independent interface for QScriptProgram
class ScriptProgram {
public:
    virtual ScriptSyntaxCheckResultPointer checkSyntax() = 0; //It cannot be const anymore because V8 doesn't have separate syntax checking function
    virtual QString fileName() const = 0;
    virtual QString sourceCode() const = 0;

protected:
    ~ScriptProgram() {}  // prevent explicit deletion of base class
};

/// [ScriptInterface] Provides an engine-independent interface for QScriptSyntaxCheckResult
class ScriptSyntaxCheckResult {
public:
    enum State
    {
        Error = 0,
        Intermediate = 1,
        Valid = 2
    };

public:
    virtual int errorColumnNumber() const = 0;
    virtual int errorLineNumber() const = 0;
    virtual QString errorMessage() const = 0;
    virtual QString errorBacktrace() const = 0;
    virtual State state() const = 0;

protected:
    ~ScriptSyntaxCheckResult() {}  // prevent explicit deletion of base class
};

#endif // hifi_ScriptProgram_h

/// @}
