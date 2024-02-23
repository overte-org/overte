//
//  DetailedMotionState.cpp
//  interface/src/avatar/
//
//  Created by Luis Cuenca 1/11/2019
//  Copyright 2019 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "DetailedMotionState.h"

#include <PhysicsCollisionGroups.h>
#include <PhysicsEngine.h>
#include <PhysicsHelpers.h>
#include "MyAvatar.h"


DetailedMotionState::DetailedMotionState(AvatarPointer avatar, const btCollisionShape* shape, int32_t jointIndex) : 
    ObjectMotionState(shape), _avatar(avatar), _jointIndex(jointIndex) {
    assert(_avatar);
    if (!_avatar->isMyAvatar()) {
        _otherAvatar = std::static_pointer_cast<OtherAvatar>(_avatar);
    }
    _type = MOTIONSTATE_TYPE_DETAILED;
}

void DetailedMotionState::handleEasyChanges(uint32_t& flags) {
    ObjectMotionState::handleEasyChanges(flags);
    if (flags & Simulation::DIRTY_PHYSICS_ACTIVATION && !_body->isActive()) {
        _body->activate();
    }

    if (flags & Simulation::DIRTY_MASS) {
        updateBodyMassProperties();
    }
}

DetailedMotionState::~DetailedMotionState() {
    assert(_avatar);
    _avatar = nullptr;
}

// virtual
uint32_t DetailedMotionState::getIncomingDirtyFlags() const {
    return _body ? _dirtyFlags : 0;
}

void DetailedMotionState::clearIncomingDirtyFlags(uint32_t mask) {
    if (_body) {
        _dirtyFlags &= ~mask;
    }
}

PhysicsMotionType DetailedMotionState::computePhysicsMotionType() const {
    return MOTION_TYPE_KINEMATIC;
}

// virtual
bool DetailedMotionState::isMoving() const {
    return false;
}

// virtual
void DetailedMotionState::getWorldTransform(btTransform& worldTrans) const {
    worldTrans.setOrigin(glmToBullet(getObjectPosition()));
    worldTrans.setRotation(glmToBullet(getObjectRotation()));
}

// virtual
void DetailedMotionState::setWorldTransform(const btTransform& worldTrans) {
    _body->setWorldTransform(worldTrans);
}

// These pure virtual methods must be implemented for each MotionState type
// and make it possible to implement more complicated methods in this base class.

// virtual
float DetailedMotionState::getObjectRestitution() const {
    return 0.5f;
}

// virtual
float DetailedMotionState::getObjectFriction() const {
    return 0.5f;
}

// virtual
float DetailedMotionState::getObjectLinearDamping() const {
    return 0.5f;
}

// virtual
float DetailedMotionState::getObjectAngularDamping() const {
    return 0.5f;
}

// virtual
glm::vec3 DetailedMotionState::getObjectPosition() const {
    if (_otherAvatar != nullptr) {
        auto bodyLOD = _otherAvatar->getBodyLOD();
        if (bodyLOD == OtherAvatar::BodyLOD::Sphere) {
            return _avatar->getFitBounds().calcCenter();
        }
    }
    return  _avatar->getJointPosition(_jointIndex);
}

// virtual
glm::quat DetailedMotionState::getObjectRotation() const {
    return _avatar->getWorldOrientation() * _avatar->getAbsoluteJointRotationInObjectFrame(_jointIndex);
}

// virtual
glm::vec3 DetailedMotionState::getObjectLinearVelocity() const {
    return glm::vec3(0.0f);
}

// virtual
glm::vec3 DetailedMotionState::getObjectAngularVelocity() const {
    return glm::vec3(0.0f);
}

// virtual
glm::vec3 DetailedMotionState::getObjectGravity() const {
    return glm::vec3(0.0f);
}

// virtual
const QUuid DetailedMotionState::getObjectID() const {
    return _avatar->getSessionUUID();
}

QString DetailedMotionState::getName() const {
    return _avatar->getName() + "_" + _jointIndex;
}

// virtual
QUuid DetailedMotionState::getSimulatorID() const {
    return _avatar->getSessionUUID();
}

// virtual
void DetailedMotionState::computeCollisionGroupAndMask(int32_t& group, int32_t& mask) const {
    group = BULLET_COLLISION_GROUP_DETAILED_AVATAR;
    mask = Physics::getDefaultCollisionMask(group);
}

// virtual
float DetailedMotionState::getMass() const {
    return 0.0f;
}

void DetailedMotionState::setRigidBody(btRigidBody* body) {
    ObjectMotionState::setRigidBody(body);
    if (_body) {
        // remove angular dynamics from this body
        _body->setAngularFactor(0.0f);
    }
}

void DetailedMotionState::setShape(const btCollisionShape* shape) {
    if (_shape != shape) {
        if (_shape) {
            getShapeManager()->releaseShape(_shape);
        }
        _shape = shape;
        if (_body) {
            assert(_shape);
            _body->setCollisionShape(const_cast<btCollisionShape*>(_shape));
        }
    } else if (shape) {
        // we need to release unused reference to shape
        getShapeManager()->releaseShape(shape);
    }
}

void DetailedMotionState::forceActive() {
    if (_body && !_body->isActive()) {
        _body->setActivationState(ACTIVE_TAG);
    }
}
