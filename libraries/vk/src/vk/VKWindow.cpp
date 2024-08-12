//
//  Created by Bradley Austin Davis on 2016/05/26
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <QtCore/QCoreApplication>
#include <QGuiApplication>
#include <QtGui/QWindow>
#include <QtGui/qevent.h>
#include <QtCore/QTimer>
#include <QtCore/QDebug>
#include <QtPlatformHeaders/QXcbWindowFunctions>
#include <qpa/qplatformnativeinterface.h>
#include <QtX11Extras/QX11Info>

#include "VKWindow.h"
#include "Config.h"
#include "VulkanSwapChain.h"
#include "Context.h"

VKWindow::VKWindow(QScreen* screen) : QWindow(screen) {
    _resizeTimer = new QTimer(this);
    _resizeTimer->setTimerType(Qt::TimerType::PreciseTimer);
    _resizeTimer->setInterval(50);
    _resizeTimer->setSingleShot(true);
    connect(_resizeTimer, &QTimer::timeout, this, &VKWindow::resizeFramebuffer);
}

const void VKWindow::createSurface() {
#ifdef WIN32
    // TODO
    _surface = _context.instance.createWin32SurfaceKHR({ {}, GetModuleHandle(NULL), (HWND)winId() });
#else
    VkXcbSurfaceCreateInfoKHR surfaceCreateInfo{};
    //dynamic_cast<QGuiApplication*>(QGuiApplication::instance())->platformNativeInterface()->connection();

    auto* platformInterface = QGuiApplication::platformNativeInterface();
    auto* handle = platformInterface->nativeResourceForWindow("handle", this);
    Q_ASSERT(winId());
    qDebug() << "VKWindow::createSurface winId:" << winId();
    _swapchain.initSurface(QX11Info::connection(), winId());
    //_swapchain.initSurface(QX11Info::connection(), QX11Info::appRootWindow());
    //VkSurfaceKHR surface;
    //VK_CHECK_RESULT(vkCreateXcbSurfaceKHR(_context.instance, &surfaceCreateInfo, nullptr, &surface));
#endif
    //_swapchain.setSurface(_surface);
}

void VKWindow::createSwapchain() {
    {
        auto qsize = size();
        _extent = VkExtent2D{(uint32_t)qsize.width(), (uint32_t)qsize.height()};
    }
    _swapchain.create(&_extent.width, &_extent.height, false, false);

    setupRenderPass();
    setupDepthStencil();
    setupFramebuffers();
}

void VKWindow::setupDepthStencil() {
    auto &device = _context.device->logicalDevice;
    if (_depthStencil.isAllocated) {
        vkDestroyImageView(device, _depthStencil.view, nullptr);
        vkDestroyImage(device, _depthStencil.image, nullptr);
#if VULKAN_USE_VMA
        vmaFreeMemory(_depthStencil.getAllocator(), _depthStencil.allocation);
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
}

void VKWindow::setupFramebuffers() {
    // Recreate the frame buffers
    if (!_frameBuffers.empty()) {
        for (auto& framebuffer : _frameBuffers) {
            vkDestroyFramebuffer(_device, framebuffer, nullptr);
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
        VK_CHECK_RESULT(vkCreateFramebuffer(_context.device->logicalDevice, &frameBufferCreateInfo, nullptr, &_frameBuffers[i]));
    }
}

void VKWindow::setupRenderPass() {
    if (_renderPass) {
        vkDestroyRenderPass(_device, _renderPass, nullptr);
    }

    std::array<VkAttachmentDescription, 2> attachments;
    // Color attachment
    attachments[0].format = _swapchain.colorFormat;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Depth attachment
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
        VkAttachmentReference colorReference;
        colorReference.attachment = 0;
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentReferences.push_back(colorReference);
    }

    std::vector<VkSubpassDescription> subpasses;
    std::vector<VkSubpassDependency> subpassDependencies;
    {
        {
            VkSubpassDependency dependency;
            dependency.srcSubpass = 0;
            dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            subpassDependencies.push_back(dependency);
        }

        VkSubpassDescription subpass;
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.pDepthStencilAttachment = &depthReference;
        subpass.colorAttachmentCount = (uint32_t)colorAttachmentReferences.size();
        subpass.pColorAttachments = colorAttachmentReferences.data();
        subpasses.push_back(subpass);
    }

    VkRenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = (uint32_t)attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = (uint32_t)subpasses.size();
    renderPassInfo.pSubpasses = subpasses.data();
    renderPassInfo.dependencyCount = (uint32_t)subpassDependencies.size();
    renderPassInfo.pDependencies = subpassDependencies.data();
    VK_CHECK_RESULT(vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass));
}


void VKWindow::resizeEvent(QResizeEvent* event) {
    QWindow::resizeEvent(event);
    auto qsize = event->size();
    if (qsize.width() != (int)(_extent.width) || qsize.height() != (int)(_extent.height)) {
        _resizeTimer->start();
    }
}

void VKWindow::resizeFramebuffer() {
    auto qsize = size();
    _extent = VkExtent2D{
        .width = (uint32_t)qsize.width(),
        .height = (uint32_t)qsize.height()
    };
    //vkQueueWaitIdle();
    VK_CHECK_RESULT(vkDeviceWaitIdle(_device));
    _swapchain.create(&_extent.width, &_extent.height, false, false);
    // TODO: add an assert here to see if width and height changed?
    setupDepthStencil();
    setupFramebuffers();
}

VKWindow::~VKWindow() {
    _swapchain.cleanup();
}

void VKWindow::emitClosing() {
    emit aboutToClose();
}

/*VkFramebuffer VKWindow::acquireFramebuffer(const VkSemaphore& semaphore) {
    uint32_t imageIndex;
    VK_CHECK_RESULT(_swapchain.acquireNextImage(semaphore, &imageIndex));
    return _framebuffers[imageIndex];
}*/
