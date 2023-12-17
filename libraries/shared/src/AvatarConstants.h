//
//  AvatarConstants.h
//  libraries/shared/src
//
//  Created by Anthony J. Thibault on Aug 16th 2017
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_AvatarConstants_h
#define hifi_AvatarConstants_h

#include "GLMHelpers.h"

// 50th Percentile Man
const float DEFAULT_AVATAR_HEIGHT = 1.755f; // meters
const float DEFAULT_AVATAR_EYE_TO_TOP_OF_HEAD = 0.11f; // meters
const float DEFAULT_AVATAR_NECK_TO_TOP_OF_HEAD = 0.185f; // meters
const float DEFAULT_AVATAR_NECK_HEIGHT = DEFAULT_AVATAR_HEIGHT - DEFAULT_AVATAR_NECK_TO_TOP_OF_HEAD;
const float DEFAULT_AVATAR_EYE_HEIGHT = DEFAULT_AVATAR_HEIGHT - DEFAULT_AVATAR_EYE_TO_TOP_OF_HEAD;
const float DEFAULT_AVATAR_HIPS_HEIGHT = 1.01327407f;  // meters
const float DEFAULT_SPINE2_SPLINE_PROPORTION = 0.71f;
const float DEFAULT_AVATAR_SUPPORT_BASE_LEFT  = -0.25f;
const float DEFAULT_AVATAR_SUPPORT_BASE_RIGHT =  0.25f;
const float DEFAULT_AVATAR_SUPPORT_BASE_FRONT = -0.20f;
const float DEFAULT_AVATAR_SUPPORT_BASE_BACK  =  0.12f;
const float DEFAULT_AVATAR_LATERAL_STEPPING_THRESHOLD = 0.10f;
const float DEFAULT_AVATAR_ANTERIOR_STEPPING_THRESHOLD = 0.04f;
const float DEFAULT_AVATAR_POSTERIOR_STEPPING_THRESHOLD = 0.05f;
const float DEFAULT_AVATAR_HEAD_ANGULAR_VELOCITY_STEPPING_THRESHOLD = 0.3f;
const float DEFAULT_AVATAR_MODE_HEIGHT_STEPPING_THRESHOLD = -0.02f;
const float DEFAULT_HANDS_VELOCITY_DIRECTION_STEPPING_THRESHOLD = 0.4f;
const float DEFAULT_HANDS_ANGULAR_VELOCITY_STEPPING_THRESHOLD = 3.3f;
const float DEFAULT_HEAD_VELOCITY_STEPPING_THRESHOLD = 0.18f;
const float DEFAULT_HEAD_PITCH_STEPPING_TOLERANCE = 7.0f;
const float DEFAULT_HEAD_ROLL_STEPPING_TOLERANCE = 7.0f;
const float DEFAULT_AVATAR_SPINE_STRETCH_LIMIT = 0.04f;
const float DEFAULT_AVATAR_FORWARD_DAMPENING_FACTOR = 0.5f;
const float DEFAULT_AVATAR_LATERAL_DAMPENING_FACTOR = 2.0f;
const float DEFAULT_AVATAR_HIPS_MASS = 40.0f;
const float DEFAULT_AVATAR_HEAD_MASS = 20.0f;
const float DEFAULT_AVATAR_LEFTHAND_MASS = 2.0f;
const float DEFAULT_AVATAR_RIGHTHAND_MASS = 2.0f;
const float DEFAULT_AVATAR_IPD = 0.064f;

// Used when avatar is missing joints... (avatar space)
const glm::quat DEFAULT_AVATAR_MIDDLE_EYE_ROT { Quaternions::Y_180 };
const glm::vec3 DEFAULT_AVATAR_HEAD_TO_MIDDLE_EYE_OFFSET = { 0.0f, 0.064f, 0.084f };
const glm::vec3 DEFAULT_AVATAR_HEAD_POS { 0.0f, 0.53f, 0.0f };
const glm::quat DEFAULT_AVATAR_HEAD_ROT { Quaternions::Y_180 };
const glm::vec3 DEFAULT_AVATAR_RIGHTARM_POS { -0.134824f, 0.396348f, -0.0515777f };
const glm::quat DEFAULT_AVATAR_RIGHTARM_ROT { -0.536241f, 0.536241f, -0.460918f, -0.460918f };
const glm::vec3 DEFAULT_AVATAR_LEFTARM_POS { 0.134795f, 0.396349f, -0.0515881f };
const glm::quat DEFAULT_AVATAR_LEFTARM_ROT { 0.536257f, 0.536258f, -0.460899f, 0.4609f };
const glm::vec3 DEFAULT_AVATAR_RIGHTHAND_POS { -0.72768f, 0.396349f, -0.0515779f };
const glm::quat DEFAULT_AVATAR_RIGHTHAND_ROT { 0.479184f, -0.520013f, 0.522537f, 0.476365f};
const glm::vec3 DEFAULT_AVATAR_LEFTHAND_POS { 0.727588f, 0.39635f, -0.0515878f };
const glm::quat DEFAULT_AVATAR_LEFTHAND_ROT { -0.479181f, -0.52001f, 0.52254f, -0.476369f };
const glm::vec3 DEFAULT_AVATAR_NECK_POS { 0.0f, 0.445f, 0.025f };
const glm::vec3 DEFAULT_AVATAR_SPINE2_POS { 0.0f, 0.32f, 0.02f };
const glm::quat DEFAULT_AVATAR_SPINE2_ROT { Quaternions::Y_180 };
const glm::vec3 DEFAULT_AVATAR_HIPS_POS { 0.0f, 0.0f, 0.0f };
const glm::quat DEFAULT_AVATAR_HIPS_ROT { Quaternions::Y_180 };
const glm::vec3 DEFAULT_AVATAR_LEFTFOOT_POS { -0.08f, -0.96f, 0.029f};
const glm::quat DEFAULT_AVATAR_LEFTFOOT_ROT { -0.40167322754859924f, 0.9154590368270874f, -0.005437685176730156f, -0.023744143545627594f };
const glm::vec3 DEFAULT_AVATAR_RIGHTFOOT_POS { 0.08f, -0.96f, 0.029f };
const glm::quat DEFAULT_AVATAR_RIGHTFOOT_ROT { -0.4016716778278351f, 0.9154615998268127f, 0.0053307069465518f, 0.023696165531873703f };

const float DEFAULT_AVATAR_MAX_WALKING_SPEED = 2.6f; // meters / second
const float DEFAULT_AVATAR_MAX_WALKING_BACKWARD_SPEED = 2.2f; // meters / second
const float DEFAULT_AVATAR_MAX_FLYING_SPEED = 30.0f; // meters / second
const float DEFAULT_AVATAR_MAX_SPRINT_SPEED = 3.4f; // meters / second
const float DEFAULT_AVATAR_WALK_SPEED_THRESHOLD = 0.15f; // meters / second

const float ANALOG_AVATAR_MAX_WALKING_SPEED = 6.0f; // meters / second
const float ANALOG_AVATAR_MAX_WALKING_BACKWARD_SPEED = 2.2f; // meters / second
const float ANALOG_AVATAR_MAX_FLYING_SPEED = 30.0f; // meters / second
const float ANALOG_AVATAR_MAX_SPRINT_SPEED = 8.0f; // meters / second
const float ANALOG_AVATAR_GEAR_1 = 0.2f;    // meters / second
const float ANALOG_AVATAR_GEAR_2 = 0.4f;    // meters / second
const float ANALOG_AVATAR_GEAR_3 = 0.6f;    // meters / second
const float ANALOG_AVATAR_GEAR_4 = 0.8f;    // meters / second
const float ANALOG_AVATAR_GEAR_5 = 1.0f;    // meters / second

const float ANALOG_PLUS_AVATAR_MAX_WALKING_SPEED = 10.0f; // meters / second
const float ANALOG_PLUS_AVATAR_MAX_WALKING_BACKWARD_SPEED = 2.42f; // meters / second
const float ANALOG_PLUS_AVATAR_MAX_FLYING_SPEED = 30.0f; // meters / second
const float ANALOG_PLUS_AVATAR_MAX_SPRINT_SPEED = 20.0f; // meters / second

const float DEFAULT_AVATAR_GRAVITY = -9.8f; // meters / second^2 (world) (originally -5.0f)
const float DEFAULT_AVATAR_JUMP_SPEED = 5.0f; // meters / second (sensor) (originally 3.5f)
const float DEFAULT_AVATAR_JUMP_HEIGHT = (DEFAULT_AVATAR_JUMP_SPEED * DEFAULT_AVATAR_JUMP_SPEED) / (2.0f * -DEFAULT_AVATAR_GRAVITY); // meters (sensor)
const float DEFAULT_AVATAR_MIN_JUMP_HEIGHT = 0.25f; // meters (world)  // hack

const float DEFAULT_AVATAR_FALL_HEIGHT = 20.0f; // meters
const float DEFAULT_AVATAR_MIN_HOVER_HEIGHT = 0.35f; // meters, normally it was 2.5f
//const float DEFAULT_AVATAR_MIN_HOVER_HEIGHT = 2.5f; // meters, normally it was 2.5f

static const float MAX_AVATAR_SCALE = 1000.0f;
static const float MIN_AVATAR_SCALE = 0.005f;

static const float MAX_AVATAR_HEIGHT = 1000.0f * DEFAULT_AVATAR_HEIGHT; // meters
static const float MIN_AVATAR_HEIGHT = 0.005f * DEFAULT_AVATAR_HEIGHT; // meters
static const float MIN_AVATAR_RADIUS = 0.5f * MIN_AVATAR_HEIGHT;
static const float AVATAR_WALK_SPEED_SCALAR = 1.0f;
static const float AVATAR_DESKTOP_SPRINT_SPEED_SCALAR = 2.0f;
static const float AVATAR_HMD_SPRINT_SPEED_SCALAR = 2.0f;

enum AvatarTriggerReaction {
    AVATAR_REACTION_POSITIVE = 0,
    AVATAR_REACTION_NEGATIVE,
    NUM_AVATAR_TRIGGER_REACTIONS
};

enum AvatarBeginEndReaction {
    AVATAR_REACTION_RAISE_HAND,
    AVATAR_REACTION_APPLAUD,
    AVATAR_REACTION_POINT,
    NUM_AVATAR_BEGIN_END_REACTIONS
};

#endif // hifi_AvatarConstants_h
