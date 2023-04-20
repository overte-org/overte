//
//  KeyEvent.h
//  script-engine/src
//
//  Created by Stephen Birarda on 2014-10-27.
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

/// @addtogroup ScriptEngine
/// @{

#ifndef hifi_KeyEvent_h
#define hifi_KeyEvent_h

#include <QKeyEvent>

#include "ScriptValue.h"

class ScriptEngine;

/// Represents a keyboard event to the scripting engine. Exposed as <code><a href="https://apidocs.overte.org/global.html#KeyEvent">KeyEvent</a></code>
class KeyEvent {
public:
    KeyEvent();
    KeyEvent(const QKeyEvent& event);
    bool operator==(const KeyEvent& other) const;
    operator QKeySequence() const;
    
    static ScriptValue toScriptValue(ScriptEngine* engine, const KeyEvent& event);
    static bool fromScriptValue(const ScriptValue& object, KeyEvent& event);
    
    int key;
    QString text;
    bool isShifted;
    bool isControl;
    bool isMeta;
    bool isAlt;
    bool isKeypad;
    bool isValid;
    bool isAutoRepeat;
};

Q_DECLARE_METATYPE(KeyEvent)

#endif // hifi_KeyEvent_h

/// @}
