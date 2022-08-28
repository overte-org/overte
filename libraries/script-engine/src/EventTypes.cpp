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
#include "ScriptManager.h"
#include "SpatialEvent.h"
#include "TouchEvent.h"
#include "WheelEvent.h"

STATIC_SCRIPT_TYPES_INITIALIZER((+[](ScriptManager* manager){
    auto scriptEngine = manager->engine().get();
    
    scriptRegisterMetaType<KeyEvent, KeyEvent::toScriptValue, KeyEvent::fromScriptValue>(scriptEngine, "KeyEvent");
    scriptRegisterMetaType<MouseEvent, MouseEvent::toScriptValue, MouseEvent::fromScriptValue>(scriptEngine, "MouseEvent");
    scriptRegisterMetaType<PointerEvent, PointerEvent::toScriptValue, PointerEvent::fromScriptValue>(scriptEngine, "PointerEvent");
    scriptRegisterMetaType<TouchEvent, TouchEvent::toScriptValue, TouchEvent::fromScriptValue>(scriptEngine, "TouchEvent");
    scriptRegisterMetaType<WheelEvent, WheelEvent::toScriptValue, WheelEvent::fromScriptValue>(scriptEngine, "WheelEvent");
    scriptRegisterMetaType<SpatialEvent, SpatialEvent::toScriptValue, SpatialEvent::fromScriptValue>(scriptEngine, "SpatialEvent");
}));
