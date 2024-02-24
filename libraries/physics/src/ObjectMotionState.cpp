//
//  ObjectMotionState.cpp
//  libraries/physics/src
//
//  Created by Andrew Meadows 2014.11.05
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ObjectMotionState.h"

#include <math.h>

#include "BulletUtil.h"
#include "PhysicsEngine.h"
#include "PhysicsHelpers.h"
#include "PhysicsLogging.h"

// these thresholds determine what updates (object-->body) will activate the physical object
const float ACTIVATION_POSITION_DELTA = 0.005f;
const float ACTIVATION_ALIGNMENT_DOT = 0.99990f;
const float ACTIVATION_LINEAR_VELOCITY_DELTA = 0.01f;
const float ACTIVATION_GRAVITY_DELTA = 0.1f;
const float ACTIVATION_ANGULAR_VELOCITY_DELTA = 0.03f;


// origin of physics simulation in world-frame
glm::vec3 _worldOffset(0.0f);

// static
void ObjectMotionState::setWorldOffset(const glm::vec3& offset) {
    _worldOffset = offset;
}

// static
const glm::vec3& ObjectMotionState::getWorldOffset() {
    return _worldOffset;
}

// static
uint32_t worldSimulationStep = 0;
void ObjectMotionState::setWorldSimulationStep(uint32_t step) {
    assert(step > worldSimulationStep);
    worldSimulationStep = step;
}

// static
uint32_t ObjectMotionState::getWorldSimulationStep() {
    return worldSimulationStep;
}

// static
ShapeManager* shapeManager = nullptr;
void ObjectMotionState::setShapeManager(ShapeManager* manager) {
    assert(manager);
    shapeManager = manager;
}

ShapeManager* ObjectMotionState::getShapeManager() {
    assert(shapeManager); // you must properly set shapeManager before calling getShapeManager()
    return shapeManager;
}

ObjectMotionState::ObjectMotionState(const btCollisionShape* shape) :
    _lastKinematicStep(worldSimulationStep)
{
    setShape(shape);
}

ObjectMotionState::~ObjectMotionState() {
    assert(!_body);
    setShape(nullptr);
}

void ObjectMotionState::setMass(float mass) {
    _density = 1.0f;
    if (_shape) {
        // we compute the density for the current shape's Aabb volume
        // and save that instead of the total mass
        btTransform transform;
        transform.setIdentity();
        btVector3 minCorner, maxCorner;
        _shape->getAabb(transform, minCorner, maxCorner);
        btVector3 diagonal = maxCorner - minCorner;
        float volume = diagonal.getX() * diagonal.getY() * diagonal.getZ();
        if (volume > EPSILON) {
            _density = fabsf(mass) / volume;
        }
    }
}

float ObjectMotionState::getMass() const {
    if (_shape && _shape->getShapeType() != TRIANGLE_MESH_SHAPE_PROXYTYPE) {
        // scale the density by the current Aabb volume to get mass
        btTransform transform;
        transform.setIdentity();
        btVector3 minCorner, maxCorner;
        _shape->getAabb(transform, minCorner, maxCorner);
        btVector3 diagonal = maxCorner - minCorner;
        float volume = diagonal.getX() * diagonal.getY() * diagonal.getZ();

        // cap the max mass for numerical stability
        const float MIN_OBJECT_MASS = 0.0f;
        const float MAX_OBJECT_DENSITY = 20000.0f; // kg/m^3 density of Tungsten
        const float MAX_OBJECT_VOLUME = 1.0e6f;
        const float MAX_OBJECT_MASS = MAX_OBJECT_DENSITY * MAX_OBJECT_VOLUME;
        return glm::clamp(_density * volume, MIN_OBJECT_MASS, MAX_OBJECT_MASS);
    }
    return 0.0f;
}

void ObjectMotionState::setBodyLinearVelocity(const glm::vec3& velocity) const {
    _body->setLinearVelocity(glmToBullet(velocity));
}

void ObjectMotionState::setBodyAngularVelocity(const glm::vec3& velocity) const {
    _body->setAngularVelocity(glmToBullet(velocity));
}

void ObjectMotionState::setBodyGravity(const glm::vec3& gravity) const {
    _body->setGravity(glmToBullet(gravity));
}

glm::vec3 ObjectMotionState::getBodyLinearVelocity() const {
    return bulletToGLM(_body->getLinearVelocity());
}

glm::vec3 ObjectMotionState::getBodyLinearVelocityGTSigma() const {
    // NOTE: the threshold to use here relates to the linear displacement threshold (dX) for sending updates
    // to objects that are tracked server-side (e.g. entities which use dX = 2mm).  Hence an object moving
    // just under this velocity threshold would trigger an update about V/dX times per second.
    const float MIN_LINEAR_SPEED_SQUARED = 0.0036f; // 6 mm/sec

    glm::vec3 velocity = bulletToGLM(_body->getLinearVelocity());
    if (glm::length2(velocity) < MIN_LINEAR_SPEED_SQUARED) {
        velocity *= 0.0f;
    }
    return velocity;
}

glm::vec3 ObjectMotionState::getObjectLinearVelocityChange() const {
    return glm::vec3(0.0f);  // Subclasses override where meaningful.
}

glm::vec3 ObjectMotionState::getBodyAngularVelocity() const {
    return bulletToGLM(_body->getAngularVelocity());
}

void ObjectMotionState::setMotionType(PhysicsMotionType motionType) {
    _motionType = motionType;
}

// Update the Continuous Collision Detection (CCD) configuration settings of our RigidBody so that
// CCD will be enabled automatically when its speed surpasses a certain threshold.
void ObjectMotionState::updateCCDConfiguration() {
    assert(_body);
    if (_shape && _shape->getShapeType() != TRIANGLE_MESH_SHAPE_PROXYTYPE) {
        // find minumum dimension of shape
        btVector3 aabbMin, aabbMax;
        btTransform transform;
        transform.setIdentity();
        _shape->getAabb(transform, aabbMin, aabbMax);
        aabbMin = aabbMax - aabbMin;
        btScalar radius = *((btScalar*)(aabbMin) + aabbMin.minAxis());

        // use the minimum dimension as the radius of the CCD proxy sphere
        _body->setCcdSweptSphereRadius(radius);

        // also use the radius as the motion threshold for enabling CCD
        _body->setCcdMotionThreshold(radius);
    } else {
        // disable CCD
        _body->setCcdSweptSphereRadius(0.0f);
        _body->setCcdMotionThreshold(0.0f);
    }
}

void ObjectMotionState::setRigidBody(btRigidBody* body) {
    // give the body a (void*) back-pointer to this ObjectMotionState
    if (_body != body) {
        if (_body) {
            _body->setUserPointer(nullptr);
        }
        _body = body;
        if (_body) {
            _body->setUserPointer(this);
            assert(_body->getCollisionShape() == _shape);
            updateCCDConfiguration();
        }
    }
}

void ObjectMotionState::setShape(const btCollisionShape* shape) {
    if (_shape != shape) {
        if (_shape) {
            getShapeManager()->releaseShape(_shape);
        }
        _shape = shape;
        if (_body) {
            assert(_shape);
            _body->setCollisionShape(const_cast<btCollisionShape*>(_shape));
            updateCCDConfiguration();
        }
    } else if (shape) {
        // we need to release unused reference to shape
        getShapeManager()->releaseShape(shape);
    }
}

void ObjectMotionState::handleEasyChanges(uint32_t& flags) {
    assert(_body && _shape);
    if (flags & Simulation::DIRTY_POSITION) {
        btTransform worldTrans = _body->getWorldTransform();
        btVector3 newPosition = glmToBullet(getObjectPosition());
        float delta = (newPosition - worldTrans.getOrigin()).length();
        if (delta > ACTIVATION_POSITION_DELTA) {
            flags |= Simulation::DIRTY_PHYSICS_ACTIVATION;
        }
        worldTrans.setOrigin(newPosition);

        if (flags & Simulation::DIRTY_ROTATION) {
            btQuaternion newRotation = glmToBullet(getObjectRotation());
            float alignmentDot = fabsf(worldTrans.getRotation().dot(newRotation));
            if (alignmentDot < ACTIVATION_ALIGNMENT_DOT) {
                flags |= Simulation::DIRTY_PHYSICS_ACTIVATION;
            }
            worldTrans.setRotation(newRotation);
        }
        _body->setWorldTransform(worldTrans);
        if (!(flags & HARD_DIRTY_PHYSICS_FLAGS) && _body->isStaticObject()) {
            // force activate static body so its Aabb is updated later
            _body->activate(true);
        }
    } else if (flags & Simulation::DIRTY_ROTATION) {
        btTransform worldTrans = _body->getWorldTransform();
        btQuaternion newRotation = glmToBullet(getObjectRotation());
        float alignmentDot = fabsf(worldTrans.getRotation().dot(newRotation));
        if (alignmentDot < ACTIVATION_ALIGNMENT_DOT) {
            flags |= Simulation::DIRTY_PHYSICS_ACTIVATION;
        }
        worldTrans.setRotation(newRotation);
        _body->setWorldTransform(worldTrans);
        if (!(flags & HARD_DIRTY_PHYSICS_FLAGS) && _body->isStaticObject()) {
            // force activate static body so its Aabb is updated later
            _body->activate(true);
        }
    }

    if (_body && _body->getCollisionShape()->getShapeType() != TRIANGLE_MESH_SHAPE_PROXYTYPE) {
        if (flags & Simulation::DIRTY_LINEAR_VELOCITY) {
            btVector3 newLinearVelocity = glmToBullet(getObjectLinearVelocity());
            if (!(flags & Simulation::DIRTY_PHYSICS_ACTIVATION)) {
                float delta = (newLinearVelocity - _body->getLinearVelocity()).length();
                if (delta > ACTIVATION_LINEAR_VELOCITY_DELTA) {
                    flags |= Simulation::DIRTY_PHYSICS_ACTIVATION;
                }
            }
            _body->setLinearVelocity(newLinearVelocity);

            btVector3 newGravity = glmToBullet(getObjectGravity());
            if (!(flags & Simulation::DIRTY_PHYSICS_ACTIVATION)) {
                float delta = (newGravity - _body->getGravity()).length();
                if (delta > ACTIVATION_GRAVITY_DELTA ||
                        (delta > 0.0f && _body->getGravity().length2() == 0.0f)) {
                    flags |= Simulation::DIRTY_PHYSICS_ACTIVATION;
                }
            }
            _body->setGravity(newGravity);
        }
        if (flags & Simulation::DIRTY_ANGULAR_VELOCITY) {
            btVector3 newAngularVelocity = glmToBullet(getObjectAngularVelocity());
            if (!(flags & Simulation::DIRTY_PHYSICS_ACTIVATION)) {
                float delta = (newAngularVelocity - _body->getAngularVelocity()).length();
                if (delta > ACTIVATION_ANGULAR_VELOCITY_DELTA) {
                    flags |= Simulation::DIRTY_PHYSICS_ACTIVATION;
                }
            }
            _body->setAngularVelocity(newAngularVelocity);
        }
    }

    if (flags & Simulation::DIRTY_MATERIAL) {
        updateBodyMaterialProperties();
    }
}

void ObjectMotionState::updateBodyMaterialProperties() {
    _body->setRestitution(getObjectRestitution());
    _body->setFriction(getObjectFriction());
    _body->setDamping(fabsf(btMin(getObjectLinearDamping(), 1.0f)), fabsf(btMin(getObjectAngularDamping(), 1.0f)));
}

void ObjectMotionState::updateBodyVelocities() {
    setBodyLinearVelocity(getObjectLinearVelocity());
    setBodyAngularVelocity(getObjectAngularVelocity());
    setBodyGravity(getObjectGravity());
    _body->setActivationState(ACTIVE_TAG);
}

void ObjectMotionState::updateLastKinematicStep() {
    // NOTE: we init to worldSimulationStep - 1 so that: when any object transitions to kinematic
    // it will compute a non-zero dt on its first step.
    _lastKinematicStep = ObjectMotionState::getWorldSimulationStep() - 1;
}

void ObjectMotionState::updateBodyMassProperties() {
    btScalar mass = getMass();
    btVector3 inertia(1.0f, 1.0f, 1.0f);
    if (mass > 0.0f) {
        _body->getCollisionShape()->calculateLocalInertia(mass, inertia);
    }
    _body->setMassProps(mass, inertia);
    _body->updateInertiaTensor();
}

void ObjectMotionState::saveKinematicState(btScalar timeStep) {
    _body->saveKinematicState(timeStep);
}

