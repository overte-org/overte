//
//  Created by Bradley Austin Davis on 2016/03/19
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once

#include <QtCore/QCoreApplication>
#include <QtGui/QWindow>
#include <QtGui/qevent.h>
#include <QtCore/QTimer>
#include <QtCore/QDebug>

#include "Allocation.h"
#include "Config.h"
#include "Context.h"
#include "VulkanSwapChain.h"

class VKWindow : public QWindow {
    Q_OBJECT
public:
    VKWindow(QScreen* screen = nullptr);
    virtual ~VKWindow();

    void createSwapchain();
    //void queuePresent(const vk::ArrayProxy<const VkSemaphore>& waitSemaphores);
    const void createSurface();
    //const VkSurfaceKHR& getSurface() { return _surface; }
    VulkanSwapChain& getSwapchain() { return _swapchain; }
    VkFramebuffer acquireFramebuffer(const VkSemaphore& semaphore);

signals:
    void aboutToClose();

protected:
    friend class VkCloseEventFilter;
    friend struct vks::Context;
    void emitClosing();

protected slots:
    virtual void resizeFramebuffer();

protected:
    void resizeEvent(QResizeEvent* event) override;

    void setupRenderPass();
    void setupDepthStencil();
    void setupFramebuffers();
    void createCommandBuffers();
    void vulkanCleanup(); // Called by the context before backend is destroyed.

public:
    vks::Context& _context{ vks::Context::get() };
    const VkDevice& _device{ _context.device->logicalDevice };
    //VkSurfaceKHR _surface;
    VkRenderPass _renderPass{};
    VkExtent2D _extent;
    VulkanSwapChain _swapchain;
    VkSemaphore _acquireCompleteSemaphore{};
    VkSemaphore _renderCompleteSemaphore{};
    std::vector<VkCommandBuffer> _drawCommandBuffers;
    struct : vks::Allocation {
        bool isAllocated {false};
        VkImage image;
        VkImageView view;
    } _depthStencil{};
    std::vector<VkFramebuffer> _frameBuffers;
    QTimer* _resizeTimer{ nullptr };
    std::atomic<bool> _isVulkanCleanupComplete{ false };
};
