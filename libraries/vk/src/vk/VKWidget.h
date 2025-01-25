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

/// customized canvas that simply forwards requests/events to the singleton application
class VKWidget : public QWidget {
    Q_OBJECT

public:
    VKWidget();
    ~VKWidget();
    int getDeviceWidth() const;
    int getDeviceHeight() const;
    QSize getDeviceSize() const { return QSize(getDeviceWidth(), getDeviceHeight()); }
    QPaintEngine* paintEngine() const override;
    void createContext(QOpenGLContext* shareContext = nullptr);
    bool makeCurrent();
    void doneCurrent();
    void swapBuffers();
    vks::Context* context() { return _context; }
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

protected:
    virtual bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
    virtual bool event(QEvent* event) override;
    vks::Context* _context { nullptr };

private:
    QPaintEngine* _paintEngine { nullptr };
    bool _vsyncSupported { false };
};
