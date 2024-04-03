//
//  ApplicationMeshProvider.h
//  interface/src
//
//  Split from Application.cpp by HifiExperiments on 3/30/24
//  Created by Andrzej Kapolka on 5/10/13.
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors.
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef hifi_ApplicationMeshProvider_h
#define hifi_ApplicationMeshProvider_h

#include <graphics/Forward.h>

class ApplicationMeshProvider : public scriptable::ModelProviderFactory  {
public:
    virtual scriptable::ModelProviderPointer lookupModelProvider(const QUuid& uuid) override {
        bool success;
        if (auto nestable = DependencyManager::get<SpatialParentFinder>()->find(uuid, success).lock()) {
            auto type = nestable->getNestableType();
#ifdef SCRIPTABLE_MESH_DEBUG
            qCDebug(interfaceapp) << "ApplicationMeshProvider::lookupModelProvider" << uuid << SpatiallyNestable::nestableTypeToString(type);
#endif
            switch (type) {
            case NestableType::Entity:
                return getEntityModelProvider(static_cast<EntityItemID>(uuid));
            case NestableType::Avatar:
                return getAvatarModelProvider(uuid);
            }
        }
        return nullptr;
    }

private:
    scriptable::ModelProviderPointer getEntityModelProvider(EntityItemID entityID) {
        scriptable::ModelProviderPointer provider;
        auto entityTreeRenderer = qApp->getEntities();
        auto entityTree = entityTreeRenderer->getTree();
        if (auto entity = entityTree->findEntityByID(entityID)) {
            if (auto renderer = entityTreeRenderer->renderableForEntityId(entityID)) {
                provider = std::dynamic_pointer_cast<scriptable::ModelProvider>(renderer);
                provider->modelProviderType = NestableType::Entity;
            } else {
                qCWarning(interfaceapp) << "no renderer for entity ID" << entityID.toString();
            }
        }
        return provider;
    }

    scriptable::ModelProviderPointer getAvatarModelProvider(QUuid sessionUUID) {
        scriptable::ModelProviderPointer provider;
        auto avatarManager = DependencyManager::get<AvatarManager>();
        if (auto avatar = avatarManager->getAvatarBySessionID(sessionUUID)) {
            provider = std::dynamic_pointer_cast<scriptable::ModelProvider>(avatar);
            provider->modelProviderType = NestableType::Avatar;
        }
        return provider;
    }
};

#endif  // hifi_ApplicationMeshProvider_h
