//
// Created by ksuprynowicz on 25.01.25.
//

//
//
//  Created by Bradley Austin Davis on 2015/12/03
//  Derived from interface/src/GLCanvas.cpp created by Stephen Birarda on 8/14/13.
//  Copyright 2013-2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "VKWidget.h"

#include "Config.h"

#include <mutex>

#include <QtGlobal>
#include <QtCore/QMimeData>
#include <QtCore/QUrl>
#include <QtCore/QCoreApplication>
#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>
#include <QtX11Extras/QX11Info>

#include <QtGui/QKeyEvent>
#include <QtGui/QPaintEngine>
#include <QtGui/QWindow>

#include "Context.h"
#include "gl/Context.h"
//#include "GLHelpers.h"

class GLPaintEngine : public QPaintEngine {
    bool begin(QPaintDevice *pdev) override { return true; }
    bool end() override { return true; }
    void updateState(const QPaintEngineState &state) override { }
    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr) override { }
    Type type() const override { return OpenGL2; }
};

class NullPaintEngine : public QPaintEngine {
    bool begin(QPaintDevice *pdev) override { return true; }
    bool end() override { return true; }
    void updateState(const QPaintEngineState &state) override { }
    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr) override { }
    Type type() const override { return User; }
};


VKWidget::VKWidget() {
    setAttribute(Qt::WA_AcceptTouchEvents);
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_InputMethodEnabled);
    setAutoFillBackground(false);
    grabGesture(Qt::PinchGesture);
    setAcceptDrops(true);
    _paintEngine = new NullPaintEngine(); // VKTODO: what is it used for?
    vks::Context::get().registerWidget(this);
    //_device = _vksContext.device->logicalDevice;
}

VKWidget::~VKWidget() {
    delete _paintEngine;
    _paintEngine = nullptr;
    // Depending on what is shut down first, cleanup is done either by window or Vulkan backend.
    /*std::lock_guard<std::recursive_mutex> lock(_vksContext.vulkanWindowsMutex);
    if (!_isVulkanCleanupComplete) {
        _vksContext.unregisterWidget(this);
        vulkanCleanup();
    }*/
}

int VKWidget::getDeviceWidth() const {
    return width() * (windowHandle() ? (float)windowHandle()->devicePixelRatio() : 1.0f);
}

int VKWidget::getDeviceHeight() const {
    return height() * (windowHandle() ? (float)windowHandle()->devicePixelRatio() : 1.0f);
}

void VKWidget::createContext(QOpenGLContext* shareContext) {
    //_context = new gl::OffscreenContext();
    _context = new gl::OffscreenContext();
    //_context->setWindow(windowHandle());
    _context->create(shareContext);
    _context->makeCurrent();
    _context->clear();
    _context->doneCurrent();
}

void VKWidget::swapBuffers() {
    _context->swapBuffers();
}

bool VKWidget::makeCurrent() {
    //gl::Context::makeCurrent(_context->qglContext(), windowHandle()); // VKTODO
    gl::Context::makeCurrent(_context->qglContext(), _context->getWindow()); // VKTODO
    return _context->makeCurrent();
}

QOpenGLContext* VKWidget::qglContext() {
    return _context->qglContext();
}

void VKWidget::doneCurrent() {
    _context->doneCurrent();
}

QVariant VKWidget::inputMethodQuery(Qt::InputMethodQuery query) const {
    // TODO: for now we just use top left corner for an IME popup location, but in the future its position could be calculated
    // for a given entry field.
    if (query == Qt::ImCursorRectangle) {
        int x = 50;
        int y = 50;
        return QRect(x, y, 10, 10);
    }
    return QWidget::inputMethodQuery(query);
}

bool VKWidget::event(QEvent* event) {
    switch (event->type()) {
        case QEvent::MouseMove:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::FocusIn:
        case QEvent::FocusOut:
        case QEvent::Resize:
        case QEvent::TouchBegin:
        case QEvent::TouchEnd:
        case QEvent::TouchUpdate:
        case QEvent::Gesture:
        case QEvent::Wheel:
        case QEvent::DragEnter:
        case QEvent::Drop:
            if (QCoreApplication::sendEvent(QCoreApplication::instance(), event)) {
                return true;
            }
            break;
        case QEvent::InputMethod:
            if (QCoreApplication::sendEvent(QCoreApplication::instance(), event)) {
                return true;
            }
            break;
        case QEvent::InputMethodQuery:
            if (QCoreApplication::sendEvent(QCoreApplication::instance(), event)) {
                return true;
            }
            break;

        default:
            break;
    }
    return QWidget::event(event);
}

bool VKWidget::nativeEvent(const QByteArray &eventType, void *message, long *result) {
#ifdef Q_OS_WIN32
    MSG* win32message = static_cast<MSG*>(message);
    switch (win32message->message) {
        case WM_ERASEBKGND:
            *result = 1L;
            return TRUE;

        default:
            break;
    }
#endif
    return QWidget::nativeEvent(eventType, message, result);
}

QPaintEngine* VKWidget::paintEngine() const {
    return _paintEngine;
}

/*void VKWidget::createSurface() {
    nativeParentWidget()->windowHandle()->setSurfaceType(QSurface::VulkanSurface); //VKTODO
    //windowHandle()->setSurfaceType(QSurface::VulkanSurface);
    _swapchain.setContext(&_vksContext);
#ifdef WIN32
    // TODO
    _surface = _context.instance.createWin32SurfaceKHR({ {}, GetModuleHandle(NULL), (HWND)winId() });
#else
    VkXcbSurfaceCreateInfoKHR surfaceCreateInfo{};
    //dynamic_cast<QGuiApplication*>(QGuiApplication::instance())->platformNativeInterface()->connection();

    auto* platformInterface = QGuiApplication::platformNativeInterface();
    auto* handle = platformInterface->nativeResourceForWindow("handle", windowHandle());
    Q_ASSERT(_mainWindow->winId());
    qDebug() << "VKWindow::createSurface winId:" << winId();
    qDebug() << "VKWindow::createSurface winId:" << _mainWindow->winId();
    _swapchain.initSurface(QX11Info::connection(), _mainWindow->winId());
    //_swapchain.initSurface(QX11Info::connection(), QX11Info::appRootWindow());
    //VkSurfaceKHR surface;
    //VK_CHECK_RESULT(vkCreateXcbSurfaceKHR(_context.instance, &surfaceCreateInfo, nullptr, &surface));
#endif
    //_swapchain.setSurface(_surface);
}*/

/*void VKWidget::createSwapchain() {
    {
        auto qsize = getDeviceSize();
        _extent = VkExtent2D{(uint32_t)qsize.width(), (uint32_t)qsize.height()};
    }
    _swapchain.create(&_extent.width, &_extent.height, false, false);

    createCommandBuffers();
    setupRenderPass();
    setupDepthStencil();
    setupFramebuffers();
}*/

/*void VKWidget::createCommandBuffers() {
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    // Create a semaphore used to synchronize image presentation
    // Ensures that the image is displayed before we start submitting new commands to the queue
    VK_CHECK_RESULT(vkCreateSemaphore(_vksContext.device->logicalDevice, &semaphoreCreateInfo, nullptr, &_acquireCompleteSemaphore));
    // Create a semaphore used to synchronize command submission
    // Ensures that the image is not presented until all commands have been submitted and executed
    VK_CHECK_RESULT(vkCreateSemaphore(_vksContext.device->logicalDevice, &semaphoreCreateInfo, nullptr, &_renderCompleteSemaphore));
    // Create one command buffer for each swap chain image
    _drawCommandBuffers.resize(_swapchain.imageCount);
    VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(_vksContext.device->graphicsCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, static_cast<uint32_t>(_drawCommandBuffers.size()));
    VK_CHECK_RESULT(vkAllocateCommandBuffers(_vksContext.device->logicalDevice, &cmdBufAllocateInfo, _drawCommandBuffers.data()));
}

void VKWidget::vulkanCleanup() {
    _isVulkanCleanupComplete = true;
    _vksContext.recycler.trashVkSemaphore(_acquireCompleteSemaphore);
    _vksContext.recycler.trashVkSemaphore(_renderCompleteSemaphore);

    if (_renderPass) {
        _vksContext.recycler.trashVkRenderPass(_renderPass);
    }

    if (_depthStencil.image) {
        _vksContext.recycler.trashVkImage(_depthStencil.image);
    }

    if (_depthStencil.view) {
        _vksContext.recycler.trashVkImageView(_depthStencil.view);
    }

    if (_depthStencil.allocation) {
        _vksContext.recycler.trashVmaAllocation(_depthStencil.allocation);
    }

    for (auto& framebuffer : _frameBuffers) {
        _vksContext.recycler.trashVkFramebuffer(framebuffer);
    }
    _frameBuffers.clear();

    _swapchain.cleanup();
}

void VKWidget::setupDepthStencil() {
    auto &device = _vksContext.device->logicalDevice;
    if (_depthStencil.isAllocated) {
        auto &recycler = _vksContext.recycler;
        recycler.trashVkImageView(_depthStencil.view);
        recycler.trashVkImage(_depthStencil.image);
#if VULKAN_USE_VMA
        recycler.trashVmaAllocation(_depthStencil.allocation);
#else
        vkFreeMemory(device, _depthStencil.memory, nullptr);
#endif
        _depthStencil.isAllocated = false;
    }

    const VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
    VkImageCreateInfo imageCI{};
    imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCI.imageType = VK_IMAGE_TYPE_2D;
    imageCI.format = depthFormat;
    imageCI.extent = { _extent.width, _extent.height, 1 };
    imageCI.mipLevels = 1;
    imageCI.arrayLayers = 1;
    imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VK_CHECK_RESULT(vkCreateImage(device, &imageCI, nullptr, &_depthStencil.image));
    VkMemoryRequirements memReqs{};
    vkGetImageMemoryRequirements(device, _depthStencil.image, &memReqs);

#if VULKAN_USE_VMA
    VmaAllocationCreateInfo allocationCI {};
    allocationCI.pool = VK_NULL_HANDLE;
    allocationCI.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    // TODO: I'm not sure of this part, how does it know size and does it call vkBindImageMemory?
    VK_CHECK_RESULT(vmaAllocateMemoryForImage(_depthStencil.getAllocator(), _depthStencil.image, &allocationCI, &_depthStencil.allocation, nullptr));
    VK_CHECK_RESULT(vmaBindImageMemory(_depthStencil.getAllocator(), _depthStencil.allocation, _depthStencil.image));
#else
    VkMemoryAllocateInfo memAlloc{};
    memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = _context.device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &depthStencil.memory));
    VK_CHECK_RESULT(vkBindImageMemory(device, depthStencil.image, depthStencil.memory, 0));
#endif

    VkImageViewCreateInfo imageViewCI{};
    imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCI.image = _depthStencil.image;
    imageViewCI.format = depthFormat;
    imageViewCI.subresourceRange.baseMipLevel = 0;
    imageViewCI.subresourceRange.levelCount = 1;
    imageViewCI.subresourceRange.baseArrayLayer = 0;
    imageViewCI.subresourceRange.layerCount = 1;
    imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    // Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
    if (depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
        imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    VK_CHECK_RESULT(vkCreateImageView(device, &imageViewCI, nullptr, &_depthStencil.view));
    _depthStencil.isAllocated = true;
}

void VKWidget::setupFramebuffers() {
    // Recreate the frame buffers
    if (!_frameBuffers.empty()) {
        for (auto& framebuffer : _frameBuffers) {
            _vksContext.recycler.trashVkFramebuffer(framebuffer);
        }
        _frameBuffers.clear();
    }

    VkImageView attachments[2];

    // Depth/Stencil attachment is the same for all frame buffers
    attachments[1] = _depthStencil.view;

    VkFramebufferCreateInfo frameBufferCreateInfo = {};
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.pNext = NULL;
    frameBufferCreateInfo.renderPass = _renderPass;
    frameBufferCreateInfo.attachmentCount = 2;
    frameBufferCreateInfo.pAttachments = attachments;
    frameBufferCreateInfo.width = _extent.width;
    frameBufferCreateInfo.height = _extent.height;
    frameBufferCreateInfo.layers = 1;

    // Create frame buffers for every swap chain image
    _frameBuffers.resize(_swapchain.imageCount);
    for (uint32_t i = 0; i < _frameBuffers.size(); i++)
    {
        attachments[0] = _swapchain.buffers[i].view;
        VK_CHECK_RESULT(vkCreateFramebuffer(_vksContext.device->logicalDevice, &frameBufferCreateInfo, nullptr, &_frameBuffers[i]));
    }
}

void VKWidget::setupRenderPass() {
    if (_renderPass) {
        _vksContext.recycler.trashVkRenderPass(_renderPass);
    }

    std::array<VkAttachmentDescription, 2> attachments{};
    // Color attachment
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].format = _swapchain.colorFormat;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // VKTODO: not needed currently
    // Depth attachment
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].format = VK_FORMAT_D24_UNORM_S8_UINT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Only one depth attachment, so put it first in the references
    VkAttachmentReference depthReference;
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    std::vector<VkAttachmentReference> colorAttachmentReferences;
    {
        VkAttachmentReference colorReference{};
        colorReference.attachment = 0;
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentReferences.push_back(colorReference);
    }

    std::vector<VkSubpassDescription> subpasses;
    std::vector<VkSubpassDependency> subpassDependencies;
    {
        {
            VkSubpassDependency dependency{};
            dependency.srcSubpass = 0;
            dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            subpassDependencies.push_back(dependency);
        }

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.pDepthStencilAttachment = &depthReference;
        subpass.colorAttachmentCount = (uint32_t)colorAttachmentReferences.size();
        subpass.pColorAttachments = colorAttachmentReferences.data();
        subpasses.push_back(subpass);
    }

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = (uint32_t)attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = (uint32_t)subpasses.size();
    renderPassInfo.pSubpasses = subpasses.data();
    renderPassInfo.dependencyCount = (uint32_t)subpassDependencies.size();
    renderPassInfo.pDependencies = subpassDependencies.data();
    VK_CHECK_RESULT(vkCreateRenderPass(_vksContext.device->logicalDevice, &renderPassInfo, nullptr, &_renderPass));
}*/


/*void VKWidget::resizeEvent(QResizeEvent* event) {
    QWindow::resizeEvent(event);
    auto qsize = event->size();
    if (qsize.width() != (int)(_extent.width) || qsize.height() != (int)(_extent.height)) {
        _resizeTimer->start();
    }
}*/

/*void VKWidget::resizeFramebuffer() {
    auto qsize = getDeviceSize(); // VKTODO: is this ok
    _extent = VkExtent2D{
        .width = (uint32_t)qsize.width(),
        .height = (uint32_t)qsize.height()
    };
    //vkQueueWaitIdle();
    VK_CHECK_RESULT(vkDeviceWaitIdle(_vksContext.device->logicalDevice));
    _swapchain.create(&_extent.width, &_extent.height, false, false);
    // TODO: add an assert here to see if width and height changed?
    setupDepthStencil();
    setupFramebuffers();
}*/
