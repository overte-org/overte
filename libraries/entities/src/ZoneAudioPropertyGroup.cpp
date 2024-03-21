//
//  ZoneAudioPropertyGroup.cpp
//  libraries/entities/src
//
//  Created by HifiExperiments on 11/28/23
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "ZoneAudioPropertyGroup.h"

#include <OctreePacketData.h>

#include "EntityItemProperties.h"
#include "EntityItemPropertiesMacros.h"

void ZoneAudioPropertyGroup::copyToScriptValue(const EntityPropertyFlags& desiredProperties, ScriptValue& properties, ScriptEngine* engine, bool skipDefaults, EntityItemProperties& defaultEntityProperties) const {
    COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE(PROP_REVERB_ENABLED, Audio, audio, ReverbEnabled, reverbEnabled);
    COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE(PROP_REVERB_TIME, Audio, audio, ReverbTime, reverbTime);
    COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE(PROP_REVERB_WET_LEVEL, Audio, audio, ReverbWetLevel, reverbWetLevel);
    COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE(PROP_LISTENER_ZONES, Audio, audio, ListenerZones, listenerZones);
    COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE(PROP_LISTENER_ATTENUATION_COEFFICIENTS, Audio, audio, ListenerAttenuationCoefficients, listenerAttenuationCoefficients);
}

void ZoneAudioPropertyGroup::copyFromScriptValue(const ScriptValue& object, const QSet<QString> &namesSet, bool& _defaultSettings) {
    COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE(audio, reverbEnabled, bool, setReverbEnabled);
    COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE(audio, reverbTime, float, setReverbTime);
    COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE(audio, reverbWetLevel, float, setReverbWetLevel);
    COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE(audio, listenerZones, qVectorQUuid, setListenerZones);
    COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE(audio, listenerAttenuationCoefficients, qVectorFloat, setListenerAttenuationCoefficients);
}

void ZoneAudioPropertyGroup::merge(const ZoneAudioPropertyGroup& other) {
    COPY_PROPERTY_IF_CHANGED(reverbEnabled);
    COPY_PROPERTY_IF_CHANGED(reverbTime);
    COPY_PROPERTY_IF_CHANGED(reverbWetLevel);
    COPY_PROPERTY_IF_CHANGED(listenerZones);
    COPY_PROPERTY_IF_CHANGED(listenerAttenuationCoefficients);
}

void ZoneAudioPropertyGroup::debugDump() const {
    qCDebug(entities) << "              ZoneAudioPropertyGroup: ---------------------------------------------";
    qCDebug(entities) << "                      _reverbEnabled:" << _reverbEnabled;
    qCDebug(entities) << "                         _reverbTime:" << _reverbTime;
    qCDebug(entities) << "                     _reverbWetLevel:" << _reverbWetLevel;
    qCDebug(entities) << "                      _listenerZones:" << _listenerZones;
    qCDebug(entities) << "    _listenerAttenuationCoefficients:" << _listenerAttenuationCoefficients;
}

void ZoneAudioPropertyGroup::listChangedProperties(QList<QString>& out) {
    if (reverbEnabledChanged()) {
        out << "reverbEnabled";
    }
    if (reverbTimeChanged()) {
        out << "reverbTime";
    }
    if (reverbWetLevelChanged()) {
        out << "reverbWetLevel";
    }
    if (listenerZonesChanged()) {
        out << "listenerZones";
    }
    if (listenerAttenuationCoefficientsChanged()) {
        out << "listenerAttenuationCoefficients";
    }
}

bool ZoneAudioPropertyGroup::appendToEditPacket(OctreePacketData* packetData,
                                                EntityPropertyFlags& requestedProperties,
                                                EntityPropertyFlags& propertyFlags,
                                                EntityPropertyFlags& propertiesDidntFit,
                                                int& propertyCount,
                                                OctreeElement::AppendState& appendState) const {
    bool successPropertyFits = true;

    APPEND_ENTITY_PROPERTY(PROP_REVERB_ENABLED, getReverbEnabled());
    APPEND_ENTITY_PROPERTY(PROP_REVERB_TIME, getReverbTime());
    APPEND_ENTITY_PROPERTY(PROP_REVERB_WET_LEVEL, getReverbWetLevel());
    APPEND_ENTITY_PROPERTY(PROP_LISTENER_ZONES, getListenerZones());
    APPEND_ENTITY_PROPERTY(PROP_LISTENER_ATTENUATION_COEFFICIENTS, getListenerAttenuationCoefficients());

    return true;
}

bool ZoneAudioPropertyGroup::decodeFromEditPacket(EntityPropertyFlags& propertyFlags, const unsigned char*& dataAt , int& processedBytes) {
    int bytesRead = 0;
    bool overwriteLocalData = true;
    bool somethingChanged = false;

    READ_ENTITY_PROPERTY(PROP_REVERB_ENABLED, bool, setReverbEnabled);
    READ_ENTITY_PROPERTY(PROP_REVERB_TIME, float, setReverbTime);
    READ_ENTITY_PROPERTY(PROP_REVERB_WET_LEVEL, float, setReverbWetLevel);
    READ_ENTITY_PROPERTY(PROP_LISTENER_ZONES, QVector<QUuid>, setListenerZones);
    READ_ENTITY_PROPERTY(PROP_LISTENER_ATTENUATION_COEFFICIENTS, QVector<float>, setListenerAttenuationCoefficients);

    DECODE_GROUP_PROPERTY_HAS_CHANGED(PROP_REVERB_ENABLED, ReverbEnabled);
    DECODE_GROUP_PROPERTY_HAS_CHANGED(PROP_REVERB_TIME, ReverbTime);
    DECODE_GROUP_PROPERTY_HAS_CHANGED(PROP_REVERB_WET_LEVEL, ReverbWetLevel);
    DECODE_GROUP_PROPERTY_HAS_CHANGED(PROP_LISTENER_ZONES, ListenerZones);
    DECODE_GROUP_PROPERTY_HAS_CHANGED(PROP_LISTENER_ATTENUATION_COEFFICIENTS, ListenerAttenuationCoefficients);

    processedBytes += bytesRead;

    Q_UNUSED(somethingChanged);

    return true;
}

void ZoneAudioPropertyGroup::markAllChanged() {
    _reverbEnabledChanged = true;
    _reverbTimeChanged = true;
    _reverbWetLevelChanged = true;
    _listenerZonesChanged = true;
    _listenerAttenuationCoefficientsChanged = true;
}

EntityPropertyFlags ZoneAudioPropertyGroup::getChangedProperties() const {
    EntityPropertyFlags changedProperties;

    CHECK_PROPERTY_CHANGE(PROP_REVERB_ENABLED, reverbEnabled);
    CHECK_PROPERTY_CHANGE(PROP_REVERB_TIME, reverbTime);
    CHECK_PROPERTY_CHANGE(PROP_REVERB_WET_LEVEL, reverbWetLevel);
    CHECK_PROPERTY_CHANGE(PROP_LISTENER_ZONES, listenerZones);
    CHECK_PROPERTY_CHANGE(PROP_LISTENER_ATTENUATION_COEFFICIENTS, listenerAttenuationCoefficients);

    return changedProperties;
}

void ZoneAudioPropertyGroup::getProperties(EntityItemProperties& properties) const {
    COPY_ENTITY_GROUP_PROPERTY_TO_PROPERTIES(Audio, ReverbEnabled, getReverbEnabled);
    COPY_ENTITY_GROUP_PROPERTY_TO_PROPERTIES(Audio, ReverbTime, getReverbTime);
    COPY_ENTITY_GROUP_PROPERTY_TO_PROPERTIES(Audio, ReverbWetLevel, getReverbWetLevel);
    COPY_ENTITY_GROUP_PROPERTY_TO_PROPERTIES(Audio, ListenerZones, getListenerZones);
    COPY_ENTITY_GROUP_PROPERTY_TO_PROPERTIES(Audio, ListenerAttenuationCoefficients, getListenerAttenuationCoefficients);
}

bool ZoneAudioPropertyGroup::setProperties(const EntityItemProperties& properties) {
    bool somethingChanged = false;

    SET_ENTITY_GROUP_PROPERTY_FROM_PROPERTIES(Audio, ReverbEnabled, reverbEnabled, setReverbEnabled);
    SET_ENTITY_GROUP_PROPERTY_FROM_PROPERTIES(Audio, ReverbTime, reverbTime, setReverbTime);
    SET_ENTITY_GROUP_PROPERTY_FROM_PROPERTIES(Audio, ReverbWetLevel, reverbWetLevel, setReverbWetLevel);
    SET_ENTITY_GROUP_PROPERTY_FROM_PROPERTIES(Audio, ListenerZones, listenerZones, setListenerZones);
    SET_ENTITY_GROUP_PROPERTY_FROM_PROPERTIES(Audio, ListenerAttenuationCoefficients, listenerAttenuationCoefficients, setListenerAttenuationCoefficients);

    return somethingChanged;
}

EntityPropertyFlags ZoneAudioPropertyGroup::getEntityProperties(EncodeBitstreamParams& params) const {
    EntityPropertyFlags requestedProperties;

    requestedProperties += PROP_REVERB_ENABLED;
    requestedProperties += PROP_REVERB_TIME;
    requestedProperties += PROP_REVERB_WET_LEVEL;
    requestedProperties += PROP_LISTENER_ZONES;
    requestedProperties += PROP_LISTENER_ATTENUATION_COEFFICIENTS;

    return requestedProperties;
}

void ZoneAudioPropertyGroup::appendSubclassData(OctreePacketData* packetData, EncodeBitstreamParams& params,
                                                EntityTreeElementExtraEncodeDataPointer entityTreeElementExtraEncodeData,
                                                EntityPropertyFlags& requestedProperties,
                                                EntityPropertyFlags& propertyFlags,
                                                EntityPropertyFlags& propertiesDidntFit,
                                                int& propertyCount,
                                                OctreeElement::AppendState& appendState) const {
    bool successPropertyFits = true;

    APPEND_ENTITY_PROPERTY(PROP_REVERB_ENABLED, getReverbEnabled());
    APPEND_ENTITY_PROPERTY(PROP_REVERB_TIME, getReverbTime());
    APPEND_ENTITY_PROPERTY(PROP_REVERB_WET_LEVEL, getReverbWetLevel());
    APPEND_ENTITY_PROPERTY(PROP_LISTENER_ZONES, getListenerZones());
    APPEND_ENTITY_PROPERTY(PROP_LISTENER_ATTENUATION_COEFFICIENTS, getListenerAttenuationCoefficients());
}

int ZoneAudioPropertyGroup::readEntitySubclassDataFromBuffer(const unsigned char* data, int bytesLeftToRead,
                                                         ReadBitstreamToTreeParams& args,
                                                         EntityPropertyFlags& propertyFlags, bool overwriteLocalData,
                                                         bool& somethingChanged) {
    int bytesRead = 0;
    const unsigned char* dataAt = data;

    READ_ENTITY_PROPERTY(PROP_REVERB_ENABLED, bool, setReverbEnabled);
    READ_ENTITY_PROPERTY(PROP_REVERB_TIME, float, setReverbTime);
    READ_ENTITY_PROPERTY(PROP_REVERB_WET_LEVEL, float, setReverbWetLevel);
    READ_ENTITY_PROPERTY(PROP_LISTENER_ZONES, QVector<QUuid>, setListenerZones);
    READ_ENTITY_PROPERTY(PROP_LISTENER_ATTENUATION_COEFFICIENTS, QVector<float>, setListenerAttenuationCoefficients);

    return bytesRead;
}
