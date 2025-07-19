//
//  Created by Bradley Austin Davis on 2015/12/03
//  Copyright 2013-2018 High Fidelity, Inc.
//  Copyright 2020 Maki.
//  Copyright 2024 Overte e.V.
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


VKWidget::VKWidget(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_AcceptTouchEvents);
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_InputMethodEnabled);
    setAutoFillBackground(false);
    grabGesture(Qt::PinchGesture);
    setAcceptDrops(true);
    _paintEngine = new NullPaintEngine(); // VKTODO: what is it used for?
    //vks::Context::get().registerWidget(this);
    //_device = _vksContext.device->logicalDevice;
}

VKWidget::~VKWidget() {
    delete _paintEngine;
    _paintEngine = nullptr;
}

int VKWidget::getDeviceWidth() const {
    return width() * (windowHandle() ? (float)windowHandle()->devicePixelRatio() : 1.0f);
}

int VKWidget::getDeviceHeight() const {
    return height() * (windowHandle() ? (float)windowHandle()->devicePixelRatio() : 1.0f);
}

void VKWidget::createContext(QOpenGLContext* shareContext) {
    _context = new gl::OffscreenContext();
    _context->create(shareContext);
    _context->makeCurrent();
    _context->clear();
    _context->doneCurrent();
}

void VKWidget::swapBuffers() {
    _context->swapBuffers();
}

bool VKWidget::makeCurrent() {
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
