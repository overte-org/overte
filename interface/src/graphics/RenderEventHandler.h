//
//  RenderEventHandler.h
//
//  Created by Bradley Austin Davis on 29/6/2018.
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_RenderEventHandler_h
#define hifi_RenderEventHandler_h

#include <functional>
#include <QEvent>
#include <QElapsedTimer>
#include "gl/OffscreenGLCanvas.h"

enum ApplicationEvent {
    // Execute a lambda function
    Lambda = QEvent::User + 1,
    // Trigger the next render
    Render,
    // Trigger the next idle
    Idle,
};

/**
 * This class does not handle actual rendering, it just generates Frame objects and submits them to be rendered
 * on the Present thread.
 */
class RenderEventHandler : public QObject {
    using Parent = QObject;
    Q_OBJECT
public:
    using CheckCall = std::function <bool()>;
    using RenderCall = std::function <void()>;

    /**
     * Function that gets called to check if given Frame object creation event should run or should be skipped.
     * Can be used to throttle rate at which Frame objects are generated..
     */
    CheckCall _checkCall;

    /**
     * Function that generates new Frame object and submits it to display plugin to be rendered on Present thread.
     */
    RenderCall _renderCall;

    /**
     *
     * @param checkCall
     * @param renderCall
     */
    RenderEventHandler(CheckCall checkCall, RenderCall renderCall);

    /**
     * Time at which last Frame object was generated.
     * Can be used for throttling by `GraphicsEngine::shouldPaint`.
     */
    QElapsedTimer _lastTimeRendered;

    /**
     * Frame object creation event is in progress currently or the thread is paused.
     * Set to `false` after the render event finishes.
     * Set to `true` in `GraphicsEngine::checkPendingRenderEvent`.
     */
    std::atomic<bool> _pendingRenderEvent{ true };

    /**
     * Processing render events is initially paused, and is resumed during the startup process using this call.
     */
    void resumeThread();

private:
    /**
     * Initialize the thread on which Frame objects will be generated.
     * Thread is called Render thread, but does not do rendering.
     */
    void initialize();

    /**
     * Generate and a new Frame object and submit it to display plugin to be rendered later on a separate thread.
     */
    void render();

    /**
     * Qt event handler. Handles "render" event (which generates a Frame object but does not render) posted my main application.
     * @param event Qt event.
     * @return True if the event was processed.
     */
    bool event(QEvent* event) override;
};

#endif // #include hifi_RenderEventHandler_h
