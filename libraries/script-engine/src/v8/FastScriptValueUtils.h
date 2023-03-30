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

// Contains V8-specific implementations of th

#ifndef overte_FastScriptValueUtils_h
#define overte_FastScriptValueUtils_h

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../ScriptValue.h"

ScriptValue vec3ToScriptValueFast(ScriptEngine* engine, const glm::vec3& vec3);

bool vec3FromScriptValueFast(const ScriptValue& object, glm::vec3& vec3);

#endif  // overte_FastScriptValueUtils_h
