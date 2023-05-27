//
//  ScriptProgram.h
//  libraries/script-engine/src
//
//  Created by Heather Anderson on 5/2/21.
//  Copyright 2021 Vircadia contributors.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
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

/**
 * @brief Engine-independent representation of a script program
 *
 * This is an analog of QScriptProgram from Qt5.
 *
 * It's used to pre-compile scripts, and to check their syntax.
 *
 */
class ScriptProgram {
public:
    virtual ScriptSyntaxCheckResultPointer checkSyntax() = 0; //It cannot be const anymore because V8 doesn't have separate syntax checking function

    /**
     * @brief Returns the filename associated with this program.
     *
     * @return QString
     */
    virtual QString fileName() const = 0;

    /**
     * @brief Returns the source code of this program.
     *
     * @return QString
     */
    virtual QString sourceCode() const = 0;

protected:
    ~ScriptProgram() {}  // prevent explicit deletion of base class
};


/**
 * @brief Engine-independent representation of a script syntax check
 *
 * This is an analog of QScriptSyntaxCheckResult from Qt5.
 *
 *
 */
class ScriptSyntaxCheckResult {
public:

    /**
     * @brief State of the syntax check
     *
     */
    enum State
    {
        Error = 0, /** The program contains a syntax error. */
        Intermediate = 1, /** The program is incomplete. */
        Valid = 2 /** The program is a syntactically correct program. */
    };

public:

    /**
     * @brief Returns the error column number of this ScriptSyntaxCheckResult, or -1 if there is no error.
     *
     * @return int
     */
    virtual int errorColumnNumber() const = 0;

    /**
     * @brief Returns the error line number of this ScriptSyntaxCheckResult, or -1 if there is no error.
     *
     * @return int
     */
    virtual int errorLineNumber() const = 0;

    /**
     * @brief Returns the error message of this ScriptSyntaxCheckResult, or an empty string if there is no error.
     *
     * @return QString
     */
    virtual QString errorMessage() const = 0;

    /**
     * @brief
     *
     * @return QString
     */
    virtual QString errorBacktrace() const = 0;

    /**
     * @brief Returns the state of this ScriptSyntaxCheckResult.
     *
     * @return State
     */
    virtual State state() const = 0;

protected:
    ~ScriptSyntaxCheckResult() {}  // prevent explicit deletion of base class
};

#endif // hifi_ScriptProgram_h

/// @}
