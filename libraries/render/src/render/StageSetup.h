//
//  StageSetup.h
//  render/src/render
//
//  Created by HifiExperiments on 10/16/24
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_render_StageSetup_h
#define hifi_render_StageSetup_h

#include "Engine.h"

namespace render {

    template <typename T>
    class StageSetup {
    public:
        StageSetup() {}

        void run(const RenderContextPointer& renderContext) {
            if (renderContext->_scene) {
                auto stage = renderContext->_scene->getStage(T::getName());
                if (!stage) {
                    renderContext->_scene->resetStage(T::getName(), std::make_shared<T>());
                }
            }
        }
    };
}

#endif // hifi_render_StageSetup_h
