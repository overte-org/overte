//
//  TransitionStage.h

//  Created by Olivier Prat on 07/07/2017.
//  Copyright 2017 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_render_TransitionStage_h
#define hifi_render_TransitionStage_h

#include "Stage.h"
#include "Engine.h"
#include "Transition.h"

namespace render {

    // Transition stage to set up Transition-related effects
    class TransitionStage : public TypedStage<Transition> {
    public:
        bool isTransitionUsed(Index index) const { return _elements.checkIndex(index) && !_elements.isElementFreed(index); }
        Index addTransition(ItemID itemId, Transition::Type type, ItemID boundId);
    };
    using TransitionStagePointer = std::shared_ptr<TransitionStage>;

    class TransitionStageSetup {
    public:
        using JobModel = render::Job::Model<TransitionStageSetup>;

        TransitionStageSetup() {}
        void run(const RenderContextPointer& renderContext);
    };
}

#endif // hifi_render_TransitionStage_h
