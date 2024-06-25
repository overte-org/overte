//
//  TonemappingPropertyGroup.h
//  libraries/entities/src
//
//  Created by HifiExperiments on 6/23/24
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef hifi_TonemappingPropertyGroup_h
#define hifi_TonemappingPropertyGroup_h

#include <TonemappingCurve.h>

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
 * Tonemapping is defined by the following properties:
 * @typedef {object} Entities.Tonemapping
 * @property {TonemappingCurve} curve="srgb" - The tonemapping curve used.
 * @property {number} exposure=0.0 - The applied exposure.
 */
class TonemappingPropertyGroup : public PropertyGroup {
public:
    // EntityItemProperty related helpers
    virtual void copyToScriptValue(const EntityPropertyFlags& desiredProperties, ScriptValue& properties,
                                   ScriptEngine* engine, bool skipDefaults,
                                   EntityItemProperties& defaultEntityProperties, bool returnNothingOnEmptyPropertyFlags,
                                   bool isMyOwnAvatarEntity) const override;
    virtual void copyFromScriptValue(const ScriptValue& object, const QSet<QString> &namesSet, bool& _defaultSettings) override;

    void merge(const TonemappingPropertyGroup& other);

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

    DEFINE_PROPERTY_REF_ENUM(PROP_TONEMAPPING_CURVE, Curve, curve, TonemappingCurve, TonemappingCurve::SRGB);
    DEFINE_PROPERTY(PROP_TONEMAPPING_EXPOSURE, Exposure, exposure, float, 0.0);
};

#endif // hifi_TonemappingPropertyGroup_h
