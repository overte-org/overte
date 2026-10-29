//
//  ResoruceTests.cpp
//
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ResourceTests.h"

#include <QNetworkDiskCache>

#include <ExternalResource.h>
#include <ResourceCache.h>
#include <LimitedNodeList.h>
#include <NodeList.h>
#include <NetworkAccessManager.h>
#include <DependencyManager.h>
#include <ResourceRequestObserver.h>
#include <StatTracker.h>

QTEST_MAIN(ResourceTests)

void ResourceTests::initTestCase() {

    //DependencyManager::set<AddressManager>();
    DependencyManager::set<StatTracker>();
    DependencyManager::registerInheritance<LimitedNodeList, NodeList>();
    DependencyManager::set<NodeList>(NodeType::Agent, INVALID_PORT);
    DependencyManager::set<ResourceCacheSharedItems>();
    DependencyManager::set<ResourceManager>();
    DependencyManager::set<ResourceRequestObserver>();

    const qint64 MAXIMUM_CACHE_SIZE = 1024 * 1024 * 1024; // 1GB

    // set up the file cache
    //QString cachePath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QString cachePath = "./resourceTestCache";
    QNetworkAccessManager& networkAccessManager = NetworkAccessManager::getInstance();
    QNetworkDiskCache* cache = new QNetworkDiskCache();
    cache->setMaximumCacheSize(MAXIMUM_CACHE_SIZE);
    cache->setCacheDirectory(cachePath);
    cache->clear(); // clear the cache
    networkAccessManager.setCache(cache);
}

void ResourceTests::cleanupTestCase() {
    DependencyManager::get<ResourceManager>()->cleanup();
}

static QSharedPointer<Resource> resource;


void ResourceTests::downloadFirst() {
    // download the Mery fst file
    QUrl meryUrl = QUrl("https://testing-assets.overte.org/networking/defaultAvatar_full.fst");
    resource = QSharedPointer<Resource>::create(meryUrl);
    resource->setSelf(resource);

    const int timeout = 1000;
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(timeout); // 1s, Qt::CoarseTimer acceptable
    timer.setSingleShot(true);

    connect(resource.data(), &Resource::loaded, &loop, &QEventLoop::quit);
    connect(resource.data(), &Resource::failed, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start();

    resource->ensureLoading();
    loop.exec();

    QVERIFY(resource->isLoaded());
}

void ResourceTests::downloadAgain() {
    // download the Mery fst file
    QUrl meryUrl = QUrl("https://testing-assets.overte.org/networking/defaultAvatar_full.fst");
    resource = QSharedPointer<Resource>::create(meryUrl);
    resource->setSelf(resource);

    const int timeout = 1000;
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(timeout); // 1s, Qt::CoarseTimer acceptable
    timer.setSingleShot(true);

    connect(resource.data(), &Resource::loaded, &loop, &QEventLoop::quit);
    connect(resource.data(), &Resource::failed, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start();

    resource->ensureLoading();
    loop.exec();

    QVERIFY(resource->isLoaded());
}
