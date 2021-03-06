//
//  ObjectActionOffset.cpp
//  libraries/physics/src
//
//  Created by Seth Alves 2015-6-17
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ObjectActionOffset.h"

#include "QVariantGLM.h"

#include "PhysicsLogging.h"


const uint16_t ObjectActionOffset::offsetVersion = 1;

ObjectActionOffset::ObjectActionOffset(const QUuid& id, EntityItemPointer ownerEntity) :
    ObjectAction(DYNAMIC_TYPE_OFFSET, id, ownerEntity),
    _pointToOffsetFrom(0.0f),
    _linearDistance(0.0f),
    _linearTimeScale(FLT_MAX),
    _positionalTargetSet(false) {
    #if WANT_DEBUG
    qCDebug(physics) << "ObjectActionOffset::ObjectActionOffset";
    #endif
}

ObjectActionOffset::~ObjectActionOffset() {
    #if WANT_DEBUG
    qCDebug(physics) << "ObjectActionOffset::~ObjectActionOffset";
    #endif
}

void ObjectActionOffset::updateActionWorker(btScalar deltaTimeStep) {
    withTryReadLock([&]{
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
            qCDebug(physics) << "ObjectActionOffset::updateActionWorker no rigidBody";
            return;
        }

        const float MAX_LINEAR_TIMESCALE = 600.0f;  // 10 minutes is a long time
        if (_positionalTargetSet && _linearTimeScale < MAX_LINEAR_TIMESCALE) {
            glm::vec3 objectPosition = bulletToGLM(rigidBody->getCenterOfMassPosition());
            glm::vec3 springAxis = objectPosition - _pointToOffsetFrom; // from anchor to object
            float distance = glm::length(springAxis);
            if (distance > FLT_EPSILON) {
                springAxis /= distance;  // normalize springAxis

                // compute (critically damped) target velocity of spring relaxation
                glm::vec3 offset = (distance - _linearDistance) * springAxis;
                glm::vec3 targetVelocity = (-1.0f / _linearTimeScale) * offset;

                // compute current velocity and its parallel component
                glm::vec3 currentVelocity = bulletToGLM(rigidBody->getLinearVelocity());
                glm::vec3 parallelVelocity = glm::dot(currentVelocity, springAxis) * springAxis;

                // we blend the parallel component with the spring's target velocity to get the new velocity
                float blend = deltaTimeStep / _linearTimeScale;
                if (blend > 1.0f) {
                    blend = 1.0f;
                }
                rigidBody->setLinearVelocity(glmToBullet(currentVelocity + blend * (targetVelocity - parallelVelocity)));
            }
        }
    });
}


bool ObjectActionOffset::updateArguments(QVariantMap arguments) {
    glm::vec3 pointToOffsetFrom;
    float linearTimeScale;
    float linearDistance;

    bool needUpdate = false;
    bool somethingChanged = ObjectDynamic::updateArguments(arguments);

    withReadLock([&]{
        bool ok = true;
        pointToOffsetFrom =
            EntityDynamicInterface::extractVec3Argument("offset action", arguments, "pointToOffsetFrom", ok, true);
        if (!ok) {
            pointToOffsetFrom = _pointToOffsetFrom;
        }

        ok = true;
        linearTimeScale =
            EntityDynamicInterface::extractFloatArgument("offset action", arguments, "linearTimeScale", ok, false);
        if (!ok) {
            linearTimeScale = _linearTimeScale;
        }

        ok = true;
        linearDistance =
            EntityDynamicInterface::extractFloatArgument("offset action", arguments, "linearDistance", ok, false);
        if (!ok) {
            linearDistance = _linearDistance;
        }

        // only change stuff if something actually changed
        if (somethingChanged ||
            _pointToOffsetFrom != pointToOffsetFrom ||
            _linearTimeScale != linearTimeScale ||
            _linearDistance != linearDistance) {
            needUpdate = true;
        }
    });


    if (needUpdate) {
        withWriteLock([&] {
            _pointToOffsetFrom = pointToOffsetFrom;
            _linearTimeScale = linearTimeScale;
            _linearDistance = linearDistance;
            _positionalTargetSet = true;
            _active = true;

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
 * The <code>"offset"</code> {@link Entities.ActionType|ActionType} moves an entity so that it is a defined distance away from 
 * a target point.
 * It has arguments in addition to the common {@link Entities.ActionArguments|ActionArguments}:
 *
 * @typedef {object} Entities.ActionArguments-Offset
 * @property {Vec3} pointToOffsetFrom=0,0,0 - The target point to offset the entity from.
 * @property {number} linearDistance=0 - The distance away from the target point to position the entity.
 * @property {number} linearTimeScale=34e+38 - Controls how long it takes for the entity's position to catch up with the
 *     target offset. The value is the time for the action to catch up to 1/e = 0.368 of the target value, where the action 
 *     is applied using an exponential decay.
 */
QVariantMap ObjectActionOffset::getArguments() {
    QVariantMap arguments = ObjectDynamic::getArguments();
    withReadLock([&] {
        arguments["pointToOffsetFrom"] = vec3ToQMap(_pointToOffsetFrom);
        arguments["linearTimeScale"] = _linearTimeScale;
        arguments["linearDistance"] = _linearDistance;
    });
    return arguments;
}

QByteArray ObjectActionOffset::serialize() const {
    QByteArray ba;
    QDataStream dataStream(&ba, QIODevice::WriteOnly);
    dataStream << DYNAMIC_TYPE_OFFSET;
    dataStream << getID();
    dataStream << ObjectActionOffset::offsetVersion;

    withReadLock([&] {
        dataStream << _pointToOffsetFrom;
        dataStream << _linearDistance;
        dataStream << _linearTimeScale;
        dataStream << _positionalTargetSet;
        dataStream << localTimeToServerTime(_expires);
        dataStream << _tag;
    });

    return ba;
}

void ObjectActionOffset::deserialize(QByteArray serializedArguments) {
    QDataStream dataStream(serializedArguments);

    EntityDynamicType type;
    dataStream >> type;
    assert(type == getType());

    QUuid id;
    dataStream >> id;
    assert(id == getID());

    uint16_t serializationVersion;
    dataStream >> serializationVersion;
    if (serializationVersion != ObjectActionOffset::offsetVersion) {
        return;
    }

    withWriteLock([&] {
        dataStream >> _pointToOffsetFrom;
        dataStream >> _linearDistance;
        dataStream >> _linearTimeScale;
        dataStream >> _positionalTargetSet;

        quint64 serverExpires;
        dataStream >> serverExpires;
        _expires = serverTimeToLocalTime(serverExpires);

        dataStream >> _tag;
        _active = true;
    });
}
