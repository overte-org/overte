//
//  Created by Bradley Austin Davis on 2015/11/19
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ClipCache.h"

#include <QThread>

#include <shared/QtHelpers.h>

#include "impl/PointerClip.h"
#include "Logging.h"

using namespace recording;
NetworkClipLoader::NetworkClipLoader(const QUrl& url) :
    Resource(url),
    _clip(std::make_shared<NetworkClip>(url)) {
    if (url.isEmpty()) {
        _loaded = false;
        _startedLoading = false;
        _failedToLoad = true;
    }
}

void NetworkClip::init(const QByteArray& clipData) {
    _clipData = clipData;
    PointerClip::init((uchar*)_clipData.data(), _clipData.size());
}

void NetworkClipLoader::downloadFinished(const QByteArray& data) {
    _clip->init(data);
    finishedLoading(true);
    emit clipLoaded();
}

ClipCache::ClipCache(QObject* parent) :
    ResourceCache(parent)
{
    
}

NetworkClipLoaderPointer ClipCache::getClipLoader(const QUrl& url) {
    if (QThread::currentThread() != thread()) {
        NetworkClipLoaderPointer result;
        BLOCKING_INVOKE_METHOD(this, "getClipLoader",
                                  Q_GENERIC_RETURN_ARG(NetworkClipLoaderPointer, result), Q_GENERIC_ARG(const QUrl&, url));
        return result;
    }

    auto clipLoader = std::dynamic_pointer_cast<NetworkClipLoader>(getResource(url));
    Q_ASSERT(clipLoader);

    return clipLoader;
}

std::shared_ptr<Resource> ClipCache::createResource(const QUrl& url) {
    qCDebug(recordingLog) << "Loading recording at" << url;
    return std::shared_ptr<NetworkClipLoader>(new NetworkClipLoader(url), Resource::sharedPtrDeleter);
}

std::shared_ptr<Resource> ClipCache::createResourceCopy(const std::shared_ptr<Resource>& resource) {
    auto clipLoader = std::dynamic_pointer_cast<NetworkClipLoader>(resource);
    Q_ASSERT(clipLoader);
    return std::shared_ptr<NetworkClipLoader>(new NetworkClipLoader(*clipLoader), Resource::sharedPtrDeleter);
}