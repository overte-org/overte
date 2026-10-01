//
//  AvatarBookmarksScriptingInterface.h
//  interface/src/scripting
//
//  Copyright 2026 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "AvatarBookmarksScriptingInterface.h"
#include "Bookmarks.h"
#include "ScriptPermissions.h"

#define CHECK_PERMS(x)                                                                                                        \
    if (ScriptPermissions::hasScriptPermission(_manager, ScriptPermissions::Permission::SCRIPT_PERMISSION_GET_AVATAR_URL)) \
    x

QVariantMap AvatarBookmarksScriptingInterface::getBookmarks() {
    CHECK_PERMS({ return _avatarBookmarks->getBookmarks(); })
    return {};
}

void AvatarBookmarksScriptingInterface::addBookmark(const QString& bookmarkName) {
    CHECK_PERMS({
        _avatarBookmarks->addBookmark(bookmarkName);
        emit bookmarkAdded(bookmarkName);
    })
}

void AvatarBookmarksScriptingInterface::saveBookmark(const QString& bookmarkName) {
    CHECK_PERMS({ _avatarBookmarks->saveBookmark(bookmarkName); })
}

void AvatarBookmarksScriptingInterface::removeBookmark(const QString& bookmarkName) {
    CHECK_PERMS({
        _avatarBookmarks->removeBookmark(bookmarkName);
        emit bookmarkDeleted(bookmarkName);
    })
}

void AvatarBookmarksScriptingInterface::loadBookmark(const QString& bookmarkName){ CHECK_PERMS({
    _avatarBookmarks->loadBookmark(bookmarkName);
}) }

QVariantMap AvatarBookmarksScriptingInterface::getBookmark(const QString& bookmarkName) {
    CHECK_PERMS({ return _avatarBookmarks->getBookmark(bookmarkName); })
    return {};
}

void AvatarBookmarksScriptingInterface::setBookmarkData(const QString& bookmarkName, const QVariantMap& data) {
    CHECK_PERMS({
        auto overwritten = _avatarBookmarks->setBookmarkData(bookmarkName, data);
        if (overwritten) {
            emit bookmarkAdded(bookmarkName);
        }
    })
}
void AvatarBookmarksScriptingInterface::updateAvatarEntities(const QVariantList& avatarEntities) {
    CHECK_PERMS({ _avatarBookmarks->updateAvatarEntities(avatarEntities); })
}
