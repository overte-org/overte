//
//  AmbientLightPropertyGroup.cpp
//  libraries/entities/src
//
//  Created by Nissim Hadar on 2017/12/24.
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "AmbientLightPropertyGroup.h"

#include <QJsonDocument>
#include <OctreePacketData.h>

#include "EntityItemProperties.h"
#include "EntityItemPropertiesMacros.h"

const float AmbientLightPropertyGroup::DEFAULT_AMBIENT_LIGHT_INTENSITY = 0.5f;
const glm::u8vec3 AmbientLightPropertyGroup::DEFAULT_COLOR = { 0, 0, 0 };

void AmbientLightPropertyGroup::copyToScriptValue(const EntityPropertyFlags& desiredProperties, ScriptValue& properties,
    ScriptEngine* engine, bool skipDefaults, EntityItemProperties& defaultEntityProperties, bool returnNothingOnEmptyPropertyFlags,
    bool isMyOwnAvatarEntity) const {

    auto nodeList = DependencyManager::get<NodeList>();
    COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE(PROP_AMBIENT_LIGHT_INTENSITY, AmbientLight, ambientLight, AmbientIntensity, ambientIntensity);
    COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE_IF_URL_PERMISSION(PROP_AMBIENT_LIGHT_URL, AmbientLight, ambientLight, AmbientURL, ambientURL);
    COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE(PROP_AMBIENT_LIGHT_COLOR, AmbientLight, ambientLight, AmbientColor, ambientColor);
}

void AmbientLightPropertyGroup::copyFromScriptValue(const ScriptValue& object, const QSet<QString> &namesSet, bool& _defaultSettings) {
    COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE(ambientLight, ambientIntensity, float, setAmbientIntensity);
    COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE(ambientLight, ambientURL, QString, setAmbientURL);
    COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE(ambientLight, ambientColor, u8vec3Color, setAmbientColor);
    
    // legacy property support
    COPY_PROPERTY_FROM_QSCRIPTVALUE_GETTER(ambientLightAmbientIntensity, float, setAmbientIntensity, getAmbientIntensity);
}

void AmbientLightPropertyGroup::merge(const AmbientLightPropertyGroup& other) {
    COPY_PROPERTY_IF_CHANGED(ambientIntensity);
    COPY_PROPERTY_IF_CHANGED(ambientURL);
    COPY_PROPERTY_IF_CHANGED(ambientColor);
}

void AmbientLightPropertyGroup::debugDump() const {
    qCDebug(entities) << "   AmbientLightPropertyGroup: ---------------------------------------------";
    qCDebug(entities) << "        ambientIntensity:" << getAmbientIntensity();
    qCDebug(entities) << "        ambientURL:" << getAmbientURL();
    qCDebug(entities) << "        ambientColor:" << getAmbientColor();
}

void AmbientLightPropertyGroup::listChangedProperties(QList<QString>& out) {
    if (ambientIntensityChanged()) {
        out << "ambientLight-ambientIntensity";
    }
    if (ambientURLChanged()) {
        out << "ambientLight-ambientURL";
    }
    if (ambientColorChanged()) {
        out << "ambientLight-ambientColor";
    }
}

bool AmbientLightPropertyGroup::appendToEditPacket(OctreePacketData* packetData,
                                    EntityPropertyFlags& requestedProperties,
                                    EntityPropertyFlags& propertyFlags,
                                    EntityPropertyFlags& propertiesDidntFit,
                                    int& propertyCount, 
                                    OctreeElement::AppendState& appendState) const {

    bool successPropertyFits = true;
    
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_LIGHT_INTENSITY, getAmbientIntensity());
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_LIGHT_URL, getAmbientURL());
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_LIGHT_COLOR, getAmbientColor());
    
    return true;
}

bool AmbientLightPropertyGroup::decodeFromEditPacket(EntityPropertyFlags& propertyFlags, const unsigned char*& dataAt,
    int& processedBytes) {
        
    int bytesRead = 0;
    bool overwriteLocalData = true;
    bool somethingChanged = false;
    
    READ_ENTITY_PROPERTY(PROP_AMBIENT_LIGHT_INTENSITY, float, setAmbientIntensity);
    READ_ENTITY_PROPERTY(PROP_AMBIENT_LIGHT_URL, QString, setAmbientURL);
    READ_ENTITY_PROPERTY(PROP_AMBIENT_LIGHT_COLOR, u8vec3Color, setAmbientColor);
    
    DECODE_GROUP_PROPERTY_HAS_CHANGED(PROP_AMBIENT_LIGHT_INTENSITY, AmbientIntensity);
    DECODE_GROUP_PROPERTY_HAS_CHANGED(PROP_AMBIENT_LIGHT_URL, AmbientURL);
    DECODE_GROUP_PROPERTY_HAS_CHANGED(PROP_AMBIENT_LIGHT_COLOR, AmbientColor);
    
    processedBytes += bytesRead;

    Q_UNUSED(somethingChanged);

    return true;
}

void AmbientLightPropertyGroup::markAllChanged() {
    _ambientIntensityChanged = true;
    _ambientURLChanged = true;
    _ambientColorChanged = true;
}

EntityPropertyFlags AmbientLightPropertyGroup::getChangedProperties() const {
    EntityPropertyFlags changedProperties;
 
    CHECK_PROPERTY_CHANGE(PROP_AMBIENT_LIGHT_INTENSITY, ambientIntensity);
    CHECK_PROPERTY_CHANGE(PROP_AMBIENT_LIGHT_URL, ambientURL);
    CHECK_PROPERTY_CHANGE(PROP_AMBIENT_LIGHT_COLOR, ambientColor);
    
    return changedProperties;
}

void AmbientLightPropertyGroup::getProperties(EntityItemProperties& properties) const {
    COPY_ENTITY_GROUP_PROPERTY_TO_PROPERTIES(AmbientLight, AmbientIntensity, getAmbientIntensity);
    COPY_ENTITY_GROUP_PROPERTY_TO_PROPERTIES(AmbientLight, AmbientURL, getAmbientURL);
    COPY_ENTITY_GROUP_PROPERTY_TO_PROPERTIES(AmbientLight, AmbientColor, getAmbientColor);
}

bool AmbientLightPropertyGroup::setProperties(const EntityItemProperties& properties) {
    bool somethingChanged = false;

    SET_ENTITY_GROUP_PROPERTY_FROM_PROPERTIES(AmbientLight, AmbientIntensity, ambientIntensity, setAmbientIntensity);
    SET_ENTITY_GROUP_PROPERTY_FROM_PROPERTIES(AmbientLight, AmbientURL, ambientURL, setAmbientURL);
    SET_ENTITY_GROUP_PROPERTY_FROM_PROPERTIES(AmbientLight, AmbientColor, ambientColor, setAmbientColor);

    return somethingChanged;
}

EntityPropertyFlags AmbientLightPropertyGroup::getEntityProperties(EncodeBitstreamParams& params) const {
    EntityPropertyFlags requestedProperties;

    requestedProperties += PROP_AMBIENT_LIGHT_INTENSITY;
    requestedProperties += PROP_AMBIENT_LIGHT_URL;
    requestedProperties += PROP_AMBIENT_LIGHT_COLOR;

    return requestedProperties;
}
    
void AmbientLightPropertyGroup::appendSubclassData(OctreePacketData* packetData, EncodeBitstreamParams& params, 
    EntityTreeElementExtraEncodeDataPointer entityTreeElementExtraEncodeData,
    EntityPropertyFlags& requestedProperties,
    EntityPropertyFlags& propertyFlags,
    EntityPropertyFlags& propertiesDidntFit,
    int& propertyCount, 
    OctreeElement::AppendState& appendState) const {

    bool successPropertyFits = true;

    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_LIGHT_INTENSITY, getAmbientIntensity());
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_LIGHT_URL, getAmbientURL());
    APPEND_ENTITY_PROPERTY(PROP_AMBIENT_LIGHT_COLOR, getAmbientColor());
}

int AmbientLightPropertyGroup::readEntitySubclassDataFromBuffer(const unsigned char* data, int bytesLeftToRead, 
    ReadBitstreamToTreeParams& args,
    EntityPropertyFlags& propertyFlags, bool overwriteLocalData,
    bool& somethingChanged) {

    int bytesRead = 0;
    const unsigned char* dataAt = data;
    
    READ_ENTITY_PROPERTY(PROP_AMBIENT_LIGHT_INTENSITY, float, setAmbientIntensity);
    READ_ENTITY_PROPERTY(PROP_AMBIENT_LIGHT_URL, QString, setAmbientURL);
    READ_ENTITY_PROPERTY(PROP_AMBIENT_LIGHT_COLOR, u8vec3Color, setAmbientColor);

    return bytesRead;
}
