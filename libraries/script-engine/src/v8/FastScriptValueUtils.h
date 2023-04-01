//
//  FastScriptValueUtils.cpp
//  libraries/script-engine/src/v8/FastScriptValueUtils.cpp
//
//  Created by dr Karol Suprynowicz on 2023/03/30.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

// Contains V8-specific implementations of the function converting Overte datatypes to and from script values.
// These are used instead of generic implementations if CONVERSIONS_OPTIMIZED_FOR_V8 is defined.
// V8-specific implementations can make script engine several times faster.

#ifndef overte_FastScriptValueUtils_h
#define overte_FastScriptValueUtils_h

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../ScriptValue.h"

#define CONVERSIONS_OPTIMIZED_FOR_V8

#ifdef CONVERSIONS_OPTIMIZED_FOR_V8
ScriptValue vec3ToScriptValue(ScriptEngine* engine, const glm::vec3& vec3);

bool vec3FromScriptValue(const ScriptValue& object, glm::vec3& vec3);
#endif

#endif  // overte_FastScriptValueUtils_h
