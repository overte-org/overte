//
//  VariantMapToScriptValue.h
//  libraries/shared/src/
//
//  Created by Brad Hefta-Gaub on 12/6/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

/// @addtogroup ScriptEngine
/// @{

#include <QVariant>
#include "ScriptValue.h"
#include "ScriptEngine.h"

ScriptValue variantToScriptValue(QVariant& qValue, ScriptEngine& scriptEngine);
ScriptValue variantMapToScriptValue(QVariantMap& variantMap, ScriptEngine& scriptEngine);
ScriptValue variantListToScriptValue(QVariantList& variantList, ScriptEngine& scriptEngine);

/// @}