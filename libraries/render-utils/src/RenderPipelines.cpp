
//
//  RenderPipelines.cpp
//  render-utils/src/
//
//  Created by Zach Pomerantz on 1/28/2016.
//  Copyright 2016 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "RenderPipelines.h"

#include <functional>

#include <gpu/Context.h>
#include <material-networking/TextureCache.h>
#include <render/DrawTask.h>
#include <shaders/Shaders.h>
#include <graphics/ShaderConstants.h>
#include <procedural/ReferenceMaterial.h>

#include "render-utils/ShaderConstants.h"
#include "StencilMaskPass.h"
#include "DeferredLightingEffect.h"
#include "TextureCache.h"

using namespace render;
using namespace std::placeholders;
using namespace shader::render_utils::program;
using Key = render::ShapeKey;

namespace ru {
    using render_utils::slot::texture::Texture;
    using render_utils::slot::buffer::Buffer;
}

namespace gr {
    using graphics::slot::texture::Texture;
    using graphics::slot::buffer::Buffer;
}

void initDeferredPipelines(ShapePlumber& plumber, const render::ShapePipeline::BatchSetter& batchSetter, const render::ShapePipeline::ItemSetter& itemSetter);
void initForwardPipelines(ShapePlumber& plumber);
void initZPassPipelines(ShapePlumber& plumber, gpu::StatePointer state, const render::ShapePipeline::BatchSetter& batchSetter, const render::ShapePipeline::ItemSetter& itemSetter);
void sortAndRenderZPassShapes(const ShapePlumberPointer& shapePlumber, const render::RenderContextPointer& renderContext, const render::ShapeBounds& inShapes, render::ItemBounds &itemBounds);

void addPlumberPipeline(ShapePlumber& plumber,
        const ShapeKey& key, int programId, gpu::StatePointer& baseState,
        const render::ShapePipeline::BatchSetter& batchSetter, const render::ShapePipeline::ItemSetter& itemSetter);

void batchSetter(const ShapePipeline& pipeline, gpu::Batch& batch, RenderArgs* args);
void lightBatchSetter(const ShapePipeline& pipeline, gpu::Batch& batch, RenderArgs* args);
static bool forceLightBatchSetter{ false };

// TOOD: build this list algorithmically so we don't have to maintain it
std::vector<std::tuple<Key::Builder, uint32_t, uint32_t>> ALL_PIPELINES = {
    // Simple
    { Key::Builder(), simple, model_shadow },
    { Key::Builder().withTranslucent(), simple_translucent, model_shadow },
    { Key::Builder().withUnlit(), simple_unlit, model_shadow },
    { Key::Builder().withTranslucent().withUnlit(), simple_translucent_unlit, model_shadow },
    // Simple Fade
    { Key::Builder().withFade(), simple_fade, model_shadow_fade },
    { Key::Builder().withTranslucent().withFade(), simple_translucent_fade, model_shadow_fade },
    { Key::Builder().withUnlit().withFade(), simple_unlit_fade, model_shadow_fade },
    { Key::Builder().withTranslucent().withUnlit().withFade(), simple_translucent_unlit_fade, model_shadow_fade },

    // Unskinned
    { Key::Builder().withMaterial(), model, model_shadow },
    { Key::Builder().withMaterial().withTangents(), model_normalmap, model_shadow },
    { Key::Builder().withMaterial().withTranslucent(), model_translucent, model_shadow },
    { Key::Builder().withMaterial().withTangents().withTranslucent(), model_normalmap_translucent, model_shadow },
    // Unskinned Unlit
    { Key::Builder().withMaterial().withUnlit(), model_unlit, model_shadow },
    { Key::Builder().withMaterial().withTangents().withUnlit(), model_normalmap_unlit, model_shadow },
    { Key::Builder().withMaterial().withTranslucent().withUnlit(), model_translucent_unlit, model_shadow },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withUnlit(), model_normalmap_translucent_unlit, model_shadow },
    // Unskinned Lightmapped
    { Key::Builder().withMaterial().withLightMap(), model_lightmap, model_shadow },
    { Key::Builder().withMaterial().withTangents().withLightMap(), model_normalmap_lightmap, model_shadow },
    { Key::Builder().withMaterial().withTranslucent().withLightMap(), model_translucent_lightmap, model_shadow },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withLightMap(), model_normalmap_translucent_lightmap, model_shadow },
    // Unskinned MToon
    { Key::Builder().withMaterial().withMToon(), model_mtoon, model_shadow_mtoon },
    { Key::Builder().withMaterial().withTangents().withMToon(), model_normalmap_mtoon, model_shadow_mtoon },
    { Key::Builder().withMaterial().withTranslucent().withMToon(), model_translucent_mtoon, model_shadow_mtoon },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withMToon(), model_normalmap_translucent_mtoon, model_shadow_mtoon },
    // Unskinned Fade
    { Key::Builder().withMaterial().withFade(), model_fade, model_shadow_fade },
    { Key::Builder().withMaterial().withTangents().withFade(), model_normalmap_fade, model_shadow_fade },
    { Key::Builder().withMaterial().withTranslucent().withFade(), model_translucent_fade, model_shadow_fade },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withFade(), model_normalmap_translucent_fade, model_shadow_fade },
    // Unskinned Unlit Fade
    { Key::Builder().withMaterial().withUnlit().withFade(), model_unlit_fade, model_shadow_fade },
    { Key::Builder().withMaterial().withTangents().withUnlit().withFade(), model_normalmap_unlit_fade, model_shadow_fade },
    { Key::Builder().withMaterial().withTranslucent().withUnlit().withFade(), model_translucent_unlit_fade, model_shadow_fade },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withUnlit().withFade(), model_normalmap_translucent_unlit_fade, model_shadow_fade },
    // Unskinned Lightmapped Fade
    { Key::Builder().withMaterial().withLightMap().withFade(), model_lightmap_fade, model_shadow_fade },
    { Key::Builder().withMaterial().withTangents().withLightMap().withFade(), model_normalmap_lightmap_fade, model_shadow_fade },
    { Key::Builder().withMaterial().withTranslucent().withLightMap().withFade(), model_translucent_lightmap_fade, model_shadow_fade },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withLightMap().withFade(), model_normalmap_translucent_lightmap_fade, model_shadow_fade },
    // Unskinned MToon Fade
    { Key::Builder().withMaterial().withMToon().withFade(), model_mtoon_fade, model_shadow_mtoon_fade },
    { Key::Builder().withMaterial().withTangents().withMToon().withFade(), model_normalmap_mtoon_fade, model_shadow_mtoon_fade },
    { Key::Builder().withMaterial().withTranslucent().withMToon().withFade(), model_translucent_mtoon_fade, model_shadow_mtoon_fade },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withMToon().withFade(), model_normalmap_translucent_mtoon_fade, model_shadow_mtoon_fade },

    // Matrix palette skinned
    { Key::Builder().withMaterial().withDeformed(), model_deformed, model_shadow_deformed },
    { Key::Builder().withMaterial().withTangents().withDeformed(), model_normalmap_deformed, model_shadow_deformed },
    { Key::Builder().withMaterial().withTranslucent().withDeformed(), model_translucent_deformed, model_shadow_deformed },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withDeformed(), model_normalmap_translucent_deformed, model_shadow_deformed },
    // Matrix palette skinned Unlit
    { Key::Builder().withMaterial().withUnlit().withDeformed(), model_unlit_deformed, model_shadow_deformed },
    { Key::Builder().withMaterial().withTangents().withUnlit().withDeformed(), model_normalmap_unlit_deformed, model_shadow_deformed },
    { Key::Builder().withMaterial().withTranslucent().withUnlit().withDeformed(), model_translucent_unlit_deformed, model_shadow_deformed },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withUnlit().withDeformed(), model_normalmap_translucent_unlit_deformed, model_shadow_deformed },
    // Matrix palette skinned Lightmapped
    { Key::Builder().withMaterial().withLightMap().withDeformed(), model_lightmap_deformed, model_shadow_deformed },
    { Key::Builder().withMaterial().withTangents().withLightMap().withDeformed(), model_normalmap_lightmap_deformed, model_shadow_deformed },
    { Key::Builder().withMaterial().withTranslucent().withLightMap().withDeformed(), model_translucent_lightmap_deformed, model_shadow_deformed },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withLightMap().withDeformed(), model_normalmap_translucent_lightmap_deformed, model_shadow_deformed },
    // Matrix palette skinned MToon
    { Key::Builder().withMaterial().withMToon().withDeformed(), model_mtoon_deformed, model_shadow_mtoon_deformed },
    { Key::Builder().withMaterial().withTangents().withMToon().withDeformed(), model_normalmap_mtoon_deformed, model_shadow_mtoon_deformed },
    { Key::Builder().withMaterial().withTranslucent().withMToon().withDeformed(), model_translucent_mtoon_deformed, model_shadow_mtoon_deformed },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withMToon().withDeformed(), model_normalmap_translucent_mtoon_deformed, model_shadow_mtoon_deformed },
    // Matrix palette skinned Fade
    { Key::Builder().withMaterial().withFade().withDeformed(), model_fade_deformed, model_shadow_fade_deformed },
    { Key::Builder().withMaterial().withTangents().withFade().withDeformed(), model_normalmap_fade_deformed, model_shadow_fade_deformed },
    { Key::Builder().withMaterial().withTranslucent().withFade().withDeformed(), model_translucent_fade_deformed, model_shadow_fade_deformed },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withFade().withDeformed(), model_normalmap_translucent_fade_deformed, model_shadow_fade_deformed },
    // Matrix palette skinned Unlit Fade
    { Key::Builder().withMaterial().withUnlit().withFade().withDeformed(), model_unlit_fade_deformed, model_shadow_fade_deformed },
    { Key::Builder().withMaterial().withTangents().withUnlit().withFade().withDeformed(), model_normalmap_unlit_fade_deformed, model_shadow_fade_deformed },
    { Key::Builder().withMaterial().withTranslucent().withUnlit().withFade().withDeformed(), model_translucent_unlit_fade_deformed, model_shadow_fade_deformed },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withUnlit().withFade().withDeformed(), model_normalmap_translucent_unlit_fade_deformed, model_shadow_fade_deformed },
    // Matrix palette skinned Lightmapped Fade
    { Key::Builder().withMaterial().withLightMap().withFade().withDeformed(), model_lightmap_fade_deformed, model_shadow_fade_deformed },
    { Key::Builder().withMaterial().withTangents().withLightMap().withFade().withDeformed(), model_normalmap_lightmap_fade_deformed, model_shadow_fade_deformed },
    { Key::Builder().withMaterial().withTranslucent().withLightMap().withFade().withDeformed(), model_translucent_lightmap_fade_deformed, model_shadow_fade_deformed },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withLightMap().withFade().withDeformed(), model_normalmap_translucent_lightmap_fade_deformed, model_shadow_fade_deformed },
    // Matrix palette skinned MToon Fade
    { Key::Builder().withMaterial().withMToon().withFade().withDeformed(), model_mtoon_fade_deformed, model_shadow_mtoon_fade_deformed },
    { Key::Builder().withMaterial().withTangents().withMToon().withFade().withDeformed(), model_normalmap_mtoon_fade_deformed, model_shadow_mtoon_fade_deformed },
    { Key::Builder().withMaterial().withTranslucent().withMToon().withFade().withDeformed(), model_translucent_mtoon_fade_deformed, model_shadow_mtoon_fade_deformed },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withMToon().withFade().withDeformed(), model_normalmap_translucent_mtoon_fade_deformed, model_shadow_mtoon_fade_deformed },

    // Dual quaternion skinned
    { Key::Builder().withMaterial().withDeformed().withDualQuatSkinned(), model_deformeddq, model_shadow_deformeddq },
    { Key::Builder().withMaterial().withTangents().withDeformed().withDualQuatSkinned(), model_normalmap_deformeddq, model_shadow_deformeddq },
    { Key::Builder().withMaterial().withTranslucent().withDeformed().withDualQuatSkinned(), model_translucent_deformeddq, model_shadow_deformeddq },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withDeformed().withDualQuatSkinned(), model_normalmap_translucent_deformeddq, model_shadow_deformeddq },
    // Dual quaternion skinned Unlit
    { Key::Builder().withMaterial().withUnlit().withDeformed().withDualQuatSkinned(), model_unlit_deformeddq, model_shadow_deformeddq },
    { Key::Builder().withMaterial().withTangents().withUnlit().withDeformed().withDualQuatSkinned(), model_normalmap_unlit_deformeddq, model_shadow_deformeddq },
    { Key::Builder().withMaterial().withTranslucent().withUnlit().withDeformed().withDualQuatSkinned(), model_translucent_unlit_deformeddq, model_shadow_deformeddq },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withUnlit().withDeformed().withDualQuatSkinned(), model_normalmap_translucent_unlit_deformeddq, model_shadow_deformeddq },
    // Dual quaternion skinned Lightmapped
    { Key::Builder().withMaterial().withLightMap().withDeformed().withDualQuatSkinned(), model_lightmap_deformeddq, model_shadow_deformeddq },
    { Key::Builder().withMaterial().withTangents().withLightMap().withDeformed().withDualQuatSkinned(), model_normalmap_lightmap_deformeddq, model_shadow_deformeddq },
    { Key::Builder().withMaterial().withTranslucent().withLightMap().withDeformed().withDualQuatSkinned(), model_translucent_lightmap_deformeddq, model_shadow_deformeddq },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withLightMap().withDeformed().withDualQuatSkinned(), model_normalmap_translucent_lightmap_deformeddq, model_shadow_deformeddq },
    // Dual quaternion skinned MToon
    { Key::Builder().withMaterial().withMToon().withDeformed().withDualQuatSkinned(), model_mtoon_deformeddq, model_shadow_mtoon_deformeddq },
    { Key::Builder().withMaterial().withTangents().withMToon().withDeformed().withDualQuatSkinned(), model_normalmap_mtoon_deformeddq, model_shadow_mtoon_deformeddq },
    { Key::Builder().withMaterial().withTranslucent().withMToon().withDeformed().withDualQuatSkinned(), model_translucent_mtoon_deformeddq, model_shadow_mtoon_deformeddq },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withMToon().withDeformed().withDualQuatSkinned(), model_normalmap_translucent_mtoon_deformeddq, model_shadow_mtoon_deformeddq },
    // Dual quaternion skinned Fade
    { Key::Builder().withMaterial().withFade().withDeformed().withDualQuatSkinned(), model_fade_deformeddq, model_shadow_fade_deformeddq },
    { Key::Builder().withMaterial().withTangents().withFade().withDeformed().withDualQuatSkinned(), model_normalmap_fade_deformeddq, model_shadow_fade_deformeddq },
    { Key::Builder().withMaterial().withTranslucent().withFade().withDeformed().withDualQuatSkinned(), model_translucent_fade_deformeddq, model_shadow_fade_deformeddq },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withFade().withDeformed().withDualQuatSkinned(), model_normalmap_translucent_fade_deformeddq, model_shadow_fade_deformeddq },
    // Dual quaternion skinned Unlit Fade
    { Key::Builder().withMaterial().withUnlit().withFade().withDeformed().withDualQuatSkinned(), model_unlit_fade_deformeddq, model_shadow_fade_deformeddq },
    { Key::Builder().withMaterial().withTangents().withUnlit().withFade().withDeformed().withDualQuatSkinned(), model_normalmap_unlit_fade_deformeddq, model_shadow_fade_deformeddq },
    { Key::Builder().withMaterial().withTranslucent().withUnlit().withFade().withDeformed().withDualQuatSkinned(), model_translucent_unlit_fade_deformeddq, model_shadow_fade_deformeddq },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withUnlit().withFade().withDeformed().withDualQuatSkinned(), model_normalmap_translucent_unlit_fade_deformeddq, model_shadow_fade_deformeddq },
    // Dual quaternion skinned Lightmapped Fade
    { Key::Builder().withMaterial().withLightMap().withFade().withDeformed().withDualQuatSkinned(), model_lightmap_fade_deformeddq, model_shadow_fade_deformeddq },
    { Key::Builder().withMaterial().withTangents().withLightMap().withFade().withDeformed().withDualQuatSkinned(), model_normalmap_lightmap_fade_deformeddq, model_shadow_fade_deformeddq },
    { Key::Builder().withMaterial().withTranslucent().withLightMap().withFade().withDeformed().withDualQuatSkinned(), model_translucent_lightmap_fade_deformeddq, model_shadow_fade_deformeddq },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withLightMap().withFade().withDeformed().withDualQuatSkinned(), model_normalmap_translucent_lightmap_fade_deformeddq, model_shadow_fade_deformeddq },
    // Dual quaternion skinned MToon Fade
    { Key::Builder().withMaterial().withMToon().withFade().withDeformed().withDualQuatSkinned(), model_mtoon_fade_deformeddq, model_shadow_mtoon_fade_deformeddq },
    { Key::Builder().withMaterial().withTangents().withMToon().withFade().withDeformed().withDualQuatSkinned(), model_normalmap_mtoon_fade_deformeddq, model_shadow_mtoon_fade_deformeddq },
    { Key::Builder().withMaterial().withTranslucent().withMToon().withFade().withDeformed().withDualQuatSkinned(), model_translucent_mtoon_fade_deformeddq, model_shadow_mtoon_fade_deformeddq },
    { Key::Builder().withMaterial().withTangents().withTranslucent().withMToon().withFade().withDeformed().withDualQuatSkinned(), model_normalmap_translucent_mtoon_fade_deformeddq, model_shadow_mtoon_fade_deformeddq },
};

void initDeferredPipelines(render::ShapePlumber& plumber, const render::ShapePipeline::BatchSetter& batchSetter, const render::ShapePipeline::ItemSetter& itemSetter) {
    auto addPipeline = std::bind(&addPlumberPipeline, std::ref(plumber), _1, _2, std::make_shared<gpu::State>(), _3, _4);

    for (auto& pipeline : ALL_PIPELINES) {
        if (std::get<0>(pipeline).build().isFaded()) {
            addPipeline(std::get<0>(pipeline), std::get<1>(pipeline), batchSetter, itemSetter);
        } else {
            addPipeline(std::get<0>(pipeline), std::get<1>(pipeline), nullptr, nullptr);
        }
    }
}

void initForwardPipelines(ShapePlumber& plumber) {
    auto addPipelineBind = std::bind(&addPlumberPipeline, std::ref(plumber), _1, _2, std::make_shared<gpu::State>(), _3, _4);

    // Disable fade on the forward pipeline, all shaders get added twice, once with the fade key and once without
    auto addPipeline = [&](const ShapeKey& key, int programId) {
        addPipelineBind(key, programId, nullptr, nullptr);
        addPipelineBind(Key::Builder(key).withFade(), programId, nullptr, nullptr);
    };

    // Forward pipelines need the lightBatchSetter for opaques and transparents
    forceLightBatchSetter = true;

    // TOOD: build this list algorithmically so we don't have to maintain it
    std::vector<std::pair<Key::Builder, uint32_t>> pipelines = {
        // Simple
        { Key::Builder(), simple_forward },
        { Key::Builder().withTranslucent(), simple_translucent_forward },
        { Key::Builder().withUnlit(), simple_unlit_forward },
        { Key::Builder().withTranslucent().withUnlit(), simple_translucent_unlit_forward },

        // Unskinned
        { Key::Builder().withMaterial(), model_forward },
        { Key::Builder().withMaterial().withTangents(), model_normalmap_forward },
        { Key::Builder().withMaterial().withTranslucent(), model_translucent_forward },
        { Key::Builder().withMaterial().withTangents().withTranslucent(), model_normalmap_translucent_forward },
        // Unskinned Unlit
        { Key::Builder().withMaterial().withUnlit(), model_unlit_forward },
        { Key::Builder().withMaterial().withTangents().withUnlit(), model_normalmap_unlit_forward },
        { Key::Builder().withMaterial().withTranslucent().withUnlit(), model_translucent_unlit_forward },
        { Key::Builder().withMaterial().withTangents().withTranslucent().withUnlit(), model_normalmap_translucent_unlit_forward },
        // Unskinned Lightmapped
        { Key::Builder().withMaterial().withLightMap(), model_lightmap_forward },
        { Key::Builder().withMaterial().withTangents().withLightMap(), model_normalmap_lightmap_forward },
        { Key::Builder().withMaterial().withTranslucent().withLightMap(), model_translucent_lightmap_forward },
        { Key::Builder().withMaterial().withTangents().withTranslucent().withLightMap(), model_normalmap_translucent_lightmap_forward },
        // Unskinned MToon
        { Key::Builder().withMaterial().withMToon(), model_mtoon_forward },
        { Key::Builder().withMaterial().withTangents().withMToon(), model_normalmap_mtoon_forward },
        { Key::Builder().withMaterial().withTranslucent().withMToon(), model_translucent_mtoon_forward },
        { Key::Builder().withMaterial().withTangents().withTranslucent().withMToon(), model_normalmap_translucent_mtoon_forward },

        // Matrix palette skinned
        { Key::Builder().withMaterial().withDeformed(), model_forward_deformed },
        { Key::Builder().withMaterial().withTangents().withDeformed(), model_normalmap_forward_deformed },
        { Key::Builder().withMaterial().withTranslucent().withDeformed(), model_translucent_forward_deformed },
        { Key::Builder().withMaterial().withTangents().withTranslucent().withDeformed(), model_normalmap_translucent_forward_deformed },
        // Matrix palette skinned Unlit
        { Key::Builder().withMaterial().withUnlit().withDeformed(), model_unlit_forward_deformed },
        { Key::Builder().withMaterial().withTangents().withUnlit().withDeformed(), model_normalmap_unlit_forward_deformed },
        { Key::Builder().withMaterial().withTranslucent().withUnlit().withDeformed(), model_translucent_unlit_forward_deformed },
        { Key::Builder().withMaterial().withTangents().withTranslucent().withUnlit().withDeformed(), model_normalmap_translucent_unlit_forward_deformed },
        // Matrix palette skinned Lightmapped
        { Key::Builder().withMaterial().withLightMap().withDeformed(), model_lightmap_forward_deformed },
        { Key::Builder().withMaterial().withTangents().withLightMap().withDeformed(), model_normalmap_lightmap_forward_deformed },
        { Key::Builder().withMaterial().withTranslucent().withLightMap().withDeformed(), model_translucent_lightmap_forward_deformed },
        { Key::Builder().withMaterial().withTangents().withTranslucent().withLightMap().withDeformed(), model_normalmap_translucent_lightmap_forward_deformed },
        // Matrix palette skinned MToon
        { Key::Builder().withMaterial().withMToon().withDeformed(), model_mtoon_forward_deformed },
        { Key::Builder().withMaterial().withTangents().withMToon().withDeformed(), model_normalmap_mtoon_forward_deformed },
        { Key::Builder().withMaterial().withTranslucent().withMToon().withDeformed(), model_translucent_mtoon_forward_deformed },
        { Key::Builder().withMaterial().withTangents().withTranslucent().withMToon().withDeformed(), model_normalmap_translucent_mtoon_forward_deformed },

        // Dual quaternion skinned
        { Key::Builder().withMaterial().withDeformed().withDualQuatSkinned(), model_forward_deformeddq },
        { Key::Builder().withMaterial().withTangents().withDeformed().withDualQuatSkinned(), model_normalmap_forward_deformeddq },
        { Key::Builder().withMaterial().withTranslucent().withDeformed().withDualQuatSkinned(), model_translucent_forward_deformeddq },
        { Key::Builder().withMaterial().withTangents().withTranslucent().withDeformed().withDualQuatSkinned(), model_normalmap_translucent_forward_deformeddq },
        // Dual quaternion skinned Unlit
        { Key::Builder().withMaterial().withUnlit().withDeformed().withDualQuatSkinned(), model_unlit_forward_deformeddq },
        { Key::Builder().withMaterial().withTangents().withUnlit().withDeformed().withDualQuatSkinned(), model_normalmap_unlit_forward_deformeddq },
        { Key::Builder().withMaterial().withTranslucent().withUnlit().withDeformed().withDualQuatSkinned(), model_translucent_unlit_forward_deformeddq },
        { Key::Builder().withMaterial().withTangents().withTranslucent().withUnlit().withDeformed().withDualQuatSkinned(), model_normalmap_translucent_unlit_forward_deformeddq },
        // Dual quaternion skinned Lightmapped
        { Key::Builder().withMaterial().withLightMap().withDeformed().withDualQuatSkinned(), model_lightmap_forward_deformeddq },
        { Key::Builder().withMaterial().withTangents().withLightMap().withDeformed().withDualQuatSkinned(), model_normalmap_lightmap_forward_deformeddq },
        { Key::Builder().withMaterial().withTranslucent().withLightMap().withDeformed().withDualQuatSkinned(), model_translucent_lightmap_forward_deformeddq },
        { Key::Builder().withMaterial().withTangents().withTranslucent().withLightMap().withDeformed().withDualQuatSkinned(), model_normalmap_translucent_lightmap_forward_deformeddq },
        // Dual quaternion skinned MToon
        { Key::Builder().withMaterial().withMToon().withDeformed().withDualQuatSkinned(), model_mtoon_forward_deformeddq },
        { Key::Builder().withMaterial().withTangents().withMToon().withDeformed().withDualQuatSkinned(), model_normalmap_mtoon_forward_deformeddq },
        { Key::Builder().withMaterial().withTranslucent().withMToon().withDeformed().withDualQuatSkinned(), model_translucent_mtoon_forward_deformeddq },
        { Key::Builder().withMaterial().withTangents().withTranslucent().withMToon().withDeformed().withDualQuatSkinned(), model_normalmap_translucent_mtoon_forward_deformeddq },
    };

    for (auto& pipeline : pipelines) {
        addPipeline(pipeline.first, pipeline.second);
    }

    forceLightBatchSetter = false;
}

void addPlumberPipeline(ShapePlumber& plumber,
        const ShapeKey& key, int programId, gpu::StatePointer& baseState,
        const render::ShapePipeline::BatchSetter& extraBatchSetter, const render::ShapePipeline::ItemSetter& itemSetter) {
    // These key-values' pipelines are added by this functor in addition to the key passed
    assert(!key.isWireframe());
    assert(!key.isDepthBiased());
    assert(key.isCullFace());

    gpu::ShaderPointer program = gpu::Shader::createProgram(programId);

    for (int i = 0; i < 4; i++) {
        bool isBiased = (i & 1);
        bool isWireframed = (i & 2);
        for (int cullFaceMode = graphics::MaterialKey::CullFaceMode::CULL_NONE; cullFaceMode < graphics::MaterialKey::CullFaceMode::NUM_CULL_FACE_MODES; cullFaceMode++) {
            auto state = std::make_shared<gpu::State>(*baseState);
            key.isTranslucent() ? PrepareStencil::testMask(*state) : PrepareStencil::testMaskDrawShape(*state);

            // Depth test depends on transparency
            state->setDepthTest(true, !key.isTranslucent(), gpu::LESS_EQUAL);
            state->setBlendFunction(key.isTranslucent(),
                    gpu::State::SRC_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::INV_SRC_ALPHA,
                    gpu::State::FACTOR_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::ONE);

            ShapeKey::Builder builder(key);
            builder.withCullFaceMode((graphics::MaterialKey::CullFaceMode)cullFaceMode);
            state->setCullMode((gpu::State::CullMode)cullFaceMode);
            if (isWireframed) {
                builder.withWireframe();
                state->setFillMode(gpu::State::FILL_LINE);
            }
            if (isBiased) {
                builder.withDepthBias();
                state->setDepthBias(1.0f);
                state->setDepthBiasSlopeScale(1.0f);
            }

            auto baseBatchSetter = (forceLightBatchSetter || key.isTranslucent() || key.isMToon()) ? &lightBatchSetter : &batchSetter;
            render::ShapePipeline::BatchSetter finalBatchSetter;
            if (extraBatchSetter) {
                finalBatchSetter = [baseBatchSetter, extraBatchSetter](const ShapePipeline& pipeline, gpu::Batch& batch, render::Args* args) {
                    baseBatchSetter(pipeline, batch, args);
                    extraBatchSetter(pipeline, batch, args);
                };
            } else {
                finalBatchSetter = baseBatchSetter;
            }
            plumber.addPipeline(builder.build(), program, state, finalBatchSetter, itemSetter);
        }
    }
}

void batchSetter(const ShapePipeline& pipeline, gpu::Batch& batch, RenderArgs* args) {
    // Set a default albedo map
    batch.setResourceTexture(gr::Texture::MaterialAlbedo, DependencyManager::get<TextureCache>()->getWhiteTexture());

    // Set a default material
    if (pipeline.locations->materialBufferUnit) {
        // Create a default schema
        static gpu::BufferView schemaBuffer;
        static std::once_flag once;
        std::call_once(once, [] {
            graphics::MultiMaterial::Schema schema;
            graphics::MaterialKey schemaKey;

            schema._albedo = ColorUtils::sRGBToLinearVec3(vec3(1.0f));
            schema._opacity = 1.0f;
            schema._metallic = 0.1f;
            schema._roughness = 0.9f;

            schemaKey.setAlbedo(true);
            schemaKey.setTranslucentFactor(false);
            schemaKey.setMetallic(true);
            schemaKey.setGlossy(true);
            schema._key = (uint32_t)schemaKey._flags.to_ulong();

            auto schemaSize = sizeof(graphics::MultiMaterial::Schema);
            schemaBuffer = gpu::BufferView(std::make_shared<gpu::Buffer>(gpu::Buffer::UniformBuffer, schemaSize, (const gpu::Byte*) &schema, schemaSize));
        });

        batch.setUniformBuffer(gr::Buffer::Material, schemaBuffer);
    }
}

void lightBatchSetter(const ShapePipeline& pipeline, gpu::Batch& batch, RenderArgs* args) {
    // Set the batch
    batchSetter(pipeline, batch, args);

    // Set the light
    if (pipeline.locations->keyLightBufferUnit) {
        DependencyManager::get<DeferredLightingEffect>()->setupKeyLightBatch(args, batch);
    }
}

void initZPassPipelines(ShapePlumber& plumber, gpu::StatePointer state, const render::ShapePipeline::BatchSetter& batchSetter, const render::ShapePipeline::ItemSetter& itemSetter) {
    auto addPipeline = std::bind(&addPlumberPipeline, std::ref(plumber), _1, _2, state, _3, _4);

    for (auto& pipeline : ALL_PIPELINES) {
        if (std::get<0>(pipeline).build().isFaded()) {
            addPipeline(std::get<0>(pipeline), std::get<2>(pipeline), batchSetter, itemSetter);
        } else {
            addPipeline(std::get<0>(pipeline), std::get<2>(pipeline), nullptr, nullptr);
        }
    }
}


void sortAndRenderZPassShapes(const ShapePlumberPointer& shapePlumber, const render::RenderContextPointer& renderContext, const render::ShapeBounds& inShapes, render::ItemBounds &itemBounds) {
    std::unordered_map<ShapeKey, std::vector<ShapeKey>, ShapeKey::Hash, ShapeKey::KeyEqual> sortedShapeKeys;
    std::unordered_map<uint8_t, std::unordered_map<ShapeKey, std::vector<ShapeKey>, ShapeKey::Hash, ShapeKey::KeyEqual>> sortedCustomShapeKeys;
    std::unordered_map<ShapeKey, std::vector<ShapeKey>, ShapeKey::Hash, ShapeKey::KeyEqual> sortedOwnPipelineShapeKeys;

    for (const auto& items : inShapes) {
        itemBounds.insert(itemBounds.end(), items.second.begin(), items.second.end());

        ShapeKey::Builder variantKey = ShapeKey::Builder();

        // The keys we need to check here have to match the ones set up in initZPassPipelines (+ addPlumberPipeline)
        if (items.first.isDeformed()) {
            variantKey.withDeformed();
            if (items.first.isDualQuatSkinned()) {
                variantKey.withDualQuatSkinned();
            }
        }

        if (items.first.isFaded()) {
            variantKey.withFade();
        }

        if (items.first.isMToon()) {
            variantKey.withMToon();
        }

        if (items.first.isCullFace()) {
            variantKey.withCullFaceMode(graphics::MaterialKey::CULL_BACK);
        } else if (items.first.isCullFaceFront()) {
            variantKey.withCullFaceMode(graphics::MaterialKey::CULL_FRONT);
        } else if (items.first.isCullFaceNone()) {
            variantKey.withCullFaceMode(graphics::MaterialKey::CULL_NONE);
        }

        if (items.first.isWireframe()) {
            variantKey.withWireframe();
        }

        if (items.first.isDepthBiased()) {
            variantKey.withDepthBias();
        }

        if (items.first.hasOwnPipeline()) {
            sortedOwnPipelineShapeKeys[variantKey.build()].push_back(items.first);
        } else if (items.first.isCustom()) {
            const uint8_t custom = items.first.getCustom();
            variantKey.withCustom(custom);
            sortedCustomShapeKeys[custom][variantKey.build()].push_back(items.first);
        } else {
            sortedShapeKeys[variantKey.build()].push_back(items.first);
        }
    }

    // Render non-withCustom, non-withOwnPipeline things
    for (const auto& variantAndKeys : sortedShapeKeys) {
        for (const auto& key : variantAndKeys.second) {
            renderShapes(renderContext, shapePlumber, inShapes.at(key));
        }
    }

    // Render withCustom things
    for (const auto& customAndSortedCustomKeys : sortedCustomShapeKeys) {
        for (const auto& variantAndKeys : customAndSortedCustomKeys.second) {
            for (const auto& key : variantAndKeys.second) {
                renderShapes(renderContext, shapePlumber, inShapes.at(key));
            }
        }
    }

    // Render withOwnPipeline things
    for (const auto& variantAndKeys : sortedOwnPipelineShapeKeys) {
        for (const auto& key : variantAndKeys.second) {
            renderShapes(renderContext, shapePlumber, inShapes.at(key));
        }
    }

    renderContext->args->_shapePipeline = nullptr;
    renderContext->args->_batch = nullptr;
}

bool RenderPipelines::bindMaterial(graphics::MaterialPointer& material, gpu::Batch& batch, render::Args::RenderMode renderMode, bool enableTextures) {
    graphics::MultiMaterial multiMaterial;
    multiMaterial.push(graphics::MaterialLayer(material, 0));
    return bindMaterials(multiMaterial, batch, renderMode, enableTextures);
}

void RenderPipelines::updateMultiMaterial(graphics::MultiMaterial& multiMaterial) {
    auto& drawMaterialTextures = multiMaterial.getTextureTable();
    multiMaterial.setTexturesLoading(false);
    multiMaterial.resetReferenceTexturesAndMaterials();
    multiMaterial.setisMToon(!multiMaterial.empty() && multiMaterial.top().material && multiMaterial.top().material->isMToon());
    multiMaterial.resetOutline();

    // The total list of things we need to look for
    static std::set<uint> allFlags;
    static std::once_flag once;
    std::call_once(once, [] {
        for (int i = 0; i < graphics::Material::NUM_TOTAL_FLAGS; i++) {
            // The opacity mask/map are derived from the albedo map
            // FIXME: OPACITY_MAP_MODE_BIT is supposed to support fallthrough
            if (i != graphics::MaterialKey::OPACITY_MASK_MAP_BIT &&
                    i != graphics::MaterialKey::OPACITY_TRANSLUCENT_MAP_BIT &&
                    i != graphics::MaterialKey::OPACITY_MAP_MODE_BIT) {
                allFlags.insert(i);
            }
        }
    });

    graphics::MultiMaterial materials = multiMaterial;
    graphics::MultiMaterial::Schema schema;
    graphics::MultiMaterial::MToonSchema toonSchema;
    graphics::MaterialKey schemaKey;

    std::set<uint> flagsToCheck = allFlags;
    std::set<uint> flagsToSetDefault;

    while (!materials.empty()) {
        auto material = materials.top().material;
        if (!material) {
            break;
        }
        materials.pop();

        if (material->isReference()) {
            multiMaterial.addReferenceMaterial(std::static_pointer_cast<ReferenceMaterial>(material)->getReferenceOperator());
        }

        bool defaultFallthrough = material->getDefaultFallthrough();
        const auto materialKey = material->getKey();
        const auto textureMaps = material->getTextureMaps();

        auto it = flagsToCheck.begin();
        while (it != flagsToCheck.end()) {
            auto flag = *it;
            bool fallthrough = defaultFallthrough || material->getPropertyFallthrough(flag);

            bool wasSet = false;
            bool forceDefault = false;
            if (!multiMaterial.isMToon()) {
                switch (flag) {
                    case graphics::MaterialKey::EMISSIVE_VAL_BIT:
                        if (materialKey.isEmissive()) {
                            schema._emissive = material->getEmissive(false);
                            schemaKey.setEmissive(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::UNLIT_VAL_BIT:
                        if (materialKey.isUnlit()) {
                            schemaKey.setUnlit(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::ALBEDO_VAL_BIT:
                        if (materialKey.isAlbedo()) {
                            schema._albedo = material->getAlbedo(false);
                            schemaKey.setAlbedo(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::METALLIC_VAL_BIT:
                        if (materialKey.isMetallic()) {
                            schema._metallic = material->getMetallic();
                            schemaKey.setMetallic(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::GLOSSY_VAL_BIT:
                        if (materialKey.isRough() || materialKey.isGlossy()) {
                            schema._roughness = material->getRoughness();
                            schemaKey.setGlossy(materialKey.isGlossy());
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::OPACITY_VAL_BIT:
                        if (materialKey.isTranslucentFactor()) {
                            schema._opacity = material->getOpacity();
                            schemaKey.setTranslucentFactor(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::OPACITY_CUTOFF_VAL_BIT:
                        if (materialKey.isOpacityCutoff()) {
                            schema._opacityCutoff = material->getOpacityCutoff();
                            schemaKey.setOpacityCutoff(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::SCATTERING_VAL_BIT:
                        if (materialKey.isScattering()) {
                            schema._scattering = material->getScattering();
                            schemaKey.setScattering(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::ALBEDO_MAP_BIT:
                        if (materialKey.isAlbedoMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::ALBEDO_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    material->resetOpacityMap();
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialAlbedo, itr->second->getTextureView());
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey.setAlbedoMap(true);
                            schemaKey.setOpacityMaskMap(material->getKey().isOpacityMaskMap());
                            schemaKey.setTranslucentMap(material->getKey().isTranslucentMap());
                        }
                        break;
                    case graphics::MaterialKey::METALLIC_MAP_BIT:
                        if (materialKey.isMetallicMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::METALLIC_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialMetallic, itr->second->getTextureView());
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey.setMetallicMap(true);
                        }
                        break;
                    case graphics::MaterialKey::ROUGHNESS_MAP_BIT:
                        if (materialKey.isRoughnessMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::ROUGHNESS_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialRoughness, itr->second->getTextureView());
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey.setRoughnessMap(true);
                        }
                        break;
                    case graphics::MaterialKey::NORMAL_MAP_BIT:
                        if (materialKey.isNormalMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::NORMAL_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialNormal, itr->second->getTextureView());
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey.setNormalMap(true);
                        }
                        break;
                    case graphics::MaterialKey::OCCLUSION_MAP_BIT:
                        if (materialKey.isOcclusionMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::OCCLUSION_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialOcclusion, itr->second->getTextureView());
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey.setOcclusionMap(true);
                        }
                        break;
                    case graphics::MaterialKey::SCATTERING_MAP_BIT:
                        if (materialKey.isScatteringMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::SCATTERING_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialScattering, itr->second->getTextureView());
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey.setScatteringMap(true);
                        }
                        break;
                    case graphics::MaterialKey::EMISSIVE_MAP_BIT:
                        // Lightmap takes precendence over emissive map for legacy reasons
                        if (materialKey.isEmissiveMap() && !materialKey.isLightMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::EMISSIVE_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialEmissiveLightmap, itr->second->getTextureView());
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey.setEmissiveMap(true);
                        } else if (materialKey.isLightMap()) {
                            // We'll set this later when we check the lightmap
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::LIGHT_MAP_BIT:
                        if (materialKey.isLightMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::LIGHT_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialEmissiveLightmap, itr->second->getTextureView());
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey.setLightMap(true);
                        }
                        break;
                    case graphics::Material::TEXCOORDTRANSFORM0:
                        if (!fallthrough) {
                            schema._texcoordTransforms[0] = material->getTexCoordTransform(0);
                            wasSet = true;
                        }
                        break;
                    case graphics::Material::TEXCOORDTRANSFORM1:
                        if (!fallthrough) {
                            schema._texcoordTransforms[1] = material->getTexCoordTransform(1);
                            wasSet = true;
                        }
                        break;
                    case graphics::Material::LIGHTMAP_PARAMS:
                        if (!fallthrough) {
                            schema._lightmapParams = material->getLightmapParams();
                            wasSet = true;
                        }
                        break;
                    case graphics::Material::MATERIAL_PARAMS:
                        if (!fallthrough) {
                            schema._materialParams = material->getMaterialParams();
                            wasSet = true;
                        }
                        break;
                    case graphics::Material::CULL_FACE_MODE:
                        if (!fallthrough) {
                            multiMaterial.setCullFaceMode(material->getCullFaceMode());
                            wasSet = true;
                        }
                        break;
                    default:
                        break;
                }
            } else {
                switch (flag) {
                    case graphics::MaterialKey::EMISSIVE_VAL_BIT:
                        if (materialKey.isEmissive()) {
                            toonSchema._emissive = material->getEmissive(false);
                            schemaKey.setEmissive(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::ALBEDO_VAL_BIT:
                        if (materialKey.isAlbedo()) {
                            toonSchema._albedo = material->getAlbedo(false);
                            schemaKey.setAlbedo(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::OPACITY_VAL_BIT:
                        if (materialKey.isTranslucentFactor()) {
                            toonSchema._opacity = material->getOpacity();
                            schemaKey.setTranslucentFactor(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::OPACITY_CUTOFF_VAL_BIT:
                        if (materialKey.isOpacityCutoff()) {
                            toonSchema._opacityCutoff = material->getOpacityCutoff();
                            schemaKey.setOpacityCutoff(true);
                            wasSet = true;
                        }
                        break;
                    case graphics::MaterialKey::ALBEDO_MAP_BIT:
                        if (materialKey.isAlbedoMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::ALBEDO_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    material->resetOpacityMap();
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialAlbedo, itr->second->getTextureView());
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey.setAlbedoMap(true);
                            schemaKey.setOpacityMaskMap(material->getKey().isOpacityMaskMap());
                            schemaKey.setTranslucentMap(material->getKey().isTranslucentMap());
                        }
                        break;
                    case graphics::MaterialKey::NORMAL_MAP_BIT:
                        if (materialKey.isNormalMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::NORMAL_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialNormal, itr->second->getTextureView());
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey.setNormalMap(true);
                        }
                        break;
                    case graphics::MaterialKey::EMISSIVE_MAP_BIT:
                        // Lightmap takes precendence over emissive map for legacy reasons
                        if (materialKey.isEmissiveMap() && !materialKey.isLightMap()) {
                            auto itr = textureMaps.find(graphics::MaterialKey::EMISSIVE_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialEmissiveLightmap, itr->second->getTextureView());
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey.setEmissiveMap(true);
                        } else if (materialKey.isLightMap()) {
                            // We'll set this later when we check the lightmap
                            wasSet = true;
                        }
                        break;
                    case graphics::Material::TEXCOORDTRANSFORM0:
                        if (!fallthrough) {
                            toonSchema._texcoordTransforms[0] = material->getTexCoordTransform(0);
                            wasSet = true;
                        }
                        break;
                    case graphics::Material::TEXCOORDTRANSFORM1:
                        if (!fallthrough) {
                            toonSchema._texcoordTransforms[1] = material->getTexCoordTransform(1);
                            wasSet = true;
                        }
                        break;
                    case graphics::Material::MATERIAL_PARAMS:
                        if (!fallthrough) {
                            toonSchema._materialParams = material->getMaterialParams();
                            wasSet = true;
                        }
                        break;
                    case graphics::Material::CULL_FACE_MODE:
                        if (!fallthrough) {
                            multiMaterial.setCullFaceMode(material->getCullFaceMode());
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::SHADE_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::SHADE_VAL_BIT]) {
                            toonSchema._shade = material->getShade();
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::SHADE_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_VAL_BIT]) {
                            toonSchema._shadingShift = material->getShadingShift();
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::SHADING_TOONY_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::SHADING_TOONY_VAL_BIT]) {
                            toonSchema._shadingToony = material->getShadingToony();
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::SHADING_TOONY_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_SCROLL_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_SCROLL_VAL_BIT]) {
                            toonSchema._uvAnimationScrollSpeed.x = material->getUVAnimationScrollXSpeed();
                            toonSchema._uvAnimationScrollSpeed.y = material->getUVAnimationScrollYSpeed();
                            toonSchema._uvAnimationScrollRotationSpeed = material->getUVAnimationRotationSpeed();
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_SCROLL_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::SHADE_MAP_BIT :
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::SHADE_MAP_BIT]) {
                            auto itr = textureMaps.find((graphics::Material::MapChannel) NetworkMToonMaterial::SHADE_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialShade, itr->second->getTextureView());
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::SHADE_MAP_BIT, true);
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_MAP_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_MAP_BIT]) {
                            auto itr = textureMaps.find((graphics::Material::MapChannel) NetworkMToonMaterial::SHADING_SHIFT_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialShadingShift, itr->second->getTextureView());
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_MAP_BIT, true);
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::MATCAP_MAP_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::MATCAP_MAP_BIT]) {
                            auto itr = textureMaps.find((graphics::Material::MapChannel) NetworkMToonMaterial::MATCAP_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialMatcap, itr->second->getTextureView());
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::MATCAP_MAP_BIT, true);
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::RIM_MAP_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::RIM_MAP_BIT]) {
                            auto itr = textureMaps.find((graphics::Material::MapChannel) NetworkMToonMaterial::RIM_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialRim, itr->second->getTextureView());
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::RIM_MAP_BIT, true);
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_MASK_MAP_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_MASK_MAP_BIT]) {
                            auto itr = textureMaps.find((graphics::Material::MapChannel) NetworkMToonMaterial::UV_ANIMATION_MASK_MAP);
                            if (itr != textureMaps.end()) {
                                if (itr->second->isDefined()) {
                                    drawMaterialTextures->setTexture(gr::Texture::MaterialUVAnimationMask, itr->second->getTextureView());
                                    if (itr->second->getTextureView().isReference()) {
                                        multiMaterial.addReferenceTexture(itr->second->getTextureView().getTextureOperator());
                                    }
                                    wasSet = true;
                                } else {
                                    multiMaterial.setTexturesLoading(true);
                                    forceDefault = true;
                                }
                            } else {
                                forceDefault = true;
                            }
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_MASK_MAP_BIT, true);
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::MATCAP_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::MATCAP_VAL_BIT]) {
                            toonSchema._matcap = material->getMatcap(false);
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::MATCAP_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_VAL_BIT]) {
                            toonSchema._parametricRim = material->getParametricRim(false);
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_POWER_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_POWER_VAL_BIT]) {
                            toonSchema._parametricRimFresnelPower = material->getParametricRimFresnelPower();
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_POWER_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_LIFT_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_LIFT_VAL_BIT]) {
                            toonSchema._parametricRimLift = material->getParametricRimLift();
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::PARAMETRIC_RIM_LIFT_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::RIM_LIGHTING_MIX_VAL_BIT:
                        if (materialKey._flags[NetworkMToonMaterial::MToonFlagBit::RIM_LIGHTING_MIX_VAL_BIT]) {
                            toonSchema._rimLightingMix = material->getRimLightingMix();
                            schemaKey._flags.set(NetworkMToonMaterial::MToonFlagBit::RIM_LIGHTING_MIX_VAL_BIT, true);
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::OUTLINE_WIDTH_MODE_VAL_BIT:
                        if (!fallthrough) {
                            multiMaterial.setOutlineWidthMode(material->getOutlineWidthMode());
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::OUTLINE_WIDTH_VAL_BIT:
                        if (!fallthrough) {
                            multiMaterial.setOutlineWidth(material->getOutlineWidth());
                            wasSet = true;
                        }
                        break;
                    case NetworkMToonMaterial::MToonFlagBit::OUTLINE_VAL_BIT:
                        if (!fallthrough) {
                            multiMaterial.setOutline(material->getOutline(false));
                            wasSet = true;
                        }
                        break;
                    default:
                        break;
                }
            }

            if (wasSet) {
                flagsToCheck.erase(it++);
            } else if (forceDefault || !fallthrough) {
                flagsToSetDefault.insert(flag);
                flagsToCheck.erase(it++);
            } else {
                ++it;
            }
        }

        if (flagsToCheck.empty()) {
            break;
        }
    }

    for (auto flagBit : flagsToCheck) {
        flagsToSetDefault.insert(flagBit);
    }

    auto textureCache = DependencyManager::get<TextureCache>();
    // Handle defaults
    for (auto flag : flagsToSetDefault) {
        if (!multiMaterial.isMToon()) {
            switch (flag) {
                case graphics::Material::CULL_FACE_MODE:
                    multiMaterial.setCullFaceMode(graphics::Material::DEFAULT_CULL_FACE_MODE);
                    break;
                case graphics::MaterialKey::ALBEDO_MAP_BIT:
                    if (schemaKey.isAlbedoMap()) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialAlbedo, textureCache->getWhiteTexture());
                    }
                    break;
                case graphics::MaterialKey::METALLIC_MAP_BIT:
                    if (schemaKey.isMetallicMap()) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialMetallic, textureCache->getBlackTexture());
                    }
                    break;
                case graphics::MaterialKey::ROUGHNESS_MAP_BIT:
                    if (schemaKey.isRoughnessMap()) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialRoughness, textureCache->getWhiteTexture());
                    }
                    break;
                case graphics::MaterialKey::NORMAL_MAP_BIT:
                    if (schemaKey.isNormalMap()) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialNormal, textureCache->getBlueTexture());
                    }
                    break;
                case graphics::MaterialKey::OCCLUSION_MAP_BIT:
                    if (schemaKey.isOcclusionMap()) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialOcclusion, textureCache->getWhiteTexture());
                    }
                    break;
                case graphics::MaterialKey::SCATTERING_MAP_BIT:
                    if (schemaKey.isScatteringMap()) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialScattering, textureCache->getWhiteTexture());
                    }
                    break;
                case graphics::MaterialKey::EMISSIVE_MAP_BIT:
                    if (schemaKey.isEmissiveMap() && !schemaKey.isLightMap()) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialEmissiveLightmap, textureCache->getGrayTexture());
                    }
                    break;
                case graphics::MaterialKey::LIGHT_MAP_BIT:
                    if (schemaKey.isLightMap()) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialEmissiveLightmap, textureCache->getBlackTexture());
                    }
                    break;
                default:
                    // everything else is initialized to the correct default values in Schema()
                    break;
            }
        } else {
            switch (flag) {
                case graphics::Material::CULL_FACE_MODE:
                    multiMaterial.setCullFaceMode(graphics::Material::DEFAULT_CULL_FACE_MODE);
                    break;
                case graphics::MaterialKey::ALBEDO_MAP_BIT:
                    if (schemaKey.isAlbedoMap()) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialAlbedo, textureCache->getWhiteTexture());
                    }
                    break;
                case graphics::MaterialKey::NORMAL_MAP_BIT:
                    if (schemaKey.isNormalMap()) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialNormal, textureCache->getBlueTexture());
                    }
                    break;
                case graphics::MaterialKey::EMISSIVE_MAP_BIT:
                    if (schemaKey.isEmissiveMap() && !schemaKey.isLightMap()) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialEmissiveLightmap, textureCache->getGrayTexture());
                    }
                    break;
                case NetworkMToonMaterial::MToonFlagBit::SHADE_MAP_BIT:
                    if (schemaKey._flags[NetworkMToonMaterial::MToonFlagBit::SHADE_MAP_BIT]) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialShade, textureCache->getWhiteTexture());
                    }
                    break;
                case NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_MAP_BIT:
                    if (schemaKey._flags[NetworkMToonMaterial::MToonFlagBit::SHADING_SHIFT_MAP_BIT]) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialShadingShift, textureCache->getBlackTexture());
                    }
                    break;
                case NetworkMToonMaterial::MToonFlagBit::MATCAP_MAP_BIT:
                    if (schemaKey._flags[NetworkMToonMaterial::MToonFlagBit::MATCAP_MAP_BIT]) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialMatcap, textureCache->getBlackTexture());
                    }
                    break;
                case NetworkMToonMaterial::MToonFlagBit::RIM_MAP_BIT:
                    if (schemaKey._flags[NetworkMToonMaterial::MToonFlagBit::RIM_MAP_BIT]) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialRim, textureCache->getWhiteTexture());
                    }
                    break;
                case NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_MASK_MAP_BIT:
                    if (schemaKey._flags[NetworkMToonMaterial::MToonFlagBit::UV_ANIMATION_MASK_MAP_BIT]) {
                        drawMaterialTextures->setTexture(gr::Texture::MaterialUVAnimationMask, textureCache->getWhiteTexture());
                    }
                    break;
                default:
                    // everything else is initialized to the correct default values in ToonSchema()
                    break;
            }
        }
    }

    auto& schemaBuffer = multiMaterial.getSchemaBuffer();
    if (multiMaterial.isMToon()) {
        toonSchema._key = (uint32_t)schemaKey._flags.to_ulong();
        schemaBuffer.edit<graphics::MultiMaterial::MToonSchema>() = toonSchema;
    } else {
        schema._key = (uint32_t)schemaKey._flags.to_ulong();
        schemaBuffer.edit<graphics::MultiMaterial::Schema>() = schema;
    }
    multiMaterial.setNeedsUpdate(false);
    multiMaterial.setInitialized();
}

bool RenderPipelines::bindMaterials(graphics::MultiMaterial& multiMaterial, gpu::Batch& batch, render::Args::RenderMode renderMode, bool enableTextures) {
    if (multiMaterial.shouldUpdate()) {
        updateMultiMaterial(multiMaterial);
    }

    if (multiMaterial.isMToon()) {
        multiMaterial.setMToonTime();
    }

    auto textureCache = DependencyManager::get<TextureCache>();

    static gpu::TextureTablePointer defaultMaterialTextures = std::make_shared<gpu::TextureTable>();
    static gpu::BufferView defaultMaterialSchema;
    static gpu::TextureTablePointer defaultMToonMaterialTextures = std::make_shared<gpu::TextureTable>();
    static gpu::BufferView defaultMToonMaterialSchema;

    static std::once_flag once;
    std::call_once(once, [textureCache] {
        graphics::MultiMaterial::Schema schema;
        defaultMaterialSchema = gpu::BufferView(std::make_shared<gpu::Buffer>(gpu::Buffer::UniformBuffer, sizeof(schema), (const gpu::Byte*) &schema, sizeof(schema)));

        defaultMaterialTextures->setTexture(gr::Texture::MaterialAlbedo, textureCache->getWhiteTexture());
        defaultMaterialTextures->setTexture(gr::Texture::MaterialMetallic, textureCache->getBlackTexture());
        defaultMaterialTextures->setTexture(gr::Texture::MaterialRoughness, textureCache->getWhiteTexture());
        defaultMaterialTextures->setTexture(gr::Texture::MaterialNormal, textureCache->getBlueTexture());
        defaultMaterialTextures->setTexture(gr::Texture::MaterialOcclusion, textureCache->getWhiteTexture());
        defaultMaterialTextures->setTexture(gr::Texture::MaterialScattering, textureCache->getWhiteTexture());
        // MaterialEmissiveLightmap has to be set later

        graphics::MultiMaterial::MToonSchema toonSchema;
        defaultMToonMaterialSchema = gpu::BufferView(std::make_shared<gpu::Buffer>(sizeof(toonSchema), (const gpu::Byte*) &toonSchema, sizeof(toonSchema)));

        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialAlbedo, textureCache->getWhiteTexture());
        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialNormal, textureCache->getBlueTexture());
        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialEmissiveLightmap, textureCache->getGrayTexture());
        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialShade, textureCache->getWhiteTexture());
        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialShadingShift, textureCache->getBlackTexture());
        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialMatcap, textureCache->getBlackTexture());
        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialRim, textureCache->getWhiteTexture());
        defaultMToonMaterialTextures->setTexture(gr::Texture::MaterialUVAnimationMask, textureCache->getWhiteTexture());
    });

    // For shadows, we only need opacity mask information
    auto key = multiMaterial.getMaterialKey();
    if (renderMode != render::Args::RenderMode::SHADOW_RENDER_MODE || (key.isOpacityMaskMap() || key.isTranslucentMap())) {
        auto& schemaBuffer = multiMaterial.getSchemaBuffer();
        batch.setUniformBuffer(gr::Buffer::Material, schemaBuffer);
        if (enableTextures) {
            batch.setResourceTextureTable(multiMaterial.getTextureTable());
        } else {
            if (!multiMaterial.isMToon()) {
                if (renderMode != render::Args::RenderMode::SHADOW_RENDER_MODE) {
                    if (key.isLightMap()) {
                        defaultMaterialTextures->setTexture(gr::Texture::MaterialEmissiveLightmap, textureCache->getBlackTexture());
                    } else if (key.isEmissiveMap()) {
                        defaultMaterialTextures->setTexture(gr::Texture::MaterialEmissiveLightmap, textureCache->getGrayTexture());
                    }
                }

                batch.setResourceTextureTable(defaultMaterialTextures);
            } else {
                batch.setResourceTextureTable(defaultMToonMaterialTextures);
            }
        }
        return true;
    } else {
        batch.setResourceTextureTable(!multiMaterial.isMToon() ? defaultMaterialTextures : defaultMToonMaterialTextures);
        batch.setUniformBuffer(gr::Buffer::Material, !multiMaterial.isMToon() ? defaultMaterialSchema : defaultMToonMaterialSchema);
        return false;
    }
}

