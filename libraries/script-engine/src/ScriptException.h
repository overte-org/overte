//
//  ScriptException.h
//  libraries/script-engine/src
//
//  Created by Dale Glass on 27/02/2023.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

#include <QObject>
#include <QString>
#include <QStringList>

#include "ScriptValue.h"

#pragma once

/**
 * @brief Scripting exception
 *
 * Emitted from the scripting engine when an exception happens inside it.
 * This is the base class.
 *
 */
class ScriptException {
    public:

    ScriptException(QString message = "", QString info = "", int line = 0, int column = 0, QStringList backtraceList = QStringList()) :
        errorMessage(message), additionalInfo(info), errorLine(line), errorColumn(column), backtrace(backtraceList) {

    }

    /**
     * @brief Error message
     *
     */
    QString errorMessage;

    /**
     * @brief Additional information about the exception
     *
     * This is additional information added at the place where the exception happened.
     * It may contain information about what the system was doing when the exception happened.
     *
     */
    QString additionalInfo;


    /**
     * @brief Error line
     *
     */
    int errorLine;

    /**
     * @brief Error column
     *
     */
    int errorColumn;

    /**
     * @brief Backtrace
     *
     */
    QStringList backtrace;

    bool isEmpty() const { return errorMessage.isEmpty(); }

    /**
     * @brief Clones this object.
     *
     * This is used in the scripting engine to ensure that while it can return different
     * exception objects depending on what happened, the returned object is a copy that
     * doesn't allow the caller to accidentally break the ScriptEngine's internal state.
     *
     * @return std::shared_ptr<ScriptException>
     */
    virtual std::shared_ptr<ScriptException> clone() const {
        return std::make_shared<ScriptException>(*this);
    }
};


/**
 * @brief Exception that ocurred inside the scripting engine on the c++ side
 *
 * This is something that went wrong inside the ScriptEngine or ScriptManager
 * infrastructure.
 *
 */
class ScriptEngineException : public ScriptException {
    public:

    ScriptEngineException(QString message = "", QString info = "", int line = 0, int column = 0, QStringList backtraceList = QStringList()) :
        ScriptException(message, info, line, column, backtraceList) {

    }


    /**
     * @brief Clones this object.
     *
     * This is used in the scripting engine to ensure that while it can return different
     * exception objects depending on what happened, the returned object is a copy that
     * doesn't allow the caller to accidentally break the ScriptEngine's internal state.
     *
     * @return std::shared_ptr<ScriptException>
     */
    virtual std::shared_ptr<ScriptException> clone() const override {
        return std::make_shared<ScriptEngineException>(*this);
    }
};



/**
 * @brief Exception that ocurred inside the running script
 *
 * This is something that went wrong inside the running script.
 *
 */
class ScriptRuntimeException : public ScriptException {
    public:

    ScriptRuntimeException(QString message = "", QString info = "", int line = 0, int column = 0, QStringList backtraceList = QStringList()) :
        ScriptException(message, info, line, column, backtraceList) {

    }


    /**
     * @brief The actual value that was thrown by the script.
     *
     * The content is completely arbitrary.
     *
     */
    ScriptValue thrownValue;

    /**
     * @brief Clones this object.
     *
     * This is used in the scripting engine to ensure that while it can return different
     * exception objects depending on what happened, the returned object is a copy that
     * doesn't allow the caller to accidentally break the ScriptEngine's internal state.
     *
     * @return std::shared_ptr<ScriptException>
     */
    virtual std::shared_ptr<ScriptException> clone() const override {
        return std::make_shared<ScriptRuntimeException>(*this);
    }
};

inline QDebug operator<<(QDebug debug, const ScriptException& e) {
    debug << "Exception:"
        << e.errorMessage
        << (e.additionalInfo.isEmpty() ? QString("") : "[" + e.additionalInfo + "]")
        << " at line " << e.errorLine << ", column " << e.errorColumn;

    if (e.backtrace.length()) {
        debug << "Backtrace:";
        debug << e.backtrace;
    }

    return debug;
}

// Is this a bad practice?
inline QDebug operator<<(QDebug debug, std::shared_ptr<ScriptException> e) {
    if (!e) {
        debug << "[Null ScriptException]";
        return debug;
    }

    debug << *e.get();
    return debug;
}