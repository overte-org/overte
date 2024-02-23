//
//  EntityEditPacketSender.cpp
//  libraries/entities/src
//
//  Created by Brad Hefta-Gaub on 8/12/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "EntityEditPacketSender.h"

#include <assert.h>

#include <QJsonDocument>

#include <AddressManager.h>
#include <PerfStat.h>
#include <OctalCode.h>
#include <udt/PacketHeaders.h>

#include "EntitiesLogging.h"
#include "EntityItem.h"
#include "EntityItemProperties.h"

EntityEditPacketSender::EntityEditPacketSender() {
    auto& packetReceiver = DependencyManager::get<NodeList>()->getPacketReceiver();
    packetReceiver.registerDirectListener(PacketType::EntityEditNack,
        PacketReceiver::makeSourcedListenerReference<EntityEditPacketSender>(this, &EntityEditPacketSender::processEntityEditNackPacket));
}

void EntityEditPacketSender::processEntityEditNackPacket(QSharedPointer<ReceivedMessage> message, SharedNodePointer sendingNode) {
    processNackPacket(*message, sendingNode);
}

void EntityEditPacketSender::adjustEditPacketForClockSkew(PacketType type, QByteArray& buffer, qint64 clockSkew) {
    if (type == PacketType::EntityAdd || type == PacketType::EntityEdit || type == PacketType::EntityEditLarge || type == PacketType::EntityPhysics) {
        EntityItem::adjustEditPacketForClockSkew(buffer, clockSkew);
    }
}

void EntityEditPacketSender::queueEditAvatarEntityMessage(EntityTreePointer entityTree, EntityItemID entityItemID) {
    assert(_myAvatar);
    if (!entityTree) {
        qCDebug(entities) << "EntityEditPacketSender::queueEditAvatarEntityMessage null entityTree.";
        return;
    }
    EntityItemPointer entity = entityTree->findEntityByEntityItemID(entityItemID);
    if (!entity) {
        qCDebug(entities) << "EntityEditPacketSender::queueEditAvatarEntityMessage can't find entity: " << entityItemID;
        return;
    }
    entity->setLastBroadcast(usecTimestampNow());

    OctreePacketData packetData(false, AvatarTraits::MAXIMUM_TRAIT_SIZE);
    EncodeBitstreamParams params;
    EntityTreeElementExtraEncodeDataPointer extra { nullptr };
    EntityPropertyList firstDidntFitProperty;
    OctreeElement::AppendState appendState = entity->appendEntityData(&packetData, params, extra, firstDidntFitProperty);

    if (appendState != OctreeElement::COMPLETED) {
        // TODO: handle EntityDataLarge
        // this entity's payload is too big
        return;
    }

    QByteArray tempArray((const char*)packetData.getUncompressedData(), packetData.getUncompressedSize());
    _myAvatar->storeAvatarEntityDataPayload(entityItemID, tempArray);
}

void EntityEditPacketSender::queueEditEntityMessage(PacketType type,
                                                    EntityTreePointer entityTree,
                                                    EntityItemID entityItemID,
                                                    const EntityItemProperties& properties) {
    if (properties.getEntityHostType() == entity::HostType::AVATAR) {
        if (!_myAvatar) {
            qCWarning(entities) << "Suppressing entity edit message: cannot send avatar entity edit with no myAvatar";
        } else if (properties.getOwningAvatarID() == _myAvatar->getID() || properties.getOwningAvatarID() == AVATAR_SELF_ID) {
            // this is a local avatar-entity --> update our avatar-data rather than sending to the entity-server
            // Note: we store AVATAR_SELF_ID in EntityItem::_owningAvatarID and we usually
            // store the actual sessionUUID in EntityItemProperties::_owningAvatarID.
            // However at this context we check for both cases just in case.  Really we just want to know
            // where to route the data: entity-server or avatar-mixer.
            queueEditAvatarEntityMessage(entityTree, entityItemID);
        } else {
            qCWarning(entities) << "Suppressing entity edit message: cannot send avatar entity edit for another avatar";
        }
        return;
    } else if (properties.getEntityHostType() == entity::HostType::LOCAL) {
        // Don't send edits for local entities
        return;
    }

    if (entityTree && entityTree->isServerlessMode()) {
        // if we are in a serverless domain, don't send edit packets
        return;
    }

    QByteArray bufferOut(NLPacket::maxPayloadSize(type), 0);

    if (type == PacketType::EntityAdd) {
        auto MAX_ADD_DATA_SIZE = NLPacket::maxPayloadSize(type) * 10; // a really big buffer
        bufferOut.resize(MAX_ADD_DATA_SIZE);
    }

    OctreeElement::AppendState encodeResult = OctreeElement::PARTIAL; // start the loop assuming there's more to send
    auto nodeList = DependencyManager::get<NodeList>();

    EntityPropertyFlags didntFitProperties;
    EntityItemProperties propertiesCopy = properties;

    if (properties.parentIDChanged() && properties.getParentID() == AVATAR_SELF_ID) {
        const QUuid myNodeID = nodeList->getSessionUUID();
        propertiesCopy.setParentID(myNodeID);
    }

    EntityPropertyFlags requestedProperties = propertiesCopy.getChangedProperties();

    if (!nodeList->getThisNodeCanGetAndSetPrivateUserData() && requestedProperties.getHasProperty(PROP_PRIVATE_USER_DATA)) {
        requestedProperties -= PROP_PRIVATE_USER_DATA;
    }

    while (encodeResult == OctreeElement::PARTIAL && !requestedProperties.isEmpty()) {
        EntityPropertyList firstDidntFitProperty = (EntityPropertyList)0;
        encodeResult = EntityItemProperties::encodeEntityEditPacket(type, entityItemID, propertiesCopy, bufferOut, requestedProperties, didntFitProperties, firstDidntFitProperty);

        if (encodeResult == OctreeElement::NONE) {
            if (firstDidntFitProperty != 0) {
                type = PacketType::EntityEditLarge;
                const EntityPropertyFlags originalRequestedProperties = requestedProperties;
                requestedProperties = firstDidntFitProperty;
                size_t bufferMultiplier = 2;
                const int bufferSize = NLPacket::maxPayloadSize(type);
                while (true) {
                    bufferOut.resize(bufferMultiplier * bufferSize);
                    encodeResult = EntityItemProperties::encodeEntityEditPacket(type, entityItemID, propertiesCopy, bufferOut, requestedProperties, didntFitProperties, firstDidntFitProperty);
                    if (encodeResult == OctreeElement::COMPLETED) {
                        queueOctreeEditMessage(type, bufferOut);
                        encodeResult = OctreeElement::PARTIAL;
                        break;
                    }
                    bufferMultiplier++;
                }
                didntFitProperties = originalRequestedProperties - firstDidntFitProperty;
            }
        } else {
            #ifdef WANT_DEBUG
                qCDebug(entities) << "calling queueOctreeEditMessage()...";
                qCDebug(entities) << "    id:" << entityItemID;
                qCDebug(entities) << "    properties:" << properties;
            #endif

            queueOctreeEditMessage(type, bufferOut);
        }

        // if we still have properties to send, switch the message type to edit, and request only the packets that didn't fit
        if (encodeResult != OctreeElement::COMPLETED) {
            type = PacketType::EntityEdit;
            requestedProperties = didntFitProperties;
        }

        bufferOut.resize(NLPacket::maxPayloadSize(type)); // resize our output buffer for the next packet
    }
}

void EntityEditPacketSender::queueEraseEntityMessage(const EntityItemID& entityItemID) {

    QByteArray bufferOut(NLPacket::maxPayloadSize(PacketType::EntityErase), 0);

    if (EntityItemProperties::encodeEraseEntityMessage(entityItemID, bufferOut)) {
        queueOctreeEditMessage(PacketType::EntityErase, bufferOut);
    }
}

void EntityEditPacketSender::queueCloneEntityMessage(const EntityItemID& entityIDToClone, const EntityItemID& newEntityID) {
    QByteArray bufferOut(NLPacket::maxPayloadSize(PacketType::EntityClone), 0);

    if (EntityItemProperties::encodeCloneEntityMessage(entityIDToClone, newEntityID, bufferOut)) {
        queueOctreeEditMessage(PacketType::EntityClone, bufferOut);
    }
}
