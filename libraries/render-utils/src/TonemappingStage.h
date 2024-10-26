//
//  TonemappingStage.h
// 
//  Created by HifiExperiments on 6/24/24
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_render_utils_TonemappingStage_h
#define hifi_render_utils_TonemappingStage_h

#include <graphics/Stage.h>
#include <set>
#include <unordered_map>
#include <render/IndexedContainer.h>
#include <render/Stage.h>

#include <render/Forward.h>
#include <render/DrawTask.h>
#include <graphics/Tonemapping.h>

// Tonemapping stage to set up tonemapping-related rendering tasks
class TonemappingStage : public render::Stage {
public:
    static std::string _stageName;
    static const std::string& getName() { return _stageName; }

    using Index = render::indexed_container::Index;
    static const Index INVALID_INDEX;
    static bool isIndexInvalid(Index index) { return index == INVALID_INDEX; }
    
    using TonemappingPointer = graphics::TonemappingPointer;
    using Tonemappings = render::indexed_container::IndexedPointerVector<graphics::Tonemapping>;
    using TonemappingMap = std::unordered_map<TonemappingPointer, Index>;

    using TonemappingIndices = std::vector<Index>;

    Index findTonemapping(const TonemappingPointer& tonemapping) const;
    Index addTonemapping(const TonemappingPointer& tonemapping);

    TonemappingPointer removeTonemapping(Index index);
    
    bool checkTonemappingId(Index index) const { return _tonemappings.checkIndex(index); }

    Index getNumTonemappings() const { return _tonemappings.getNumElements(); }
    Index getNumFreeTonemappings() const { return _tonemappings.getNumFreeIndices(); }
    Index getNumAllocatedTonemappings() const { return _tonemappings.getNumAllocatedIndices(); }

    TonemappingPointer getTonemapping(Index tonemappingId) const {
        return _tonemappings.get(tonemappingId);
    }

    Tonemappings _tonemappings;
    TonemappingMap _tonemappingMap;

    class Frame {
    public:
        Frame() {}
        
        void clear() { _tonemappings.clear(); }

        void pushTonemapping(TonemappingStage::Index index) { _tonemappings.emplace_back(index); }

        TonemappingStage::TonemappingIndices _tonemappings;
    };
    using FramePointer = std::shared_ptr<Frame>;
    
    Frame _currentFrame;
};
using TonemappingStagePointer = std::shared_ptr<TonemappingStage>;

class TonemappingStageSetup {
public:
    using JobModel = render::Job::Model<TonemappingStageSetup>;

    TonemappingStageSetup();
    void run(const render::RenderContextPointer& renderContext);

protected:
};

#endif
