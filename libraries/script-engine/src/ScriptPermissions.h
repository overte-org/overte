//
//  ScriptPermissions.h
//  libraries/script-engine/src/ScriptPermissions.h
//
//  Created by dr Karol Suprynowicz on 2024/03/24.
//  Copyright 2024-2026 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once

#include "ScriptManager.h"

class ScriptPermissions {
public:
    enum class Permission
    {
        SCRIPT_PERMISSION_GET_AVATAR_URL,
        SCRIPT_PERMISSION_BOOKMARKS,
        SCRIPT_PERMISSION_KEYBOARD_EVENTS,
        SCRIPT_PERMISSIONS_SIZE
    };

    static bool hasScriptPermission(ScriptManager* manager, Permission permission);
    //TODO: add a function to request permission through a popup
};

// TODO: add ScriptPermissionsScriptingInterface, where script can check if they have permissions
// and request permissions through a tablet popup.
