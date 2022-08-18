//
//  ResourceScriptingInterface.cpp
//  libraries/script-engine
//
//  Created by dr Karol Suprynowicz on 2022/08/18.
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <QtCore/QObject>
#include <QtCore/QString>

#include <ResourceCache.h>

#include "ScriptEngineCast.h"
#include "ScriptManager.h"

STATIC_SCRIPT_TYPES_INITIALIZER(+[](ScriptManager* manager){
    auto scriptEngine = manager->engine().get();

    scriptRegisterMetaType(scriptEngine, scriptValueFromEnumClass<ScriptableResource::State>, scriptValueToEnumClass<ScriptableResource::State>, "State");
});
