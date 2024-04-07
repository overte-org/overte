//
//  Created by HifiExperiments on 12/30/2023
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_SoundEntityItem_h
#define hifi_SoundEntityItem_h

#include "EntityItem.h"

#include <SoundCache.h>
#include <AudioInjectorManager.h>

class SoundEntityItem : public EntityItem {
public:
    static EntityItemPointer factory(const EntityItemID& entityID, const EntityItemProperties& properties);

    SoundEntityItem(const EntityItemID& entityItemID);
    ~SoundEntityItem();

    ALLOW_INSTANTIATION  // This class can be instantiated

    // methods for getting/setting all properties of an entity
    EntityItemProperties getProperties(const EntityPropertyFlags& desiredProperties, bool allowEmptyDesiredProperties) const override;
    bool setSubClassProperties(const EntityItemProperties& properties) override;

    EntityPropertyFlags getEntityProperties(EncodeBitstreamParams& params) const override;

    void appendSubclassData(OctreePacketData* packetData, EncodeBitstreamParams& params,
                            EntityTreeElementExtraEncodeDataPointer entityTreeElementExtraEncodeData,
                            EntityPropertyFlags& requestedProperties,
                            EntityPropertyFlags& propertyFlags,
                            EntityPropertyFlags& propertiesDidntFit,
                            int& propertyCount,
                            OctreeElement::AppendState& appendState) const override;

    int readEntitySubclassDataFromBuffer(const unsigned char* data, int bytesLeftToRead,
                                         ReadBitstreamToTreeParams& args,
                                         EntityPropertyFlags& propertyFlags, bool overwriteLocalData,
                                         bool& somethingChanged) override;

    bool shouldBePhysical() const override { return false; }

    virtual void debugDump() const override;

    virtual bool supportsDetailedIntersection() const override { return false; }

    virtual void update(const quint64& now) override;
    bool needsToCallUpdate() const override { return _updateNeeded; }

    void setLocalPosition(const glm::vec3& value, bool tellPhysics = true) override;
    void setLocalOrientation(const glm::quat& value) override;

    void setURL(const QString& value);
    QString getURL() const;

    void setVolume(float value);
    float getVolume() const;

    void setTimeOffset(float value);
    float getTimeOffset() const;

    void setPitch(float value);
    float getPitch() const;

    void setPlaying(bool value);
    bool getPlaying() const;

    void setLoop(bool value);
    bool getLoop() const;

    void setPositional(bool value);
    bool getPositional() const;

    void setLocalOnly(bool value);
    bool getLocalOnly() const;

    bool restartSound();

protected:
    bool shouldCreateSound(const EntityTreePointer& tree) const;
    void updateSound(bool restart = false);

    QString _url { "" };
    float _volume { 1.0f };
    float _timeOffset { 0.0f };
    float _pitch { 1.0f };
    bool _playing { true };
    bool _loop { true };
    bool _positional { true };
    bool _localOnly { false };

    SharedSoundPointer _sound;
    AudioInjectorPointer _injector;
    bool _updateNeeded { false };
};

#endif // hifi_SoundEntityItem_h
