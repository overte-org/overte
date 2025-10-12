//
//  AutoUpdater.h
//  libraries/auto-update/src
//
//  Created by Leonardo Murillo on 6/1/2015.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_AutoUpdater_h
#define hifi_AutoUpdater_h

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QXmlStreamAttributes>
#include <QtCore/QXmlStreamReader>
#include <QtGui/QDesktopServices>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkInformation>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <ApplicationVersion.h>
#include <DependencyManager.h>

class AutoUpdater : public QObject, public Dependency {
    Q_OBJECT
    SINGLETON_DEPENDENCY
    
public:
    AutoUpdater();

    enum class InstallerType {
        CLIENT_ONLY = 0,
        FULL
    };
    
    void checkForUpdate();
    const QMap<ApplicationVersion, QMap<QString, QString>>& getBuildData() { return _builds; }
    void openLatestUpdateURL();
    void setInstallerType(InstallerType type) { _installerType = type;  }
    void setInstallerCampaign(QString campaign) { _installerCampaign = campaign;  }

    const ApplicationVersion& getCurrentVersion() const { return _currentVersion; }

signals:
    void latestVersionDataParsed();
    void newVersionIsAvailable();
    void newVersionIsDownloaded();

private:
    QMap<ApplicationVersion, QMap<QString, QString>> _builds;
    QString _operatingSystem;
    InstallerType _installerType { InstallerType::FULL };
    QString _installerCampaign { "" };

    ApplicationVersion _currentVersion;
    
    void getLatestVersionData();
    void downloadUpdateVersion(const QString& version);
    void appendBuildData(const QString& versionNumber,
                         const QString& downloadURL,
                         const QString& releaseTime,
                         const QString& releaseNotes,
                         const QString& pullRequestNumber);

private slots:
    void parseLatestVersionData();
    void checkVersionAndNotify();
};

#endif // _hifi_AutoUpdater_h
