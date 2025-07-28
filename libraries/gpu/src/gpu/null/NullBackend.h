//
//  Created by Bradley Austin Davis on 2016/05/16
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_Null_Backend_h
#define hifi_gpu_Null_Backend_h

#include <assert.h>
#include <functional>
#include <bitset>
#include <queue>
#include <utility>
#include <list>
#include <array>

#include <QtCore/QLoggingCategory>

#include "../Context.h"

namespace gpu { namespace null {

class Backend : public gpu::Backend {
    using Parent = gpu::Backend;
    // Context Backend static interface required
    friend class gpu::Context;
    static void init() {}
    static gpu::Backend* createBackend() { return new Backend(); }

protected:
    explicit Backend(bool syncCache) : Parent() { }
    Backend() : Parent() { }
public:
    ~Backend() { }

    void executeFrame(const FramePointer& frame) {}
    
    // This call synchronize the Full Backend cache with the current GLState
    // THis is only intended to be used when mixing raw gl calls with the gpu api usage in order to sync
    // the gpu::Backend state with the true gl state which has probably been messed up by these ugly naked gl calls
    // Let's try to avoid to do that as much as possible!
    void syncCache() final { }

    void syncProgram(const gpu::ShaderPointer& program) final {}

    // This is the ugly "download the pixels to sysmem for taking a snapshot"
    // Just avoid using it, it's ugly and will break performances
    virtual void downloadFramebuffer(const FramebufferPointer& srcFramebuffer, const Vec4i& region, QImage& destImage) final { }
};

} }

#endif
