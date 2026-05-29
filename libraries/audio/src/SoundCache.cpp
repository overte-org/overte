//
//  SoundCache.cpp
//  libraries/audio/src
//
//  Created by Stephen Birarda on 2014-11-13.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "SoundCache.h"

#include <QtCore/QThread>

#include <shared/QtHelpers.h>

#include "AudioLogging.h"

static const int SOUNDS_LOADING_PRIORITY { -7 }; // Make sure sounds load after the low rez texture mips

int soundPointerMetaTypeId = qRegisterMetaType<SharedSoundPointer>();

SoundCache::SoundCache(QObject* parent) :
    ResourceCache(parent)
{
    const qint64 SOUND_DEFAULT_UNUSED_MAX_SIZE = 50 * BYTES_PER_MEGABYTES;
    setUnusedResourceCacheSize(SOUND_DEFAULT_UNUSED_MAX_SIZE);
    setObjectName("SoundCache");
}

SharedSoundPointer SoundCache::getSound(const QUrl& url) {
    auto sound = std::dynamic_pointer_cast<Sound>(getResource(url));
    Q_ASSERT(sound);
    return sound;
}

std::shared_ptr<Resource> SoundCache::createResource(const QUrl& url) {
    auto resource = std::shared_ptr<Sound>(new Sound(url), Resource::sharedPtrDeleter);
    resource->setLoadPriorityOperator(this, []() { return SOUNDS_LOADING_PRIORITY; });
    return resource;
}

std::shared_ptr<Resource> SoundCache::createResourceCopy(const std::shared_ptr<Resource>& resource) {
    auto sound = std::dynamic_pointer_cast<Sound>(resource);
    Q_ASSERT(sound);
    return std::shared_ptr<Sound>(new Sound(*sound), Resource::sharedPtrDeleter);
}