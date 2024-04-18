//
//  ScriptPermissions.h
//  libraries/script-engine/src/ScriptPermissions.h
//
//  Created by dr Karol Suprynowicz on 2024/03/24.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once

#include <vector>

#include "SettingHandle.h"
#include "DependencyManager.h"

class ScriptPermissions {
public:
    enum class Permission {
        SCRIPT_PERMISSION_GET_AVATAR_URL,
        SCRIPT_PERMISSIONS_SIZE
    };

    static bool isCurrentScriptAllowed(Permission permission);
    //TODO: add a function to request permission through a popup
};

// TODO: add ScriptPermissionsScriptingInterface, where script can check if they have permissions
// and request permissions through a tablet popup.
