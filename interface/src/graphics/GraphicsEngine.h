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
     *
     * @return
     */
    bool checkPendingRenderEvent();

    /**
     *
     * @return
     */
    size_t getRenderFrameCount() const { return _renderFrameCount; }

    /**
     *
     * @return
     */
    float getRenderLoopRate() const { return _renderLoopCounter.rate(); }

    // Feed Graphics Engine with new frame configuration
    /**
     *
     * @param editor
     */
    void editRenderArgs(RenderArgsEditor editor);

private:
    // Thread specific calls
    /**
     *
     */
    void render_performFrame();

    /**
     *
     * @param renderArgs
     */
    void render_runRenderFrame(RenderArgs* renderArgs);

protected:
    /**
     *
     */
    mutable QRecursiveMutex _renderArgsMutex;

    /**
     *
     */
    AppRenderArgs _appRenderArgs;

    /**
     *
     */
    RateCounter<500> _renderLoopCounter;

    /**
     *
     */
    uint32_t _renderFrameCount{ 0 };

    /**
     *
     */
    render::ScenePointer _renderScene{ new render::Scene(glm::vec3(-0.5f * (float)TREE_SCALE), (float)TREE_SCALE) };

    /**
     *
     */
    render::EnginePointer _renderEngine{ new render::RenderEngine() };

    /**
     *
     */
    gpu::ContextPointer _gpuContext; // initialized during window creation

    /**
     *
     */
    QObject* _renderEventHandler{ nullptr };

    /**
     *
     */
    friend class RenderEventHandler;

    /**
     *
     */
    FrameTimingsScriptingInterface _frameTimingsScriptingInterface;

    /**
     *
     */
    std::shared_ptr<ProceduralSkybox> _splashScreen { std::make_shared<ProceduralSkybox>() };

    /**
     *
     */
    NetworkTexturePointer _texture;
#ifndef Q_OS_ANDROID
    /**
     *
     */
    std::atomic<bool> _programsCompiled { false };
#else
    std::atomic<bool> _programsCompiled { true };
#endif

    friend class Application;
};

#endif // hifi_GraphicsEngine_h
