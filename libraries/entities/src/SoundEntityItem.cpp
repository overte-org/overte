//
//  Created by HifiExperiments on 12/30/2023
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "SoundEntityItem.h"

#include <QtCore/QDebug>

#include "EntitiesLogging.h"
#include "EntityItemProperties.h"
#include "EntityTree.h"
#include "EntityTreeElement.h"

EntityItemPointer SoundEntityItem::factory(const EntityItemID& entityID, const EntityItemProperties& properties) {
    std::shared_ptr<SoundEntityItem> entity(new SoundEntityItem(entityID), [](SoundEntityItem* ptr) { ptr->deleteLater(); });
    entity->setProperties(properties);
    return entity;
}

// our non-pure virtual subclass for now...
SoundEntityItem::SoundEntityItem(const EntityItemID& entityItemID) : EntityItem(entityItemID) {
    _type = EntityTypes::Sound;
}

SoundEntityItem::~SoundEntityItem() {
    auto manager = DependencyManager::get<AudioInjectorManager>();
    if (manager && _injector) {
        manager->stop(_injector);
    }
}

EntityItemProperties SoundEntityItem::getProperties(const EntityPropertyFlags& desiredProperties, bool allowEmptyDesiredProperties) const {
    EntityItemProperties properties = EntityItem::getProperties(desiredProperties, allowEmptyDesiredProperties); // get the properties from our base class

    COPY_ENTITY_PROPERTY_TO_PROPERTIES(soundURL, getURL);
    COPY_ENTITY_PROPERTY_TO_PROPERTIES(volume, getVolume);
    COPY_ENTITY_PROPERTY_TO_PROPERTIES(timeOffset, getTimeOffset);
    COPY_ENTITY_PROPERTY_TO_PROPERTIES(pitch, getPitch);
    COPY_ENTITY_PROPERTY_TO_PROPERTIES(playing, getPlaying);
    COPY_ENTITY_PROPERTY_TO_PROPERTIES(loop, getLoop);
    COPY_ENTITY_PROPERTY_TO_PROPERTIES(positional, getPositional);
    COPY_ENTITY_PROPERTY_TO_PROPERTIES(localOnly, getLocalOnly);

    return properties;
}

bool SoundEntityItem::setSubClassProperties(const EntityItemProperties& properties) {
    bool somethingChanged = false;

    SET_ENTITY_PROPERTY_FROM_PROPERTIES(soundURL, setURL);
    SET_ENTITY_PROPERTY_FROM_PROPERTIES(volume, setVolume);
    SET_ENTITY_PROPERTY_FROM_PROPERTIES(timeOffset, setTimeOffset);
    SET_ENTITY_PROPERTY_FROM_PROPERTIES(pitch, setPitch);
    SET_ENTITY_PROPERTY_FROM_PROPERTIES(playing, setPlaying);
    SET_ENTITY_PROPERTY_FROM_PROPERTIES(loop, setLoop);
    SET_ENTITY_PROPERTY_FROM_PROPERTIES(positional, setPositional);
    SET_ENTITY_PROPERTY_FROM_PROPERTIES(localOnly, setLocalOnly);

    return somethingChanged;
}

int SoundEntityItem::readEntitySubclassDataFromBuffer(const unsigned char* data, int bytesLeftToRead,
                                                      ReadBitstreamToTreeParams& args,
                                                      EntityPropertyFlags& propertyFlags, bool overwriteLocalData,
                                                      bool& somethingChanged) {

    int bytesRead = 0;
    const unsigned char* dataAt = data;

    READ_ENTITY_PROPERTY(PROP_SOUND_URL, QString, setURL);
    READ_ENTITY_PROPERTY(PROP_SOUND_VOLUME, float, setVolume);
    READ_ENTITY_PROPERTY(PROP_SOUND_TIME_OFFSET, float, setTimeOffset);
    READ_ENTITY_PROPERTY(PROP_SOUND_PITCH, float, setPitch);
    READ_ENTITY_PROPERTY(PROP_SOUND_PLAYING, bool, setPlaying);
    READ_ENTITY_PROPERTY(PROP_SOUND_LOOP, bool, setLoop);
    READ_ENTITY_PROPERTY(PROP_SOUND_POSITIONAL, bool, setPositional);
    READ_ENTITY_PROPERTY(PROP_SOUND_LOCAL_ONLY, bool, setLocalOnly);

    return bytesRead;
}

EntityPropertyFlags SoundEntityItem::getEntityProperties(EncodeBitstreamParams& params) const {
    EntityPropertyFlags requestedProperties = EntityItem::getEntityProperties(params);

    requestedProperties += PROP_SOUND_URL;
    requestedProperties += PROP_SOUND_VOLUME;
    requestedProperties += PROP_SOUND_TIME_OFFSET;
    requestedProperties += PROP_SOUND_PITCH;
    requestedProperties += PROP_SOUND_PLAYING;
    requestedProperties += PROP_SOUND_LOOP;
    requestedProperties += PROP_SOUND_POSITIONAL;
    requestedProperties += PROP_SOUND_LOCAL_ONLY;

    return requestedProperties;
}

void SoundEntityItem::appendSubclassData(OctreePacketData* packetData, EncodeBitstreamParams& params,
                                         EntityTreeElementExtraEncodeDataPointer entityTreeElementExtraEncodeData,
                                         EntityPropertyFlags& requestedProperties,
                                         EntityPropertyFlags& propertyFlags,
                                         EntityPropertyFlags& propertiesDidntFit,
                                         int& propertyCount,
                                         OctreeElement::AppendState& appendState) const {

    bool successPropertyFits = true;

    APPEND_ENTITY_PROPERTY(PROP_SOUND_URL, getURL());
    APPEND_ENTITY_PROPERTY(PROP_SOUND_VOLUME, getVolume());
    APPEND_ENTITY_PROPERTY(PROP_SOUND_TIME_OFFSET, getTimeOffset());
    APPEND_ENTITY_PROPERTY(PROP_SOUND_PITCH, getPitch());
    APPEND_ENTITY_PROPERTY(PROP_SOUND_PLAYING, getPlaying());
    APPEND_ENTITY_PROPERTY(PROP_SOUND_LOOP, getLoop());
    APPEND_ENTITY_PROPERTY(PROP_SOUND_POSITIONAL, getPositional());
    APPEND_ENTITY_PROPERTY(PROP_SOUND_LOCAL_ONLY, getLocalOnly());
}

void SoundEntityItem::debugDump() const {
    quint64 now = usecTimestampNow();
    qCDebug(entities) << "SOUND EntityItem id:" << getEntityItemID() << "---------------------------------------------";
    qCDebug(entities) << "               name:" << _name;
    qCDebug(entities) << "                url:" << _url;
    qCDebug(entities) << "           position:" << debugTreeVector(getWorldPosition());
    qCDebug(entities) << "         dimensions:" << debugTreeVector(getScaledDimensions());
    qCDebug(entities) << "      getLastEdited:" << debugTime(getLastEdited(), now);
    qCDebug(entities) << "SOUND EntityItem Ptr:" << this;
}

bool SoundEntityItem::shouldCreateSound(const EntityTreePointer& tree) const {
    bool clientShouldMakeSound = _localOnly || isMyAvatarEntity() || tree->isServerlessMode();
    bool serverShouldMakeSound = !_localOnly;
    return (clientShouldMakeSound && tree->getIsClient()) || (serverShouldMakeSound && tree->isEntityServer());
}

void SoundEntityItem::update(const quint64& now) {
    const auto tree = getTree();
    if (tree) {
        _updateNeeded = false;

        withWriteLock([&] {
            if (shouldCreateSound(tree)) {
                _sound = DependencyManager::get<SoundCache>()->getSound(_url);
            }
        });

        withReadLock([&] {
            if (_sound) {
                if (_sound->isLoaded()) {
                    updateSound(true);
                } else {
                    connect(_sound.data(), &Resource::finished, this, [&] {
                        withReadLock([&] {
                            updateSound(true);
                        });
                    });
                }
            }
        });
    }
}

void SoundEntityItem::locationChanged(bool tellPhysics, bool tellChildren) {
    EntityItem::locationChanged(tellPhysics, tellChildren);
    withReadLock([&] {
        updateSound();
    });
}

void SoundEntityItem::dimensionsChanged() {
    EntityItem::dimensionsChanged();
    withReadLock([&] {
        updateSound();
    });
}

void SoundEntityItem::setURL(const QString& value) {
    withWriteLock([&] {
        if (value != _url) {
            _url = value;

            const auto tree = getTree();
            if (!tree) {
                _updateNeeded = true;
                return;
            }

            if (shouldCreateSound(tree)) {
                _sound = DependencyManager::get<SoundCache>()->getSound(_url);
            }

            if (_sound) {
                if (_sound->isLoaded()) {
                    updateSound(true);
                } else {
                    connect(_sound.data(), &Resource::finished, this, [&] { updateSound(true); });
                }
            }
        }
    });
}

QString SoundEntityItem::getURL() const {
    return resultWithReadLock<QString>([&] {
        return _url;
    });
}

void SoundEntityItem::setVolume(float value) {
    withWriteLock([&] {
        if (value != _volume) {
            _volume = value;
            updateSound();
        }
    });
}

float SoundEntityItem::getVolume() const {
    return resultWithReadLock<float>([&] {
        return _volume;
    });
}

void SoundEntityItem::setTimeOffset(float value) {
    withWriteLock([&] {
        if (value != _timeOffset) {
            _timeOffset = value;
            updateSound(true);
        }
    });
}

float SoundEntityItem::getTimeOffset() const {
    return resultWithReadLock<float>([&] {
        return _timeOffset;
    });
}

void SoundEntityItem::setPitch(float value) {
    withWriteLock([&] {
        if (value != _pitch) {
            _pitch = value;
            updateSound(true);
        }
    });
}

float SoundEntityItem::getPitch() const {
    return resultWithReadLock<float>([&] {
        return _pitch;
    });
}

void SoundEntityItem::setPlaying(bool value) {
    withWriteLock([&] {
        if (value != _playing) {
            _playing = value;
            updateSound();
        }
    });
}

bool SoundEntityItem::getPlaying() const {
    return resultWithReadLock<float>([&] {
        return _playing;
    });
}

void SoundEntityItem::setLoop(bool value) {
    withWriteLock([&] {
        if (value != _loop) {
            _loop = value;
            updateSound(true);
        }
    });
}

bool SoundEntityItem::getLoop() const {
    return resultWithReadLock<float>([&] {
        return _loop;
    });
}

void SoundEntityItem::setPositional(bool value) {
    withWriteLock([&] {
        if (value != _positional) {
            _positional = value;
            updateSound();
        }
    });
}

bool SoundEntityItem::getPositional() const {
    return resultWithReadLock<float>([&] {
        return _positional;
    });
}

void SoundEntityItem::setLocalOnly(bool value) {
    withWriteLock([&] {
        if (value != _localOnly) {
            _localOnly = value;

            const auto tree = getTree();
            if (!tree) {
                _updateNeeded = true;
                return;
            }

            if (shouldCreateSound(tree)) {
                _sound = DependencyManager::get<SoundCache>()->getSound(_url);
            } else {
                _sound = nullptr;

                if (_injector) {
                    DependencyManager::get<AudioInjectorManager>()->stop(_injector);
                }
                _injector = nullptr;
            }

            if (_sound) {
                if (_sound->isLoaded()) {
                    updateSound(true);
                } else {
                    connect(_sound.data(), &Resource::finished, this, [&] { updateSound(true); });
                }
            }
        }
    });
}

bool SoundEntityItem::getLocalOnly() const {
    return resultWithReadLock<float>([&] {
        return _localOnly;
    });
}

bool SoundEntityItem::restartSound() {
    if (!_sound) {
        return false;
    }

    AudioInjectorOptions options;
    const glm::quat orientation = getWorldOrientation();
    options.position = getWorldPosition() + orientation * (getScaledDimensions() * (ENTITY_ITEM_DEFAULT_REGISTRATION_POINT - getRegistrationPoint()));
    options.positionSet = _positional;
    options.volume = _volume;
    options.loop = _loop;
    options.orientation = orientation;
    options.localOnly = _localOnly || _sound->isAmbisonic(); // force localOnly when ambisonic
    options.secondOffset = _timeOffset;
    options.pitch = _pitch;

    // stereo option isn't set from script, this comes from sound metadata or filename
    options.stereo = _sound->isStereo();
    options.ambisonic = _sound->isAmbisonic();

    if (_injector) {
        DependencyManager::get<AudioInjectorManager>()->setOptionsAndRestart(_injector, options);
    } else {
        _injector = DependencyManager::get<AudioInjectorManager>()->playSound(_sound, options);
    }

    return true;
}

void SoundEntityItem::updateSound(bool restart) {
    if (!_sound) {
        return;
    }

    if (restart) {
        if (_injector) {
            DependencyManager::get<AudioInjectorManager>()->stop(_injector);
        }
        _injector = nullptr;
    }

    if (_playing) {
        restartSound();
    } else {
        if (_injector) {
            DependencyManager::get<AudioInjectorManager>()->stop(_injector);
        }
    }
}