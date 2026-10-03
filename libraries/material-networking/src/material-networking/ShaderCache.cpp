//
//  Created by Bradley Austin Davis on 2015/05/26
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "ShaderCache.h"

NetworkShader::NetworkShader(const QUrl& url) :
    Resource(url) {}

void NetworkShader::downloadFinished(const QByteArray& data) {
    _source = QString::fromUtf8(data);
    finishedLoading(true);
}

ShaderCache& ShaderCache::instance() {
    static ShaderCache _instance;
    return _instance;
}

NetworkShaderPointer ShaderCache::getShader(const QUrl& url) {
    auto shader = std::dynamic_pointer_cast<NetworkShader>(ResourceCache::getResource(url));
    Q_ASSERT(shader);
    return shader;
}

std::shared_ptr<Resource> ShaderCache::createResource(const QUrl& url) {
    return std::shared_ptr<NetworkShader>(new NetworkShader(url), Resource::sharedPtrDeleter);
}

std::shared_ptr<Resource> ShaderCache::createResourceCopy(const std::shared_ptr<Resource>& resource) {
    auto shader = std::dynamic_pointer_cast<NetworkShader>(resource);
    Q_ASSERT(shader);
    return std::shared_ptr<NetworkShader>(new NetworkShader(*shader), Resource::sharedPtrDeleter);
}
