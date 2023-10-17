//
//  Scriptable.cpp
//  libraries/script-engine/src
//
//  Created by Heather Anderson on 5/22/21.
//  Copyright 2021 Vircadia contributors.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "Scriptable.h"

static thread_local ScriptContext* scriptContextStore;

ScriptContext* Scriptable::context() {
    return scriptContextStore;
}

void Scriptable::setContext(ScriptContext* context) {
    scriptContextStore = context;
}
