//
//  PhysicsHelpers.h
//  libraries/shared/src
//
//  Created by Andrew Meadows 2015.01.27
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_PhysicsHelpers_h
#define hifi_PhysicsHelpers_h

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <QUuid>

// TODO: move everything in here to the physics library after the physics/entities library
// dependency order is swapped.

const int32_t PHYSICS_ENGINE_MAX_NUM_SUBSTEPS = 6; // Bullet will start to "lose time" at 10 FPS.
const uint32_t NUM_SUBSTEPS_PER_SECOND = 90;
const float PHYSICS_ENGINE_FIXED_SUBSTEP = 1.0f / (float)NUM_SUBSTEPS_PER_SECOND;

const float DYNAMIC_LINEAR_SPEED_THRESHOLD = 0.05f;  // 5 cm/sec
const float DYNAMIC_ANGULAR_SPEED_THRESHOLD = 0.087266f;  // ~5 deg/sec
const float KINEMATIC_LINEAR_SPEED_THRESHOLD = 0.001f;  // 1 mm/sec
const float KINEMATIC_ANGULAR_SPEED_THRESHOLD = 0.0004f;  // ~0.025 deg/sec

// return incremental rotation (Bullet-style) caused by angularVelocity over timeStep
glm::quat computeBulletRotationStep(const glm::vec3& angularVelocity, float timeStep);

namespace Physics {
    int32_t getDefaultCollisionMask(int32_t group);

    void setSessionUUID(const QUuid& sessionID);
    const QUuid& getSessionUUID();
};

#endif // hifi_PhysicsHelpers_h
