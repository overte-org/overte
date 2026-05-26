//
//  Created by Bradley Austin Davis on 2016/07/26
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_Frame_h
#define hifi_gpu_Frame_h

#include <functional>
#include <queue>

#include "Forward.h"
#include "Batch.h"
#include "Resource.h"

namespace gpu {

    class Frame {
        friend class Context;
        friend class Backend;

    public:
        Frame();
        virtual ~Frame();
        using Batches = std::vector<BatchPointer>;
        using FramebufferRecycler = std::function<void(const FramebufferPointer&)>;
        using OverlayRecycler = std::function<void(const TexturePointer&)>;

        StereoState stereoState;
        uint32_t frameIndex{ 0 };
        // TODO: view doesn't seem to be used anywhere?
        /// The view matrix used for rendering the frame, only applicable for HMDs.
        Mat4 view;
        /// The sensor pose used for rendering the frame, only applicable for HMDs.
        Mat4 pose;
        /// The collection of batches which make up the frame.
        Batches batches;
        /// The main thread updates to buffers that are applicable for this frame.
        BufferUpdates bufferUpdates;
        /// The destination framebuffer in which the results will be placed.
        FramebufferPointer framebuffer;
        /// How to process the framebuffer when the frame dies.  MUST BE THREAD SAFE.
        FramebufferRecycler framebufferRecycler;

        std::queue<std::tuple<std::function<void(const QImage&)>, float, bool>> snapshotOperators;

    protected:
        friend class Deserializer;

        /**
         * @brief Finalize the frame after all the draw call commands have been collected.
         *
         * Should be called once per frame, on the recording thread.
         * Applies all the named calls to the end of the batch (for instancing) and prepares updates for the render shadow copies of the buffers.
         */
        void finish();

        /**
         * @brief Applies buffer updates for this frame.
         * Called by `Context::consumeFrameUpdates` on the Present thread (which does the rendering).
         */
        void preRender();
    };

};


#endif
