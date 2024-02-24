//
//  RenderFetchCullSortTask.h
//  render/src/
//
//  Created by Zach Pomerantz on 12/22/2016.
//  Copyright 2016 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_RenderFetchCullSortTask_h
#define hifi_RenderFetchCullSortTask_h

#include <gpu/Pipeline.h>
#include "CullTask.h"

class RenderFetchCullSortTask {
public:

    enum Buckets {
        OPAQUE_SHAPE = 0,
        TRANSPARENT_SHAPE,
        LIGHT,
        META,
        OUTLINE,
        LAYER_FRONT_OPAQUE_SHAPE,
        LAYER_FRONT_TRANSPARENT_SHAPE,
        LAYER_HUD_OPAQUE_SHAPE,
        LAYER_HUD_TRANSPARENT_SHAPE,

        BACKGROUND,

        NUM_BUCKETS
    };

    using BucketList = render::VaryingArray<render::ItemBounds, Buckets::NUM_BUCKETS>;
    using Output = render::VaryingSet2<BucketList, render::ItemSpatialTree::ItemSelection>;
    using JobModel = render::Task::ModelO<RenderFetchCullSortTask, Output>;

    RenderFetchCullSortTask() {}

    void build(JobModel& task, const render::Varying& inputs, render::Varying& outputs, render::CullFunctor cullFunctor, uint8_t tagBits, uint8_t tagMask);
};

#endif // hifi_RenderFetchCullSortTask_h
