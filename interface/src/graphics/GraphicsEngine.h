//
//  GraphicsEngine.h
//
//  Created by Sam Gateau on 29/6/2018.
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_GraphicsEngine_h
#define hifi_GraphicsEngine_h

#include <gl/OffscreenGLCanvas.h>
#ifdef USE_GL
#include <gl/GLWidget.h>
#else
#include <vk/VKWidget.h>
#endif
#include <qmutex.h>

#include <render/Engine.h>
#include <procedural/ProceduralSkybox.h>

#include <OctreeConstants.h>
#include <shared/RateCounter.h>

#include "FrameTimingsScriptingInterface.h"

/**
 * Contains current frame configuration for the renderer. Instance of `RenderArgs` is stored in `GraphicsEngine` and edited using
 * `GraphicsEngine::editRenderArgs` but the application on the main thread.
 */
struct AppRenderArgs {
    render::Args _renderArgs;
    glm::mat4 _eyeToWorld;
    glm::mat4 _view;
    glm::mat4 _eyeOffsets[2];
    glm::mat4 _eyeProjections[2];
    glm::mat4 _headPose;
    glm::mat4 _sensorToWorld;
    float _sensorToWorldScale{ 1.0f };
    bool _isStereo{ false };
};

using RenderArgsEditor = std::function <void(AppRenderArgs&)>;

/**
 * Main class for the graphics engine.
 */
class GraphicsEngine {
public:
    GraphicsEngine();
    ~GraphicsEngine();

#ifdef USE_GL
    /**
     * Creates render event handler, rendering backend and context.
     * Initializes compiling the basic set of shaders if needed.
     *
     * @param primaryWidget Widget containing 3D view on the main window.
     */
    void initializeGPU(GLWidget*  primaryWidget);
#else
    /**
     * Creates render event handler, rendering backend and context.
     * Initializes compiling the basic set of shaders if needed.
     *
     * @param primaryWidget Widget containing 3D view on the main window.
     */
    void initializeGPU(VKWidget*  primaryWidget);
#endif

    /**
     * Adds renderer jobs, loads renderer configuration from a json file and initializes shape pipelines.
     */
    void initializeRender();

    /**
     * Tells render thread to start operation.
     */
    void startup();

    /**
     * Flushes and processes all transactions, shutdown the gpu::Context and the render engine.
     * Schedules render event handler for deletion.
     * Called on main thread from the Application destructor.
     */
    void shutdown();

    /**
     * @return Shared pointer to the render::Scene.
     */
    render::ScenePointer getRenderScene() const { return _renderScene; }

    /**
     * @return Shared pointer to the RenderEngine.
     */
    render::EnginePointer getRenderEngine() const { return _renderEngine; }

    /**
     * @return Shared pointer to the gpu::Context.
     */
    gpu::ContextPointer getGPUContext() const { return _gpuContext; }

    /**
     * Same as the one in application.
     * Used by render event handler.
     * Can return false during shutdown, and when framerate is throttled.
     * There's a setting to throttle framerate when out of focus which uses this function.
     * @return `true` if application should render frame, `false` if not.
     */
    bool shouldPaint() const;

    /**
     * Used by `Application::onPresent` to check if a new Frame object creation event can be posted.
     * Internally it changes state of `_pendingRenderingEvent` atomic_bool.
     *
     * @return `true` if previous Frame object creation event has finished and new one can be posted.
     */
    bool checkPendingRenderEvent();

    /**
     * @return Number of Frame objects generated this far. Does not have to be equal to number of frames actually rendered.
     */
    size_t getRenderFrameCount() const { return _renderFrameCount; }

    /**
     * @return Framerate of the "Render" loop. Render loop does not actually render, it only generates frames to be rendered on a separate thread.
     */
    float getRenderLoopRate() const { return _renderLoopCounter.rate(); }

    /**
     * Give Graphics Engine new frame configuration.
     * @param editor
     */
    void editRenderArgs(RenderArgsEditor editor);

private:
    // Thread specific calls
    /**
     * Creates a new frame object with all rendering operations and submits it to the display plugin.
     * Does not actually render frame.
     */
    void render_performFrame();

    /**
     * Called by `render_performFrame`. Generates `Frame` object to be rendered later on the Present thread.
     * @param renderArgs Configuration for this frame.
     */
    void render_runRenderFrame(RenderArgs* renderArgs);

protected:
    /**
     * Mutex used to protect frame configuration.
     */
    mutable QRecursiveMutex _renderArgsMutex;

    /**
     * Frame configuration. Updated from the main thread and used by the "Render" thread, which generates `Frame` objects but
     * does not render them.
     */
    AppRenderArgs _appRenderArgs;

    /**
     * Used for measuring rate at which frame objects are generated.
     */
    RateCounter<500> _renderLoopCounter;

    /**
     * Counts number of frame objects that were created.
     */
    uint32_t _renderFrameCount{ 0 };

    /**
     * DOCTODO
     */
    render::ScenePointer _renderScene{ new render::Scene(glm::vec3(-0.5f * (float)TREE_SCALE), (float)TREE_SCALE) };

    /**
     * `RenderEngine` object contains all the tasks needed to generate a Frame object that can later be submitted to
     * display plugin to be rendered.
     */
    render::EnginePointer _renderEngine{ new render::RenderEngine() };

    /**
     * Object that manages rendering backend and GPU statistics.
     */
    gpu::ContextPointer _gpuContext; // initialized during window creation

    /**
     * `RenderEventHandler` responsible for creating Frame objects and submitting them to display plugin, where later
     * they get rendered on Present thread.
     */
    QObject* _renderEventHandler{ nullptr };

    friend class RenderEventHandler;

    /**
     * Used for "FrameTimings" scripting API.
     */
    FrameTimingsScriptingInterface _frameTimingsScriptingInterface;

    /**
     * A simple skybox shown before the shaders are compiled.
     */
    std::shared_ptr<ProceduralSkybox> _splashScreen { std::make_shared<ProceduralSkybox>() };

    /**
     * Splash screen texture. Shown as a skybox before the shaders load and compile.
     */
    NetworkTexturePointer _texture;
#ifndef Q_OS_ANDROID
    /**
     * Changes to true when the shaders are loaded and compiled.
     * Used to show the splash screen skybox.
     */
    std::atomic<bool> _programsCompiled { false };
#else
    std::atomic<bool> _programsCompiled { true };
#endif

    friend class Application;
};

#endif // hifi_GraphicsEngine_h
