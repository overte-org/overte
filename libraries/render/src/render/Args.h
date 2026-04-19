//
//  Created by Brad Hefta-Gaub on 10/29/14.
//  Copyright 2013-2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once
#ifndef hifi_render_Args_h
#define hifi_render_Args_h

#include <functional>
#include <memory>
#include <stack>
#include <unordered_set>

#include <GLMHelpers.h>
#include <ViewFrustum.h>
#include <StencilMaskMode.h>
#include <UUIDHasher.h>

#include <gpu/Forward.h>
#include "Forward.h"

class AABox;

namespace render {
    /// Used for gathering statistics about culling in current frame.
    class RenderDetails {
    public:
        /// Statistics are generated separately for culling in different render modes.
        enum Type {
            /// Used during regular culling.
            ITEM,

            /// Used during culling items for shadow bounds.
            SHADOW,

            /// Not used currently.
            OTHER
        };

        /// Object containing culling statistics.
        struct Item {
            /// Items considered for culling.
            int _considered = 0;

            /// Items outside of view frustum.
            int _outOfView = 0;

            /// Items that are too small based on current LOD settings.
            int _tooSmall = 0;

            /// Items that will be rendered.
            int _rendered = 0;
        };

        /// Number of material switches during this frame.
        int _materialSwitches = 0;

        /// Number of triangles rendered during this frame.
        int _trianglesRendered = 0;

        /// Used during regular culling.
        Item _item;

        /// Used during culling items for shadow bounds.
        Item _shadow;

        /// Not used currently.
        Item _other;

        /**
         * @brief Gets an editable reference to statistics for a requested render mode.
         *
         * If type is not valid, it will be considered as `Type::Other`.
         * @param type Mode in which items are drawn.
         * @return Editable reference to statistics for a given render mode.
         */
        Item& edit(Type type) {
            switch (type) {
                case SHADOW:
                    return _shadow;
                case ITEM:
                    return _item;
                default:
                    return _other;
            }
        }
    };


    /**
     * Used by `AppRenderArgs` class in `GraphicsEngine`.
     * They contain current frame setup and some statistics.
     * During some of the rendering steps, render arguments are modified and then restored, for example
     * during rendering of mirrored views.
     */
    class Args {
    public:
        enum RenderMode {
            /// Used for regular view.
            DEFAULT_RENDER_MODE,

            /// Used for shadows and maybe highlight effect(?).
            SHADOW_RENDER_MODE,

            /// This is for the mirror camera mode for the main camera and not for actual mirrors in-game.
            MIRROR_RENDER_MODE,

            /// Secondary camera can render to texture for effects such as mirrors or to screen.
            SECONDARY_CAMERA_RENDER_MODE
        };

        enum DisplayMode { MONO, STEREO_MONITOR, STEREO_HMD };

        enum RenderMethod { DEFERRED, FORWARD };

        Args() {}

        /**
         *
         * @param context Shared pointer to the GPU context.
         * @param sizeScale Refer to _sizeScale comment.
         * @param boundaryLevelAdjust A coefficient related to octree depth.
         * @param lodFarAngleHalfTan Precomputed half tangent of angle threshold for size-based culling. Applied at distance `_lodFarDist`.
         * @param lodNearAngleHalfTan Precomputed half tangent of angle threshold for size-based culling. Applied at distance `_lodNearDist`.
         * @param lodFarDist Distance in meters at which `_lodFarAngleHalfTan` applies for size-based culling.
         * @param lodNearDist Distance in meters at which `_lodNearAngleHalfTan` applies for size-based culling.
         * @param renderMode `DEFAULT_RENDER_MODE` is always passed here.
         * @param displayMode TODO: It's always set to MONO in Application::updateRenderArgs() but then set to correct value soon after.
         * @param renderMethod TODO: It's always set to `DEFERRED`?
         * @param batch `nullptr` is passed here.
         */
        Args(const gpu::ContextPointer& context,
             float sizeScale = 1.0f,
             int boundaryLevelAdjust = 0,
             float lodFarAngleHalfTan = 0.1f,
             float lodNearAngleHalfTan = 0.01f,
             float lodFarDist = 200.0f,
             float lodNearDist = 4.0f,
             RenderMode renderMode = DEFAULT_RENDER_MODE,
             DisplayMode displayMode = MONO,
             RenderMethod renderMethod = DEFERRED, // TODO: Application_Grapics.cpp always passes DEFERRED here, even when forward renderer is used. Maybe it's better to not have a parameter in the constructor at all?
             gpu::Batch* batch = nullptr) :
            _context(context),
            _sizeScale(sizeScale),
            _boundaryLevelAdjust(boundaryLevelAdjust),
            _lodFarAngleHalfTan(lodFarAngleHalfTan),
            _lodFarAngleHalfTanSq(lodFarAngleHalfTan * lodFarAngleHalfTan),
            _lodNearAngleHalfTan(lodNearAngleHalfTan),
            _lodNearAngleHalfTanSq(lodNearAngleHalfTan * lodNearAngleHalfTan),
            _lodFarDist(lodFarDist),
            _lodNearDist(lodNearDist),
            _lodFarDistSq(lodFarDist * lodFarDist),
            _lodNearDistSq(lodNearDist * lodNearDist),
            _renderMode(renderMode),
            _displayMode(displayMode),
            _renderMethod(renderMethod),
            _batch(batch) {
        }

        /**
         * @return `true` if there is one or more frustums stored currently.
         */
        bool hasViewFrustum() const { return _viewFrustums.size() > 0; }

        /**
         * @brief Removes currently stored view frustums and adds a given one.
         *
         * @param viewFrustum View frustum to add.
         */
        void setViewFrustum(const ViewFrustum& viewFrustum) {
            while (_viewFrustums.size() > 0) {
                _viewFrustums.pop();
            }
            _viewFrustums.push(viewFrustum);
        }

        /**
         * @brief Returns reference to the top view frustum on the stack.
         *
         * This function should never be called when frustum stack is empty.
         * @return Top view frustum from the stack.
         */
        const ViewFrustum& getViewFrustum() const { assert(_viewFrustums.size() > 0); return _viewFrustums.top(); }

        /**
         * @brief Adds a view frustum to the top of the stack.
         *
         * Used for mirrors, secondary camera, shadow cameras and debugging purposes.
         * @param viewFrustum View frustum to be added to the stack.
         */
        void pushViewFrustum(const ViewFrustum& viewFrustum) { _viewFrustums.push(viewFrustum); }

        /**
         * @brief Removes the top view frustum from the stack.
         *
         * Should never be called on empty stack.
         */
        void popViewFrustum() { _viewFrustums.pop(); }

        /**
         * @return True if view is rendered in stereo (both HMD and desktop 3D displays).
         */
        bool isStereo() const { return _displayMode != MONO; }

        std::shared_ptr<gpu::Context> _context;

        /// Framebuffer where rendered image will be stored.
        /// For regular frame it's the final framebuffer. It's temporarily changed to other buffers when rendering
        /// mirrors and for secondary camera.
        std::shared_ptr<gpu::Framebuffer> _blitFramebuffer;

        /// Temporarily set to a proper pipeline when generating batch commands to render shapes.
        std::shared_ptr<render::ShapePipeline> _shapePipeline;

        /// Stores view frustums.
        /// Storing multiple frustums is required when tasks such as generating render batches for mirrors, shadows
        /// and secondary camera are executed.
        std::stack<ViewFrustum> _viewFrustums;

        /// Viewport rectangle in pixels.
        glm::ivec4 _viewport { 0.0f, 0.0f, 1.0f, 1.0f };

        /// DOCTODO: this is very confusing.
        /// When Args are created, lodManager->getVisibilityDistance() is put here, even though variable is called scale.
        /// It seems to be used only when fetching spatial tree for orthographic camera and for drawing LOD reticle.
        float _sizeScale { 1.0f };

        /// A coefficient related to octree depth.
        /// DOCTODO: needs to be documented in LODManager.
        int _boundaryLevelAdjust { 0 };

        /// Precomputed half tangent of angle threshold for size-based culling.
        /// Applied at distance `_lodFarDist`.
        float _lodFarAngleHalfTan{ 0.1f };

        /// See `_lodFarAngleHalfTan`.
        float _lodFarAngleHalfTanSq{ _lodFarAngleHalfTan  * _lodFarAngleHalfTan };

        /// Precomputed half tangent of angle threshold for size-based culling.
        /// Applied at distance `_lodNearDist`.
        float _lodNearAngleHalfTan{ 0.01f };

        /// See `_lodFarAngleHalfTan`.
        float _lodNearAngleHalfTanSq{ _lodNearAngleHalfTan  * _lodNearAngleHalfTan };

        /// Distance in meters at which `_lodFarAngleHalfTan` applies for size-based culling.
        /// For distances between `_lodFarDist` and `_lodNearDist` the value is interpolated between near and far value.
        float _lodFarDist { 200.0f };

        /// Distance in meters at which `_lodNearAngleHalfTan` applies for size-based culling.
        float _lodNearDist { 4.0f };

        /// See `_lodNearDist`.
        float _lodFarDistSq { _lodFarDist * _lodFarDist };

        /// See `_lodFarDist`.
        float _lodNearDistSq { _lodNearDist * _lodNearDist };

        /// Current render mode for a given task.
        /// During execution of tasks that generate batch commands this is changed to reflect current task and
        /// then restored. This happens for example for shadow rendering task.
        RenderMode _renderMode { DEFAULT_RENDER_MODE };

        /// Current display mode. It can be temporarily overridden for secondary camera task and then restored when task is done.
        DisplayMode _displayMode { MONO };

        /// Current render method.
        /// Sometimes it's overridden locally and then restored, for example in `DrawLayered3D::run`.
        /// It's also set by SetRenderMethod job, for example in `RenderForwardTask` and `RenderShadowsAndDeferredTask`.
        RenderMethod _renderMethod { DEFERRED };

        /// Current batch to which the commands will be written.
        gpu::Batch* _batch = nullptr;

        /// Contains shared settings for all rendered shapes.
        /// Currently used to render all shapes as wireframe.
        uint32_t _globalShapeKey { 0 };

        /// Settings for the currently rendered shape.
        /// It's set before generating batch commands for rendering a shape and set to 0 after it's finished.
        uint32_t _itemShapeKey { 0 };

        /// Set to `false` in `MakeLightingModel` task if texturing is disabled.
        bool _enableTexturing { true };

        /// Set to `false` in `MakeLightingModel` task if blendshapes are disabled.
        bool _enableBlendshape { true };

        /// Set to `false` in `MakeLightingModel` task if skinning is disabled.
        bool _enableSkinning { true };

        /// Contains statistics about culling in current frame.
        RenderDetails _details;

        /// Pointer to the main 3D scene.
        render::ScenePointer _scene;

        /// Current camera mode for the primary camera.
        /// It's set once before command generation for a given frame starts.
        int8_t _cameraMode { -1 };

        /// Used to generate commands for drawing HUD.
        std::function<void(gpu::Batch&, const gpu::TexturePointer&)> _hudOperator { nullptr };

        /// Texture that will be drawn as HUD.
        gpu::TexturePointer _hudTexture { nullptr };

        /// `true` when snapshot is being taken on primary camera.
        /// `false` when snapshot is being taken on secondary camera or not taken at all.
        bool _takingSnapshot { false };

        /// Used for view stencil in VR.
        /// Temporarily changed to StencilMaskMode::NONE for secondary camera and then set back to previous value.
        StencilMaskMode _stencilMaskMode { StencilMaskMode::NONE };

        /// Function that adds batch commands to draw stencil mesh for VR views.
        std::function<void(gpu::Batch&)> _stencilMaskOperator;

        /// Used to avoid drawing mirror or portal item.
        /// Set temporarily for mirror/portal views and then restored.
        ItemID _ignoreItem { 0 };

        /// Current mirror depth. 0 for main view, 1 for a view reflected once so on.
        size_t _mirrorDepth { 0 };

        /// Incremented on each reflection. Not incremented for portals.
        size_t _numMirrorFlips { 0 };
    };

}

using RenderArgs = render::Args;

#endif // hifi_render_Args_h
