//
//  BloomStage.h

//  Created by Sam Gondelman on 8/7/2018
//  Copyright 2018 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_render_utils_BloomStage_h
#define hifi_render_utils_BloomStage_h

#include <graphics/Bloom.h>
#include <render/Forward.h>
#include <render/DrawTask.h>
#include <render/Stage.h>

// Bloom stage to set up bloom-related rendering tasks
class BloomStage : public render::PointerStage<graphics::Bloom, graphics::BloomPointer> {};
using BloomStagePointer = std::shared_ptr<BloomStage>;

class BloomStageSetup {
public:
    using JobModel = render::Job::Model<BloomStageSetup>;

    BloomStageSetup() {}
    void run(const render::RenderContextPointer& renderContext);

protected:
};

#endif
