//
//  EventTypes.cpp
//  libraries/script-engine/src
//
//  Created by Brad Hefta-Gaub on 1/28/14.
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "EventTypes.h"

#include "KeyEvent.h"
#include "MouseEvent.h"
#include "PointerEvent.h"
#include "ScriptEngine.h"
#include "ScriptEngineCast.h"
#include "SpatialEvent.h"
#include "TouchEvent.h"
#include "WheelEvent.h"

void registerEventTypes(ScriptEngine* engine) {
    scriptRegisterMetaType(engine, KeyEvent::toScriptValue, KeyEvent::fromScriptValue, "KeyEvent");
    scriptRegisterMetaType(engine, MouseEvent::toScriptValue, MouseEvent::fromScriptValue, "MouseEvent");
    scriptRegisterMetaType(engine, PointerEvent::toScriptValue, PointerEvent::fromScriptValue, "PointerEvent");
    scriptRegisterMetaType(engine, TouchEvent::toScriptValue, TouchEvent::fromScriptValue, "TouchEvent");
    scriptRegisterMetaType(engine, WheelEvent::toScriptValue, WheelEvent::fromScriptValue, "WheelEvent");
    scriptRegisterMetaType(engine, SpatialEvent::toScriptValue, SpatialEvent::fromScriptValue, "SpatialEvent");
}
