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
#include <QtX11Extras/QX11Info>

#include "VKWindow.h"
#include "Config.h"
#include "Swapchain.h"
#include "Context.h"

VKWindow::VKWindow(QScreen* screen) : QWindow(screen) {
    _resizeTimer = new QTimer(this);
    _resizeTimer->setTimerType(Qt::TimerType::PreciseTimer);
    _resizeTimer->setInterval(50);
    _resizeTimer->setSingleShot(true);
    connect(_resizeTimer, &QTimer::timeout, this, &VKWindow::resizeFramebuffer);
}

const vk::SurfaceKHR& VKWindow::createSurface() {
#ifdef WIN32
    _surface = _context.instance.createWin32SurfaceKHR({ {}, GetModuleHandle(NULL), (HWND)winId() });
#else
    vk::XcbSurfaceCreateInfoKHR surfaceCreateInfo;
    //dynamic_cast<QGuiApplication*>(QGuiApplication::instance())->platformNativeInterface()->connection();
    surfaceCreateInfo.connection = QX11Info::connection();
    surfaceCreateInfo.window = QX11Info::appRootWindow();
    _surface = _context.instance.createXcbSurfaceKHR(surfaceCreateInfo);
#endif
    _swapchain.setSurface(_surface);
    return _surface;
}

void VKWindow::createSwapchain() {
    if (!_surface) {
        throw std::runtime_error("No surface");
    }

    {
        auto qsize = size();
        _extent = vk::Extent2D((uint32_t)qsize.width(), (uint32_t)qsize.height());
    }
    _swapchain.create(_extent, true);

    setupRenderPass();
    setupDepthStencil();
    setupFramebuffers();
}

void VKWindow::setupDepthStencil() {
    if (_depthStencil) {
        _depthStencil.destroy();
        _depthStencil = {};
    }

    vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
    vk::ImageCreateInfo depthStencilCreateInfo;
    depthStencilCreateInfo.imageType = vk::ImageType::e2D;
    depthStencilCreateInfo.extent = vk::Extent3D{ _extent.width, _extent.height, 1 };
    depthStencilCreateInfo.format = vk::Format::eD24UnormS8Uint;
    depthStencilCreateInfo.mipLevels = 1;
    depthStencilCreateInfo.arrayLayers = 1;
    depthStencilCreateInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc;
    _depthStencil = _context.createImage(depthStencilCreateInfo, vk::MemoryPropertyFlagBits::eDeviceLocal);

    vk::ImageViewCreateInfo depthStencilView;
    depthStencilView.viewType = vk::ImageViewType::e2D;
    depthStencilView.format = vk::Format::eD24UnormS8Uint;
    depthStencilView.subresourceRange.aspectMask = aspect;
    depthStencilView.subresourceRange.levelCount = 1;
    depthStencilView.subresourceRange.layerCount = 1;
    depthStencilView.image = _depthStencil.image;
    _depthStencil.view = _device.createImageView(depthStencilView);
}

void VKWindow::setupFramebuffers() {
    // Recreate the frame buffers
    if (!_framebuffers.empty()) {
        for (auto& framebuffer : _framebuffers) {
            _device.destroy(framebuffer);
        }
        _framebuffers.clear();
    }

    vk::ImageView attachments[2];

    // Depth/Stencil attachment is the same for all frame buffers
    attachments[1] = _depthStencil.view;

    vk::FramebufferCreateInfo framebufferCreateInfo;
    framebufferCreateInfo.renderPass = _renderPass;
    framebufferCreateInfo.attachmentCount = 2;
    framebufferCreateInfo.pAttachments = attachments;
    framebufferCreateInfo.width = _extent.width;
    framebufferCreateInfo.height = _extent.height;
    framebufferCreateInfo.layers = 1;

    // Create frame buffers for every swap chain image
    _framebuffers = _swapchain.createFramebuffers(framebufferCreateInfo);

}

void VKWindow::setupRenderPass() {
    if (_renderPass) {
        _device.destroy(_renderPass);
    }

    std::array<vk::AttachmentDescription, 2> attachments;
    // Color attachment
    attachments[0].format = _swapchain.colorFormat;
    attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[0].initialLayout = vk::ImageLayout::eUndefined;
    attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;

    // Depth attachment
    attachments[1].format = vk::Format::eD24UnormS8Uint;
    attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[1].storeOp = vk::AttachmentStoreOp::eDontCare;
    attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eClear;
    attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[1].initialLayout = vk::ImageLayout::eUndefined;
    attachments[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    // Only one depth attachment, so put it first in the references
    vk::AttachmentReference depthReference;
    depthReference.attachment = 1;
    depthReference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    std::vector<vk::AttachmentReference> colorAttachmentReferences;
    {
        vk::AttachmentReference colorReference;
        colorReference.attachment = 0;
        colorReference.layout = vk::ImageLayout::eColorAttachmentOptimal;
        colorAttachmentReferences.push_back(colorReference);
    }

    std::vector<vk::SubpassDescription> subpasses;
    std::vector<vk::SubpassDependency> subpassDependencies;
    {
        {
            vk::SubpassDependency dependency;
            dependency.srcSubpass = 0;
            dependency.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
            dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead;
            dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            subpassDependencies.push_back(dependency);
        }

        vk::SubpassDescription subpass;
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.pDepthStencilAttachment = &depthReference;
        subpass.colorAttachmentCount = (uint32_t)colorAttachmentReferences.size();
        subpass.pColorAttachments = colorAttachmentReferences.data();
        subpasses.push_back(subpass);
    }

    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = (uint32_t)attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = (uint32_t)subpasses.size();
    renderPassInfo.pSubpasses = subpasses.data();
    renderPassInfo.dependencyCount = (uint32_t)subpassDependencies.size();
    renderPassInfo.pDependencies = subpassDependencies.data();
    _renderPass = _device.createRenderPass(renderPassInfo);
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
    _extent = vk::Extent2D((uint32_t)qsize.width(), (uint32_t)qsize.height());
    _swapchain.waitIdle();
    _swapchain.create(_extent, true);
    setupDepthStencil();
    setupFramebuffers();
}

VKWindow::~VKWindow() {
    _swapchain.destroy();
}

void VKWindow::emitClosing() {
    emit aboutToClose();
}

vk::Framebuffer VKWindow::acquireFramebuffer(const vk::Semaphore& semaphore) {
    auto result = _swapchain.acquireNextImage(semaphore);
    auto imageIndex = result.value;
    return _framebuffers[imageIndex];
}
