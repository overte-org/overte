//
//  ProceduralParticleEffectEntityItem.h
//  libraries/entities/src
//
//  Created by HifiExperiements on 11/19/23
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_ProceduralParticleEffectEntityItem_h
#define hifi_ProceduralParticleEffectEntityItem_h

#include "EntityItem.h"

namespace particle {
    static const uint32_t DEFAULT_NUM_PROCEDURAL_PARTICLES = 10000;
    static const uint32_t MAXIMUM_NUM_PROCEDURAL_PARTICLES = 1024 * 1024;
    static const uint8_t DEFAULT_NUM_TRIS_PER = 1;
    static const uint8_t MINIMUM_TRIS_PER = 1;
    static const uint8_t MAXIMUM_TRIS_PER = 15;
    static const uint8_t DEFAULT_NUM_UPDATE_PROPS = 0;
    static const uint8_t MINIMUM_NUM_UPDATE_PROPS = 0;
    static const uint8_t MAXIMUM_NUM_UPDATE_PROPS = 5;
}

class ProceduralParticleEffectEntityItem : public EntityItem {
public:
    ALLOW_INSTANTIATION // This class can be instantiated

    static EntityItemPointer factory(const EntityItemID& entityID, const EntityItemProperties& properties);

    ProceduralParticleEffectEntityItem(const EntityItemID& entityItemID);

    // methods for getting/setting all properties of this entity
    virtual EntityItemProperties getProperties(const EntityPropertyFlags& desiredProperties, bool allowEmptyDesiredProperties) const override;
    virtual bool setSubClassProperties(const EntityItemProperties& properties) override;

    virtual EntityPropertyFlags getEntityProperties(EncodeBitstreamParams& params) const override;

    virtual void appendSubclassData(OctreePacketData* packetData, EncodeBitstreamParams& params,
                                    EntityTreeElementExtraEncodeDataPointer entityTreeElementExtraEncodeData,
                                    EntityPropertyFlags& requestedProperties,
                                    EntityPropertyFlags& propertyFlags,
                                    EntityPropertyFlags& propertiesDidntFit,
                                    int& propertyCount,
                                    OctreeElement::AppendState& appendState) const override;

    virtual int readEntitySubclassDataFromBuffer(const unsigned char* data, int bytesLeftToRead,
                                                 ReadBitstreamToTreeParams& args,
                                                 EntityPropertyFlags& propertyFlags, bool overwriteLocalData,
                                                 bool& somethingChanged) override;

    bool shouldBePhysical() const override { return false; }

    virtual void debugDump() const override;

    virtual bool supportsDetailedIntersection() const override { return false; }

    uint32_t getNumParticles() const;
    void setNumParticles(uint32_t numParticles);

    uint8_t getNumTrianglesPerParticle() const;
    void setNumTrianglesPerParticle(uint8_t numTrianglesPerParticle);

    uint8_t getNumUpdateProps() const;
    void setNumUpdateProps(uint8_t numUpdateProps);

    bool getParticleTransparent() const { return _particleTransparent; }
    void setParticleTransparent(bool particleTransparent);

    QString getParticleUpdateData() const;
    void setParticleUpdateData(const QString& particleUpdateData);

    QString getParticleRenderData() const;
    void setParticleRenderData(const QString& particleRenderData);

protected:
    uint32_t _numParticles;
    uint8_t _numTrianglesPerParticle;
    uint8_t _numUpdateProps;
    bool _particleTransparent;
    QString _particleUpdateData;
    QString _particleRenderData;
};

#endif // hifi_ProceduralParticleEffectEntityItem_h
