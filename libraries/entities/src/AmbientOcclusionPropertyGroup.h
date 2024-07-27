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
 * @property {AmbientOcclusionTechnique} technique="ssao" - The ambient occlusion technique used. Different techniques have
 *     different tradeoffs.
 * @property {boolean} jitter=false - Whether or not the ambient occlusion sampling is jittered.
 * @property {number} resolutionLevel=2 - How high the resolution of the ambient occlusion buffer should be. Higher levels
 *     mean lower resolution buffers.
 * @property {number} edgeSharpness=1.0 - How much to sharpen the edges during the ambient occlusion blurring.
 * @property {number} blurRadius=4 - The radius used for blurring, in pixels.
 * @property {number} aoRadius=1.0 - The radius used for ambient occlusion.
 * @property {number} aoObscuranceLevel=0.5 - Intensify or dim ambient occlusion.
 * @property {number} aoFalloffAngle=0.25 - The falloff angle for the AO calculation.
 * @property {number} aoSamplingAmount=0.5 - The fraction of AO samples to use, out of the maximum for each technique.
 * @property {number} ssaoNumSpiralTurns=7.0 - The angle span used to distribute the AO samples ray directions. SSAO only.
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

    // FIXME: On some machines, SSAO seems to be causing performance problems.  Let's default to HBAO for now and maybe
    // revisit when we have Vulkan
    DEFINE_PROPERTY_REF_ENUM(PROP_AMBIENT_OCCLUSION_TECHNIQUE, Technique, technique, AmbientOcclusionTechnique, AmbientOcclusionTechnique::HBAO);
    DEFINE_PROPERTY(PROP_AMBIENT_OCCLUSION_JITTER, Jitter, jitter, bool, false);
    DEFINE_PROPERTY(PROP_AMBIENT_OCCLUSION_RESOLUTION_LEVEL, ResolutionLevel, resolutionLevel, uint8_t, 2);
    DEFINE_PROPERTY(PROP_AMBIENT_OCCLUSION_EDGE_SHARPNESS, EdgeSharpness, edgeSharpness, float, 1.0f);
    DEFINE_PROPERTY(PROP_AMBIENT_OCCLUSION_BLUR_RADIUS, BlurRadius, blurRadius, uint8_t, 4);
    DEFINE_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_RADIUS, AORadius, aoRadius, float, 1.0f);
    DEFINE_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_OBSCURANCE_LEVEL, AOObscuranceLevel, aoObscuranceLevel, float, 0.5f);
    DEFINE_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_FALLOFF_ANGLE, AOFalloffAngle, aoFalloffAngle, float, 0.25f);
    DEFINE_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_SAMPLING_AMOUNT, AOSamplingAmount, aoSamplingAmount, float, 0.5f);
    DEFINE_PROPERTY(PROP_AMBIENT_OCCLUSION_SSAO_NUM_SPIRAL_TURNS, SSAONumSpiralTurns, ssaoNumSpiralTurns, float, 7.0f);
};

#endif // hifi_AmbientOcclusionPropertyGroup_h
