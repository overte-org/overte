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

#include "Config.h"
#include "Context.h"
#include "Swapchain.h"

class VKWindow : public QWindow {
    Q_OBJECT
public:
    VKWindow(QScreen* screen = nullptr);
    virtual ~VKWindow();

    void createSwapchain();
    void queuePresent(const vk::ArrayProxy<const vk::Semaphore>& waitSemaphores);
    const vk::SurfaceKHR& createSurface();
    const vk::SurfaceKHR& getSurface() { return _surface; }
    vks::Swapchain& getSwapchain() { return _swapchain; }
    vk::Framebuffer acquireFramebuffer(const vk::Semaphore& semaphore);

signals:
    void aboutToClose();

protected:
    friend class VkCloseEventFilter;
    void emitClosing();

protected slots:
    virtual void resizeFramebuffer();

protected:
    void resizeEvent(QResizeEvent* event) override;

    void setupRenderPass();
    void setupDepthStencil();
    void setupFramebuffers();

public:
    vks::Context& _context{ vks::Context::get() };
    const vk::Device& _device{ _context.device };
    vk::SurfaceKHR _surface;
    vk::RenderPass _renderPass;
    vk::Extent2D _extent;
    vks::Swapchain _swapchain;
    vks::Image _depthStencil;
    std::vector<vk::Framebuffer> _framebuffers;
    QTimer* _resizeTimer{ nullptr };
};
