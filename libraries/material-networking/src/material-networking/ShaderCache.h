//
//  Created by Bradley Austin Davis on 2015/05/26
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once
#ifndef hifi_ShaderCache_h
#define hifi_ShaderCache_h

#include <QtCore/QSharedPointer>
#include <ResourceCache.h>

class NetworkShader : public Resource {
public:
    NetworkShader(const QUrl& url);
    NetworkShader(const NetworkShader& other) : Resource(other), _source(other._source) {}
    virtual ~NetworkShader() {}

    QString getType() const override { return "NetworkShader"; }

    virtual void downloadFinished(const QByteArray& data) override;

    QString _source;
};

using NetworkShaderPointer = std::shared_ptr<NetworkShader>;

class ShaderCache : public ResourceCache {
public:
    static ShaderCache& instance();

    NetworkShaderPointer getShader(const QUrl& url);

protected:
    virtual std::shared_ptr<Resource> createResource(const QUrl& url) override;
    std::shared_ptr<Resource> createResourceCopy(const std::shared_ptr<Resource>& resource) override;
};

#endif
