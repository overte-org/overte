//
//  Created by Bradley Austin Davis on 2018/10/21
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "RenderThread.h"
#include <QtGui/QWindow>
#include <QtCore/QThreadPool>
#include <QtX11Extras/QX11Info>
//#include <QVulkanInstance>
#ifdef USE_GL
#include <gl/OffscreenGLCanvas.h>
#include <gl/QOpenGLContextWrapper.h>
#endif

#include <gpu/vk/VKFramebuffer.h>

#include <dlfcn.h>
#include "/home/ksuprynowicz/programy/renderdoc_1.35/include/renderdoc_app.h"
RENDERDOC_API_1_1_2 *rdoc_api = NULL;

void RenderThread::submitFrame(const gpu::FramePointer& frame) {
    std::unique_lock<std::mutex> lock(_frameLock);
    _pendingFrames.push(frame);
}

void RenderThread::resize(const QSize& newSize) {
    std::unique_lock<std::mutex> lock(_frameLock);
    _pendingSize.push(newSize);
}

void RenderThread::move(const glm::vec3& v) {
    std::unique_lock<std::mutex> lock(_frameLock);
    _correction = glm::inverse(glm::translate(mat4(), v)) * _correction;
}

void RenderThread::initialize(QWindow* window) {
    if(void *mod = dlopen("/home/ksuprynowicz/programy/renderdoc_1.35/lib/librenderdoc.so", RTLD_NOW | RTLD_NOLOAD))
    {
        pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)dlsym(mod, "RENDERDOC_GetAPI");
        int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&rdoc_api);
        assert(ret == 1);
    }

    std::unique_lock<std::mutex> lock(_frameLock);
    setObjectName("RenderThread");
    Parent::initialize();

    _window = window;
    _vkcontext.setValidationEnabled(true);

#ifdef USE_GL
    _vkcontext.createInstance();
    _vkcontext.createDevice();
    _vkstagingBuffer = _vkcontext.createStagingBuffer(1024 * 1024 * 512, nullptr);

    _window->setFormat(getDefaultOpenGLSurfaceFormat());
    _context.setWindow(window);
    _context.create();
    if (!_context.makeCurrent()) {
        qFatal("Unable to make context current");
    }
    QOpenGLContextWrapper(_context.qglContext()).makeCurrent(_window);
    glGenTextures(1, &_externalTexture);
    glBindTexture(GL_TEXTURE_2D, _externalTexture);
    static const glm::u8vec4 color{ 0 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &color);
    gl::setSwapInterval(0);
    // GPU library init
    gpu::Context::init<gpu::gl::GLBackend>();
    _context.makeCurrent();
    _gpuContext = std::make_shared<gpu::Context>();
    _backend = _gpuContext->getBackend();
    _context.doneCurrent();
    _context.moveToThread(_thread);
    
    if (!_presentPipeline) {
        gpu::ShaderPointer program = gpu::Shader::createProgram(shader::gpu::program::DrawTexture);
        gpu::StatePointer state = std::make_shared<gpu::State>();
        _presentPipeline = gpu::Pipeline::create(program, state);
    }
#else
    auto size = window->size();
    _extent = VkExtent2D{ (uint32_t)size.width(), (uint32_t)size.height() };

    _vkcontext.setValidationEnabled(true);
    _vkcontext.requireExtensions({
        std::string{ VK_KHR_SURFACE_EXTENSION_NAME },
#ifdef WIN32
        std::string{ VK_KHR_WIN32_SURFACE_EXTENSION_NAME },
#else
        std::string{ VK_KHR_XCB_SURFACE_EXTENSION_NAME },
#endif
    });
    _vkcontext.requireDeviceExtensions({ VK_KHR_SWAPCHAIN_EXTENSION_NAME });
    _vkcontext.createInstance();
#ifdef WIN32
    _surface = _vkcontext.instance.createWin32SurfaceKHR({ {}, GetModuleHandle(NULL), (HWND)window->winId() });
#else
    _vkcontext.createDevice();
    _swapchain.setContext(&_vkcontext);
    _swapchain.initSurface(QX11Info::connection(), (xcb_window_t)(window->winId()));
#endif
    _swapchain.create(&_extent.width, &_extent.height, true);

    setupRenderPass();
    setupFramebuffers();

    VkSemaphoreCreateInfo semaphoreCI = vks::initializers::semaphoreCreateInfo();
    VK_CHECK_RESULT(vkCreateSemaphore(_vkcontext.device->logicalDevice, &semaphoreCI, nullptr, &acquireComplete));
    VK_CHECK_RESULT(vkCreateSemaphore(_vkcontext.device->logicalDevice, &semaphoreCI, nullptr, &renderComplete));

    // GPU library init
    gpu::Context::init<gpu::vk::VKBackend>();
    _gpuContext = std::make_shared<gpu::Context>();
    _backend = _gpuContext->getBackend();
    auto vkBackend = std::dynamic_pointer_cast<gpu::vk::VKBackend>(_gpuContext->getBackend());
    vkBackend->setIsFramePlayer(true);
#endif
}

void RenderThread::setup() {
    // Wait until the context has been moved to this thread
    { std::unique_lock<std::mutex> lock(_frameLock); }
    _gpuContext->beginFrame();
    _gpuContext->endFrame();

#ifdef USE_GL
    _context.makeCurrent();
    glViewport(0, 0, 800, 600);
    (void)CHECK_GL_ERROR();
#endif
    _elapsed.start();
}

void RenderThread::shutdown() {
    _activeFrame.reset();
    while (!_pendingFrames.empty()) {
        _gpuContext->consumeFrameUpdates(_pendingFrames.front());
        _pendingFrames.pop();
    }
    _gpuContext->shutdown();
    _gpuContext.reset();
}

void RenderThread::renderFrame(gpu::FramePointer& frame) {
    if (frame && !frame->batches.empty()) {
        if (rdoc_api)
            rdoc_api->StartFrameCapture(NULL, NULL);
    }
#ifdef USE_GL
    PROFILE_RANGE(render_gpu_gl, __FUNCTION__);
#else
    PROFILE_RANGE(render_gpu, __FUNCTION__);
#endif
    ++_presentCount;
#ifdef USE_GL
    _context.makeCurrent();
#endif
    auto vkBackend = std::dynamic_pointer_cast<gpu::vk::VKBackend>(_gpuContext->getBackend());

    if (_correction != glm::mat4()) {
        std::unique_lock<std::mutex> lock(_frameLock);
        if (_correction != glm::mat4()) {
            _backend->setCameraCorrection(_correction, _activeFrame->view, true);
            vkBackend->enableContextViewCorrectionForFramePlayer();
            //_prevRenderView = _correction * _activeFrame->view;
        }
    }
    _backend->recycle();
    _backend->syncCache();

    auto windowSize = _window->size();

#ifndef USE_GL
    auto windowExtent = VkExtent2D{ (uint32_t)windowSize.width(), (uint32_t)windowSize.height() };
    if (windowExtent.width != _extent.width || windowExtent.height != _extent.height) {
        return;
    }

    if (_extent.width != _swapchain.extent.width || _extent.width != _swapchain.extent.width) {
        _swapchain.create(&_extent.width, &_extent.height, false, false);
        setupFramebuffers();
        return;
    }

    static const VkOffset2D offset{0, 0};
    static const std::array<VkClearValue, 2> clearValues{
        VkClearValue{color: VkClearColorValue{float32: { 0.2f, 0.2f, 0.2f, 0.2f }}},
        VkClearValue{depthStencil: VkClearDepthStencilValue{ 1.0f, 0 }},
    };

    uint32_t swapchainIndex;
    VK_CHECK_RESULT(_swapchain.acquireNextImage(acquireComplete, &swapchainIndex));
    auto framebuffer = _framebuffers[swapchainIndex];
    const auto& commandBuffer = _vkcontext.createCommandBuffer(_vkcontext.device->graphicsCommandPool);
    //auto vkBackend = dynamic_pointer_cast<gpu::vulkan::VKBackend>(getBackend());
    //Q_ASSERT(vkBackend);

    auto rect = VkRect2D{ offset, _extent };
    VkRenderPassBeginInfo beginInfo = vks::initializers::renderPassBeginInfo();
    beginInfo.renderPass = _renderPass;
    beginInfo.framebuffer = framebuffer;
    beginInfo.renderArea = rect;
    beginInfo.clearValueCount = (uint32_t)clearValues.size();
    beginInfo.pClearValues = clearValues.data();
        //_renderPass, framebuffer, rect, (uint32_t)clearValues.size(), clearValues.data() };
    VkCommandBufferBeginInfo commandBufferBeginInfo = vks::initializers::commandBufferBeginInfo();
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

    using namespace vks::debugutils;
    cmdBeginLabel(commandBuffer, "executeFrame", glm::vec4{ 1, 1, 1, 1 });
#endif
    auto& glbackend = (gpu::gl::GLBackend&)(*_backend);
    glm::uvec2 fboSize{ frame->framebuffer->getWidth(), frame->framebuffer->getHeight() };
#ifdef USE_GL
    auto fbo = glbackend.getFramebufferID(frame->framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glClearColor(0, 0, 0, 1);
    glClearDepth(0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
#endif

    //_gpuContext->enableStereo(true);
    //auto vkBackend = std::dynamic_pointer_cast<gpu::vk::VKBackend>(_gpuContext->getBackend());
    vkBackend->setDrawCommandBuffer(commandBuffer);

    if (frame && !frame->batches.empty()) {
        // VKTODO: this is a temporary workaround, until render passes are selected inside Vulkan backend
        //vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
        _gpuContext->executeFrame(frame);
        _renderedFrameCount++;
        qDebug() << "Frame rendered: " << _renderedFrameCount;
        //vkCmdEndRenderPass(commandBuffer);
    }

#ifdef USE_GL
    static gpu::BatchPointer batch = nullptr;
    if (!batch) {
        batch = std::make_shared<gpu::Batch>();
        batch->setPipeline(_presentPipeline);
        batch->setFramebuffer(nullptr);
        batch->setResourceTexture(0, frame->framebuffer->getRenderBuffer(0));
        batch->setViewportTransform(ivec4(uvec2(0), ivec2(windowSize.width(), windowSize.height())));
        batch->draw(gpu::TRIANGLE_STRIP, 4);
    }
    glDisable(GL_FRAMEBUFFER_SRGB);
    _gpuContext->executeBatch(*batch);
    
    // Keep this raw gl code here for reference
    //glDisable(GL_FRAMEBUFFER_SRGB);
    //glClear(GL_COLOR_BUFFER_BIT);
  /*  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(
        0, 0, fboSize.x, fboSize.y, 
        0, 0, windowSize.width(), windowSize.height(),
        GL_COLOR_BUFFER_BIT, GL_NEAREST);
*/
    (void)CHECK_GL_ERROR();
    _context.swapBuffers();
    _context.doneCurrent();
#else
    cmdEndLabel(commandBuffer);
    cmdBeginLabel(commandBuffer, "renderpass:testClear", glm::vec4{ 0, 1, 1, 1 });
    vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(commandBuffer);

    // Blit the image into the swapchain.
    // is vks::tools::insertImageMemoryBarrier needed?
    VkImageBlit imageBlit{};
    imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBlit.srcSubresource.layerCount = 1;
    imageBlit.srcSubresource.mipLevel = 0;
    imageBlit.srcOffsets[1].x = vkBackend->_outputTexture->_gpuObject.getWidth();
    imageBlit.srcOffsets[1].y = vkBackend->_outputTexture->_gpuObject.getHeight();
    imageBlit.srcOffsets[1].z = 1;

    imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBlit.dstSubresource.layerCount = 1;
    imageBlit.dstSubresource.mipLevel = 0;
    imageBlit.dstOffsets[1].x = _swapchain.extent.width;
    imageBlit.dstOffsets[1].y = _swapchain.extent.height;
    imageBlit.dstOffsets[1].z = 1;

    VkImageSubresourceRange mipSubRange = {};
    mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    mipSubRange.baseMipLevel = 0;
    mipSubRange.levelCount = 1;
    mipSubRange.layerCount = 1;

    vks::tools::insertImageMemoryBarrier(
        commandBuffer,
        vkBackend->_outputTexture->attachments[0].image,
        VK_ACCESS_TRANSFER_READ_BIT,
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        mipSubRange);

    vks::tools::insertImageMemoryBarrier(
        commandBuffer,
        _swapchain.images[swapchainIndex],
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        mipSubRange);

    vkCmdBlitImage(
        commandBuffer,
        vkBackend->_outputTexture->attachments[0].image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        _swapchain.images[swapchainIndex],
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &imageBlit,
        VK_FILTER_LINEAR);

    vks::tools::insertImageMemoryBarrier(
        commandBuffer,
        _swapchain.images[swapchainIndex],
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        mipSubRange);

    cmdEndLabel(commandBuffer);
    VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

    static const VkPipelineStageFlags waitFlags{ VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT };
    VkSubmitInfo submitInfo = vks::initializers::submitInfo();
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &acquireComplete;
    submitInfo.pWaitDstStageMask = &waitFlags;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderComplete;
    submitInfo.commandBufferCount = 1;
    VkFenceCreateInfo fenceCI = vks::initializers::fenceCreateInfo();
    VkFence frameFence;
    vkCreateFence(_vkcontext.device->logicalDevice, &fenceCI, nullptr, &frameFence);
    vkQueueSubmit(_vkcontext.graphicsQueue, 1, &submitInfo, frameFence);
    _swapchain.queuePresent(_vkcontext.graphicsQueue, swapchainIndex, renderComplete);
    //VKTODO _vkcontext.trashCommandBuffers({ commandBuffer });
    vkBackend->waitForGPU();
    vkBackend->recycleFrame();
    if (frame && !frame->batches.empty()) {
        if (rdoc_api)
            rdoc_api->EndFrameCapture(NULL, NULL);
    }

#endif
}

bool RenderThread::process() {
    std::queue<gpu::FramePointer> pendingFrames;
    std::queue<QSize> pendingSize;

    {
        std::unique_lock<std::mutex> lock(_frameLock);
        pendingFrames.swap(_pendingFrames);
        pendingSize.swap(_pendingSize);
    }

    while (!pendingFrames.empty()) {
        _activeFrame = pendingFrames.front();
        pendingFrames.pop();
        _gpuContext->consumeFrameUpdates(_activeFrame);
    }

    while (!pendingSize.empty()) {
#ifndef USE_GL
        const auto& size = pendingSize.front();
        _extent = VkExtent2D{ (uint32_t)size.width(), (uint32_t)size.height() };
#endif
        pendingSize.pop();
    }

    if (!_activeFrame) {
        QThread::msleep(1);
        return true;
    }

    renderFrame(_activeFrame);
    /*if (_renderedFrameCount == 1) {
        return false;
    }*/
    return true;
}

#ifndef USE_GL

void RenderThread::setupFramebuffers() {
    // Recreate the frame buffers
    for (auto framebuffer : _framebuffers) {
        _vkcontext.recycler.trashVkFramebuffer(framebuffer);
    }
    _framebuffers.clear();

    std::vector<VkImageView> attachments;
    attachments.resize(1);
    VkFramebufferCreateInfo framebufferCreateInfo{};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = NULL;
    framebufferCreateInfo.renderPass = _renderPass;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.pAttachments = attachments.data();
    framebufferCreateInfo.width = _extent.width;
    framebufferCreateInfo.height = _extent.height;
    framebufferCreateInfo.layers = 1;

    // Create frame buffers for every swap chain image
    _framebuffers.resize(_swapchain.imageCount);
    for (uint32_t i = 0; i < _swapchain.imageCount; i++) {
        attachments[0] = _swapchain.buffers[i].view;
        VK_CHECK_RESULT(vkCreateFramebuffer(_vkcontext.device->logicalDevice, &framebufferCreateInfo, nullptr, &_framebuffers[i]));
    }
}

void RenderThread::setupRenderPass() {
    if (_renderPass) {
        _vkcontext.recycler.trashVkRenderPass(_renderPass);
    }

    VkAttachmentDescription attachment{};
    // Color attachment
    attachment.format = _swapchain.colorFormat;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;

    VkAttachmentReference colorAttachmentReference{};
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;
    VkSubpassDependency subpassDependency{};
    subpassDependency.srcSubpass = 0;
    subpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &attachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    // VKTODO
    // renderPassInfo.dependencyCount = 1;
    // renderPassInfo.pDependencies = &subpassDependency;
    renderPassInfo.dependencyCount = 0;
    VK_CHECK_RESULT(vkCreateRenderPass(_vkcontext.device->logicalDevice, &renderPassInfo, nullptr, &_renderPass));
}
#endif

class QLambdaRunnable : public QRunnable {
public:
    QLambdaRunnable(const std::function<void()>& f) : _f(f) {}

    void run() override { _f(); }

private:
    std::function<void()> _f;
};

/*struct VkBufferTransferItem {
    using Vector = std::vector<VkBufferTransferItem>;
    VkDeviceSize offset{ 0 };
    gpu::BufferPointer gpuBuffer;
    vks::Buffer deviceBuffer;

    void allocateDeviceBuffer(const vks::Context& context) {
        static const VkBufferUsageFlags flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                                VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        deviceBuffer = context.createDeviceBuffer(flags, gpuBuffer->getSize());
    }

    static void populateStagingBuffer(vks::Buffer& stagingBuffer, Vector& vector) {
        VkDeviceSize totalSize = 0;
        for (auto& item : vector) {
            item.offset = totalSize;
            totalSize += item.gpuBuffer->getSize();
        }
        stagingBuffer.map();

        for (auto& item : vector) {
            const auto& gpuBuffer = item.gpuBuffer;
            auto itemSize = gpuBuffer->getSize();
            item.offset = totalSize;
            if ((itemSize + item.offset) >= stagingBuffer.size) {
                qDebug() << "Bad copy";
            }
            stagingBuffer.copy(itemSize, gpuBuffer->getData(), item.offset);
            totalSize += itemSize;
        }
        stagingBuffer.unmap();
    }

    static void transferBuffers(const vks::Context& context, vks::Buffer& stagingBuffer, Vector& vector) {
        // VKTODO: use existing command pool from VulkanDevice class
        VkCommandPool commandPool;
        {
            VkCommandPoolCreateInfo createInfo = vks::initializers::commandPoolCreateInfo();
            createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            createInfo.queueFamilyIndex = context.device->queueFamilyIndices.transfer;
            VK_CHECK_RESULT(vkCreateCommandPool(context.device->logicalDevice, &createInfo, nullptr, &commandPool));
        }

        VkCommandBuffer commandBuffer;
        {
            VkCommandBufferAllocateInfo allocateInfo = vks::initializers::commandBufferAllocateInfo(commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
            VK_CHECK_RESULT(vkAllocateCommandBuffers(context.device->logicalDevice,&allocateInfo,&commandBuffer));
        }
        VkCommandBufferBeginInfo commandBufferBeginInfo = vks::initializers::commandBufferBeginInfo();
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
        for (auto& item : vector) {
            VkBufferCopy region{ item.offset, 0, item.gpuBuffer->getSize() };
            vkCmdCopyBuffer(commandBuffer, stagingBuffer.buffer, item.deviceBuffer.buffer, 1, &region);
        }
        VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

        {
#ifdef USE_GL
            PROFILE_RANGE(render_gpu_gl, "vk_submitTranferCommandBuffer");
#endif
            VkQueue transferQueue;
            vkGetDeviceQueue(context.device->logicalDevice, context.device->queueFamilyIndices.transfer, 0, &transferQueue);
            VkFence fence;
            auto fenceCI = vks::initializers::fenceCreateInfo();
            VK_CHECK_RESULT(vkCreateFence(context.device->logicalDevice, &fenceCI, nullptr, &fence));
            {
                VkSubmitInfo submitInfo{};
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &commandBuffer;
                VK_CHECK_RESULT(vkQueueSubmit(transferQueue, 1, &submitInfo, fence));
            }
            {
#ifdef USE_GL
                PROFILE_RANGE(render_gpu_gl, "vk_submitTranferCommandBufferWait");
#endif
                VK_CHECK_RESULT(vkQueueWaitIdle(transferQueue));
                while (VK_SUCCESS != vkWaitForFences(context.device->logicalDevice, 1, &fence, VK_TRUE, UINT64_MAX)) {
                    QThread::msleep(1);
                }
            }
        }
    }
};*/

/*void RenderThread::testVkTransfer() {
    static std::atomic<bool> running{ false };
    if (running) {
        return;
    }

    running = true;
    QThreadPool::globalInstance()->start(new QLambdaRunnable([this] {
#ifdef USE_GL
        PROFILE_RANGE(render_gpu_gl, "vk_bufferCopyTest");
#endif
        VkBufferTransferItem::Vector bufferTransfers;
        {
            std::unordered_set<gpu::BufferPointer> allBuffers;
            for (const auto& batch : _activeFrame->batches) {
                for (const auto& buffer : batch->_buffers._items) {
                    if (buffer._data && 0 != buffer._data->getSize()) {
                        allBuffers.insert(buffer._data);
                    }
                }
            }
            {
#ifdef USE_GL
                PROFILE_RANGE(render_gpu_gl, "vk_populateStagingBuffers");
#endif
                bufferTransfers.resize(allBuffers.size());
                size_t i = 0;
                for (const auto& buffer : allBuffers) {
                    auto& item = bufferTransfers[i++];
                    item.gpuBuffer = buffer;
                }
                VkBufferTransferItem::populateStagingBuffer(_vkstagingBuffer, bufferTransfers);
            }
        }

        //{
        //    PROFILE_RANGE(render_gpu_gl, "vk_defragStagingBuffers");
        //    VkBufferTransferItem::defragStagingBuffers(_vkcontext, bufferTransfers);
        //}

        {
#ifdef USE_GL
            PROFILE_RANGE(render_gpu_gl, "vk_allocateDeviceBuffers");
#endif
            for (auto& item : bufferTransfers) {
                item.allocateDeviceBuffer(_vkcontext);
            }
        }

        {
#ifdef USE_GL
            PROFILE_RANGE(render_gpu_gl, "vk_transferBuffers");
#endif
            VkBufferTransferItem::transferBuffers(_vkcontext, _vkstagingBuffer, bufferTransfers);
        }

        {
#ifdef USE_GL
            PROFILE_RANGE(render_gpu_gl, "vk_destroyTransferDeviceBuffers");
#endif
            for (auto& item : bufferTransfers) {
                item.deviceBuffer.destroy();
            }
        }

        running = false;
    }));
}*/

#ifdef USE_GL
struct GlBufferTransferItem {
    using Vector = std::vector<GlBufferTransferItem>;
    gpu::BufferPointer gpuBuffer;
    GLuint glbuffer;

    void allocateDeviceBuffer() {
        glCreateBuffers(1, &glbuffer);
        glNamedBufferStorage(glbuffer, gpuBuffer->getSize(), gpuBuffer->getData(), 0);
    }

    void destroyDeviceBuffer() { glDeleteBuffers(1, &glbuffer); }
};
#endif

#ifdef USE_GL
void RenderThread::testGlTransfer() {
    static std::atomic<bool> running{ false };
    if (running) {
        return;
    }

    running = true;
    QThreadPool::globalInstance()->start(new QLambdaRunnable([this] {
        static std::once_flag once;
        static OffscreenGLCanvas glcanvas;
        std::call_once(once, [] {
            glcanvas.create();
            glcanvas.makeCurrent();
        });
        PROFILE_RANGE(render_gpu_gl, "gl_bufferTransferTest");


        GlBufferTransferItem::Vector bufferTransfers;
        {
            std::unordered_set<gpu::BufferPointer> allBuffers;
            for (const auto& batch : _activeFrame->batches) {
                for (const auto& buffer : batch->_buffers._items) {
                    if (buffer._data && 0 != buffer._data->getSize()) {
                        allBuffers.insert(buffer._data);
                    }
                }
            }

            {
                PROFILE_RANGE(render_gpu_gl, "gl_initBuffers");
                bufferTransfers.resize(allBuffers.size());
                size_t i = 0;
                for (const auto& buffer : allBuffers) {
                    auto& item = bufferTransfers[i++];
                    item.gpuBuffer = buffer;
                }
            }
        }

        {
            PROFILE_RANGE(render_gpu_gl, "gl_allocateDeviceBuffers");
            for (auto& item : bufferTransfers) {
                item.allocateDeviceBuffer();
            }
        }

        {
            PROFILE_RANGE(render_gpu_gl, "gl_destroyDeviceBuffers");
            for (auto& item : bufferTransfers) {
                item.destroyDeviceBuffer();
            }
        }

        running = false;
}));
}
#endif
