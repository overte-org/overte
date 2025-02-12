//
//  AmbientOcclusionStage.h
//
//  Created by HifiExperiments on 6/24/24
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_render_utils_AmbientOcclusionStage_h
#define hifi_render_utils_AmbientOcclusionStage_h

#include <graphics/Stage.h>
#include <set>
#include <unordered_map>
#include <render/IndexedContainer.h>
#include <render/Stage.h>

#include <render/Forward.h>
#include <render/DrawTask.h>
#include <graphics/AmbientOcclusion.h>

// AmbientOcclusion stage to set up ambientOcclusion-related rendering tasks
class AmbientOcclusionStage : public render::Stage {
public:
    static std::string _stageName;
    static const std::string& getName() { return _stageName; }

    using Index = render::indexed_container::Index;
    static const Index INVALID_INDEX;
    static bool isIndexInvalid(Index index) { return index == INVALID_INDEX; }
    
    using AmbientOcclusionPointer = graphics::AmbientOcclusionPointer;
    using AmbientOcclusions = render::indexed_container::IndexedPointerVector<graphics::AmbientOcclusion>;
    using AmbientOcclusionMap = std::unordered_map<AmbientOcclusionPointer, Index>;

    using AmbientOcclusionIndices = std::vector<Index>;

    Index findAmbientOcclusion(const AmbientOcclusionPointer& ambientOcclusion) const;
    Index addAmbientOcclusion(const AmbientOcclusionPointer& ambientOcclusion);

    AmbientOcclusionPointer removeAmbientOcclusion(Index index);
    
    bool checkAmbientOcclusionId(Index index) const { return _ambientOcclusions.checkIndex(index); }

    Index getNumAmbientOcclusions() const { return _ambientOcclusions.getNumElements(); }
    Index getNumFreeAmbientOcclusions() const { return _ambientOcclusions.getNumFreeIndices(); }
    Index getNumAllocatedAmbientOcclusions() const { return _ambientOcclusions.getNumAllocatedIndices(); }

    AmbientOcclusionPointer getAmbientOcclusion(Index ambientOcclusionId) const {
        return _ambientOcclusions.get(ambientOcclusionId);
    }

    AmbientOcclusions _ambientOcclusions;
    AmbientOcclusionMap _ambientOcclusionMap;

    class Frame {
    public:
        Frame() {}
        
        void clear() { _ambientOcclusions.clear(); }

        void pushAmbientOcclusion(AmbientOcclusionStage::Index index) { _ambientOcclusions.emplace_back(index); }

        AmbientOcclusionStage::AmbientOcclusionIndices _ambientOcclusions;
    };
    using FramePointer = std::shared_ptr<Frame>;
    
    Frame _currentFrame;
};
using AmbientOcclusionStagePointer = std::shared_ptr<AmbientOcclusionStage>;

class AmbientOcclusionStageSetup {
public:
    using JobModel = render::Job::Model<AmbientOcclusionStageSetup>;

    AmbientOcclusionStageSetup();
    void run(const render::RenderContextPointer& renderContext);

protected:
};

#endif
