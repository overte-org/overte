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
#include <qtmetamacros.h>
#include "LocationBookmarks.h"
#include "ScriptManager.h"

/*@jsdoc
 * The <code>LocationBookmarks</code> API provides facilities for working with location bookmarks. A location bookmark
 * associates a name with a directory services address.
 *
 * <p><strong>Note:</strong> This is a protected API and it is only available
 * to scripts with the appropriate permissions. Without them, the functions in
 * this namespace won't do anything and will return empty values.</p>
 *
 * @namespace LocationBookmarks
 *
 * @hifi-client-entity
 * @hifi-interface
 * @hifi-avatar
 */
class LocationBookmarksScriptingInterface : public QObject {
    Q_OBJECT
public:
    LocationBookmarksScriptingInterface(LocationBookmarks* locationBookmarks, ScriptManager* manager) :
        QObject(nullptr), _locationBookmarks(locationBookmarks), _manager(manager) {}

    /*@jsdoc
     * Gets the directory services address associated with a bookmark.
     * @function LocationBookmarks.getAddress
     * @param {string} bookmarkName - Name of the bookmark to get the directory services address for (case sensitive).
     * @returns {string} The directory services address for the bookmark. If the bookmark does not exist, <code>""</code> is returned.
     * @example <caption>Report the "Home" bookmark's directory services address.</caption>
     * print("Home bookmark's address: " + LocationBookmarks.getAddress("Home"));
     */
    Q_INVOKABLE QString getAddress(const QString& bookmarkName);

public slots:

    /*@jsdoc
     * Prompts the user to bookmark their current location. The user can specify the name of the bookmark in the dialog that is
     * opened.
     * @function LocationBookmarks.addBookmark
     */
    void addBookmark();

    /*@jsdoc
     * Sets the directory services address associated with the "Home" bookmark.
     * @function LocationBookmarks.setHomeLocationToAddress
     * @param {string} address - The directory services address to set the "Home" bookmark to.
     */
    void setHomeLocationToAddress(const QVariant& address);

    /*@jsdoc
     * Gets the directory services address associated with the "Home" bookmark.
     * @function LocationBookmarks.getHomeLocationAddress
     * @returns {string} The directory services address for the "Home" bookmark.
     */
    QString getHomeLocationAddress();

    /*@jsdoc
     * Gets the details of all bookmarks.
     * @function LocationBookmarks.getBookmarks
     * @returns {object} The current bookmarks in an object where the keys
     * are the bookmark names and the values are the bookmark URLs.
     * @example <caption>List the names and URLs of all the bookmarks.</caption>
     * var bookmarks = LocationBookmarks.getBookmarks();
     * print("Location bookmarks:");
     * for (const [name, url] of Object.entries(bookmarks)) {
     *     print(`- ${name} ${url}`);
     * }
     */
    QVariantMap getBookmarks();

    /*@jsdoc
     * Adds a new bookmark or replaces an existing one.
     * @function LocationBookmarks.addBookmark
     * @param {string} name - The name of the bookmark.
     * @param {string} url - The bookmark's URL.
     */
    void addBookmark(const QString& name, const QString& url);

    /*@jsdoc
     * Deletes a bookmark, if it exists.
     * @function LocationBookmarks.removeBookmark
     * @param {string} name - The name of the bookmark.
     */
    void removeBookmark(const QString& name);

private:
    LocationBookmarks* _locationBookmarks{ nullptr };
    ScriptManager* _manager{ nullptr };
};
