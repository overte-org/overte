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
    static QString getConfigPath();	// I suspect there may be a way to avoid this.
    static QString getAppConfigPath();

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
};

QString fileNameWithoutExtension(const QString& fileName, const QVector<QString> possibleExtensions);
QString findMostRecentFileExtension(const QString& originalFileName, QVector<QString> possibleExtensions);

#endif // hifi_PathUtils_h
