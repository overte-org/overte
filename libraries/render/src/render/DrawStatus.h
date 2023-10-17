//
//  DrawStatus.h
//  render/src/render
//
//  Created by Niraj Venkat on 6/29/15.
//  Copyright 2015 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef hifi_render_DrawStatus_h
#define hifi_render_DrawStatus_h

#include "DrawTask.h"
#include "gpu/Batch.h"

namespace render {
    class DrawStatusConfig : public Job::Config {
        Q_OBJECT
        Q_PROPERTY(bool showDisplay MEMBER showDisplay WRITE setShowDisplay)
        Q_PROPERTY(bool showNetwork MEMBER showNetwork WRITE setShowNetwork)
    public:
        DrawStatusConfig() : Job::Config(false) {} // FIXME FOR debug

        void dirtyHelper();

        bool showDisplay{ false };
        bool showNetwork{ false };
        bool showFade{ false };

    public slots:
        void setShowDisplay(bool enabled) { showDisplay = enabled; dirtyHelper(); }
        void setShowNetwork(bool enabled) { showNetwork = enabled; dirtyHelper(); }
        void setShowFade(bool enabled) { showFade = enabled; dirtyHelper(); }

    signals:
        void dirty();
    };

    class DrawStatus {
    public:
        using Config = DrawStatusConfig;
        using Input = VaryingSet2<ItemBounds, glm::vec2>;
        using JobModel = Job::ModelI<DrawStatus, Input, Config>;

        DrawStatus() {}
        DrawStatus(const gpu::TexturePointer statusIconMap) { setStatusIconMap(statusIconMap); }

        void configure(const Config& config);
        void run(const RenderContextPointer& renderContext, const Input& input);

        const gpu::PipelinePointer getDrawItemBoundsPipeline();
        const gpu::PipelinePointer getDrawItemStatusPipeline();

        void setStatusIconMap(const gpu::TexturePointer& map);
        const gpu::TexturePointer getStatusIconMap() const;

    protected:
        bool _showDisplay { false }; // initialized by Config
        bool _showNetwork { false }; // initialized by Config
        bool _showFade { false }; // initialized by Config

        gpu::Stream::FormatPointer _drawItemFormat;
        gpu::PipelinePointer _drawItemBoundsPipeline;
        gpu::PipelinePointer _drawItemStatusPipeline;

        gpu::BufferPointer _boundsBuffer;
        gpu::BufferPointer _instanceBuffer;
        gpu::Stream::FormatPointer _vertexFormat;
        gpu::TexturePointer _statusIconMap;
    };
}

#endif // hifi_render_DrawStatus_h
