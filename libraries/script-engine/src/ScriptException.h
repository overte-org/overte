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

#pragma once

/**
 * @brief Scripting exception
 *
 * Emitted from the scripting engine when an exception happens inside it.
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