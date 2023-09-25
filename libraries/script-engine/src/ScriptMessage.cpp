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

QJsonObject ScriptMessage::toJson() {
    QJsonObject object;
    object["message"] = _messageContent;
    object["lineNumber"] = _lineNumber;
    object["fileName"] = _fileName;
    object["type"] = scriptTypeToString(_scriptType);
    object["severity"] = severityToString(_severity);
    return object;
}