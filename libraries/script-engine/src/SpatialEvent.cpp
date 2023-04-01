//
//  SpatialEvent.cpp
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

#include "SpatialEvent.h"

#include <RegisteredMetaTypes.h>
#include "ScriptEngine.h"
#include "ScriptValueUtils.h"
#include "ScriptValue.h"
#include "v8/FastScriptValueUtils.h"

SpatialEvent::SpatialEvent() :
    locTranslation(0.0f),
    locRotation(),
    absTranslation(0.0f),
    absRotation()
{
    
}

SpatialEvent::SpatialEvent(const SpatialEvent& event) {
    locTranslation = event.locTranslation;
    locRotation = event.locRotation;
    absTranslation = event.absTranslation;
    absRotation = event.absRotation;
}


ScriptValue SpatialEvent::toScriptValue(ScriptEngine* engine, const SpatialEvent& event) {
    ScriptValue obj = engine->newObject();
    
#ifdef CONVERSIONS_OPTIMIZED_FOR_V8
    obj.setProperty("locTranslation", vec3ToScriptValueFast(engine, event.locTranslation) );
    //V8TODO: optimize
    obj.setProperty("locRotation", quatToScriptValue(engine, event.locRotation) );
    obj.setProperty("absTranslation", vec3ToScriptValueFast(engine, event.absTranslation));
    //V8TODO: optimize
    obj.setProperty("absRotation", quatToScriptValue(engine, event.absRotation));
#else
    obj.setProperty("locTranslation", vec3ToScriptValue(engine, event.locTranslation) );
    obj.setProperty("locRotation", quatToScriptValue(engine, event.locRotation) );
    obj.setProperty("absTranslation", vec3ToScriptValue(engine, event.absTranslation));
    obj.setProperty("absRotation", quatToScriptValue(engine, event.absRotation));
#endif

    return obj;
}

bool SpatialEvent::fromScriptValue(const ScriptValue& object, SpatialEvent& event) {
    // nothing for now...
    return false;
}
