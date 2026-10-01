//
//  AvatarBookmarksScriptingInterface.h
//  interface/src/scripting
//
//  Copyright 2026 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once

#include <qobject.h>
#include "AvatarBookmarks.h"
#include "ScriptManager.h"

class AvatarBookmarksScriptingInterface : public QObject {
    Q_OBJECT
public:
    AvatarBookmarksScriptingInterface(AvatarBookmarks* avatarBookmarks, ScriptManager* manager) :
        QObject(nullptr), _avatarBookmarks(avatarBookmarks), _manager(manager) {
        connect(_avatarBookmarks, &AvatarBookmarks::bookmarkLoaded, this, &AvatarBookmarksScriptingInterface::bookmarkLoaded);
    }

    /*@jsdoc
     * Gets the details of an avatar bookmark.
     * @function AvatarBookmarks.getBookmark
     * @param {string} bookmarkName - The name of the avatar bookmark (case sensitive).
     * @returns {AvatarBookmarks.BookmarkData|{}} The bookmark data if the bookmark exists, <code>{}</code> if it doesn't.
     */
    Q_INVOKABLE QVariantMap getBookmark(const QString& bookmarkName);

public slots:
    /*@jsdoc
     * Adds a new (or updates an existing) avatar bookmark with your current avatar model, scale, and avatar entities.
     * @function AvatarBookmarks.addBookmark
     * @param {string} bookmarkName - The name of the avatar bookmark (case sensitive).
     * @example <caption>Add a new avatar bookmark and report the bookmark data.</caption>
     * var bookmarkName = "New Bookmark";
     * AvatarBookmarks.addBookmark(bookmarkName);
     * var bookmarkData = AvatarBookmarks.getBookmark(bookmarkName);
     * print("Bookmark data: " + JSON.stringify(bookmarkData));
     */
    void addBookmark(const QString& bookmarkName);

    /*@jsdoc
     * Updates an existing bookmark with your current avatar model, scale, and wearables. No action is taken if the bookmark
     * doesn't exist.
     * @function AvatarBookmarks.saveBookmark
     * @param {string} bookmarkName - The name of the avatar bookmark (case sensitive).
     */
    void saveBookmark(const QString& bookmarkName);

    /*@jsdoc
     * Loads an avatar bookmark, setting your avatar model, scale, and avatar entities to those in the bookmark.
     * @function AvatarBookmarks.loadBookmark
     * @param {string} bookmarkName - The name of the avatar bookmark to load (case sensitive).
     */
    void loadBookmark(const QString& bookmarkName);

    /*@jsdoc
     * Deletes an avatar bookmark.
     * @function AvatarBookmarks.removeBookmark
     * @param {string} bookmarkName - The name of the avatar bookmark to delete (case sensitive).
     */
    void removeBookmark(const QString& bookmarkName);

    /*@jsdoc
     * Updates the avatar entities and their properties. Current avatar entities not included in the list provided are deleted.
     * @function AvatarBookmarks.updateAvatarEntities
     * @param {MyAvatar.AvatarEntityData[]} avatarEntities - The avatar entity IDs and properties.
     * @deprecated This function is deprecated and will be removed. Use the {@link MyAvatar} API instead.
     */
    void updateAvatarEntities(const QVariantList& avatarEntities);

    /*@jsdoc
     * Gets the details of all avatar bookmarks.
     * @function AvatarBookmarks.getBookmarks
     * @returns {Object<string,AvatarBookmarks.BookmarkData>} The current avatar bookmarks in an object where the keys are the
     *     bookmark names and the values are the bookmark details.
     * @example <caption>List the names and URLs of all the avatar bookmarks.</caption>
     * var bookmarks = AvatarBookmarks.getBookmarks();
     * print("Avatar bookmarks:");
     * for (var key in bookmarks) {
     *     print("- " + key + " " + bookmarks[key].avatarUrl);
     * };
     */
    QVariantMap getBookmarks();

    /*@jsdoc
     * Directly sets the data object of a bookmark.
     * @function AvatarBookmarks.setBookmarkData
     * @param {string} - The name of the bookmark. If no bookmark with the specified name exists, it will be created. Otherwise, it will edit the existing one.
     * @param {object} - The bookmark data. See the example below for valid keys and their defaults.
     * @example
     * AvatarBookmarks.setBookmarkData("Woody", {
     *     version: 3,
     *     avatarScale: 1.0,
     *     avatarEntities: [],
     *     avatarIcon: "",
     *     avatarUrl: "qrc:///meshes/defaultAvatar_full.fst",
     * });
     */
    void setBookmarkData(const QString& bookmarkName, const QVariantMap& data);

signals:
    /*@jsdoc
     * Triggered when an avatar bookmark is loaded, setting your avatar model, scale, and avatar entities to those in the bookmark.
     * @function AvatarBookmarks.bookmarkLoaded
     * @param {string} bookmarkName - The name of the avatar bookmark loaded.
     * @returns {Signal}
     */
    void bookmarkLoaded(const QString& bookmarkName);

    /*@jsdoc
     * Triggered when an avatar bookmark is deleted.
     * @function AvatarBookmarks.bookmarkDeleted
     * @param {string} bookmarkName - The name of the avatar bookmark deleted.
     * @returns {Signal}
     * @example <caption>Report when a bookmark is deleted.</caption>
     * AvatarBookmarks.bookmarkDeleted.connect(function (bookmarkName) {
     *     print("Bookmark deleted: " + bookmarkName);
     * });
     */
    void bookmarkDeleted(const QString& bookmarkName);

    /*@jsdoc
     * Triggered when a new avatar bookmark is added or an existing avatar bookmark is updated, using
     * {@link AvatarBookmarks.addBookmark|addBookmark}.
     * @function AvatarBookmarks.bookmarkAdded
     * @param {string} bookmarkName - The name of the avatar bookmark added or updated.
     * @returns {Signal}
     */
    void bookmarkAdded(const QString& bookmarkName);

private:
    AvatarBookmarks* _avatarBookmarks{ nullptr };
    ScriptManager* _manager{ nullptr };
};
