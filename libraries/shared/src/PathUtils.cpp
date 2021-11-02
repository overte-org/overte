//
//  PathUtils.cpp
//  libraries/shared/src
//
//  Created by Brad Hefta-Gaub on 12/15/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "PathUtils.h"

#include <mutex> // std::once

#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFileInfo>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QRegularExpression>
#include <QtCore/QStandardPaths>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QVector>

#if defined(Q_OS_OSX)
#include <mach-o/dyld.h>
#endif

#ifdef Q_OS_LINUX
#include <unistd.h>
#include <sys/types.h>
#endif

#include "shared/GlobalAppProperties.h"
#include "SharedUtil.h"

// These are the default, system paths.
//
// The default values are only defined on Linux. On Windows and OSX the correct
// procedure is to calculate the paths relative to where the binary is located.
#ifdef Q_OS_LINUX
#  ifndef VIRCADIA_DEFAULT_RESOURCES_PATH
#    define VIRCADIA_DEFAULT_RESOURCES_PATH "/usr/share/vircadia/resources"
#  endif
#  ifndef VIRCADIA_DEFAULT_CONFIG_PATH
#    define VIRCADIA_DEFAULT_CONFIG_PATH "/etc/vircadia"
#  endif
#  ifndef VIRCADIA_DEFAULT_APPDATA_PATH
#    define VIRCADIA_DEFAULT_APPDATA_PATH "/var/lib/vircadia"
#  endif
#  ifndef VIRCADIA_DEFAULT_LOCAL_APPDATA_PATH
#    define VIRCADIA_DEFAULT_LOCAL_APPDATA_PATH "/var/lib/vircadia"
#  endif
#  ifndef VIRCADIA_DEFAULT_PLUGINS_PATH
#    define VIRCADIA_DEFAULT_PLUGINS_PATH "/usr/lib/vircadia/plugins"
#  endif
#endif


QString PathUtils::_server_resources_path{""};
QString PathUtils::_config_path{""};
QString PathUtils::_appdata_path{""};
QString PathUtils::_local_appdata_path{""};
QString PathUtils::_plugins_path{""};
QString PathUtils::_server_name{"main"};
bool PathUtils::_initialized{false};
std::mutex PathUtils::_lock{};



// Format: AppName-PID-Timestamp
// Example: ...
QString TEMP_DIR_FORMAT { "%1-%2-%3" };

#if !defined(Q_OS_ANDROID) && defined(DEV_BUILD)
static bool USE_SOURCE_TREE_RESOURCES() {
    static bool result = false;
    static std::once_flag once;
    std::call_once(once, [&] {
        const QString USE_SOURCE_TREE_RESOURCES_FLAG("HIFI_USE_SOURCE_TREE_RESOURCES");
        result = QProcessEnvironment::systemEnvironment().contains(USE_SOURCE_TREE_RESOURCES_FLAG);
    });
    return result;
}
#endif

/**
 * @brief Whether the user we're running under is a system user
 *
 * This only applies on Linux systems. When running under a system account (uid < 1000),
 * we're assuming the server is installed server-wide as a package. In that case we'll be
 * looking for files in system paths like /usr/share/vircadia.
 *
 * When running as a normal user account we look for data relative to the binary's location,
 * and storing data in user-local directories like ~/.local/share.
 *
 * @return true If it's a system user
 * @return false Non-system user on Linux, or not a Linux system.
 */
static bool isSystemUser() {
    if ( qgetenv("VIRCADIA_FORCE_SYSTEM_PATHS").length() > 0 ) {
        // This is here to make debugging easier -- not intended as an useful setting for end-users.
        qWarning() << "Forced usage of system paths through VIRCADIA_FORCE_SYSTEM_PATHS";
        return true;
    }

#ifdef Q_OS_LINUX
    // There doesn't appear to be an API to easily fetch SYS_UID_MAX from /etc/login.defs other than
    // actually parsing it.
    //
    // systemd appears to have resulted in things having standardized on 1000 being the minimum UID
    // that can be given to a normal user account. This applies both on Red Hat and Debian based
    // distros. So it seems we can avoid doing any parsing and just make this assumption here.

    return getuid() < 1000;
#else
    return false;
#endif
}

const QString& PathUtils::getRccPath() {
    static QString rccLocation;
    static std::once_flag once;
    std::call_once(once, [&] {
        static const QString rccName{ "/resources.rcc" };
#if defined(Q_OS_OSX)
        char buffer[8192];
        uint32_t bufferSize = sizeof(buffer);
        _NSGetExecutablePath(buffer, &bufferSize);
        rccLocation = QDir::cleanPath(QFileInfo(buffer).dir().absoluteFilePath("../Resources")) + rccName;
#elif defined(Q_OS_ANDROID)
        rccLocation = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + rccName;
#else
        rccLocation = QCoreApplication::applicationDirPath() + rccName;
#endif
    });
    return rccLocation;
}

#ifdef DEV_BUILD
const QString& PathUtils::projectRootPath() {
    static QString sourceFolder;
    static std::once_flag once;
    std::call_once(once, [&] {
        QDir thisDir = QFileInfo(__FILE__).absoluteDir();
        sourceFolder = QDir::cleanPath(thisDir.absoluteFilePath("../../../"));
    });
    return sourceFolder;
}
#endif

const QString& PathUtils::resourcesPath() {
    static QString staticResourcePath{ ":/" };
    static std::once_flag once;
    std::call_once(once, [&]{
#if !defined(Q_OS_ANDROID) && defined(DEV_BUILD)
        if (USE_SOURCE_TREE_RESOURCES()) {
            // For dev builds, optionally load content from the Git source tree
            staticResourcePath = projectRootPath() + "/interface/resources/";
        }
#endif
    });
    return staticResourcePath;
}

const QString& PathUtils::resourcesUrl() {
    static QString staticResourcePath{ "qrc:///" };
    static std::once_flag once;
    std::call_once(once, [&]{
#if !defined(Q_OS_ANDROID) && defined(DEV_BUILD)
        if (USE_SOURCE_TREE_RESOURCES()) {
            // For dev builds, optionally load content from the Git source tree
            staticResourcePath = QUrl::fromLocalFile(projectRootPath() + "/interface/resources/").toString();
        }
#endif
    });
    return staticResourcePath;
}


QUrl PathUtils::resourcesUrl(const QString& relativeUrl) {
    return QUrl(resourcesUrl() + relativeUrl);
}

QUrl PathUtils::expandToLocalDataAbsolutePath(const QUrl& fileUrl) {
    QString path = fileUrl.path();

    if (path.startsWith("/~/")) {
        // this results in a qrc:// url...
        // return resourcesUrl(path.mid(3));

#ifdef Q_OS_MAC
        static const QString staticResourcePath = QCoreApplication::applicationDirPath() + "/../Resources/";
#elif defined (ANDROID)
        static const QString staticResourcePath =
            QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/resources/";
#else
        static const QString staticResourcePath = QCoreApplication::applicationDirPath() + "/resources/";
#endif
        path.replace(0, 3, staticResourcePath);
        QUrl expandedURL = QUrl::fromLocalFile(path);
        return expandedURL;
    }

    return fileUrl;
}

const QString& PathUtils::qmlBaseUrl() {
    static const QString staticResourcePath = resourcesUrl() + "qml/";
    return staticResourcePath;
}

QUrl PathUtils::qmlUrl(const QString& relativeUrl) {
    return QUrl(qmlBaseUrl() + relativeUrl);
}

QString PathUtils::getAppDataPath() {
    std::lock_guard<std::mutex> guard(_lock);
    initialize();
    return _appdata_path + "/";
    //return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/";
}

QString PathUtils::getAppLocalDataPath() {
    QString overriddenPath = qApp->property(hifi::properties::APP_LOCAL_DATA_PATH).toString();
    // return overridden path if set
    if (!overriddenPath.isEmpty()) {
        return overriddenPath;
    }

    // otherwise return standard path
#ifdef Q_OS_ANDROID
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/";
#else
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/";
#endif
}

QString PathUtils::getAppDataFilePath(const QString& filename) {
    return QDir(getAppDataPath()).absoluteFilePath(filename);
}

QString PathUtils::getAppLocalDataFilePath(const QString& filename) {
    return QDir(getAppLocalDataPath()).absoluteFilePath(filename);
}

QString PathUtils::generateTemporaryDir() {
    QDir rootTempDir = QDir::tempPath();
    QString appName = qApp->applicationName();
    for (auto i = 0; i < 64; ++i) {
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        auto dirName = TEMP_DIR_FORMAT.arg(appName).arg(qApp->applicationPid()).arg(now);
        QDir tempDir = rootTempDir.filePath(dirName);
        if (tempDir.mkpath(".")) {
            return tempDir.absolutePath();
        }
    }
    return "";
}

bool PathUtils::deleteMyTemporaryDir(QString dirName) {
    QDir rootTempDir = QDir::tempPath();

    QString appName = qApp->applicationName();
    QRegularExpression re { "^" + QRegularExpression::escape(appName) + "\\-(?<pid>\\d+)\\-(?<timestamp>\\d+)$" };

    auto match = re.match(dirName);
    auto pid = match.capturedRef("pid").toLongLong();

    if (match.hasMatch() && rootTempDir.exists(dirName) && pid == qApp->applicationPid()) {
        auto absoluteDirPath = QDir(rootTempDir.absoluteFilePath(dirName));

        bool success = absoluteDirPath.removeRecursively();
        if (success) {
            qDebug() << "  Removing temporary directory: " << absoluteDirPath.absolutePath();
        } else {
            qDebug() << "  Failed to remove temporary directory: " << absoluteDirPath.absolutePath();
        }
        return success;
    }

    return false;
}

// Delete all temporary directories for an application
int PathUtils::removeTemporaryApplicationDirs(QString appName) {
    if (appName.isNull()) {
        appName = qApp->applicationName();
    }

    auto dirName = TEMP_DIR_FORMAT.arg(appName).arg("*").arg("*");

    QDir rootTempDir = QDir::tempPath();
    auto dirs = rootTempDir.entryInfoList({ dirName }, QDir::Dirs);
    int removed = 0;
    for (auto& dir : dirs) {
        auto dirName = dir.fileName();
        auto absoluteDirPath = QDir(dir.absoluteFilePath());
        QRegularExpression re { "^" + QRegularExpression::escape(appName) + "\\-(?<pid>\\d+)\\-(?<timestamp>\\d+)$" };

        auto match = re.match(dirName);
        if (match.hasMatch()) {
            auto pid = match.capturedRef("pid").toLongLong();
            auto timestamp = match.capturedRef("timestamp");
            if (!processIsRunning(pid)) {
                qDebug() << "  Removing old temporary directory: " << dir.absoluteFilePath();
                absoluteDirPath.removeRecursively();
                removed++;
            } else {
                qDebug() << "  Not removing (process is running): " << dir.absoluteFilePath();
            }
        }
    }

    return removed;
}

QString fileNameWithoutExtension(const QString& fileName, const QVector<QString> possibleExtensions) {
    QString fileNameLowered = fileName.toLower();
    foreach (const QString possibleExtension, possibleExtensions) {
        if (fileNameLowered.endsWith(possibleExtension.toLower())) {
            return fileName.left(fileName.count() - possibleExtension.count() - 1);
        }
    }
    return fileName;
}

QString findMostRecentFileExtension(const QString& originalFileName, QVector<QString> possibleExtensions) {
    QString sansExt = fileNameWithoutExtension(originalFileName, possibleExtensions);
    QString newestFileName = originalFileName;
    QDateTime newestTime = QDateTime::fromMSecsSinceEpoch(0);
    foreach (QString possibleExtension, possibleExtensions) {
        QString fileName = sansExt + "." + possibleExtension;
        QFileInfo fileInfo(fileName);
        if (fileInfo.exists() && fileInfo.lastModified() > newestTime) {
            newestFileName = fileName;
            newestTime = fileInfo.lastModified();
        }
    }
    return newestFileName;
}

QUrl PathUtils::defaultScriptsLocation(const QString& newDefaultPath) {
    static QString overriddenDefaultScriptsLocation = "";
    QString path;

    // set overriddenDefaultScriptLocation if it was passed in
    if (!newDefaultPath.isEmpty()) {
        overriddenDefaultScriptsLocation = newDefaultPath;
    }

    // use the overridden location if it is set
    if (!overriddenDefaultScriptsLocation.isEmpty()) {
        path = overriddenDefaultScriptsLocation;
    } else {
#if defined(Q_OS_OSX)
        path = QCoreApplication::applicationDirPath() + "/../Resources/scripts";
#elif defined(Q_OS_ANDROID)
        path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/scripts";
#else
        path = QCoreApplication::applicationDirPath() + "/scripts";
#endif
    }

    // turn the string into a legit QUrl
    return QUrl::fromLocalFile(QFileInfo(path).canonicalFilePath());
}

QString PathUtils::stripFilename(const QUrl& url) {
    // Guard against meaningless query and fragment parts.
    // Do NOT use PreferLocalFile as its behavior is unpredictable (e.g., on defaultScriptsLocation())
    return url.toString(QUrl::RemoveFilename | QUrl::RemoveQuery | QUrl::RemoveFragment);
}

Qt::CaseSensitivity PathUtils::getFSCaseSensitivity() {
    static Qt::CaseSensitivity sensitivity { Qt::CaseSensitive };
    static std::once_flag once;
    std::call_once(once, [&] {
            QString path = defaultScriptsLocation().toLocalFile();
            QFileInfo upperFI(path.toUpper());
            QFileInfo lowerFI(path.toLower());
            sensitivity = (upperFI == lowerFI) ? Qt::CaseInsensitive : Qt::CaseSensitive;
        });
    return sensitivity;
}

bool PathUtils::isDescendantOf(const QUrl& childURL, const QUrl& parentURL) {
    QString child = stripFilename(childURL);
    QString parent = stripFilename(parentURL);
    return child.startsWith(parent, PathUtils::getFSCaseSensitivity());
}


QString PathUtils::getServerName() {
    std::lock_guard<std::mutex> guard(_lock);
    return _server_name;
}

void PathUtils::setServerName(const QString &name) {
    std::lock_guard<std::mutex> guard(_lock);
    _server_name = name;
}

void PathUtils::setResourcesPath(const QString &dir) {
    std::lock_guard<std::mutex> guard(_lock);
    initialize();
    _server_resources_path = dir;
}

QString PathUtils::getServerDataPath() {
    std::lock_guard<std::mutex> guard(_lock);
    initialize();
    return _appdata_path + "/" + _server_name;
}

QString PathUtils::getServerDataFilePath(const QString& filename) {
    std::lock_guard<std::mutex> guard(_lock);
    initialize();
    return QDir(_appdata_path + "/" + _server_name).absoluteFilePath(filename);
}

QString PathUtils::getSettingsDescriptionPath() {
    std::lock_guard<std::mutex> guard(_lock);
    initialize();
    return _server_resources_path + "/describe-settings.json";
}

QString PathUtils::getConfigFilePath(const QString &filename) {
    std::lock_guard<std::mutex> guard(_lock);
    initialize();
    return QDir(_config_path + "/" + _server_name).absoluteFilePath(filename);
}

QString PathUtils::getSettingsDescriptionPath() {
    std::lock_guard<std::mutex> guard(_lock);
    initialize();
    return _server_resources_path + "/describe-settings.json";
}

QString PathUtils::getServerContentPath(const QString &dir_name) {
    std::lock_guard<std::mutex> guard(_lock);
    initialize();
    return _server_resources_path + "/" + dir_name + "/";
}

QString PathUtils::getPluginsPath() {
    std::lock_guard<std::mutex> guard(_lock);
    initialize();
    return _plugins_path + "/";
}

void PathUtils::initialize() {
    if (_initialized) {
        return;
    }

    _server_resources_path = qgetenv("VIRCADIA_RESOURCES_PATH");
    if ( _server_resources_path.isEmpty() ) {
        if (isSystemUser()) {
            _server_resources_path = VIRCADIA_DEFAULT_RESOURCES_PATH;
        } else {
            _server_resources_path = QCoreApplication::applicationDirPath() + "/resources";
        }
    }

    _config_path = qgetenv("VIRCADIA_CONFIG_PATH");
    if ( _config_path.isEmpty() ) {
        if (isSystemUser()) {
            _config_path = VIRCADIA_DEFAULT_CONFIG_PATH;
        } else {
            _config_path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
        }
    }

    _appdata_path = qgetenv("VIRCADIA_APPDATA_PATH");
    if ( _appdata_path.isEmpty()) {
        if (isSystemUser()) {
            _appdata_path = VIRCADIA_DEFAULT_APPDATA_PATH;
        } else {
            _appdata_path =  QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        }
    }

    _local_appdata_path = qgetenv("VIRCADIA_LOCAL_APPDATA_PATH");
    if ( _local_appdata_path.isEmpty()) {
        if (isSystemUser()) {
            _local_appdata_path = VIRCADIA_DEFAULT_APPDATA_PATH;
        } else {
            _local_appdata_path =  QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        }
    }

    _plugins_path = qgetenv("VIRCADIA_PLUGINS_PATH");
    if ( _plugins_path.isEmpty()) {
        if (isSystemUser()) {
            _plugins_path = VIRCADIA_DEFAULT_PLUGINS_PATH;
        } else {
#if defined(Q_OS_ANDROID)
            _plugins_path = QCoreApplication::applicationDirPath() + "/";
#elif defined(Q_OS_MAC)
            _plugins_path = QCoreApplication::applicationDirPath() + "/../PlugIns/";
#else
            _plugins_path = QCoreApplication::applicationDirPath() + "/plugins/";
#endif
        }
    }

    qInfo() << "Initialized default paths:";
    qInfo() << "Running as system user:" << isSystemUser();
    qInfo() << "Resource base path:" << _server_resources_path;
    qInfo() << "Config base path:" << _config_path;
    qInfo() << "Data base path:" << _appdata_path;
    qInfo() << "Local data base path:" << _local_appdata_path;
    qInfo() << "Plugins path:" << _plugins_path;
    _initialized = true;
}

QString PathUtils::findFirstDir(const QStringList &paths, const QString &description) {
    for(const auto &path : paths ) {
        QFileInfo fi(path);
        if ( fi.exists() && fi.isDir() ) {
            qInfo() << "Found directory for" << description << ":" << fi.absoluteFilePath();
            return fi.absoluteFilePath() + "/";
        }
    }

    qCritical() << "Failed to find directory for " << description << "; looked in" << paths;
    return "";
}