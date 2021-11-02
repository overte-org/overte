//
//  PathUtils.h
//  libraries/shared/src
//
//  Created by Brad Hefta-Gaub on 12/15/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_PathUtils_h
#define hifi_PathUtils_h

#include <QtCore/QObject>
#include <QtCore/QUrl>
#include <QtCore/QDir>
#include <mutex>
#include "DependencyManager.h"

/*@jsdoc
 * The <code>Paths</code> API provides absolute paths to the scripts and resources directories.
 *
 * @namespace Paths
 *
 * @hifi-interface
 * @hifi-client-entity
 * @hifi-avatar
 *
 * @deprecated The Paths API is deprecated. Use {@link Script.resolvePath} and {@link Script.resourcesPath} instead.
 * @readonly
 * @property {string} defaultScripts - The path to the scripts directory. <em>Read-only.</em>
 * @property {string} resources - The path to the resources directory. <em>Read-only.</em>
 */
class PathUtils : public QObject, public Dependency {
    Q_OBJECT
    SINGLETON_DEPENDENCY
    Q_PROPERTY(QString resources READ resourcesPath CONSTANT)
    Q_PROPERTY(QUrl defaultScripts READ defaultScriptsLocation CONSTANT)
public:
    static const QString& getRccPath();
    static const QString& resourcesUrl();
    static QUrl resourcesUrl(const QString& relative);
    static const QString& resourcesPath();
    static const QString& qmlBaseUrl();
    static QUrl expandToLocalDataAbsolutePath(const QUrl& fileUrl);
    static QUrl qmlUrl(const QString& relative);
#ifdef DEV_BUILD
    static const QString& projectRootPath();
#endif

    static QString getAppDataPath();
    static QString getAppLocalDataPath();

    static QString getAppDataFilePath(const QString& filename);
    static QString getAppLocalDataFilePath(const QString& filename);

    static QString generateTemporaryDir();
    static bool deleteMyTemporaryDir(QString dirName);

    static int removeTemporaryApplicationDirs(QString appName = QString());

    static Qt::CaseSensitivity getFSCaseSensitivity();
    static QString stripFilename(const QUrl& url);
    // note: this is FS-case-sensitive version of parentURL.isParentOf(childURL)
    static bool isDescendantOf(const QUrl& childURL, const QUrl& parentURL);
    static QUrl defaultScriptsLocation(const QString& newDefault = "");


    /**
     * @brief Set the Server Name
     *
     * The server name is used as a component of paths to allow for multiple instances to run on the same machine.
     * The default name is "main".
     *
     * For instance, by default the server-wide config files will be in "/etc/vircadia/main".
     * Starting a second instance with a server name of "Bob" will result in a domain that loads the config from "/etc/vircadia/bob"
     *
     * @param server_name
     */
    static void setServerName(const QString& server_name);

    /**
     * @brief Get the Server Name
     *
     * @return QString Current server name
     */
    static QString getServerName();


    static void setResourcesPath(const QString &resource_dir);

    /**
     * @brief Return the path to the config file
     *
     * On Linux this will return a path within /etc in system mode.
     *
     * @param filename Configuration filename
     * @return QString
     */
    static QString getConfigFilePath(const QString &filename);

    /**
     * @brief Get the location of the describe-settings.json
     * The server name doesn't affect this function, this data is static and shared between instances.
     *
     * @return QString Location of describe-settings.json
     */
    static QString getSettingsDescriptionPath();

    /**
     * @brief Get the location of a server content directory
     *
     * This finds resource directories like 'web' or 'prometheus_exporter'.
     * The server name doesn't affect this function, this data is static and shared between instances.
     *
     * @param dir_name Directory being sought
     * @return QString  Location of the directory
     */
    static QString getServerContentPath(const QString &dir_name);


    /**
     * @brief Get the path of the plugins
     *
     * @return QString Path to the plugins directory
     */
    static QString getPluginsPath();

private:
    /**
     * @brief Initializes the default values
     *
     * Sets the paths to the default ones detected for the system. This may be overriden later from code.
     * Only runs once.
     *
     * The priority order is:
     * 1. Command-like argument (not handled here)
     * 2. Environment variable
     * 3. System-wide path (eg, /usr/share/vircadia). Linux only.
     * 4. Path relative to the executable's location
     */
    static void initialize();

    /**
     * @brief Given a list of paths, find the first one that exists
     *
     * @param paths Paths to check
     * @param description Description of the kind of path being sought, for error messages
     * @return QString Found path or empty string
     */
    static QString findFirstDir(const QStringList &paths, const QString &description);

    // Name for our server instance. This allows us to run multiple instances on the same machine.
    static QString _server_name;

    // Location of the static server resources directory. This is where the 'web' content is found.
    static QString _server_resources_path;

    // Location of the configuration. This may be written by the code
    static QString _config_path;

    // Location of the writable data files
    static QString _appdata_path;

    // Location of the local writable data files
    static QString _local_appdata_path;

    // Location of assignment client plugins
    static QString _plugins_path;

    static bool _initialized;
    static std::mutex _lock;

};

QString fileNameWithoutExtension(const QString& fileName, const QVector<QString> possibleExtensions);
QString findMostRecentFileExtension(const QString& originalFileName, QVector<QString> possibleExtensions);

#endif // hifi_PathUtils_h
