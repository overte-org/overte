//
//  EntityPseudoPropertyFlags.h
//  libraries/entities/src
//
//  Created by Thijs Wenker on 9/18/18.
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once

#ifndef hifi_EntityPseudoPropertyFlag_h
#define hifi_EntityPseudoPropertyFlag_h

#include <bitset>
#include <type_traits>

namespace EntityPseudoPropertyFlag {
    enum {
        None = 0,
        FlagsActive,
        ID,
        Type,
        Age,
        AgeAsText,
        LastEdited,
        BoundingBox,
        OriginalTextures,
        RenderInfo,
        ClientOnly,
        AvatarEntity,
        LocalEntity,
        FaceCamera,
        IsFacingAvatar,

        NumFlags
    };
}
typedef std::bitset<EntityPseudoPropertyFlag::NumFlags> EntityPseudoPropertyFlags;

#endif // hifi_EntityPseudoPropertyFlag_h
