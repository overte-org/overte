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
#include <gl/OffscreenGLCanvas.h>
//#include <QVulkanInstance>
#ifdef USE_GL
#include <gl/QOpenGLContextWrapper.h>
#endif

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
    _extent = vk::Extent2D{ (uint32_t)size.width(), (uint32_t)size.height() };

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
    vk::XcbSurfaceCreateInfoKHR surfaceCreateInfo;
    //dynamic_cast<QGuiApplication*>(QGuiApplication::instance())->platformNativeInterface()->connection();
    surfaceCreateInfo.connection = QX11Info::connection();
    surfaceCreateInfo.window = (xcb_window_t)(window->winId());
    _surface = _vkcontext.instance.createXcbSurfaceKHR(surfaceCreateInfo);
#endif
    _vkcontext.createDevice(_surface);
    _swapchain.setSurface(_surface);
    _swapchain.create(_extent, true);

    setupRenderPass();
    setupFramebuffers();

    acquireComplete = _vkcontext.device.createSemaphore(vk::SemaphoreCreateInfo{});
    renderComplete = _vkcontext.device.createSemaphore(vk::SemaphoreCreateInfo{});

    // GPU library init
    gpu::Context::init<gpu::vulkan::VKBackend>();
    _gpuContext = std::make_shared<gpu::Context>();
    _backend = _gpuContext->getBackend();
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

#ifndef USE_GL
extern vk::CommandBuffer currentCommandBuffer;
#endif

void RenderThread::renderFrame(gpu::FramePointer& frame) {
#ifdef USE_GL
    PROFILE_RANGE(render_gpu_gl, __FUNCTION__);
#else
    PROFILE_RANGE(render_gpu, __FUNCTION__);
#endif
    ++_presentCount;
#ifdef USE_GL
    _context.makeCurrent();
#endif
    if (_correction != glm::mat4()) {
       std::unique_lock<std::mutex> lock(_frameLock);
       if (_correction != glm::mat4()) {
           _backend->setCameraCorrection(_correction, _activeFrame->view);
           //_prevRenderView = _correction * _activeFrame->view;
       }
    }
    _backend->recycle();
    _backend->syncCache();

    auto windowSize = _window->size();

#ifndef USE_GL
    auto windowExtent = vk::Extent2D{ (uint32_t)windowSize.width(), (uint32_t)windowSize.height() };
    if (windowExtent != _extent) {
        return;
    }

    if (_extent != _swapchain.extent) {
        _swapchain.create(_extent);
        setupFramebuffers();
        return;
    }

    static const vk::Offset2D offset;
    static const std::array<vk::ClearValue, 2> clearValues{
        vk::ClearColorValue(std::array<float, 4>{ { 0.2f, 0.2f, 0.2f, 0.2f } }),
        vk::ClearDepthStencilValue( 1.0f, 0 ),
    };

    auto swapchainIndex = _swapchain.acquireNextImage(acquireComplete).value;
    auto framebuffer = _framebuffers[swapchainIndex];
    const auto& commandBuffer = currentCommandBuffer = _vkcontext.createCommandBuffer();

    auto rect = vk::Rect2D{ offset, _extent };
    vk::RenderPassBeginInfo beginInfo{ _renderPass, framebuffer, rect, (uint32_t)clearValues.size(), clearValues.data() };
    commandBuffer.begin(vk::CommandBufferBeginInfo{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

    using namespace vks::debug::marker;
    beginRegion(commandBuffer, "executeFrame", glm::vec4{ 1, 1, 1, 1 });
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
    if (frame && !frame->batches.empty()) {
        _gpuContext->executeFrame(frame);
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
    endRegion(commandBuffer);
    beginRegion(commandBuffer, "renderpass:testClear", glm::vec4{ 0, 1, 1, 1 });
    commandBuffer.beginRenderPass(beginInfo, vk::SubpassContents::eInline);
    commandBuffer.endRenderPass();
    endRegion(commandBuffer);
    commandBuffer.end();

    static const vk::PipelineStageFlags waitFlags{ vk::PipelineStageFlagBits::eBottomOfPipe };
    vk::SubmitInfo submitInfo{ 1, &acquireComplete, &waitFlags, 1, &commandBuffer, 1, &renderComplete };
    vk::Fence frameFence = _vkcontext.device.createFence(vk::FenceCreateInfo{});
    _vkcontext.queue.submit(submitInfo, frameFence);
    _swapchain.queuePresent(renderComplete);
    _vkcontext.trashCommandBuffers({ commandBuffer });
    _vkcontext.emptyDumpster(frameFence);
    _vkcontext.recycle();
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
        _extent = vk::Extent2D( (uint32_t)size.width(), (uint32_t)size.height() );
#endif
        pendingSize.pop();
    }

    if (!_activeFrame) {
        QThread::msleep(1);
        return true;
    }

    renderFrame(_activeFrame);
    return true;
}

#ifndef USE_GL

void RenderThread::setupFramebuffers() {
    // Recreate the frame buffers
    _vkcontext.trashAll<vk::Framebuffer>(_framebuffers, [this](const std::vector<vk::Framebuffer>& framebuffers) {
        for (const auto& framebuffer : framebuffers) {
            _vkdevice.destroy(framebuffer);
        }
    });

    vk::ImageView attachment;
    vk::FramebufferCreateInfo framebufferCreateInfo;
    framebufferCreateInfo.renderPass = _renderPass;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.pAttachments = &attachment;
    framebufferCreateInfo.width = _extent.width;
    framebufferCreateInfo.height = _extent.height;
    framebufferCreateInfo.layers = 1;

    // Create frame buffers for every swap chain image
    _framebuffers = _swapchain.createFramebuffers(framebufferCreateInfo);
}

void RenderThread::setupRenderPass() {
    if (_renderPass) {
        _vkdevice.destroy(_renderPass);
    }

    vk::AttachmentDescription attachment;
    // Color attachment
    attachment.format = _swapchain.colorFormat;
    attachment.loadOp = vk::AttachmentLoadOp::eClear;
    attachment.storeOp = vk::AttachmentStoreOp::eStore;
    attachment.initialLayout = vk::ImageLayout::eUndefined;
    attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference colorAttachmentReference;
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpass;
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;
    vk::SubpassDependency subpassDependency;
    subpassDependency.srcSubpass = 0;
    subpassDependency.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
    subpassDependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    subpassDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead;
    subpassDependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &attachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &subpassDependency;
    _renderPass = _vkdevice.createRenderPass(renderPassInfo);
}
#endif

class QLambdaRunnable : public QRunnable {
public:
    QLambdaRunnable(const std::function<void()>& f) : _f(f) {}

    void run() override { _f(); }

private:
    std::function<void()> _f;
};

struct VkBufferTransferItem {
    using Vector = std::vector<VkBufferTransferItem>;
    vk::DeviceSize offset{ 0 };
    gpu::BufferPointer gpuBuffer;
    vks::Buffer deviceBuffer;

    void allocateDeviceBuffer(const vks::Context& context) {
        static const vk::BufferUsageFlags flags = vk::BufferUsageFlagBits::eVertexBuffer |
                                                  vk::BufferUsageFlagBits::eUniformBuffer |
                                                  vk::BufferUsageFlagBits::eTransferDst;
        deviceBuffer = context.createDeviceBuffer(flags, gpuBuffer->getSize());
    }

    static void populateStagingBuffer(vks::Buffer& stagingBuffer, Vector& vector) {
        vk::DeviceSize totalSize = 0;
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
        vk::CommandPool commandPool;
        {
            vk::CommandPoolCreateInfo createInfo{ vk::CommandPoolCreateFlagBits::eTransient, context.queueIndices.transfer };
            commandPool = context.device.createCommandPool(createInfo);
        }

        vk::CommandBuffer commandBuffer;
        {
            vk::CommandBufferAllocateInfo allocInfo{ commandPool, vk::CommandBufferLevel::ePrimary, 1 };
            commandBuffer = context.device.allocateCommandBuffers(allocInfo)[0];
        }
        commandBuffer.begin(vk::CommandBufferBeginInfo{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
        for (auto& item : vector) {
            vk::BufferCopy region{ item.offset, 0, item.gpuBuffer->getSize() };
            commandBuffer.copyBuffer(stagingBuffer.buffer, item.deviceBuffer.buffer, region);
        }
        commandBuffer.end();

        {
#ifdef USE_GL
            PROFILE_RANGE(render_gpu_gl, "vk_submitTranferCommandBuffer");
#endif
            auto transferQueue = context.device.getQueue(context.queueIndices.transfer, 0);
            auto fence = context.device.createFence({});
            {
                vk::SubmitInfo submitInfo{};
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &commandBuffer;
                transferQueue.submit(submitInfo, fence);
            }
            {
#ifdef USE_GL
                PROFILE_RANGE(render_gpu_gl, "vk_submitTranferCommandBufferWait");
#endif
                transferQueue.waitIdle();
                while (vk::Result::eSuccess != context.device.waitForFences(fence, VK_TRUE, UINT64_MAX)) {
                    QThread::msleep(1);
                }
            }
        }
    }
};

void RenderThread::testVkTransfer() {
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
}

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
