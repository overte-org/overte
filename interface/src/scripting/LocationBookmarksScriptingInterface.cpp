//
//  AvatarBookmarksScriptingInterface.h
//  interface/src/scripting
//
//  Copyright 2026 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "LocationBookmarksScriptingInterface.h"
#include "ScriptPermissions.h"

#define CHECK_PERMS(x)                                                                                                   \
    if (ScriptPermissions::hasScriptPermission(_manager, ScriptPermissions::Permission::SCRIPT_PERMISSION_BOOKMARKS)) \
    x

void LocationBookmarksScriptingInterface::addBookmark() {
    CHECK_PERMS({ _locationBookmarks->addBookmark(); })
}

void LocationBookmarksScriptingInterface::setHomeLocationToAddress(const QVariant& address){ CHECK_PERMS({
    _locationBookmarks->setHomeLocationToAddress(address);
}) }

QString LocationBookmarksScriptingInterface::getHomeLocationAddress() {
    CHECK_PERMS({ return _locationBookmarks->getHomeLocationAddress(); })
    return {};
}

QString LocationBookmarksScriptingInterface::getAddress(const QString& bookmarkName) {
    CHECK_PERMS({ return _locationBookmarks->getAddress(bookmarkName); })
    return {};
}

QVariantMap LocationBookmarksScriptingInterface::getBookmarks() {
    CHECK_PERMS({ return _locationBookmarks->getBookmarks(); })
    return {};
}

void LocationBookmarksScriptingInterface::addBookmark(const QString& name, const QString& url) {
    CHECK_PERMS({ _locationBookmarks->addBookmark(name, url); })
}

void LocationBookmarksScriptingInterface::removeBookmark(const QString& name) {
    CHECK_PERMS({ _locationBookmarks->removeBookmark(name); })
}
