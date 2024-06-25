//
//  AmbientOcclusionPropertyGroup.cpp
//  libraries/entities/src
//
//  Created by HifiExperiments on 6/23/24
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "AmbientOcclusionPropertyGroup.h"

#include <OctreePacketData.h>

#include "EntityItemProperties.h"
#include "EntityItemPropertiesMacros.h"

inline void addAmbientOcclusionTechnique(QHash<QString, AmbientOcclusionTechnique>& lookup, AmbientOcclusionTechnique technique) { lookup[AmbientOcclusionTechniqueHelpers::getNameForAmbientOcclusionTechnique(technique)] = technique; }
const QHash<QString, AmbientOcclusionTechnique> stringToAmbientOcclusionTechniqueLookup = [] {
    QHash<QString, AmbientOcclusionTechnique> toReturn;
    addAmbientOcclusionTechnique(toReturn, AmbientOcclusionTechnique::SSAO);
    addAmbientOcclusionTechnique(toReturn, AmbientOcclusionTechnique::HBAO);
    return toReturn;
}();
QString AmbientOcclusionPropertyGroup::getTechniqueAsString() const { return AmbientOcclusionTechniqueHelpers::getNameForAmbientOcclusionTechnique(_technique); }
void AmbientOcclusionPropertyGroup::setTechniqueFromString(const QString& technique) {
    auto techniqueItr = stringToAmbientOcclusionTechniqueLookup.find(technique.toLower());
    if (techniqueItr != stringToAmbientOcclusionTechniqueLookup.end()) {
        _technique = techniqueItr.value();
        _techniqueChanged = true;
    }
}

void AmbientOcclusionPropertyGroup::copyToScriptValue(const EntityPropertyFlags& desiredProperties, ScriptValue& properties, ScriptEngine* engine,
        bool skipDefaults, EntityItemProperties& defaultEntityProperties, bool returnNothingOnEmptyPropertyFlags, bool isMyOwnAvatarEntity) const {
    COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE_GETTER(PROP_AMBIENT_OCCLUSION_TECHNIQUE, AmbientOcclusion, ambientOcclusion, Technique, technique, getTechniqueAsString);
    COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE(PROP_AMBIENT_OCCLUSION_JITTER, AmbientOcclusion, ambientOcclusion, Jitter, jitter);
    COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE(PROP_AMBIENT_OCCLUSION_RESOLUTION_LEVEL, AmbientOcclusion, ambientOcclusion, ResolutionLevel, resolutionLevel);
    COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE(PROP_AMBIENT_OCCLUSION_EDGE_SHARPNESS, AmbientOcclusion, ambientOcclusion, EdgeSharpness, edgeSharpness);
    COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE(PROP_AMBIENT_OCCLUSION_BLUR_RADIUS, AmbientOcclusion, ambientOcclusion, BlurRadius, blurRadius);
    COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE(PROP_AMBIENT_OCCLUSION_AO_RADIUS, AmbientOcclusion, ambientOcclusion, AORadius, aoRadius);
    COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE(PROP_AMBIENT_OCCLUSION_AO_OBSCURANCE_LEVEL, AmbientOcclusion, ambientOcclusion, AOObscuranceLevel, aoObscuranceLevel);
    COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE(PROP_AMBIENT_OCCLUSION_AO_FALLOFF_ANGLE, AmbientOcclusion, ambientOcclusion, AOFalloffAngle, aoFalloffAngle);
    COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE(PROP_AMBIENT_OCCLUSION_AO_NUM_SAMPLES, AmbientOcclusion, ambientOcclusion, AONumSamples, aoNumSamples);
    COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE(PROP_AMBIENT_OCCLUSION_SSAO_NUM_SPIRAL_TURNS, AmbientOcclusion, ambientOcclusion, SSAONumSpiralTurns, ssaoNumSpiralTurns);
}

void AmbientOcclusionPropertyGroup::copyFromScriptValue(const ScriptValue& object, const QSet<QString> &namesSet, bool& _defaultSettings) {
    COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE_ENUM(ambientOcclusion, technique, Technique);
    COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE(ambientOcclusion, jitter, bool, setJitter);
    COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE(ambientOcclusion, resolutionLevel, uint8_t, setResolutionLevel);
    COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE(ambientOcclusion, edgeSharpness, float, setEdgeSharpness);
    COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE(ambientOcclusion, blurRadius, uint8_t, setBlurRadius);
    COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE(ambientOcclusion, aoRadius, float, setAORadius);
    COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE(ambientOcclusion, aoObscuranceLevel, float, setAOObscuranceLevel);
    COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE(ambientOcclusion, aoFalloffAngle, float, setAOFalloffAngle);
    COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE(ambientOcclusion, aoNumSamples, uint8_t, setAONumSamples);
    COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE(ambientOcclusion, ssaoNumSpiralTurns, float, setSSAONumSpiralTurns);
}

void AmbientOcclusionPropertyGroup::merge(const AmbientOcclusionPropertyGroup& other) {
    COPY_PROPERTY_IF_CHANGED(technique);
    COPY_PROPERTY_IF_CHANGED(jitter);
    COPY_PROPERTY_IF_CHANGED(resolutionLevel);
    COPY_PROPERTY_IF_CHANGED(edgeSharpness);
    COPY_PROPERTY_IF_CHANGED(blurRadius);
    COPY_PROPERTY_IF_CHANGED(aoRadius);
    COPY_PROPERTY_IF_CHANGED(aoObscuranceLevel);
    COPY_PROPERTY_IF_CHANGED(aoFalloffAngle);
    COPY_PROPERTY_IF_CHANGED(aoNumSamples);
    COPY_PROPERTY_IF_CHANGED(ssaoNumSpiralTurns);
}

void AmbientOcclusionPropertyGroup::debugDump() const {
    qCDebug(entities) << "   AmbientOcclusionPropertyGroup: ---------------------------------------------";
    qCDebug(entities) << "       Technique:" << getTechniqueAsString();
    qCDebug(entities) << "       Jitter:" << getJitter();
    qCDebug(entities) << "       ResolutionLevel:" << getResolutionLevel();
    qCDebug(entities) << "       EdgeSharpness:" << getEdgeSharpness();
    qCDebug(entities) << "       BlurRadius:" << getBlurRadius();
    qCDebug(entities) << "       AORadius:" << getAORadius();
    qCDebug(entities) << "       AOObscuranceLevel:" << getAOObscuranceLevel();
    qCDebug(entities) << "       AOFalloffAngle:" << getAOFalloffAngle();
    qCDebug(entities) << "       AONumSamples:" << getAONumSamples();
    qCDebug(entities) << "       SSAONumSpiralTurns:" << getSSAONumSpiralTurns();
}

void AmbientOcclusionPropertyGroup::listChangedProperties(QList<QString>& out) {
    if (techniqueChanged()) {
        out << "ambientOcclusion-technique";
    }
    if (jitterChanged()) {
        out << "ambientOcclusion-jitter";
    }
    if (resolutionLevelChanged()) {
        out << "ambientOcclusion-resolutionLevel";
    }
    if (edgeSharpnessChanged()) {
        out << "ambientOcclusion-edgeSharpness";
    }
    if (blurRadiusChanged()) {
        out << "ambientOcclusion-blurRadius";
    }
    if (aoRadiusChanged()) {
        out << "ambientOcclusion-aoRadius";
    }
    if (aoObscuranceLevelChanged()) {
        out << "ambientOcclusion-aoObscuranceLevel";
    }
    if (aoFalloffAngleChanged()) {
        out << "ambientOcclusion-aoFalloffAngle";
    }
    if (aoNumSamplesChanged()) {
        out << "ambientOcclusion-aoNumSamples";
    }
    if (ssaoNumSpiralTurnsChanged()) {
        out << "ambientOcclusion-ssaoNumSpiralTurns";
    }
}

bool AmbientOcclusionPropertyGroup::appendToEditPacket(OctreePacketData* packetData,
                                    EntityPropertyFlags& requestedProperties,
                                    EntityPropertyFlags& propertyFlags,
                                    EntityPropertyFlags& propertiesDidntFit,
                                    int& propertyCount,
                                    OctreeElement::AppendState& appendState) const {

    bool successPropertyFits = true;

    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_TECHNIQUE, (uint32_t)getTechnique());
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_JITTER, getJitter());
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_RESOLUTION_LEVEL, getResolutionLevel());
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_EDGE_SHARPNESS, getEdgeSharpness());
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_BLUR_RADIUS, getBlurRadius());
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_RADIUS, getAORadius());
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_OBSCURANCE_LEVEL, getAOObscuranceLevel());
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_FALLOFF_ANGLE, getAOFalloffAngle());
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_NUM_SAMPLES, getAONumSamples());
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_SSAO_NUM_SPIRAL_TURNS, getSSAONumSpiralTurns());

    return true;
}

bool AmbientOcclusionPropertyGroup::decodeFromEditPacket(EntityPropertyFlags& propertyFlags, const unsigned char*& dataAt , int& processedBytes) {

    int bytesRead = 0;
    bool overwriteLocalData = true;
    bool somethingChanged = false;

    READ_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_TECHNIQUE, AmbientOcclusionTechnique, setTechnique);
    READ_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_JITTER, bool, setJitter);
    READ_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_RESOLUTION_LEVEL, uint8_t, setResolutionLevel);
    READ_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_EDGE_SHARPNESS, float, setEdgeSharpness);
    READ_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_BLUR_RADIUS, uint8_t, setBlurRadius);
    READ_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_RADIUS, float, setAORadius);
    READ_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_OBSCURANCE_LEVEL, float, setAOObscuranceLevel);
    READ_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_FALLOFF_ANGLE, float, setAOFalloffAngle);
    READ_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_NUM_SAMPLES, uint8_t, setAONumSamples);
    READ_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_SSAO_NUM_SPIRAL_TURNS, float, setSSAONumSpiralTurns);

    DECODE_GROUP_PROPERTY_HAS_CHANGED(PROP_AMBIENT_OCCLUSION_TECHNIQUE, Technique);
    DECODE_GROUP_PROPERTY_HAS_CHANGED(PROP_AMBIENT_OCCLUSION_JITTER, Jitter);
    DECODE_GROUP_PROPERTY_HAS_CHANGED(PROP_AMBIENT_OCCLUSION_RESOLUTION_LEVEL, ResolutionLevel);
    DECODE_GROUP_PROPERTY_HAS_CHANGED(PROP_AMBIENT_OCCLUSION_EDGE_SHARPNESS, EdgeSharpness);
    DECODE_GROUP_PROPERTY_HAS_CHANGED(PROP_AMBIENT_OCCLUSION_BLUR_RADIUS, BlurRadius);
    DECODE_GROUP_PROPERTY_HAS_CHANGED(PROP_AMBIENT_OCCLUSION_AO_RADIUS, AORadius);
    DECODE_GROUP_PROPERTY_HAS_CHANGED(PROP_AMBIENT_OCCLUSION_AO_OBSCURANCE_LEVEL, AOObscuranceLevel);
    DECODE_GROUP_PROPERTY_HAS_CHANGED(PROP_AMBIENT_OCCLUSION_AO_FALLOFF_ANGLE, AOFalloffAngle);
    DECODE_GROUP_PROPERTY_HAS_CHANGED(PROP_AMBIENT_OCCLUSION_AO_NUM_SAMPLES, AONumSamples);
    DECODE_GROUP_PROPERTY_HAS_CHANGED(PROP_AMBIENT_OCCLUSION_SSAO_NUM_SPIRAL_TURNS, SSAONumSpiralTurns);

    processedBytes += bytesRead;

    Q_UNUSED(somethingChanged);

    return true;
}

void AmbientOcclusionPropertyGroup::markAllChanged() {
    _techniqueChanged = true;
    _jitterChanged = true;
    _resolutionLevelChanged = true;
    _edgeSharpnessChanged = true;
    _blurRadiusChanged = true;
    _aoRadiusChanged = true;
    _aoObscuranceLevelChanged = true;
    _aoFalloffAngleChanged = true;
    _aoNumSamplesChanged = true;
    _ssaoNumSpiralTurnsChanged = true;
}

EntityPropertyFlags AmbientOcclusionPropertyGroup::getChangedProperties() const {
    EntityPropertyFlags changedProperties;

    CHECK_PROPERTY_CHANGE(PROP_AMBIENT_OCCLUSION_TECHNIQUE, technique);
    CHECK_PROPERTY_CHANGE(PROP_AMBIENT_OCCLUSION_JITTER, jitter);
    CHECK_PROPERTY_CHANGE(PROP_AMBIENT_OCCLUSION_RESOLUTION_LEVEL, resolutionLevel);
    CHECK_PROPERTY_CHANGE(PROP_AMBIENT_OCCLUSION_EDGE_SHARPNESS, edgeSharpness);
    CHECK_PROPERTY_CHANGE(PROP_AMBIENT_OCCLUSION_BLUR_RADIUS, blurRadius);
    CHECK_PROPERTY_CHANGE(PROP_AMBIENT_OCCLUSION_AO_RADIUS, aoRadius);
    CHECK_PROPERTY_CHANGE(PROP_AMBIENT_OCCLUSION_AO_OBSCURANCE_LEVEL, aoObscuranceLevel);
    CHECK_PROPERTY_CHANGE(PROP_AMBIENT_OCCLUSION_AO_FALLOFF_ANGLE, aoFalloffAngle);
    CHECK_PROPERTY_CHANGE(PROP_AMBIENT_OCCLUSION_AO_NUM_SAMPLES, aoNumSamples);
    CHECK_PROPERTY_CHANGE(PROP_AMBIENT_OCCLUSION_SSAO_NUM_SPIRAL_TURNS, ssaoNumSpiralTurns);

    return changedProperties;
}

void AmbientOcclusionPropertyGroup::getProperties(EntityItemProperties& properties) const {
    COPY_ENTITY_GROUP_PROPERTY_TO_PROPERTIES(AmbientOcclusion, Technique, getTechnique);
    COPY_ENTITY_GROUP_PROPERTY_TO_PROPERTIES(AmbientOcclusion, Jitter, getJitter);
    COPY_ENTITY_GROUP_PROPERTY_TO_PROPERTIES(AmbientOcclusion, ResolutionLevel, getResolutionLevel);
    COPY_ENTITY_GROUP_PROPERTY_TO_PROPERTIES(AmbientOcclusion, EdgeSharpness, getEdgeSharpness);
    COPY_ENTITY_GROUP_PROPERTY_TO_PROPERTIES(AmbientOcclusion, BlurRadius, getBlurRadius);
    COPY_ENTITY_GROUP_PROPERTY_TO_PROPERTIES(AmbientOcclusion, AORadius, getAORadius);
    COPY_ENTITY_GROUP_PROPERTY_TO_PROPERTIES(AmbientOcclusion, AOObscuranceLevel, getAOObscuranceLevel);
    COPY_ENTITY_GROUP_PROPERTY_TO_PROPERTIES(AmbientOcclusion, AOFalloffAngle, getAOFalloffAngle);
    COPY_ENTITY_GROUP_PROPERTY_TO_PROPERTIES(AmbientOcclusion, AONumSamples, getAONumSamples);
    COPY_ENTITY_GROUP_PROPERTY_TO_PROPERTIES(AmbientOcclusion, SSAONumSpiralTurns, getSSAONumSpiralTurns);
}

bool AmbientOcclusionPropertyGroup::setProperties(const EntityItemProperties& properties) {
    bool somethingChanged = false;

    SET_ENTITY_GROUP_PROPERTY_FROM_PROPERTIES(AmbientOcclusion, Technique, technique, setTechnique);
    SET_ENTITY_GROUP_PROPERTY_FROM_PROPERTIES(AmbientOcclusion, Jitter, jitter, setJitter);
    SET_ENTITY_GROUP_PROPERTY_FROM_PROPERTIES(AmbientOcclusion, ResolutionLevel, resolutionLevel, setResolutionLevel);
    SET_ENTITY_GROUP_PROPERTY_FROM_PROPERTIES(AmbientOcclusion, EdgeSharpness, edgeSharpness, setEdgeSharpness);
    SET_ENTITY_GROUP_PROPERTY_FROM_PROPERTIES(AmbientOcclusion, BlurRadius, blurRadius, setBlurRadius);
    SET_ENTITY_GROUP_PROPERTY_FROM_PROPERTIES(AmbientOcclusion, AORadius, aoRadius, setAORadius);
    SET_ENTITY_GROUP_PROPERTY_FROM_PROPERTIES(AmbientOcclusion, AOObscuranceLevel, aoObscuranceLevel, setAOObscuranceLevel);
    SET_ENTITY_GROUP_PROPERTY_FROM_PROPERTIES(AmbientOcclusion, AOFalloffAngle, aoFalloffAngle, setAOFalloffAngle);
    SET_ENTITY_GROUP_PROPERTY_FROM_PROPERTIES(AmbientOcclusion, AONumSamples, aoNumSamples, setAONumSamples);
    SET_ENTITY_GROUP_PROPERTY_FROM_PROPERTIES(AmbientOcclusion, SSAONumSpiralTurns, ssaoNumSpiralTurns, setSSAONumSpiralTurns);

    return somethingChanged;
}

EntityPropertyFlags AmbientOcclusionPropertyGroup::getEntityProperties(EncodeBitstreamParams& params) const {
    EntityPropertyFlags requestedProperties;

    requestedProperties += PROP_AMBIENT_OCCLUSION_TECHNIQUE;
    requestedProperties += PROP_AMBIENT_OCCLUSION_JITTER;
    requestedProperties += PROP_AMBIENT_OCCLUSION_RESOLUTION_LEVEL;
    requestedProperties += PROP_AMBIENT_OCCLUSION_EDGE_SHARPNESS;
    requestedProperties += PROP_AMBIENT_OCCLUSION_BLUR_RADIUS;
    requestedProperties += PROP_AMBIENT_OCCLUSION_AO_RADIUS;
    requestedProperties += PROP_AMBIENT_OCCLUSION_AO_OBSCURANCE_LEVEL;
    requestedProperties += PROP_AMBIENT_OCCLUSION_AO_FALLOFF_ANGLE;
    requestedProperties += PROP_AMBIENT_OCCLUSION_AO_NUM_SAMPLES;
    requestedProperties += PROP_AMBIENT_OCCLUSION_SSAO_NUM_SPIRAL_TURNS;

    return requestedProperties;
}

void AmbientOcclusionPropertyGroup::appendSubclassData(OctreePacketData* packetData, EncodeBitstreamParams& params,
                                EntityTreeElementExtraEncodeDataPointer entityTreeElementExtraEncodeData,
                                EntityPropertyFlags& requestedProperties,
                                EntityPropertyFlags& propertyFlags,
                                EntityPropertyFlags& propertiesDidntFit,
                                int& propertyCount,
                                OctreeElement::AppendState& appendState) const {

    bool successPropertyFits = true;

    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_TECHNIQUE, (uint32_t)getTechnique());
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_JITTER, getJitter());
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_RESOLUTION_LEVEL, getResolutionLevel());
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_EDGE_SHARPNESS, getEdgeSharpness());
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_BLUR_RADIUS, getBlurRadius());
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_RADIUS, getAORadius());
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_OBSCURANCE_LEVEL, getAOObscuranceLevel());
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_FALLOFF_ANGLE, getAOFalloffAngle());
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_NUM_SAMPLES, getAONumSamples());
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_SSAO_NUM_SPIRAL_TURNS, getSSAONumSpiralTurns());
}

int AmbientOcclusionPropertyGroup::readEntitySubclassDataFromBuffer(const unsigned char* data, int bytesLeftToRead,
                                            ReadBitstreamToTreeParams& args,
                                            EntityPropertyFlags& propertyFlags, bool overwriteLocalData,
                                            bool& somethingChanged) {

    int bytesRead = 0;
    const unsigned char* dataAt = data;

    READ_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_TECHNIQUE, AmbientOcclusionTechnique, setTechnique);
    READ_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_JITTER, bool, setJitter);
    READ_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_RESOLUTION_LEVEL, uint8_t, setResolutionLevel);
    READ_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_EDGE_SHARPNESS, float, setEdgeSharpness);
    READ_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_BLUR_RADIUS, uint8_t, setBlurRadius);
    READ_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_RADIUS, float, setAORadius);
    READ_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_OBSCURANCE_LEVEL, float, setAOObscuranceLevel);
    READ_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_FALLOFF_ANGLE, float, setAOFalloffAngle);
    READ_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_AO_NUM_SAMPLES, uint8_t, setAONumSamples);
    READ_ENTITY_PROPERTY(PROP_AMBIENT_OCCLUSION_SSAO_NUM_SPIRAL_TURNS, float, setSSAONumSpiralTurns);

    return bytesRead;
}
