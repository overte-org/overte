//
//  Created by HifiExperiments on 6/19/25
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_FadeProperties_h
#define hifi_FadeProperties_h

#include "FadeTiming.h"

enum class TransitionType {
    ELEMENT_ENTER_DOMAIN = 0,
    ELEMENT_LEAVE_DOMAIN,
    BUBBLE_ISECT_OWNER,
    BUBBLE_ISECT_TRESPASSER,
    USER_ENTER_DOMAIN,
    USER_LEAVE_DOMAIN,
    AVATAR_CHANGE,

    TYPE_COUNT,
    NONE = TYPE_COUNT
};

struct FadeProperties {
    float duration { 1.0f };
    FadeTiming timing { FadeTiming::LINEAR };

    vec3 noiseSpeed { 0.0f };
    vec3 noiseSizeInv { 1.0f };
    float noiseLevel { 1.0f };
    vec3 baseSizeInv { 1.0f };
    float baseLevel { 0.0f };

    vec4 innerEdgeColor { 1.0f };
    vec4 outerEdgeColor { 1.0f };
    float edgeWidth { 1.0f };
    bool inverted { false };
};

#endif // hifi_FadeProperties_h
