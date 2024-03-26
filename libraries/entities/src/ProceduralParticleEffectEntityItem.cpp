//
//  ProceduralParticleEffectEntityItem.cpp
//  libraries/entities/src
//
//  Created by HifiExperiements on 11/19/23
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ProceduralParticleEffectEntityItem.h"

#include "EntityTree.h"
#include "EntityTreeElement.h"
#include "EntitiesLogging.h"
#include "EntityScriptingInterface.h"

EntityItemPointer ProceduralParticleEffectEntityItem::factory(const EntityItemID& entityID, const EntityItemProperties& properties) {
    std::shared_ptr<ProceduralParticleEffectEntityItem> entity(new ProceduralParticleEffectEntityItem(entityID), [](ProceduralParticleEffectEntityItem* ptr) { ptr->deleteLater(); });
    entity->setProperties(properties);
    return entity;
}

// our non-pure virtual subclass for now...
ProceduralParticleEffectEntityItem::ProceduralParticleEffectEntityItem(const EntityItemID& entityItemID) :
    EntityItem(entityItemID)
{
    _type = EntityTypes::ProceduralParticleEffect;
}

EntityItemProperties ProceduralParticleEffectEntityItem::getProperties(const EntityPropertyFlags& desiredProperties, bool allowEmptyDesiredProperties) const {
    EntityItemProperties properties = EntityItem::getProperties(desiredProperties, allowEmptyDesiredProperties); // get the properties from our base class

    COPY_ENTITY_PROPERTY_TO_PROPERTIES(numParticles, getNumParticles);
    COPY_ENTITY_PROPERTY_TO_PROPERTIES(numTrianglesPerParticle, getNumTrianglesPerParticle);
    COPY_ENTITY_PROPERTY_TO_PROPERTIES(numUpdateProps, getNumUpdateProps);
    COPY_ENTITY_PROPERTY_TO_PROPERTIES(particleTransparent, getParticleTransparent);
    COPY_ENTITY_PROPERTY_TO_PROPERTIES(particleUpdateData, getParticleUpdateData);
    COPY_ENTITY_PROPERTY_TO_PROPERTIES(particleRenderData, getParticleRenderData);

    return properties;
}

bool ProceduralParticleEffectEntityItem::setSubClassProperties(const EntityItemProperties& properties) {
    bool somethingChanged = false;

    SET_ENTITY_PROPERTY_FROM_PROPERTIES(numParticles, setNumParticles);
    SET_ENTITY_PROPERTY_FROM_PROPERTIES(numTrianglesPerParticle, setNumTrianglesPerParticle);
    SET_ENTITY_PROPERTY_FROM_PROPERTIES(numUpdateProps, setNumUpdateProps);
    SET_ENTITY_PROPERTY_FROM_PROPERTIES(particleTransparent, setParticleTransparent);
    SET_ENTITY_PROPERTY_FROM_PROPERTIES(particleUpdateData, setParticleUpdateData);
    SET_ENTITY_PROPERTY_FROM_PROPERTIES(particleRenderData, setParticleRenderData);

    return somethingChanged;
}

int ProceduralParticleEffectEntityItem::readEntitySubclassDataFromBuffer(const unsigned char* data, int bytesLeftToRead,
                                                                         ReadBitstreamToTreeParams& args,
                                                                         EntityPropertyFlags& propertyFlags, bool overwriteLocalData,
                                                                         bool& somethingChanged) {

    int bytesRead = 0;
    const unsigned char* dataAt = data;

    READ_ENTITY_PROPERTY(PROP_PROCEDURAL_PARTICLE_NUM_PARTICLES, uint32_t, setNumParticles);
    READ_ENTITY_PROPERTY(PROP_PROCEDURAL_PARTICLE_NUM_TRIS_PER, uint8_t, setNumTrianglesPerParticle);
    READ_ENTITY_PROPERTY(PROP_PROCEDURAL_PARTICLE_NUM_UPDATE_PROPS, uint8_t, setNumUpdateProps);
    READ_ENTITY_PROPERTY(PROP_PROCEDURAL_PARTICLE_TRANSPARENT, bool, setParticleTransparent);
    READ_ENTITY_PROPERTY(PROP_PROCEDURAL_PARTCILE_UPDATE_DATA, QString, setParticleUpdateData);
    READ_ENTITY_PROPERTY(PROP_PROCEDURAL_PARTCILE_RENDER_DATA, QString, setParticleRenderData);

    return bytesRead;
}

EntityPropertyFlags ProceduralParticleEffectEntityItem::getEntityProperties(EncodeBitstreamParams& params) const {
    EntityPropertyFlags requestedProperties = EntityItem::getEntityProperties(params);

    requestedProperties += PROP_PROCEDURAL_PARTICLE_NUM_PARTICLES;
    requestedProperties += PROP_PROCEDURAL_PARTICLE_NUM_TRIS_PER;
    requestedProperties += PROP_PROCEDURAL_PARTICLE_NUM_UPDATE_PROPS;
    requestedProperties += PROP_PROCEDURAL_PARTICLE_TRANSPARENT;
    requestedProperties += PROP_PROCEDURAL_PARTCILE_UPDATE_DATA;
    requestedProperties += PROP_PROCEDURAL_PARTCILE_RENDER_DATA;

    return requestedProperties;
}

void ProceduralParticleEffectEntityItem::appendSubclassData(OctreePacketData* packetData, EncodeBitstreamParams& params,
                                                            EntityTreeElementExtraEncodeDataPointer entityTreeElementExtraEncodeData,
                                                            EntityPropertyFlags& requestedProperties,
                                                            EntityPropertyFlags& propertyFlags,
                                                            EntityPropertyFlags& propertiesDidntFit,
                                                            int& propertyCount,
                                                            OctreeElement::AppendState& appendState) const {

    bool successPropertyFits = true;
    APPEND_ENTITY_PROPERTY(PROP_PROCEDURAL_PARTICLE_NUM_PARTICLES, getNumParticles());
    APPEND_ENTITY_PROPERTY(PROP_PROCEDURAL_PARTICLE_NUM_TRIS_PER, getNumTrianglesPerParticle());
    APPEND_ENTITY_PROPERTY(PROP_PROCEDURAL_PARTICLE_NUM_UPDATE_PROPS, getNumUpdateProps());
    APPEND_ENTITY_PROPERTY(PROP_PROCEDURAL_PARTICLE_TRANSPARENT, getParticleTransparent());
    APPEND_ENTITY_PROPERTY(PROP_PROCEDURAL_PARTCILE_UPDATE_DATA, getParticleUpdateData());
    APPEND_ENTITY_PROPERTY(PROP_PROCEDURAL_PARTCILE_RENDER_DATA, getParticleRenderData());
}

void ProceduralParticleEffectEntityItem::debugDump() const {
    quint64 now = usecTimestampNow();
    qCDebug(entities) << "PROC PA EFFECT EntityItem id:" << getEntityItemID() << "---------------------------------------------";
    qCDebug(entities) << "                    position:" << debugTreeVector(getWorldPosition());
    qCDebug(entities) << "                  dimensions:" << debugTreeVector(getScaledDimensions());
    qCDebug(entities) << "               getLastEdited:" << debugTime(getLastEdited(), now);
}

uint32_t ProceduralParticleEffectEntityItem::getNumParticles() const {
    return resultWithReadLock<uint32_t>([&] { return _numParticles; });
}

void ProceduralParticleEffectEntityItem::setNumParticles(uint32_t numParticles) {
    withWriteLock([&] {
        _needsRenderUpdate |= _numParticles != numParticles;
        _numParticles = numParticles;
    });
}

uint8_t ProceduralParticleEffectEntityItem::getNumTrianglesPerParticle() const {
    return resultWithReadLock<uint8_t>([&] { return _numTrianglesPerParticle; });
}

void ProceduralParticleEffectEntityItem::setNumTrianglesPerParticle(uint8_t numTrianglesPerParticle) {
    withWriteLock([&] {
        _needsRenderUpdate |= _numTrianglesPerParticle != numTrianglesPerParticle;
        _numTrianglesPerParticle = numTrianglesPerParticle;
    });
}

uint8_t ProceduralParticleEffectEntityItem::getNumUpdateProps() const {
    return resultWithReadLock<uint8_t>([&] { return _numUpdateProps; });
}

void ProceduralParticleEffectEntityItem::setNumUpdateProps(uint8_t numUpdateProps) {
    withWriteLock([&] {
        _needsRenderUpdate |= _numUpdateProps != numUpdateProps;
        _numUpdateProps = numUpdateProps;
    });
}

void ProceduralParticleEffectEntityItem::setParticleTransparent(bool particleTransparent) {
    withWriteLock([&] {
        _needsRenderUpdate |= _particleTransparent != particleTransparent;
        _particleTransparent = particleTransparent;
    });
}

QString ProceduralParticleEffectEntityItem::getParticleUpdateData() const {
    return resultWithReadLock<QString>([&] { return _particleUpdateData; });
}

void ProceduralParticleEffectEntityItem::setParticleUpdateData(const QString& particleUpdateData) {
    withWriteLock([&] {
        _needsRenderUpdate |= _particleUpdateData != particleUpdateData;
        _particleUpdateData = particleUpdateData;
    });
}

QString ProceduralParticleEffectEntityItem::getParticleRenderData() const {
    return resultWithReadLock<QString>([&] { return _particleRenderData; });
}

void ProceduralParticleEffectEntityItem::setParticleRenderData(const QString& particleRenderData) {
    withWriteLock([&] {
        _needsRenderUpdate |= _particleRenderData != particleRenderData;
        _particleRenderData = particleRenderData;
    });
}