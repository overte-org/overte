//
//  Application_Entities.cpp
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

#include "Application.h"

#include <AnimationCache.h>
#include <input-plugins/KeyboardMouseDevice.h>
#include <MainWindow.h>
#include <recording/ClipCache.h>
#include <RenderableEntityItem.h>
#include <SoundCache.h>

#include "InterfaceLogging.h"
#include "LODManager.h"

static const float FOCUS_HIGHLIGHT_EXPANSION_FACTOR = 1.05f;

static const float INITIAL_QUERY_RADIUS = 10.0f;  // priority radius for entities before physics enabled

void Application::setMaxOctreePacketsPerSecond(int maxOctreePPS) {
    if (maxOctreePPS != _maxOctreePPS) {
        _maxOctreePPS = maxOctreePPS;
        _maxOctreePacketsPerSecond.set(_maxOctreePPS);
    }
}

QVector<EntityItemID> Application::pasteEntities(const QString& entityHostType, float x, float y, float z) {
    return _entityClipboard->sendEntities(_entityEditSender.get(), getEntities()->getTree(), entityHostType, x, y, z);
}

bool Application::exportEntities(const QString& filename,
                                 const QVector<QUuid>& entityIDs,
                                 const glm::vec3* givenOffset) {
    QHash<EntityItemID, EntityItemPointer> entities;

    auto nodeList = DependencyManager::get<NodeList>();
    const QUuid myAvatarID = nodeList->getSessionUUID();

    auto entityTree = getEntities()->getTree();
    auto exportTree = std::make_shared<EntityTree>();
    exportTree->setMyAvatar(getMyAvatar());
    exportTree->createRootElement();
    glm::vec3 root(TREE_SCALE, TREE_SCALE, TREE_SCALE);
    bool success = true;
    entityTree->withReadLock([entityIDs, entityTree, givenOffset, myAvatarID, &root, &entities, &success, &exportTree] {
        for (auto entityID : entityIDs) { // Gather entities and properties.
            auto entityItem = entityTree->findEntityByEntityItemID(entityID);
            if (!entityItem) {
                qCWarning(interfaceapp) << "Skipping export of" << entityID << "that is not in scene.";
                continue;
            }

            if (!givenOffset) {
                EntityItemID parentID = entityItem->getParentID();
                bool parentIsAvatar = (parentID == AVATAR_SELF_ID || parentID == myAvatarID);
                if (!parentIsAvatar && (parentID.isInvalidID() ||
                                        !entityIDs.contains(parentID) ||
                                        !entityTree->findEntityByEntityItemID(parentID))) {
                    // If parent wasn't selected, we want absolute position, which isn't in properties.
                    auto position = entityItem->getWorldPosition();
                    root.x = glm::min(root.x, position.x);
                    root.y = glm::min(root.y, position.y);
                    root.z = glm::min(root.z, position.z);
                }
            }
            entities[entityID] = entityItem;
        }

        if (entities.size() == 0) {
            success = false;
            return;
        }

        if (givenOffset) {
            root = *givenOffset;
        }
        for (EntityItemPointer& entityDatum : entities) {
            auto properties = entityDatum->getProperties();
            EntityItemID parentID = properties.getParentID();
            bool parentIsAvatar = (parentID == AVATAR_SELF_ID || parentID == myAvatarID);
            if (parentIsAvatar) {
                properties.setParentID(AVATAR_SELF_ID);
            } else {
                if (parentID.isInvalidID()) {
                    properties.setPosition(properties.getPosition() - root);
                } else if (!entities.contains(parentID)) {
                    entityDatum->globalizeProperties(properties, "Parent %3 of %2 %1 is not selected for export.", -root);
                } // else valid parent -- don't offset
            }
            exportTree->addEntity(entityDatum->getEntityItemID(), properties);
        }
    });
    if (success) {
        success = exportTree->writeToJSONFile(filename.toLocal8Bit().constData());

        // restore the main window's active state
        _window->activateWindow();
    }
    return success;
}

bool Application::exportEntities(const QString& filename, float x, float y, float z, float scale) {
    glm::vec3 center(x, y, z);
    glm::vec3 minCorner = center - vec3(scale);
    float cubeSize = scale * 2;
    AACube boundingCube(minCorner, cubeSize);
    QVector<QUuid> entities;
    auto entityTree = getEntities()->getTree();
    entityTree->withReadLock([&] {
        entityTree->evalEntitiesInCube(boundingCube, PickFilter(), entities);
    });
    return exportEntities(filename, entities, &center);
}

bool Application::importEntities(const QString& urlOrFilename, const bool isObservable, const qint64 callerId) {
    bool success = false;
    _entityClipboard->withWriteLock([&] {
        _entityClipboard->eraseAllOctreeElements();

        // FIXME: readFromURL() can take over the main event loop which may cause problems, especially if downloading the JSON
        // from the Web.
        success = _entityClipboard->readFromURL(urlOrFilename, isObservable, callerId, true);
        if (success) {
            _entityClipboard->reaverageOctreeElements();
        }
    });
    return success;
}

void Application::setKeyboardFocusHighlight(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& dimensions) {
    if (qApp->getLoginDialogPoppedUp()) {
        return;
    }

    auto entityScriptingInterface = DependencyManager::get<EntityScriptingInterface>();
    if (_keyboardFocusHighlightID == UNKNOWN_ENTITY_ID || !entityScriptingInterface->isAddedEntity(_keyboardFocusHighlightID)) {
        EntityItemProperties properties;
        properties.setType(EntityTypes::Box);
        properties.setAlpha(1.0f);
        properties.setColor({ 0xFF, 0xEF, 0x00 });
        properties.setPrimitiveMode(PrimitiveMode::LINES);
        properties.getPulse().setMin(0.5);
        properties.getPulse().setMax(1.0f);
        properties.getPulse().setColorMode(PulseMode::IN_PHASE);
        properties.setIgnorePickIntersection(true);
        _keyboardFocusHighlightID = entityScriptingInterface->addEntityInternal(properties, entity::HostType::LOCAL);
    }

    // Position focus
    EntityItemProperties properties;
    properties.setPosition(position);
    properties.setRotation(rotation);
    properties.setDimensions(dimensions);
    properties.setVisible(true);
    entityScriptingInterface->editEntity(_keyboardFocusHighlightID, properties);
}

void Application::setKeyboardFocusEntity(const QUuid& id) {
    if (_keyboardFocusedEntity.get() != id) {
        if (qApp->getLoginDialogPoppedUp() && !_loginDialogID.isNull()) {
            if (id == _loginDialogID) {
                emit loginDialogFocusEnabled();
            } else if (!_keyboardFocusWaitingOnRenderable) {
                // that's the only entity we want in focus;
                return;
            }
        }

        _keyboardFocusedEntity.set(id);

        auto entityScriptingInterface = DependencyManager::get<EntityScriptingInterface>();
        if (id != UNKNOWN_ENTITY_ID) {
            EntityPropertyFlags desiredProperties;
            desiredProperties += PROP_VISIBLE;
            desiredProperties += PROP_SHOW_KEYBOARD_FOCUS_HIGHLIGHT;
            auto properties = entityScriptingInterface->getEntityProperties(id);
            if (properties.getVisible()) {
                auto entities = getEntities();
                auto entityId = _keyboardFocusedEntity.get();
                auto entityItemRenderable = entities->renderableForEntityId(entityId);
                if (!entityItemRenderable) {
                    _keyboardFocusWaitingOnRenderable = true;
                } else if (entityItemRenderable->wantsKeyboardFocus()) {
                    entities->setProxyWindow(entityId, _window->windowHandle());
                    if (_keyboardMouseDevice->isActive()) {
                        _keyboardMouseDevice->pluginFocusOutEvent();
                    }
                    _lastAcceptedKeyPress = usecTimestampNow();

                    if (properties.getShowKeyboardFocusHighlight()) {
                        if (auto entity = entities->getEntity(entityId)) {
                            setKeyboardFocusHighlight(entity->getWorldPosition(), entity->getWorldOrientation(),
                                entity->getScaledDimensions() * FOCUS_HIGHLIGHT_EXPANSION_FACTOR);
                            return;
                        }
                    }
                }
            }
        }

        EntityItemProperties properties;
        properties.setVisible(false);
        entityScriptingInterface->editEntity(_keyboardFocusHighlightID, properties);
    }
}

void Application::clearDomainOctreeDetails(bool clearAll) {
    // if we're about to quit, we really don't need to do the rest of these things...
    if (_aboutToQuit) {
        return;
    }

    qCDebug(interfaceapp) << "Clearing domain octree details...";

    resetPhysicsReadyInformation();
    setIsInterstitialMode(true);

    auto octreeServerSceneStats = getOcteeSceneStats();
    octreeServerSceneStats->withWriteLock([&] {
        octreeServerSceneStats->clear();
    });

    // reset the model renderer
    clearAll ? getEntities()->clear() : getEntities()->clearDomainAndNonOwnedEntities();

    DependencyManager::get<AnimationCache>()->clearUnusedResources();
    DependencyManager::get<SoundCache>()->clearUnusedResources();
    DependencyManager::get<MaterialCache>()->clearUnusedResources();
    DependencyManager::get<ModelCache>()->clearUnusedResources();
    ShaderCache::instance().clearUnusedResources();
    DependencyManager::get<TextureCache>()->clearUnusedResources();
    DependencyManager::get<recording::ClipCache>()->clearUnusedResources();
}

void Application::resettingDomain() {
    _notifiedPacketVersionMismatchThisDomain = false;

    clearDomainOctreeDetails(false);
}

void Application::queryOctree(NodeType_t serverType, PacketType packetType) {
    if (!_settingsLoaded) {
        return; // bail early if settings are not loaded
    }

    const bool isModifiedQuery = !_physicsEnabled;
    if (isModifiedQuery) {
        if (!_octreeProcessor->safeLandingIsActive()) {
            // don't send the octreeQuery until SafeLanding knows it has started
            return;
        }
        // Create modified view that is a simple sphere.
        bool interstitialModeEnabled = DependencyManager::get<NodeList>()->getDomainHandler().getInterstitialModeEnabled();

        ConicalViewFrustum sphericalView;
        AABox box = getMyAvatar()->getGlobalBoundingBox();
        float radius = glm::max(INITIAL_QUERY_RADIUS, 0.5f * glm::length(box.getDimensions()));
        sphericalView.setPositionAndSimpleRadius(box.calcCenter(), radius);

        if (interstitialModeEnabled) {
            ConicalViewFrustum farView;
            farView.set(_viewFrustum);
            _octreeQuery.setConicalViews({ sphericalView, farView });
        } else {
            _octreeQuery.setConicalViews({ sphericalView });
        }

        _octreeQuery.setOctreeSizeScale(DEFAULT_OCTREE_SIZE_SCALE);
        static constexpr float MIN_LOD_ADJUST = -20.0f;
        _octreeQuery.setBoundaryLevelAdjust(MIN_LOD_ADJUST);
    } else {
        _octreeQuery.setConicalViews(_conicalViews);
        auto lodManager = DependencyManager::get<LODManager>();
        _octreeQuery.setOctreeSizeScale(lodManager->getOctreeSizeScale());
        _octreeQuery.setBoundaryLevelAdjust(lodManager->getBoundaryLevelAdjust());
    }
    _octreeQuery.setReportInitialCompletion(isModifiedQuery);

    auto nodeList = DependencyManager::get<NodeList>();

    auto node = nodeList->soloNodeOfType(serverType);
    if (node && node->getActiveSocket()) {
        _octreeQuery.setMaxQueryPacketsPerSecond(getMaxOctreePacketsPerSecond());

        auto queryPacket = NLPacket::create(packetType);

        // encode the query data
        auto packetData = reinterpret_cast<unsigned char*>(queryPacket->getPayload());
        int packetSize = _octreeQuery.getBroadcastData(packetData);
        queryPacket->setPayloadSize(packetSize);

        // make sure we still have an active socket
        nodeList->sendUnreliablePacket(*queryPacket, *node);
    }
}

int Application::sendNackPackets() {
    // iterates through all nodes in NodeList
    auto nodeList = DependencyManager::get<NodeList>();

    int packetsSent = 0;

    nodeList->eachNode([&](const SharedNodePointer& node){

        if (node->getActiveSocket() && node->getType() == NodeType::EntityServer) {

            auto nackPacketList = NLPacketList::create(PacketType::OctreeDataNack);

            QUuid nodeUUID = node->getUUID();

            // if there are octree packets from this node that are waiting to be processed,
            // don't send a NACK since the missing packets may be among those waiting packets.
            if (_octreeProcessor->hasPacketsToProcessFrom(nodeUUID)) {
                return;
            }

            QSet<OCTREE_PACKET_SEQUENCE> missingSequenceNumbers;
            auto octreeServerSceneStats = getOcteeSceneStats();
            octreeServerSceneStats->withReadLock([&] {
                // retrieve octree scene stats of this node
                if (octreeServerSceneStats->find(nodeUUID) == octreeServerSceneStats->end()) {
                    return;
                }
                // get sequence number stats of node, prune its missing set, and make a copy of the missing set
                SequenceNumberStats& sequenceNumberStats = (*octreeServerSceneStats)[nodeUUID].getIncomingOctreeSequenceNumberStats();
                sequenceNumberStats.pruneMissingSet();
                missingSequenceNumbers = sequenceNumberStats.getMissingSet();
            });

            _isMissingSequenceNumbers = (missingSequenceNumbers.size() != 0);

            // construct nack packet(s) for this node
            foreach(const OCTREE_PACKET_SEQUENCE& missingNumber, missingSequenceNumbers) {
                nackPacketList->writePrimitive(missingNumber);
            }

            if (nackPacketList->getNumPackets()) {
                packetsSent += (int)nackPacketList->getNumPackets();

                // send the packet list
                nodeList->sendPacketList(std::move(nackPacketList), *node);
            }
        }
    });

    return packetsSent;
}