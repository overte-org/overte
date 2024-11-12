//
//  FadeEffect.h

//  Created by Olivier Prat on 17/07/2017.
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_render_utils_FadeEffect_h
#define hifi_render_utils_FadeEffect_h

#include <render/Engine.h>

class FadeEffect {
public:
    using JobModel = render::Task::Model<FadeEffect>;

    FadeEffect();

    void build(JobModel& task, const render::Varying& inputs, render::Varying& outputs);

    static render::ShapePipeline::BatchSetter getBatchSetter();
    static render::ShapePipeline::ItemSetter getItemUniformSetter();

private:
    static gpu::TexturePointer _maskMap;
};

#endif // hifi_render_utils_FadeEffect_h
