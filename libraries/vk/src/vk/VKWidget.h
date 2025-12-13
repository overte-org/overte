//
//  Created by Bradley Austin Davis on 2015/12/03
//  Copyright 2013-2018 High Fidelity, Inc.
//  Copyright 2020 Maki.
//  Copyright 2023-2025 Overte e.V.
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
class OffscreenContext;
}

namespace vks {
struct Context;
}

class QOpenGLContext;

// VKTODO: make this and GLWidget inherit from the same base class, for example called GraphicsWidget.
/// customized canvas that simply forwards requests/events to the singleton application
class VKWidget : public QWidget {
    Q_OBJECT

    friend struct vks::Context;

public:
    VKWidget(QWidget *parent = nullptr);
    ~VKWidget() override;

    [[nodiscard]] int getDeviceWidth() const;
    [[nodiscard]] int getDeviceHeight() const;
    [[nodiscard]] QSize getDeviceSize() const { return QSize(getDeviceWidth(), getDeviceHeight()); }
    [[nodiscard]] QPaintEngine* paintEngine() const override;
    void createContext(QOpenGLContext* shareContext = nullptr);
    bool makeCurrent();
    void doneCurrent();
    void swapBuffers();
    gl::OffscreenContext* context() { return _context; }
    QOpenGLContext* qglContext();
    [[nodiscard]] QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
    bool event(QEvent* event) override;
    gl::OffscreenContext* _context { nullptr };

private:
    QPaintEngine* _paintEngine { nullptr };

    bool _vsyncSupported { false };

public:
    VKWindow *_mainWindow; // For getting Vulkan surface, VKTODO: make a better way of setting it
};