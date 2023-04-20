//
//  MouseEvent.h
//  script-engine/src
//
//  Created by Stephen Birarda on 2014-10-27.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

/// @addtogroup ScriptEngine
/// @{

#ifndef hifi_MouseEvent_h
#define hifi_MouseEvent_h

#include <QMouseEvent>

#include "ScriptValue.h"

class ScriptEngine;

/// Represents a mouse event to the scripting engine. Exposed as <code><a href="https://apidocs.overte.org/global.html#MouseEvent">MouseEvent</a></code>
class MouseEvent {
public:
    MouseEvent();
    MouseEvent(const QMouseEvent& event);
    
    static ScriptValue toScriptValue(ScriptEngine* engine, const MouseEvent& event);
    static void fromScriptValue(const ScriptValue& object, MouseEvent& event);

    ScriptValue toScriptValue(ScriptEngine* engine) const { return MouseEvent::toScriptValue(engine, *this); }
    
    int x;
    int y;
    QString button;
    bool isLeftButton;
    bool isRightButton;
    bool isMiddleButton;
    bool isShifted;
    bool isControl;
    bool isMeta;
    bool isAlt;
};

Q_DECLARE_METATYPE(MouseEvent)

#endif // hifi_MouseEvent_h

/// @}
