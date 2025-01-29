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


#pragma once

#include <QtWidgets/QWidget>
#include "Context.h"
#include "VulkanSwapChain.h"

namespace gl {
class Context;
}

namespace vks {
class Context;
}

class QOpenGLContext;

/// customized canvas that simply forwards requests/events to the singleton application
class VKWidget : public QWidget {
    Q_OBJECT

    friend struct vks::Context;

public:
    VKWidget();
    ~VKWidget() override;

    void createSwapchain();
    void createSurface();

    int getDeviceWidth() const;
    int getDeviceHeight() const;
    QSize getDeviceSize() const { return QSize(getDeviceWidth(), getDeviceHeight()); }
    QPaintEngine* paintEngine() const override;
    void createContext(QOpenGLContext* shareContext = nullptr);
    bool makeCurrent();
    void doneCurrent();
    void swapBuffers();
    gl::Context* context() { return _context; }
    QOpenGLContext* qglContext();
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

protected slots:
    virtual void resizeFramebuffer();

protected:
    virtual bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
    virtual bool event(QEvent* event) override;
    gl::Context* _context { nullptr };

    void setupRenderPass();
    void setupDepthStencil();

    void setupFramebuffers();
    void createCommandBuffers();
    void vulkanCleanup(); // Called by the context before backend is destroyed.

private:
    QPaintEngine* _paintEngine { nullptr };

    bool _vsyncSupported { false };

public:
    vks::Context& _vksContext{ vks::Context::get() };
    //VkDevice _device{ VK_NULL_HANDLE };
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