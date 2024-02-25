//
//  ScriptMessage.h
//  libraries/script-engine/src/v8/FastScriptValueUtils.cpp
//
//  Created by dr Karol Suprynowicz on 2023/09/24.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef OVERTE_SCRIPTMESSAGE_H
#define OVERTE_SCRIPTMESSAGE_H

// Used to store script messages on entity script server before transmitting them to clients who subscribed to them.
// EntityServerScriptLog packet type is used.
// In the future will also be used for storing assignment client script messages before transmission

#include <QString>
#include <QJsonObject>
#include "EntityItemID.h"

// SEVERITY_ERROR is defined as a macro in winerror.h
#undef SEVERITY_ERROR

class ScriptMessage {
public:
    enum class ScriptType {
        TYPE_NONE,
        TYPE_ENTITY_SCRIPT
    };
    enum class Severity {
        SEVERITY_NONE,
        SEVERITY_PRINT,
        SEVERITY_INFO,
        SEVERITY_DEBUG,
        SEVERITY_WARNING,
        SEVERITY_ERROR
    };

    ScriptMessage() {};
    ScriptMessage(const QString &messageContent, const QString &fileName, int lineNumber, const EntityItemID& entityID, ScriptType scriptType, Severity severity)
        : _messageContent(messageContent), _fileName(fileName), _lineNumber(lineNumber), _entityID(entityID), _scriptType(scriptType), _severity(severity) {}

    QJsonObject toJson();
    bool fromJson(const QJsonObject &object);

    QString getMessage() { return _messageContent; }
    QString getFileName() { return _fileName; }
    int getLineNumber() { return _lineNumber; }
    ScriptType getScriptType() { return _scriptType; }
    Severity getSeverity() { return _severity; }
    EntityItemID getEntityID() { return _entityID; }

private:
    QString _messageContent;
    QString _fileName;
    int _lineNumber {-1};
    EntityItemID _entityID;
    ScriptType _scriptType {ScriptType::TYPE_NONE};
    Severity _severity {Severity::SEVERITY_NONE};
};

#endif  //OVERTE_SCRIPTMESSAGE_H
