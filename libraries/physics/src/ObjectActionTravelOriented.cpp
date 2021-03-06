//
//  ObjectActionTravelOriented.cpp
//  libraries/physics/src
//
//  Created by Seth Alves 2015-6-5
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ObjectActionTravelOriented.h"

#include <glm/gtc/quaternion.hpp>

#include <EntityItem.h>
#include <QVariantGLM.h>

#include "PhysicsLogging.h"

const uint16_t ObjectActionTravelOriented::actionVersion = 1;


ObjectActionTravelOriented::ObjectActionTravelOriented(const QUuid& id, EntityItemPointer ownerEntity) :
    ObjectAction(DYNAMIC_TYPE_TRAVEL_ORIENTED, id, ownerEntity) {
    #if WANT_DEBUG
    qCDebug(physics) << "ObjectActionTravelOriented::ObjectActionTravelOriented";
    #endif
}

ObjectActionTravelOriented::~ObjectActionTravelOriented() {
    #if WANT_DEBUG
    qCDebug(physics) << "ObjectActionTravelOriented::~ObjectActionTravelOriented";
    #endif
}

void ObjectActionTravelOriented::updateActionWorker(btScalar deltaTimeStep) {
    withReadLock([&] {
        auto ownerEntity = _ownerEntity.lock();
        if (!ownerEntity) {
            return;
        }
        void* physicsInfo = ownerEntity->getPhysicsInfo();
        if (!physicsInfo) {
            return;
        }
        ObjectMotionState* motionState = static_cast<ObjectMotionState*>(physicsInfo);
        btRigidBody* rigidBody = motionState->getRigidBody();
        if (!rigidBody) {
            qCDebug(physics) << "ObjectActionTravelOriented::updateActionWorker no rigidBody";
            return;
        }
        const float MAX_TIMESCALE = 600.0f; // 10 min is a long time
        if (_angularTimeScale > MAX_TIMESCALE) {
            return;
        }

        // find normalized velocity
        glm::vec3 velocity = bulletToGLM(rigidBody->getLinearVelocity());
        float speed = glm::length(velocity);
        const float TRAVEL_ORIENTED_TOO_SLOW = 0.001f; // meters / second
        if (speed < TRAVEL_ORIENTED_TOO_SLOW) {
            return;
        }
        glm::vec3 direction = glm::normalize(velocity);

        // find current angle of "forward"
        btQuaternion bodyRotation = rigidBody->getOrientation();
        glm::quat orientation = bulletToGLM(bodyRotation);
        glm::vec3 forwardInWorldFrame = glm::normalize(orientation * _forward);

        // find the rotation that would line up direction and forward
        glm::quat neededRotation = glm::rotation(forwardInWorldFrame, direction);
        glm::quat rotationalTarget = neededRotation * orientation;

        btVector3 targetAngularVelocity(0.0f, 0.0f, 0.0f);

        auto alignmentDot = bodyRotation.dot(glmToBullet(rotationalTarget));
        const float ALMOST_ONE = 0.99999f;
        if (glm::abs(alignmentDot) < ALMOST_ONE) {
            btQuaternion target = glmToBullet(rotationalTarget);
            if (alignmentDot < 0.0f) {
                target = -target;
            }
            // if dQ is the incremental rotation that gets an object from Q0 to Q1 then:
            //
            //      Q1 = dQ * Q0
            //
            // solving for dQ gives:
            //
            //      dQ = Q1 * Q0^
            btQuaternion deltaQ = target * bodyRotation.inverse();
            float speed = deltaQ.getAngle() / _angularTimeScale;
            targetAngularVelocity = speed * deltaQ.getAxis();
            if (speed > rigidBody->getAngularSleepingThreshold()) {
                rigidBody->activate();
            }
        }
        // this action is aggresively critically damped and defeats the current velocity
        rigidBody->setAngularVelocity(targetAngularVelocity);
    });
}

const float MIN_TIMESCALE = 0.1f;


bool ObjectActionTravelOriented::updateArguments(QVariantMap arguments) {
    glm::vec3 forward;
    float angularTimeScale;

    bool needUpdate = false;
    bool somethingChanged = ObjectDynamic::updateArguments(arguments);
    withReadLock([&]{
        bool ok = true;
        forward = EntityDynamicInterface::extractVec3Argument("travel oriented action", arguments, "forward", ok, true);
        if (!ok) {
            forward = _forward;
        }
        ok = true;
        angularTimeScale =
            EntityDynamicInterface::extractFloatArgument("travel oriented action", arguments, "angularTimeScale", ok, false);
        if (!ok) {
            angularTimeScale = _angularTimeScale;
        }

        if (somethingChanged ||
            forward != _forward ||
            angularTimeScale != _angularTimeScale) {
            // something changed
            needUpdate = true;
        }
    });

    if (needUpdate) {
        withWriteLock([&] {
            _forward = forward;
            _angularTimeScale = glm::max(MIN_TIMESCALE, glm::abs(angularTimeScale));
            _active = (_forward != glm::vec3());

            auto ownerEntity = _ownerEntity.lock();
            if (ownerEntity) {
                ownerEntity->setDynamicDataDirty(true);
                ownerEntity->setDynamicDataNeedsTransmit(true);
            }
        });
        activateBody();
    }

    return true;
}

/*@jsdoc
 * The <code>"travel-oriented"</code> {@link Entities.ActionType|ActionType} orients an entity to align with its direction of 
 * travel.
 * It has arguments in addition to the common {@link Entities.ActionArguments|ActionArguments}:
 *
 * @typedef {object} Entities.ActionArguments-TravelOriented
 * @property {Vec3} forward=0,0,0 - The axis of the entity to align with the entity's direction of travel.
 * @property {number} angularTimeScale=0.1 - Controls how long it takes for the entity's orientation to catch up with the 
 *     direction of travel. The value is the time for the action to catch up to 1/e = 0.368 of the target value, where the 
 *     action is applied using an exponential decay.
 */
QVariantMap ObjectActionTravelOriented::getArguments() {
    QVariantMap arguments = ObjectDynamic::getArguments();
    withReadLock([&] {
        arguments["forward"] = vec3ToQMap(_forward);
        arguments["angularTimeScale"] = _angularTimeScale;
    });
    return arguments;
}

QByteArray ObjectActionTravelOriented::serialize() const {
    QByteArray serializedActionArguments;
    QDataStream dataStream(&serializedActionArguments, QIODevice::WriteOnly);

    dataStream << DYNAMIC_TYPE_TRAVEL_ORIENTED;
    dataStream << getID();
    dataStream << ObjectActionTravelOriented::actionVersion;

    withReadLock([&] {
        dataStream << _forward;
        dataStream << _angularTimeScale;

        dataStream << localTimeToServerTime(_expires);
        dataStream << _tag;
    });

    return serializedActionArguments;
}

void ObjectActionTravelOriented::deserialize(QByteArray serializedArguments) {
    QDataStream dataStream(serializedArguments);

    EntityDynamicType type;
    dataStream >> type;
    assert(type == getType());

    QUuid id;
    dataStream >> id;
    assert(id == getID());

    uint16_t serializationVersion;
    dataStream >> serializationVersion;
    if (serializationVersion != ObjectActionTravelOriented::actionVersion) {
        assert(false);
        return;
    }

    withWriteLock([&] {
        dataStream >> _forward;
        dataStream >> _angularTimeScale;

        quint64 serverExpires;
        dataStream >> serverExpires;
        _expires = serverTimeToLocalTime(serverExpires);

        dataStream >> _tag;

        _active = (_forward != glm::vec3());
    });
}
