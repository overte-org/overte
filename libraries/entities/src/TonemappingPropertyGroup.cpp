//
//  TonemappingPropertyGroup.cpp
//  libraries/entities/src
//
//  Created by HifiExperiments on 6/23/24
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "TonemappingPropertyGroup.h"

#include <OctreePacketData.h>

#include "EntityItemProperties.h"
#include "EntityItemPropertiesMacros.h"

inline void addTonemappingCurve(QHash<QString, TonemappingCurve>& lookup, TonemappingCurve curve) { lookup[TonemappingCurveHelpers::getNameForTonemappingCurve(curve)] = curve; }
const QHash<QString, TonemappingCurve> stringToTonemappingCurveLookup = [] {
    QHash<QString, TonemappingCurve> toReturn;
    addTonemappingCurve(toReturn, TonemappingCurve::RGB);
    addTonemappingCurve(toReturn, TonemappingCurve::SRGB);
    addTonemappingCurve(toReturn, TonemappingCurve::FILMIC);
    addTonemappingCurve(toReturn, TonemappingCurve::REINHARD);
    return toReturn;
}();
QString TonemappingPropertyGroup::getCurveAsString() const { return TonemappingCurveHelpers::getNameForTonemappingCurve(_curve); }
void TonemappingPropertyGroup::setCurveFromString(const QString& curve) {
    auto curveItr = stringToTonemappingCurveLookup.find(curve.toLower());
    if (curveItr != stringToTonemappingCurveLookup.end()) {
        _curve = curveItr.value();
        _curveChanged = true;
    }
}

void TonemappingPropertyGroup::copyToScriptValue(const EntityPropertyFlags& desiredProperties, ScriptValue& properties, ScriptEngine* engine,
        bool skipDefaults, EntityItemProperties& defaultEntityProperties, bool returnNothingOnEmptyPropertyFlags, bool isMyOwnAvatarEntity) const {
    COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE_GETTER(PROP_TONEMAPPING_CURVE, Tonemapping, tonemapping, Curve, curve, getCurveAsString);
    COPY_GROUP_PROPERTY_TO_QSCRIPTVALUE(PROP_TONEMAPPING_EXPOSURE, Tonemapping, tonemapping, Exposure, exposure);
}

void TonemappingPropertyGroup::copyFromScriptValue(const ScriptValue& object, const QSet<QString> &namesSet, bool& _defaultSettings) {
    COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE_ENUM(tonemapping, curve, Curve);
    COPY_GROUP_PROPERTY_FROM_QSCRIPTVALUE(tonemapping, exposure, float, setExposure);
}

void TonemappingPropertyGroup::merge(const TonemappingPropertyGroup& other) {
    COPY_PROPERTY_IF_CHANGED(curve);
    COPY_PROPERTY_IF_CHANGED(exposure);
}

void TonemappingPropertyGroup::debugDump() const {
    qCDebug(entities) << "   TonemappingPropertyGroup: ---------------------------------------------";
    qCDebug(entities) << "       Curve:" << getCurveAsString();
    qCDebug(entities) << "       Exposure:" << getExposure();
}

void TonemappingPropertyGroup::listChangedProperties(QList<QString>& out) {
    if (curveChanged()) {
        out << "tonemapping-curve";
    }
    if (exposureChanged()) {
        out << "tonemapping-exposure";
    }
}

bool TonemappingPropertyGroup::appendToEditPacket(OctreePacketData* packetData,
                                    EntityPropertyFlags& requestedProperties,
                                    EntityPropertyFlags& propertyFlags,
                                    EntityPropertyFlags& propertiesDidntFit,
                                    int& propertyCount,
                                    OctreeElement::AppendState& appendState) const {

    bool successPropertyFits = true;

    APPEND_ENTITY_PROPERTY(PROP_TONEMAPPING_CURVE, (uint32_t)getCurve());
    APPEND_ENTITY_PROPERTY(PROP_TONEMAPPING_EXPOSURE, getExposure());

    return true;
}


bool TonemappingPropertyGroup::decodeFromEditPacket(EntityPropertyFlags& propertyFlags, const unsigned char*& dataAt , int& processedBytes) {

    int bytesRead = 0;
    bool overwriteLocalData = true;
    bool somethingChanged = false;

    READ_ENTITY_PROPERTY(PROP_TONEMAPPING_CURVE, TonemappingCurve, setCurve);
    READ_ENTITY_PROPERTY(PROP_TONEMAPPING_EXPOSURE, float, setExposure);

    DECODE_GROUP_PROPERTY_HAS_CHANGED(PROP_TONEMAPPING_CURVE, Curve);
    DECODE_GROUP_PROPERTY_HAS_CHANGED(PROP_TONEMAPPING_EXPOSURE, Exposure);

    processedBytes += bytesRead;

    Q_UNUSED(somethingChanged);

    return true;
}

void TonemappingPropertyGroup::markAllChanged() {
    _curveChanged = true;
    _exposureChanged = true;
}

EntityPropertyFlags TonemappingPropertyGroup::getChangedProperties() const {
    EntityPropertyFlags changedProperties;

    CHECK_PROPERTY_CHANGE(PROP_TONEMAPPING_CURVE, curve);
    CHECK_PROPERTY_CHANGE(PROP_TONEMAPPING_EXPOSURE, exposure);

    return changedProperties;
}

void TonemappingPropertyGroup::getProperties(EntityItemProperties& properties) const {
    COPY_ENTITY_GROUP_PROPERTY_TO_PROPERTIES(Tonemapping, Curve, getCurve);
    COPY_ENTITY_GROUP_PROPERTY_TO_PROPERTIES(Tonemapping, Exposure, getExposure);
}

bool TonemappingPropertyGroup::setProperties(const EntityItemProperties& properties) {
    bool somethingChanged = false;

    SET_ENTITY_GROUP_PROPERTY_FROM_PROPERTIES(Tonemapping, Curve, curve, setCurve);
    SET_ENTITY_GROUP_PROPERTY_FROM_PROPERTIES(Tonemapping, Exposure, exposure, setExposure);

    return somethingChanged;
}

EntityPropertyFlags TonemappingPropertyGroup::getEntityProperties(EncodeBitstreamParams& params) const {
    EntityPropertyFlags requestedProperties;

    requestedProperties += PROP_TONEMAPPING_CURVE;
    requestedProperties += PROP_TONEMAPPING_EXPOSURE;

    return requestedProperties;
}

void TonemappingPropertyGroup::appendSubclassData(OctreePacketData* packetData, EncodeBitstreamParams& params,
                                EntityTreeElementExtraEncodeDataPointer entityTreeElementExtraEncodeData,
                                EntityPropertyFlags& requestedProperties,
                                EntityPropertyFlags& propertyFlags,
                                EntityPropertyFlags& propertiesDidntFit,
                                int& propertyCount,
                                OctreeElement::AppendState& appendState) const {

    bool successPropertyFits = true;

    APPEND_ENTITY_PROPERTY(PROP_TONEMAPPING_CURVE, (uint32_t)getCurve());
    APPEND_ENTITY_PROPERTY(PROP_TONEMAPPING_EXPOSURE, getExposure());
}

int TonemappingPropertyGroup::readEntitySubclassDataFromBuffer(const unsigned char* data, int bytesLeftToRead,
                                            ReadBitstreamToTreeParams& args,
                                            EntityPropertyFlags& propertyFlags, bool overwriteLocalData,
                                            bool& somethingChanged) {

    int bytesRead = 0;
    const unsigned char* dataAt = data;

    READ_ENTITY_PROPERTY(PROP_TONEMAPPING_CURVE, TonemappingCurve, setCurve);
    READ_ENTITY_PROPERTY(PROP_TONEMAPPING_EXPOSURE, float, setExposure);

    return bytesRead;
}
