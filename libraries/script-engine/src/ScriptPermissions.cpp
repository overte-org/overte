//
//  ScriptPermissions.cpp
//  libraries/script-engine/src/ScriptPermissions.cpp
//
//  Created by dr Karol Suprynowicz on 2024/03/24.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ScriptPermissions.h"

#include <array>
#include <QJsonArray>

#include "ScriptEngine.h"
#include "ScriptManager.h"
#include "Scriptable.h"

static const bool PERMISSIONS_DEBUG_ENABLED = false;

extern const std::array<QString, static_cast<int>(ScriptPermissions::Permission::SCRIPT_PERMISSIONS_SIZE)> scriptPermissionNames {
    "Permission to get user's avatar URL" //SCRIPT_PERMISSION_GET_AVATAR_URL
};

extern const std::array<QString, static_cast<int>(ScriptPermissions::Permission::SCRIPT_PERMISSIONS_SIZE)> scriptPermissionSettingKeyNames {
    "private/scriptPermissionGetAvatarURLSafeURLs" //SCRIPT_PERMISSION_GET_AVATAR_URL
};

extern const std::array<QString, static_cast<int>(ScriptPermissions::Permission::SCRIPT_PERMISSIONS_SIZE)> scriptPermissionSettingEnableKeyNames {
    "private/scriptPermissionGetAvatarURLEnable" //SCRIPT_PERMISSION_GET_AVATAR_URL
};

extern const std::array<bool, static_cast<int>(ScriptPermissions::Permission::SCRIPT_PERMISSIONS_SIZE)> scriptPermissionSettingEnableDefaultValues {
    true //SCRIPT_PERMISSION_GET_AVATAR_URL
};

bool ScriptPermissions::isCurrentScriptAllowed(ScriptPermissions::Permission permission) {
    if (permission >= ScriptPermissions::Permission::SCRIPT_PERMISSIONS_SIZE) {
        return false;
    }
    int permissionIndex = static_cast<int>(permission);
    // Check if the permission checking is active
    Setting::Handle<bool> isCheckingEnabled(scriptPermissionSettingEnableKeyNames[permissionIndex], scriptPermissionSettingEnableDefaultValues[permissionIndex]);
    if (!isCheckingEnabled.get()) {
        return true;
    }
    // Get the script manager:
    auto engine = Scriptable::engine();
    if (!engine) {
        // When this happens it means that function was called from QML or C++ and should always be allowed
        if (PERMISSIONS_DEBUG_ENABLED) {
            qDebug() << "ScriptPermissions::isCurrentScriptAllowed called outside script engine for permission: "
                     << scriptPermissionNames[permissionIndex];
        }
        return true;
    }
    auto manager = engine->manager();
    if (!manager) {
        qDebug() << "ScriptPermissions::isCurrentScriptAllowed called from script engine with no script manager for permission: " << scriptPermissionNames[permissionIndex];
        return false;
    }
    std::vector<QString> urlsToCheck;
    QString scriptURL = manager->getAbsoluteFilename();

    // If this is an entity script manager, we need to find the file name of the current script instead
    if (!scriptURL.startsWith("about:Entities")) {
        urlsToCheck.push_back(scriptURL);
    }

    auto currentURL = Scriptable::context()->currentFileName();
    if (!currentURL.isEmpty() && currentURL != scriptURL) {
        urlsToCheck.push_back(currentURL);
    }

    if (PERMISSIONS_DEBUG_ENABLED) {
        qDebug() << "ScriptPermissions::isCurrentScriptAllowed: filename: " << scriptURL;
    }
    auto parentContext = Scriptable::context()->parentContext();
    while (parentContext) {
        QString parentFilename = parentContext->currentFileName();
        if (!parentFilename.isEmpty()) {
            urlsToCheck.push_back(parentContext->currentFileName());
            if (PERMISSIONS_DEBUG_ENABLED) {
                qDebug() << "ScriptPermissions::isCurrentScriptAllowed: parent filename: " << parentContext->currentFileName();
            }
        }
        parentContext = parentContext->parentContext();
    }

    // Check if the script is allowed:
    QList<QString> safeURLPrefixes = { "file:///", "qrc:/", NetworkingConstants::OVERTE_COMMUNITY_APPLICATIONS,
                                       NetworkingConstants::OVERTE_TUTORIAL_SCRIPTS, "about:console"};
    Setting::Handle<QString> allowedURLsSetting(scriptPermissionSettingKeyNames[permissionIndex]);
    QList<QString> allowedURLs = allowedURLsSetting.get().split("\n");

    for (auto entry : allowedURLs) {
        safeURLPrefixes.push_back(entry);
    }

    for (auto urlToCheck : urlsToCheck) {
        bool urlIsAllowed = false;
        for (const auto& str : safeURLPrefixes) {
            if (!str.isEmpty() && urlToCheck.startsWith(str)) {
                urlIsAllowed = true;
                if (PERMISSIONS_DEBUG_ENABLED) {
                    qDebug() << "ScriptPermissions::isCurrentScriptAllowed: " << scriptPermissionNames[permissionIndex]
                             << " for script " << urlToCheck << " accepted with rule: " << str;
                }
            }
        }

        if (!urlIsAllowed) {
            if (PERMISSIONS_DEBUG_ENABLED) {
                qDebug() << "ScriptPermissions::isCurrentScriptAllowed: " << scriptPermissionNames[permissionIndex]
                         << " for script " << urlToCheck << " rejected.";
            }
            return false;
        }
    }

    return true;
}