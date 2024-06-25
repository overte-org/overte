//
//  AmbientOcclusionPropertyGroup.h
//  libraries/entities/src
//
//  Created by HifiExperiments on 6/23/24
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef hifi_AmbientOcclusionPropertyGroup_h
#define hifi_AmbientOcclusionPropertyGroup_h

#include <AmbientOcclusionTechnique.h>

#include "PropertyGroup.h"
#include "EntityItemPropertiesMacros.h"

class EntityItemProperties;
class EncodeBitstreamParams;
class OctreePacketData;
class EntityTreeElementExtraEncodeData;
class ReadBitstreamToTreeParams;
class ScriptEngine;
class ScriptValue;

/*@jsdoc
 * AmbientOcclusion is defined by the following properties:
 * @typedef {object} Entities.AmbientOcclusion
 * @property {AmbientOcclusionTechnique} technique="ssao" - The AO technique used.
 * TODO
 */
class AmbientOcclusionPropertyGroup : public PropertyGroup {
public:
    // EntityItemProperty related helpers
    virtual void copyToScriptValue(const EntityPropertyFlags& desiredProperties, ScriptValue& properties,
                                   ScriptEngine* engine, bool skipDefaults,
                                   EntityItemProperties& defaultEntityProperties, bool returnNothingOnEmptyPropertyFlags,
                                   bool isMyOwnAvatarEntity) const override;
    virtual void copyFromScriptValue(const ScriptValue& object, const QSet<QString> &namesSet, bool& _defaultSettings) override;

    void merge(const AmbientOcclusionPropertyGroup& other);

    virtual void debugDump() const override;
    virtual void listChangedProperties(QList<QString>& out) override;

    virtual bool appendToEditPacket(OctreePacketData* packetData,
                                    EntityPropertyFlags& requestedProperties,
                                    EntityPropertyFlags& propertyFlags,
                                    EntityPropertyFlags& propertiesDidntFit,
                                    int& propertyCount,
                                    OctreeElement::AppendState& appendState) const override;

    virtual bool decodeFromEditPacket(EntityPropertyFlags& propertyFlags,
                                      const unsigned char*& dataAt, int& processedBytes) override;
    virtual void markAllChanged() override;
    virtual EntityPropertyFlags getChangedProperties() const override;

    // EntityItem related helpers
    // methods for getting/setting all properties of an entity
    virtual void getProperties(EntityItemProperties& propertiesOut) const override;

    /// returns true if something changed
    virtual bool setProperties(const EntityItemProperties& properties) override;

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

    DEFINE_PROPERTY_REF_ENUM(PROP_AMBIENT_OCCLUSION_TECHNIQUE, Technique, technique, AmbientOcclusionTechnique, AmbientOcclusionTechnique::SSAO);
    DEFINE_PROPERTY(PROP_AMBIENT_OCCLUSION_JITTER, Jitter, jitter, bool, false);
    DEFINE_PROPERTY(PROP_AMBIENT_OCCLUSION_RESOLUTION_LEVEL, ResolutionLevel, resolutionLevel, uint8_t, 2);
    DEFINE_PROPERTY(PROP_AMBIENT_OCCLUSION_EDGE_SHARPNESS, EdgeSharpness, edgeSharpness, float, 1.0f);
    DEFINE_PROPERTY(PROP_AMBIENT_OCCLUSION_BLUR_RADIUS, BlurRadius, blurRadius, uint8_t, 4);
    DEFINE_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_RADIUS, AORadius, aoRadius, float, 1.0f);
    DEFINE_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_OBSCURANCE_LEVEL, AOObscuranceLevel, aoObscuranceLevel, float, 0.5f);
    DEFINE_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_FALLOFF_ANGLE, AOFalloffAngle, aoFalloffAngle, float, 0.25f);
    DEFINE_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_NUM_SAMPLES, AONumSamples, aoNumSamples, uint8_t, 32);
    DEFINE_PROPERTY(PROP_AMBIENT_OCCLUSION_SSAO_NUM_SPIRAL_TURNS, SSAONumSpiralTurns, ssaoNumSpiralTurns, float, 7.0f);
};

#endif // hifi_AmbientOcclusionPropertyGroup_h
