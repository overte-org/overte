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

#include "ScriptMessage.h"

#include <qhash.h>

QJsonObject ScriptMessage::toJson() {
    QJsonObject object;
    object["message"] = _messageContent;
    object["lineNumber"] = _lineNumber;
    object["fileName"] = _fileName;
    object["type"] = static_cast<int>(_scriptType);
    object["severity"] = static_cast<int>(_severity);
    return object;
}

bool ScriptMessage::fromJson(const QJsonObject &object) {
    if (!object["message"].isString()
        || !object["lineNumber"].isDouble()
        || !object["fileName"].isString()
        || !object["type"].isDouble()
        || !object["severity"].isDouble()) {
        qDebug() << "ScriptMessage::fromJson failed to find required fields in JSON file";
        return false;
    }
    _messageContent = object["message"].toString();
    _lineNumber = object["lineNumber"].toInt();
    _fileName = object["fileName"].toInt();
    _scriptType = static_cast<ScriptMessage::ScriptType>(object["type"].toInt());
    _severity = static_cast<ScriptMessage::Severity>(object["severity"].toInt());
}