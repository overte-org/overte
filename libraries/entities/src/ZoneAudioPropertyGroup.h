//
//  ZoneAudioPropertyGroup.h
//  libraries/entities/src
//
//  Created by HifiExperiments on 11/28/23
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef hifi_ZoneAudioPropertyGroup_h
#define hifi_ZoneAudioPropertyGroup_h

#include <stdint.h>
#include <glm/glm.hpp>

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
 * Zone audio is defined by the following properties:
 * @typedef {object} Entities.ZoneAudio
 * @property {boolean} reverbEnabled=false - If reverb should be enabled for listeners in this zone.
 * @property {number} reverbTime=1.0 - The time (seconds) for the reverb tail to decay by 60dB, also known as RT60.
 * @property {number} reverbWetLevel=50 - Adjusts the wet/dry percentage, from completely dry (0%) to completely wet (100%).
 * @property {Uuid[]} listenerZones=[] - A list of entity IDs representing listener zones with this zone as a source.
 *     Sounds from this zone being heard by a listener in a listener zone will be attenuated by the corresponding
 *     <code>listenerAttenuationCoefficient</code>.
 * @property {number[]} listenerAttenuationCoefficients=[] - A list of attenuation coefficients.  Each coefficient will be
 *     applied to sounds coming from this zone and being heard by a listener in the corresponding <code>listenerZone</code>.
 */
class ZoneAudioPropertyGroup : public PropertyGroup {
public:
    // EntityItemProperty related helpers
    virtual void copyToScriptValue(const EntityPropertyFlags& desiredProperties, ScriptValue& properties,
                                   ScriptEngine* engine, bool skipDefaults,
                                   EntityItemProperties& defaultEntityProperties, bool returnNothingOnEmptyPropertyFlags,
                                   bool isMyOwnAvatarEntity) const override;
    virtual void copyFromScriptValue(const ScriptValue& object, const QSet<QString> &namesSet, bool& _defaultSettings) override;

    void merge(const ZoneAudioPropertyGroup& other);

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

    DEFINE_PROPERTY(PROP_REVERB_ENABLED, ReverbEnabled, reverbEnabled, bool, false);
    DEFINE_PROPERTY(PROP_REVERB_TIME, ReverbTime, reverbTime, float, 1.0f);
    DEFINE_PROPERTY(PROP_REVERB_WET_LEVEL, ReverbWetLevel, reverbWetLevel, float, 50.0f);
    DEFINE_PROPERTY(PROP_LISTENER_ZONES, ListenerZones, listenerZones, QVector<QUuid>, QVector<QUuid>());
    DEFINE_PROPERTY(PROP_LISTENER_ATTENUATION_COEFFICIENTS, ListenerAttenuationCoefficients, listenerAttenuationCoefficients, QVector<float>, QVector<float>());

};

#endif // hifi_ZoneAudioPropertyGroup_h
