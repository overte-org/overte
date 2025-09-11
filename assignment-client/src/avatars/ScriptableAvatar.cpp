//
//  ScriptableAvatar.cpp
//  assignment-client/src/avatars
//
//  Created by Clement on 7/22/14.
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "ScriptableAvatar.h"

#include <QDebug>
#include <QThread>
#include <glm/gtx/transform.hpp>

#include <shared/QtHelpers.h>
#include <AnimUtil.h>
#include <AvatarHashMap.h>
#include <ClientTraitsHandler.h>
#include <GLMHelpers.h>
#include <ResourceRequestObserver.h>
#include <AvatarLogging.h>
#include <EntityItem.h>
#include <EntityItemProperties.h>
#include <NetworkAccessManager.h>
#include <NetworkingConstants.h>


ScriptableAvatar::ScriptableAvatar() {
    _clientTraitsHandler.reset(new ClientTraitsHandler(this));
    static std::once_flag once;
    std::call_once(once, [] {
        qRegisterMetaType<HFMModel::Pointer>("HFMModel::Pointer");
    });
}

QByteArray ScriptableAvatar::toByteArrayStateful(AvatarDataDetail dataDetail, bool dropFaceTracking) {
    _globalPosition = getWorldPosition();
    return AvatarData::toByteArrayStateful(dataDetail, dropFaceTracking);
}


// hold and priority unused but kept so that client side JS can run.
void ScriptableAvatar::startAnimation(const QString& url, float fps, float priority,
                              bool loop, bool hold, float firstFrame, float lastFrame, const QStringList& maskedJoints) {
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, "startAnimation", Q_ARG(const QString&, url), Q_ARG(float, fps),
                                  Q_ARG(float, priority), Q_ARG(bool, loop), Q_ARG(bool, hold), Q_ARG(float, firstFrame),
                                  Q_ARG(float, lastFrame), Q_ARG(const QStringList&, maskedJoints));
        return;
    }
    _animation = DependencyManager::get<AnimationCache>()->getAnimation(url);
    _animationDetails = AnimationDetails("", QUrl(url), fps, 0, loop, hold, false, firstFrame, lastFrame, true, firstFrame, false);
    _maskedJoints = maskedJoints;
    _isAnimationRigValid = false;
}

void ScriptableAvatar::stopAnimation() {
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, "stopAnimation");
        return;
    }
    _animation.clear();
}

AnimationDetails ScriptableAvatar::getAnimationDetails() {
    if (QThread::currentThread() != thread()) {
        AnimationDetails result;
        BLOCKING_INVOKE_METHOD(this, "getAnimationDetails", 
                                  Q_RETURN_ARG(AnimationDetails, result));
        return result;
    }
    return _animationDetails;
}

int ScriptableAvatar::getJointIndex(const QString& name) const {
    // Faux joints:
    int result = AvatarData::getJointIndex(name);
    if (result != -1) {
        return result;
    }
    QReadLocker readLock(&_jointDataLock);
    return _fstJointIndices.value(name) - 1;
}

QStringList ScriptableAvatar::getJointNames() const {
    QReadLocker readLock(&_jointDataLock);
    return _fstJointNames;
    return QStringList();
}

void ScriptableAvatar::setSkeletonModelURL(const QUrl& skeletonModelURL) {
    _avatarAnimSkeleton.reset();
    _geometryResource.reset();

    AvatarData::setSkeletonModelURL(skeletonModelURL);
    updateJointMappings();
    _isRigValid = false;
}

int ScriptableAvatar::sendAvatarDataPacket(bool sendAll) {
    using namespace std::chrono;
    auto now = Clock::now();

    int MAX_DATA_RATE_MBPS = 3;
    int maxDataRateBytesPerSeconds = MAX_DATA_RATE_MBPS * BYTES_PER_KILOBYTE * KILO_PER_MEGA / BITS_IN_BYTE;
    int maxDataRateBytesPerMilliseconds = maxDataRateBytesPerSeconds / MSECS_PER_SECOND;

    auto bytesSent = 0;

    if (now > _nextTraitsSendWindow) {
        if (getIdentityDataChanged()) {
            bytesSent += sendIdentityPacket();
        }

        bytesSent += _clientTraitsHandler->sendChangedTraitsToMixer();

        // Compute the next send window based on how much data we sent and what
        // data rate we're trying to max at.
        milliseconds timeUntilNextSend { bytesSent / maxDataRateBytesPerMilliseconds };
        _nextTraitsSendWindow += timeUntilNextSend;

        // Don't let the next send window lag behind if we're not sending a lot of data.
        if (_nextTraitsSendWindow < now) {
            _nextTraitsSendWindow = now;
        }
    }

    bytesSent += AvatarData::sendAvatarDataPacket(sendAll);

    return bytesSent;
}

static AnimPose composeAnimPose(const HFMJoint& joint, const glm::quat rotation, const glm::vec3 translation) {
    glm::mat4 translationMat = glm::translate(translation);
    glm::mat4 rotationMat = glm::mat4_cast(joint.preRotation * rotation * joint.postRotation);
    glm::mat4 finalMat = translationMat * joint.preTransform * rotationMat * joint.postTransform;
    return AnimPose(finalMat);
}

void ScriptableAvatar::update(float deltatime) {
    if (!_geometryResource && !_skeletonModelFilenameURL.isEmpty()) { // AvatarData will parse the .fst, but not get the .fbx skeleton.
        _geometryResource = DependencyManager::get<ModelCache>()->getGeometryResource(_skeletonModelFilenameURL);
    }

    // Run animation
    Q_ASSERT(QThread::currentThread() == thread());
    if (_animation && _animation->isLoaded()) {
        Q_ASSERT(thread() == _animation->thread());
        auto frames = _animation->getFramesReference();
        if (frames.size() > 0 && _geometryResource && _geometryResource->isHFMModelLoaded()) {
            if (!_isRigValid) {
                _rig.reset(_geometryResource->getHFMModel());
                _isRigValid = true;
            }
            if (!_isAnimationRigValid) {
                _animationRig.reset(_animation->getHFMModel());
                _isAnimationRigValid = true;
            }
            if (!_avatarAnimSkeleton) {
                _avatarAnimSkeleton = std::make_shared<AnimSkeleton>(_geometryResource->getHFMModel());
            }
            float currentFrame = _animationDetails.currentFrame + deltatime * _animationDetails.fps;
            if (_animationDetails.loop || currentFrame < _animationDetails.lastFrame) {
                while (currentFrame >= _animationDetails.lastFrame) {
                    currentFrame -= (_animationDetails.lastFrame - _animationDetails.firstFrame);
                }
                _animationDetails.currentFrame = currentFrame;

                const QVector<HFMJoint>& modelJoints = _geometryResource->getHFMModel().joints;
                QStringList animationJointNames = _animation->getJointNames();

                const int nJoints = modelJoints.size();
                if (_jointData.size() != nJoints) {
                    _jointData.resize(nJoints);
                }

                const int frameCount = frames.size();
                const HFMAnimationFrame& floorFrame = frames.at((int)glm::floor(currentFrame) % frameCount);
                const HFMAnimationFrame& ceilFrame = frames.at((int)glm::ceil(currentFrame) % frameCount);
                const float frameFraction = glm::fract(currentFrame);
                std::vector<AnimPose> poses = _avatarAnimSkeleton->getRelativeDefaultPoses();

                // TODO: this needs more testing, it's possible that we need not only scale but also rotation and translation
                // According to tests with unmatching avatar and animation armatures, sometimes bones are not rotated correctly.
                // Matching armatures already work very well now.
                const float UNIT_SCALE = _animationRig.GetScaleFactorGeometryToUnscaledRig() / _rig.GetScaleFactorGeometryToUnscaledRig();

                for (int i = 0; i < animationJointNames.size(); i++) {
                    const QString& name = animationJointNames[i];
                    // As long as we need the model preRotations anyway, let's get the jointIndex from the bind skeleton rather than
                    // trusting the .fst (which is sometimes not updated to match changes to .fbx).
                    int mapping = _geometryResource->getHFMModel().getJointIndex(name);
                    if (mapping != -1 && !_maskedJoints.contains(name)) {
                        AnimPose floorPose = composeAnimPose(modelJoints[mapping], floorFrame.rotations[i],
                                                             floorFrame.translations[i] * UNIT_SCALE);
                        AnimPose ceilPose = composeAnimPose(modelJoints[mapping], ceilFrame.rotations[i],
                                                            ceilFrame.translations[i] * UNIT_SCALE);
                        blend(1, &floorPose, &ceilPose, frameFraction, &poses[mapping]);
                    }
                }

                std::vector<AnimPose> absPoses = poses;
                Q_ASSERT(_avatarAnimSkeleton != nullptr);
                _avatarAnimSkeleton->convertRelativePosesToAbsolute(absPoses);
                for (int i = 0; i < nJoints; i++) {
                    JointData& data = _jointData[i];
                    AnimPose& absPose = absPoses[i];
                    if (data.rotation != absPose.rot()) {
                        data.rotation = absPose.rot();
                        data.rotationIsDefaultPose = false;
                    }
                    AnimPose& relPose = poses[i];
                    if (data.translation != relPose.trans()) {
                        data.translation = relPose.trans();
                        data.translationIsDefaultPose = false;
                    }
                }

            } else {
                _animation.clear();
            }
        }
    }

    quint64 now = usecTimestampNow();
    quint64 dt = now - _lastSendAvatarDataTime;

    if (dt > MIN_TIME_BETWEEN_MY_AVATAR_DATA_SENDS) {
        sendAvatarDataPacket();
        _lastSendAvatarDataTime = now;
    }
}

void ScriptableAvatar::updateJointMappings() {
    {
        QWriteLocker writeLock(&_jointDataLock);
        _fstJointIndices.clear();
        _fstJointNames.clear();
        _jointData.clear();
    }

    if (_skeletonModelURL.fileName().toLower().endsWith(".fst")) {
        ////
        // TODO: Should we rely upon HTTPResourceRequest for ResourceRequestObserver instead?
        // HTTPResourceRequest::doSend() covers all of the following and
        // then some. It doesn't cover the connect() call, so we may
        // want to add a HTTPResourceRequest::doSend() method that does
        // connects.
        QNetworkAccessManager& networkAccessManager = NetworkAccessManager::getInstance();
        QNetworkRequest networkRequest = QNetworkRequest(_skeletonModelURL);
        networkRequest.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        networkRequest.setHeader(QNetworkRequest::UserAgentHeader, NetworkingConstants::OVERTE_USER_AGENT);
        DependencyManager::get<ResourceRequestObserver>()->update(
            _skeletonModelURL, -1, "AvatarData::updateJointMappings");
        QNetworkReply* networkReply = networkAccessManager.get(networkRequest);
        //
        ////
        connect(networkReply, &QNetworkReply::finished, this, &ScriptableAvatar::setJointMappingsFromNetworkReply);
    }
}

void ScriptableAvatar::setJointMappingsFromNetworkReply() {
    QNetworkReply* networkReply = static_cast<QNetworkReply*>(sender());
    // before we process this update, make sure that the skeleton model URL hasn't changed
    // since we made the FST request
    if (networkReply->url() != _skeletonModelURL) {
        qCDebug(avatars) << "Refusing to set joint mappings for FST URL that does not match the current URL";
        networkReply->deleteLater();
        return;
    }
    // TODO: this works only with .fst files currently, not directly with FBX and GLB models
    {
        QWriteLocker writeLock(&_jointDataLock);
        QByteArray line;
        while (!(line = networkReply->readLine()).isEmpty()) {
            line = line.trimmed();
            if (line.startsWith("filename")) {
                int filenameIndex = line.indexOf('=') + 1;
                if (filenameIndex > 0) {
                    _skeletonModelFilenameURL = _skeletonModelURL.resolved(QString(line.mid(filenameIndex).trimmed()));
                }
            }
            if (!line.startsWith("jointIndex")) {
                continue;
            }
            int jointNameIndex = line.indexOf('=') + 1;
            if (jointNameIndex == 0) {
                continue;
            }
            int secondSeparatorIndex = line.indexOf('=', jointNameIndex);
            if (secondSeparatorIndex == -1) {
                continue;
            }
            QString jointName = line.mid(jointNameIndex, secondSeparatorIndex - jointNameIndex).trimmed();
            bool ok;
            int jointIndex = line.mid(secondSeparatorIndex + 1).trimmed().toInt(&ok);
            if (ok) {
                while (_fstJointNames.size() < jointIndex + 1) {
                    _fstJointNames.append(QString());
                }
                _fstJointNames[jointIndex] = jointName;
            }
        }
        for (int i = 0; i < _fstJointNames.size(); i++) {
            _fstJointIndices.insert(_fstJointNames.at(i), i + 1);
        }
    }
    networkReply->deleteLater();
}

AvatarEntityMap ScriptableAvatar::getAvatarEntityData() const {
    auto data = getAvatarEntityDataInternal(true);
    return data;
}

AvatarEntityMap ScriptableAvatar::getAvatarEntityDataNonDefault() const {
    auto data = getAvatarEntityDataInternal(false);
    return data;

}

AvatarEntityMap ScriptableAvatar::getAvatarEntityDataInternal(bool allProperties) const {
    // DANGER: Now that we store the AvatarEntityData in packed format this call is potentially Very Expensive!
    // Avoid calling this method if possible.
    AvatarEntityMap data;
    QUuid sessionID = getID();
    _avatarEntitiesLock.withReadLock([&] {
        for (const auto& itr : _entities) {
            QUuid id = itr.first;
            EntityItemPointer entity = itr.second;

            EncodeBitstreamParams params;
            auto desiredProperties = entity->getEntityProperties(params);
            desiredProperties += PROP_LOCAL_POSITION;
            desiredProperties += PROP_LOCAL_ROTATION;
            desiredProperties += PROP_LOCAL_VELOCITY;
            desiredProperties += PROP_LOCAL_ANGULAR_VELOCITY;
            desiredProperties += PROP_LOCAL_DIMENSIONS;
            EntityItemProperties properties = entity->getProperties(desiredProperties);

            QByteArray blob;
            _helperScriptEngine.run( [&] {
                EntityItemProperties::propertiesToBlob(*_helperScriptEngine.get(), sessionID, properties, blob, allProperties);
            });
            data[id] = blob;
        }
    });
    return data;
}

void ScriptableAvatar::setAvatarEntityData(const AvatarEntityMap& avatarEntityData) {
    // Note: this is an invokable Script call
    // avatarEntityData is expected to be a map of QByteArrays that represent EntityItemProperties objects from JavaScript
    //
    if (avatarEntityData.size() > MAX_NUM_AVATAR_ENTITIES) {
        // the data is suspect
        qCDebug(avatars) << "discard suspect avatarEntityData with size =" << avatarEntityData.size();
        return;
    }

    // convert binary data to EntityItemProperties
    // NOTE: this operation is NOT efficient
    std::map<QUuid, EntityItemProperties> newProperties;
    AvatarEntityMap::const_iterator dataItr = avatarEntityData.begin();
    while (dataItr != avatarEntityData.end()) {
        EntityItemProperties properties;
        const QByteArray& blob = dataItr.value();
        if (!blob.isNull()) {
            _helperScriptEngine.run([&] {
                if (EntityItemProperties::blobToProperties(*_helperScriptEngine.get(), blob, properties)) {
                    newProperties[dataItr.key()] = properties;
                }
            });
        }
        ++dataItr;
    }

    // delete existing entities not found in avatarEntityData
    std::vector<QUuid> idsToClear;
    _avatarEntitiesLock.withWriteLock([&] {
        std::map<QUuid, EntityItemPointer>::iterator entityItr = _entities.begin();
        while (entityItr != _entities.end()) {
            QUuid id = entityItr->first;
            std::map<QUuid, EntityItemProperties>::const_iterator propertiesItr = newProperties.find(id);
            if (propertiesItr == newProperties.end()) {
                idsToClear.push_back(id);
                entityItr = _entities.erase(entityItr);
            } else {
                ++entityItr;
            }
        }
    });

    // add or update entities
    _avatarEntitiesLock.withWriteLock([&] {
        std::map<QUuid, EntityItemProperties>::const_iterator propertiesItr = newProperties.begin();
        while (propertiesItr != newProperties.end()) {
            QUuid id = propertiesItr->first;
            const EntityItemProperties& properties = propertiesItr->second;
            std::map<QUuid, EntityItemPointer>::iterator entityItr = _entities.find(id);
            EntityItemPointer entity;
            if (entityItr != _entities.end()) {
                entity = entityItr->second;
                entity->setProperties(properties);
            } else {
                entity = EntityTypes::constructEntityItem(id, properties);
            }
            if (entity) {
                // build outgoing payload
                OctreePacketData packetData(false, AvatarTraits::MAXIMUM_TRAIT_SIZE);
                EncodeBitstreamParams params;
                EntityTreeElementExtraEncodeDataPointer extra { nullptr };
                EntityPropertyList firstDidntFitProperty;
                OctreeElement::AppendState appendState = entity->appendEntityData(&packetData, params, extra, firstDidntFitProperty);

                if (appendState == OctreeElement::COMPLETED) {
                    _entities[id] = entity;
                    QByteArray tempArray((const char*)packetData.getUncompressedData(), packetData.getUncompressedSize());
                    storeAvatarEntityDataPayload(id, tempArray);
                } else {
                    // payload doesn't fit
                    entityItr = _entities.find(id);
                    if (entityItr != _entities.end()) {
                        _entities.erase(entityItr);
                        idsToClear.push_back(id);
                    }

                }
            }
            ++propertiesItr;
        }
    });

    // clear deleted traits
    for (const auto& id : idsToClear) {
        clearAvatarEntityInternal(id);
    }
}

void ScriptableAvatar::updateAvatarEntity(const QUuid& entityID, const QByteArray& entityData) {
    if (entityData.isNull()) {
        // interpret this as a DELETE
        std::map<QUuid, EntityItemPointer>::iterator itr = _entities.find(entityID);
        if (itr != _entities.end()) {
            _entities.erase(itr);
            clearAvatarEntityInternal(entityID);
        }
        return;
    }

    EntityItemPointer entity;
    EntityItemProperties properties;
    {
        // TODO: checking how often this happens and what is the performance impact of having the script engine on separate thread
        // If it's happening often, a method to move HelperScriptEngine into the current thread would be a good idea
        bool result = _helperScriptEngine.runWithResult<bool> ( [&]() {
            return EntityItemProperties::blobToProperties(*_helperScriptEngine.get(), entityData, properties);
        });
        if (!result) {
            // entityData is corrupt
            return;
        }
    }

    std::map<QUuid, EntityItemPointer>::iterator itr = _entities.find(entityID);
    if (itr == _entities.end()) {
        // this is an ADD
        entity = EntityTypes::constructEntityItem(entityID, properties);
        if (entity) {
            OctreePacketData packetData(false, AvatarTraits::MAXIMUM_TRAIT_SIZE);
            EncodeBitstreamParams params;
            EntityTreeElementExtraEncodeDataPointer extra { nullptr };
            EntityPropertyList firstDidntFitProperty;
            OctreeElement::AppendState appendState = entity->appendEntityData(&packetData, params, extra, firstDidntFitProperty);

            if (appendState == OctreeElement::COMPLETED) {
                _entities[entityID] = entity;
                QByteArray tempArray((const char*)packetData.getUncompressedData(), packetData.getUncompressedSize());
                storeAvatarEntityDataPayload(entityID, tempArray);
            }
        }
    } else {
        // this is an UPDATE
        entity = itr->second;
        bool somethingChanged = entity->setProperties(properties);
        if (somethingChanged) {
            OctreePacketData packetData(false, AvatarTraits::MAXIMUM_TRAIT_SIZE);
            EncodeBitstreamParams params;
            EntityTreeElementExtraEncodeDataPointer extra { nullptr };
            EntityPropertyList firstDidntFitProperty;
            OctreeElement::AppendState appendState = entity->appendEntityData(&packetData, params, extra, firstDidntFitProperty);

            if (appendState == OctreeElement::COMPLETED) {
                QByteArray tempArray((const char*)packetData.getUncompressedData(), packetData.getUncompressedSize());
                storeAvatarEntityDataPayload(entityID, tempArray);
            }
        }
    }
}
