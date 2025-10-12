//
//  AvatarActionHold.cpp
//  interface/src/avatar/
//
//  Created by Seth Alves 2015-6-9
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "AvatarActionHold.h"

#include <QVariantGLM.h>

#include "avatar/AvatarManager.h"
#include "CharacterController.h"

const uint16_t AvatarActionHold::holdVersion = 1;
const int AvatarActionHold::velocitySmoothFrames = 6;


AvatarActionHold::AvatarActionHold(const QUuid& id, EntityItemPointer ownerEntity) :
    ObjectActionTractor(id, ownerEntity)
{
    _type = DYNAMIC_TYPE_HOLD;
    _measuredLinearVelocities.resize(AvatarActionHold::velocitySmoothFrames);

    auto myAvatar = DependencyManager::get<AvatarManager>()->getMyAvatar();
    if (myAvatar) {
        myAvatar->addHoldAction(this);
    }

    _positionalTargetSet = true;
    _rotationalTargetSet = true;

#if WANT_DEBUG
    qDebug() << "AvatarActionHold::AvatarActionHold" << (void*)this;
#endif
}

AvatarActionHold::~AvatarActionHold() {
    // Sometimes actions are destroyed after the AvatarManager is destroyed by the Application.
    auto avatarManager = DependencyManager::get<AvatarManager>();
    if (avatarManager) {
        auto myAvatar = avatarManager->getMyAvatar();
        if (myAvatar) {
            myAvatar->removeHoldAction(this);
        }
    }
    auto ownerEntity = _ownerEntity.lock();
    if (ownerEntity) {
        ownerEntity->setTransitingWithAvatar(false);
    }

#if WANT_DEBUG
    qDebug() << "AvatarActionHold::~AvatarActionHold" << (void*)this;
#endif
}

void AvatarActionHold::removeFromOwner() {
    auto avatarManager = DependencyManager::get<AvatarManager>();
    if (avatarManager) {
        auto myAvatar = avatarManager->getMyAvatar();
        if (myAvatar) {
            myAvatar->removeHoldAction(this);
        }
    }
}

bool AvatarActionHold::getAvatarRigidBodyLocation(glm::vec3& avatarRigidBodyPosition, glm::quat& avatarRigidBodyRotation) {
    auto myAvatar = DependencyManager::get<AvatarManager>()->getMyAvatar();
    MyCharacterController* controller = myAvatar ? myAvatar->getCharacterController() : nullptr;
    if (!controller) {
        qDebug() << "AvatarActionHold::getAvatarRigidBodyLocation failed to get character controller";
        return false;
    }
    controller->getRigidBodyLocation(avatarRigidBodyPosition, avatarRigidBodyRotation);
    return true;
}

void AvatarActionHold::prepareForPhysicsSimulation() {
    auto avatarManager = DependencyManager::get<AvatarManager>();
    auto holdingAvatar = std::static_pointer_cast<Avatar>(avatarManager->getAvatarBySessionID(_holderID));

    if (!holdingAvatar || !holdingAvatar->isMyAvatar()) {
        return;
    }

    withWriteLock([&]{
        glm::vec3 avatarRigidBodyPosition;
        glm::quat avatarRigidBodyRotation;
        getAvatarRigidBodyLocation(avatarRigidBodyPosition, avatarRigidBodyRotation);

        if (_ignoreIK) {
            return;
        }

        glm::vec3 palmPosition;
        glm::quat palmRotation;
        if (_hand == "right") {
            palmPosition = holdingAvatar->getUncachedRightPalmPosition();
            palmRotation = holdingAvatar->getUncachedRightPalmRotation();
        } else {
            palmPosition = holdingAvatar->getUncachedLeftPalmPosition();
            palmRotation = holdingAvatar->getUncachedLeftPalmRotation();
        }


        // determine the difference in translation and rotation between the avatar's
        // rigid body and the palm position.  The avatar's rigid body will be moved by bullet
        // between this call and the call to getTarget, below.  A call to get*PalmPosition in
        // getTarget would get the palm position of the previous location of the avatar (because
        // bullet has moved the av's rigid body but the rigid body's location has not yet been
        // copied out into the Avatar class.
        //glm::quat avatarRotationInverse = glm::inverse(avatarRigidBodyRotation);

        // the offset should be in the frame of the avatar, but something about the order
        // things are updated makes this wrong:
        //   _palmOffsetFromRigidBody = avatarRotationInverse * (palmPosition - avatarRigidBodyPosition);
        // I'll leave it here as a comment in case avatar handling changes.
        _palmOffsetFromRigidBody = palmPosition - avatarRigidBodyPosition;

        // rotation should also be needed, but again, the order of updates makes this unneeded.  leaving
        // code here for future reference.
        // _palmRotationFromRigidBody = avatarRotationInverse * palmRotation;
    });

    activateBody(true);
}

bool AvatarActionHold::getTarget(float deltaTimeStep, glm::quat& rotation, glm::vec3& position,
                                 glm::vec3& linearVelocity, glm::vec3& angularVelocity,
                                 float& linearTimeScale, float& angularTimeScale) {
    auto avatarManager = DependencyManager::get<AvatarManager>();
    auto holdingAvatar = std::static_pointer_cast<Avatar>(avatarManager->getAvatarBySessionID(_holderID));

    if (!holdingAvatar) {
        return false;;
    }

    withReadLock([&]{
        bool isRightHand = (_hand == "right");

        glm::vec3 palmPosition;
        glm::quat palmRotation;

        bool isTransitingWithAvatar = holdingAvatar->getTransit()->isActive();
        if (isTransitingWithAvatar != _isTransitingWithAvatar) {
            _isTransitingWithAvatar = isTransitingWithAvatar;
            auto ownerEntity = _ownerEntity.lock();
            if (ownerEntity) {
                ownerEntity->setTransitingWithAvatar(_isTransitingWithAvatar);
            }
        }

        if (holdingAvatar->isMyAvatar()) {
            std::shared_ptr<MyAvatar> myAvatar = avatarManager->getMyAvatar();

            // fetch the hand controller pose
            controller::Pose pose;
            if (isRightHand) {
                pose = myAvatar->getControllerPoseInWorldFrame(controller::Action::RIGHT_HAND);
            } else {
                pose = myAvatar->getControllerPoseInWorldFrame(controller::Action::LEFT_HAND);
            }

            if (pose.isValid()) {
                linearVelocity = pose.getVelocity();
                angularVelocity = pose.getAngularVelocity();
            }

            if (_ignoreIK && pose.isValid()) {

                // this position/rotation should be the same as the one in scripts/system/libraries/controllers.js
                // otherwise things will do a little hop when you grab them.

                // if (isRightHand) {
                //     pose = myAvatar->getRightHandControllerPoseInAvatarFrame();
                // } else {
                //     pose = myAvatar->getLeftHandControllerPoseInAvatarFrame();
                // }
                // glm::vec3 camRelPos = pose.getTranslation();
                // glm::quat camRelRot = pose.getRotation();

                int camRelIndex = isRightHand ?
                    CAMERA_RELATIVE_CONTROLLER_RIGHTHAND_INDEX :
                    CAMERA_RELATIVE_CONTROLLER_LEFTHAND_INDEX;
                glm::vec3 camRelPos = myAvatar->getAbsoluteJointTranslationInObjectFrame(camRelIndex);
                glm::quat camRelRot = myAvatar->getAbsoluteJointRotationInObjectFrame(camRelIndex);

                Transform avatarTransform;
                avatarTransform = myAvatar->getTransform();
                palmPosition = avatarTransform.transform(camRelPos);
                palmRotation = avatarTransform.getRotation() * camRelRot;
            } else {
                glm::vec3 avatarRigidBodyPosition;
                glm::quat avatarRigidBodyRotation;
                getAvatarRigidBodyLocation(avatarRigidBodyPosition, avatarRigidBodyRotation);

                // the offset and rotation between the avatar's rigid body and the palm were determined earlier
                // in prepareForPhysicsSimulation.  At this point, the avatar's rigid body has been moved by bullet
                // and the data in the Avatar class is stale.  This means that the result of get*PalmPosition will
                // be stale.  Instead, determine the current palm position with the current avatar's rigid body
                // location and the saved offsets.

                // this line is more correct but breaks for the current way avatar data is updated.
                // palmPosition = avatarRigidBodyPosition + avatarRigidBodyRotation * _palmOffsetFromRigidBody;
                // instead, use this for now:
                palmPosition = avatarRigidBodyPosition + _palmOffsetFromRigidBody;

                // the item jitters the least by getting the rotation based on the opinion of Avatar.h rather
                // than that of the rigid body.  leaving this next line here for future reference:
                // palmRotation = avatarRigidBodyRotation * _palmRotationFromRigidBody;

                if (isRightHand) {
                    palmRotation = holdingAvatar->getRightPalmRotation();
                } else {
                    palmRotation = holdingAvatar->getLeftPalmRotation();
                }
            }
        } else { // regular avatar
            if (isRightHand) {
                Transform controllerRightTransform = Transform(holdingAvatar->getControllerRightHandMatrix());
                Transform avatarTransform = holdingAvatar->getTransform();
                palmRotation = avatarTransform.getRotation() * controllerRightTransform.getRotation();
                palmPosition = avatarTransform.getTranslation() +
                    (avatarTransform.getRotation() * controllerRightTransform.getTranslation());
            } else {
                Transform controllerLeftTransform = Transform(holdingAvatar->getControllerLeftHandMatrix());
                Transform avatarTransform = holdingAvatar->getTransform();
                palmRotation = avatarTransform.getRotation() * controllerLeftTransform.getRotation();
                palmPosition = avatarTransform.getTranslation() +
                    (avatarTransform.getRotation() * controllerLeftTransform.getTranslation());
            }
        }

        rotation = palmRotation * _relativeRotation;
        position = palmPosition + palmRotation * _relativePosition;

        // update linearVelocity based on offset via _relativePosition;
        linearVelocity = linearVelocity + glm::cross(angularVelocity, position - palmPosition);

        linearTimeScale = _linearTimeScale;
        angularTimeScale = _angularTimeScale;
    });

    return true;
}

void AvatarActionHold::updateActionWorker(float deltaTimeStep) {
    if (_kinematic) {
        if (prepareForTractorUpdate(deltaTimeStep)) {
            doKinematicUpdate(deltaTimeStep);
        }
    } else {
        forceBodyNonStatic();
        ObjectActionTractor::updateActionWorker(deltaTimeStep);
    }
}

void AvatarActionHold::doKinematicUpdate(float deltaTimeStep) {
    auto ownerEntity = _ownerEntity.lock();
    if (!ownerEntity) {
        qDebug() << "AvatarActionHold::doKinematicUpdate -- no owning entity";
        return;
    }
    if (ownerEntity->getParentID() != QUuid()) {
        // if the held entity has been given a parent, stop acting on it.
        return;
    }

    void* physicsInfo = ownerEntity->getPhysicsInfo();
    if (!physicsInfo) {
        qDebug() << "AvatarActionHold::doKinematicUpdate -- no owning physics info";
        return;
    }
    ObjectMotionState* motionState = static_cast<ObjectMotionState*>(physicsInfo);
    btRigidBody* rigidBody = motionState ? motionState->getRigidBody() : nullptr;
    if (!rigidBody) {
        qDebug() << "AvatarActionHold::doKinematicUpdate -- no rigidBody";
        return;
    }

    withWriteLock([&]{
        if (_previousSet &&
            _positionalTarget != _previousPositionalTarget) { // don't average in a zero velocity if we get the same data
            glm::vec3 oneFrameVelocity = (_positionalTarget - _previousPositionalTarget) / deltaTimeStep;

            _measuredLinearVelocities[_measuredLinearVelocitiesIndex++] = oneFrameVelocity;
            _measuredLinearVelocitiesIndex %= AvatarActionHold::velocitySmoothFrames;
        }

        if (_kinematicSetVelocity) {
            glm::vec3 measuredLinearVelocity = _measuredLinearVelocities[0];
            for (int i = 1; i < AvatarActionHold::velocitySmoothFrames; i++) {
                // there is a bit of lag between when someone releases the trigger and when the software reacts to
                // the release.  we calculate the velocity from previous frames but we don't include several
                // of the most recent.
                //
                // if _measuredLinearVelocitiesIndex is
                //     0 -- ignore i of 3 4 5
                //     1 -- ignore i of 4 5 0
                //     2 -- ignore i of 5 0 1
                //     3 -- ignore i of 0 1 2
                //     4 -- ignore i of 1 2 3
                //     5 -- ignore i of 2 3 4

                // This code is now disabled, but I'm leaving it commented-out because I suspect it will come back.
                // if ((i + 1) % AvatarActionHold::velocitySmoothFrames == _measuredLinearVelocitiesIndex ||
                //     (i + 2) % AvatarActionHold::velocitySmoothFrames == _measuredLinearVelocitiesIndex ||
                //     (i + 3) % AvatarActionHold::velocitySmoothFrames == _measuredLinearVelocitiesIndex) {
                //     continue;
                // }

                measuredLinearVelocity += _measuredLinearVelocities[i];
            }
            measuredLinearVelocity /= (float)(AvatarActionHold::velocitySmoothFrames
                                          // - 3  // 3 because of the 3 we skipped, above
                                          );

            rigidBody->setLinearVelocity(glmToBullet(measuredLinearVelocity));
            rigidBody->setAngularVelocity(glmToBullet(_angularVelocityTarget));
        }

        btTransform worldTrans = rigidBody->getWorldTransform();
        worldTrans.setOrigin(glmToBullet(_positionalTarget));
        worldTrans.setRotation(glmToBullet(_rotationalTarget));
        rigidBody->setWorldTransform(worldTrans);

        motionState->dirtyInternalKinematicChanges();

        _previousPositionalTarget = _positionalTarget;
        _previousRotationalTarget = _rotationalTarget;
        _previousDeltaTimeStep = deltaTimeStep;
        _previousSet = true;
    });

    forceBodyNonStatic();
    activateBody(true);
}

bool AvatarActionHold::updateArguments(QVariantMap arguments) {
    glm::vec3 relativePosition;
    glm::quat relativeRotation;
    float timeScale;
    QString hand;
    QUuid holderID;
    bool kinematic;
    bool kinematicSetVelocity;
    bool ignoreIK;
    bool needUpdate = false;

    bool somethingChanged = ObjectDynamic::updateArguments(arguments);
    withReadLock([&]{
        bool ok = true;
        relativePosition = EntityDynamicInterface::extractVec3Argument("hold", arguments, "relativePosition", ok, false);
        if (!ok) {
            relativePosition = _relativePosition;
        }

        ok = true;
        relativeRotation = EntityDynamicInterface::extractQuatArgument("hold", arguments, "relativeRotation", ok, false);
        if (!ok) {
            relativeRotation = _relativeRotation;
        }

        ok = true;
        timeScale = EntityDynamicInterface::extractFloatArgument("hold", arguments, "timeScale", ok, false);
        if (!ok) {
            timeScale = _linearTimeScale;
        }

        ok = true;
        hand = EntityDynamicInterface::extractStringArgument("hold", arguments, "hand", ok, false);
        if (!ok || !(hand == "left" || hand == "right")) {
            hand = _hand;
        }

        ok = true;
        holderID = QUuid(EntityDynamicInterface::extractStringArgument("hold", arguments, "holderID", ok, false));
        if (!ok) {
            auto myAvatar = DependencyManager::get<AvatarManager>()->getMyAvatar();
            holderID = myAvatar->getSessionUUID();
        }

        ok = true;
        kinematic = EntityDynamicInterface::extractBooleanArgument("hold", arguments, "kinematic", ok, false);
        if (!ok) {
            kinematic = _kinematic;
        }

        ok = true;
        kinematicSetVelocity = EntityDynamicInterface::extractBooleanArgument("hold", arguments,
                                                                             "kinematicSetVelocity", ok, false);
        if (!ok) {
            kinematicSetVelocity = _kinematicSetVelocity;
        }

        ok = true;
        ignoreIK = EntityDynamicInterface::extractBooleanArgument("hold", arguments, "ignoreIK", ok, false);
        if (!ok) {
            ignoreIK = _ignoreIK;
        }

        if (somethingChanged ||
            relativePosition != _relativePosition ||
            relativeRotation != _relativeRotation ||
            timeScale != _linearTimeScale ||
            hand != _hand ||
            holderID != _holderID ||
            kinematic != _kinematic ||
            kinematicSetVelocity != _kinematicSetVelocity ||
            ignoreIK != _ignoreIK) {
            needUpdate = true;
        }
    });

    if (needUpdate) {
        withWriteLock([&] {
            _relativePosition = relativePosition;
            _relativeRotation = relativeRotation;
            const float MIN_TIMESCALE = 0.1f;
            _linearTimeScale = glm::max(MIN_TIMESCALE, timeScale);
            _angularTimeScale = _linearTimeScale;
            _hand = hand;
            _holderID = holderID;
            _kinematic = kinematic;
            _kinematicSetVelocity = kinematicSetVelocity;
            _ignoreIK = ignoreIK;
            _active = true;

            auto myAvatar = DependencyManager::get<AvatarManager>()->getMyAvatar();

            auto ownerEntity = _ownerEntity.lock();
            if (ownerEntity) {
                ownerEntity->setDynamicDataDirty(true);
                ownerEntity->setDynamicDataNeedsTransmit(true);
                ownerEntity->setTransitingWithAvatar(myAvatar->getTransit()->isActive());
            }
        });
    }

    return true;
}

/*@jsdoc
 * The <code>"hold"</code> {@link Entities.ActionType|ActionType} positions and rotates an entity relative to an avatar's hand. 
 * Collisions between the entity and the user's avatar are disabled during the hold.
 * It has arguments in addition to the common {@link Entities.ActionArguments|ActionArguments}:
 *
 * @typedef {object} Entities.ActionArguments-Hold
 * @property {Uuid} holderID=MyAvatar.sessionUUID - The ID of the avatar holding the entity.
 * @property {string} hand=right - The hand holding the entity: <code>"left"</code> or <code>"right"</code>.
 * @property {Vec3} relativePosition=0,0,0 - The target position relative to the avatar's hand.
 * @property {Vec3} relativeRotation=0,0,0,1 - The target rotation relative to the avatar's hand.
 * @property {number} timeScale=3.4e+38 - Controls how long it takes for the entity's position and rotation to catch up with 
 *     the target. The value is the time for the action to catch up to 1/e = 0.368 of the target value, where the action is 
 *     applied using an exponential decay.
 * @property {boolean} kinematic=false - <code>true</code> if the entity is made kinematic during the action; the entity won't 
 *     lag behind the hand but constraint actions such as <code>"hinge"</code> won't act properly. <code>false</code> if the 
 *     entity is not made kinematic during the action
 * @property {boolean} kinematicSetVelocity=false - <code>true</code> if, when <code>kinematic</code> is <code>true</code>, the 
 *     entity's velocity will be set during the action, e.g., so that other scripts may use the value. <code>false</code> if 
 *     the entity's velocity will not be set during the action.
 * @property {boolean} ignoreIK=false - <code>true</code> if the entity follows the HMD controller, <code>false</code> if it 
 *     follows the avatar's hand.
 */
QVariantMap AvatarActionHold::getArguments() {
    QVariantMap arguments = ObjectDynamic::getArguments();
    withReadLock([&]{
        arguments["holderID"] = _holderID;
        arguments["relativePosition"] = vec3ToQMap(_relativePosition);
        arguments["relativeRotation"] = quatToQMap(_relativeRotation);
        arguments["timeScale"] = _linearTimeScale;
        arguments["hand"] = _hand;
        arguments["kinematic"] = _kinematic;
        arguments["kinematicSetVelocity"] = _kinematicSetVelocity;
        arguments["ignoreIK"] = _ignoreIK;
    });
    return arguments;
}

QByteArray AvatarActionHold::serialize() const {
    QByteArray serializedActionArguments;
    QDataStream dataStream(&serializedActionArguments, QIODevice::WriteOnly);

    withReadLock([&]{
        dataStream << DYNAMIC_TYPE_HOLD;
        dataStream << getID();
        dataStream << AvatarActionHold::holdVersion;

        dataStream << _holderID;
        dataStream << _relativePosition;
        dataStream << _relativeRotation;
        dataStream << _linearTimeScale;
        dataStream << _hand;

        dataStream << localTimeToServerTime(_expires);
        dataStream << _tag;
        dataStream << _kinematic;
        dataStream << _kinematicSetVelocity;
    });

    return serializedActionArguments;
}

void AvatarActionHold::deserialize(QByteArray serializedArguments) {
    QDataStream dataStream(serializedArguments);

    EntityDynamicType type;
    dataStream >> type;
    assert(type == getType());

    QUuid id;
    dataStream >> id;
    assert(id == getID());

    uint16_t serializationVersion;
    dataStream >> serializationVersion;
    if (serializationVersion != AvatarActionHold::holdVersion) {
        return;
    }

    withWriteLock([&]{
        dataStream >> _holderID;
        dataStream >> _relativePosition;
        dataStream >> _relativeRotation;
        dataStream >> _linearTimeScale;
        _angularTimeScale = _linearTimeScale;
        dataStream >> _hand;

        quint64 serverExpires;
        dataStream >> serverExpires;
        _expires = serverTimeToLocalTime(serverExpires);

        dataStream >> _tag;
        dataStream >> _kinematic;
        dataStream >> _kinematicSetVelocity;

        #if WANT_DEBUG
        qDebug() << "deserialize AvatarActionHold: " << _holderID
                 << _relativePosition.x << _relativePosition.y << _relativePosition.z
                 << _hand << _expires;
        #endif

        _active = true;
    });

    forceBodyNonStatic();
}

void AvatarActionHold::lateAvatarUpdate(const AnimPose& prePhysicsRoomPose, const AnimPose& postAvatarUpdateRoomPose) {
    auto ownerEntity = _ownerEntity.lock();
    if (!ownerEntity) {
        return;
    }
    void* physicsInfo = ownerEntity->getPhysicsInfo();
    if (!physicsInfo) {
        return;
    }
    ObjectMotionState* motionState = static_cast<ObjectMotionState*>(physicsInfo);
    btRigidBody* rigidBody = motionState ? motionState->getRigidBody() : nullptr;
    if (!rigidBody) {
        return;
    }
    auto avatarManager = DependencyManager::get<AvatarManager>();
    auto holdingAvatar = std::static_pointer_cast<Avatar>(avatarManager->getAvatarBySessionID(_holderID));
    if (!holdingAvatar || !holdingAvatar->isMyAvatar()) {
        return;
    }

    btTransform worldTrans = rigidBody->getWorldTransform();
    AnimPose worldBodyPose(glm::vec3(1), bulletToGLM(worldTrans.getRotation()), bulletToGLM(worldTrans.getOrigin()));

    // transform the body transform into sensor space with the prePhysics sensor-to-world matrix.
    // then transform it back into world uisng the postAvatarUpdate sensor-to-world matrix.
    AnimPose newWorldBodyPose = postAvatarUpdateRoomPose * prePhysicsRoomPose.inverse() * worldBodyPose;

    worldTrans.setOrigin(glmToBullet(newWorldBodyPose.trans()));
    worldTrans.setRotation(glmToBullet(newWorldBodyPose.rot()));
    rigidBody->setWorldTransform(worldTrans);

    bool positionSuccess;
    ownerEntity->setWorldPosition(bulletToGLM(worldTrans.getOrigin()) + ObjectMotionState::getWorldOffset(), positionSuccess, false);
    bool orientationSuccess;
    ownerEntity->setWorldOrientation(bulletToGLM(worldTrans.getRotation()), orientationSuccess, false);
}
