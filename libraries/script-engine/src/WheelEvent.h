//
//  WheelEvent.h
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

#ifndef hifi_WheelEvent_h
#define hifi_WheelEvent_h

#include <QString>
#include <QWheelEvent>

#include "ScriptValue.h"

class ScriptEngine;

/// Represents a mouse wheel event to the scripting engine. Exposed as <code><a href="https://apidocs.overte.org/global.html#WheelEvent">WheelEvent</a></code>
class WheelEvent {
public:
    WheelEvent();
    WheelEvent(const QWheelEvent& event);
    
    static ScriptValue toScriptValue(ScriptEngine* engine, const WheelEvent& event);
    static void fromScriptValue(const ScriptValue& object, WheelEvent& event);
    
    int x;
    int y;
    int delta;
    QString orientation;
    bool isLeftButton;
    bool isRightButton;
    bool isMiddleButton;
    bool isShifted;
    bool isControl;
    bool isMeta;
    bool isAlt;
};

Q_DECLARE_METATYPE(WheelEvent)

#endif // hifi_WheelEvent_h

/// @}
