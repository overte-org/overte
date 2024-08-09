//
//  Created by Bradley Austin Davis on 2018/10/21
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once

#include <QtCore/QElapsedTimer>
#include <QtGui/QWindow>

#include <GenericThread.h>
#include <shared/RateCounter.h>

#include <vk/Config.h>
#include <vk/Context.h>

#ifdef USE_GL
#include <gl/Config.h>
#include <gl/Context.h>
#include <gpu/gl/GLBackend.h>
#else
#include <gpu/vk/VKBackend.h>
#include <vk/VulkanSwapChain.h>
#endif

class RenderThread : public GenericThread {
    using Parent = GenericThread;
public:
    QWindow* _window{ nullptr };

    vks::Context& _vkcontext{ vks::Context::get() };
    // TODO: is the _vkcontext.device->logicalDevice created at the time?
    const VkDevice& _vkdevice{ _vkcontext.device->logicalDevice };
    vks::Buffer _vkstagingBuffer;

#ifdef USE_GL
    gl::Context _context;
#else

    //VkSurfaceKHR _surface;
    VkRenderPass _renderPass;
    VulkanSwapChain _swapchain;
    VkSemaphore acquireComplete, renderComplete;
    std::vector<VkFramebuffer> _framebuffers;
    VkExtent2D _extent;

    void setupFramebuffers();
    void setupRenderPass();
#endif

    std::mutex _mutex;
    gpu::ContextPointer _gpuContext;  // initialized during window creation
    std::shared_ptr<gpu::Backend> _backend;
    std::atomic<size_t> _presentCount{ 0 };
    QElapsedTimer _elapsed;
    size_t _frameIndex{ 0 };
    std::mutex _frameLock;
    std::queue<gpu::FramePointer> _pendingFrames;
    std::queue<QSize> _pendingSize;
    gpu::FramePointer _activeFrame;
    uint32_t _externalTexture{ 0 };
    void move(const glm::vec3& v);
    glm::mat4 _correction;
    gpu::PipelinePointer _presentPipeline;

    void resize(const QSize& newSize);
    void setup() override;
    bool process() override;
    void shutdown() override;
#ifdef USE_GL
    void testGlTransfer();
#else
    void testVkTransfer();
#endif
    void submitFrame(const gpu::FramePointer& frame);
    void initialize(QWindow* window);
    void renderFrame(gpu::FramePointer& frame);
};
