//
//  Application_Graphics.cpp
//  interface/src
//
//  Split from Application.cpp by HifiExperiments on 3/30/24
//  Created by Andrzej Kapolka on 5/10/13.
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors.
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "Application.h"

#include <memory>

#include <QtQml/QQmlContext>

#include <AudioScriptingInterface.h>
#include <display-plugins/CompositorHelper.h>
#include <ErrorDialog.h>
#include <FramebufferCache.h>
#include <gl/GLHelpers.h>
#include <input-plugins/KeyboardMouseDevice.h>
#include <input-plugins/TouchscreenDevice.h>
#include <input-plugins/TouchscreenVirtualPadDevice.h>
#include <MainWindow.h>
#include <plugins/PluginManager.h>
#include <Preferences.h>
#include <RenderableWebEntityItem.h>
#include <scripting/AccountServicesScriptingInterface.h>
#include <scripting/HMDScriptingInterface.h>
#include <scripting/PlatformInfoScriptingInterface.h>
#include <scripting/RenderScriptingInterface.h>
#include <scripting/TTSScriptingInterface.h>
#include <Tooltip.h>
#include <ui/AddressBarDialog.h>
#include <ui/Keyboard.h>
#include <ui/LoginDialog.h>
#include <ui/OffscreenQmlSurfaceCache.h>
#include <ui/PreferencesDialog.h>
#include <ui/ResourceImageItem.h>
#include <ui/TabletScriptingInterface.h>
#include <ui/types/ContextAwareProfile.h>
#include <ui/UpdateDialog.h>

#include "DeadlockWatchdog.h"
#include "GLCanvas.h"
#include "InterfaceLogging.h"
#include "LODManager.h"
#include "Menu.h"
#include "webbrowser/WebBrowserSuggestionsEngine.h"

#if defined(Q_OS_ANDROID)
#include "AndroidHelper.h"
#endif

Q_GUI_EXPORT void qt_gl_set_global_share_context(QOpenGLContext *context);
Q_GUI_EXPORT QOpenGLContext *qt_gl_global_share_context();

static const QString SYSTEM_TABLET = "com.highfidelity.interface.tablet.system";

void Application::initializeGL() {
    qCDebug(interfaceapp) << "Created Display Window.";

#ifdef DISABLE_QML
    setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity);
#endif

    // initialize glut for shape drawing; Qt apparently initializes it on OS X
    if (_isGLInitialized) {
        return;
    } else {
        _isGLInitialized = true;
    }

    _glWidget->windowHandle()->setFormat(getDefaultOpenGLSurfaceFormat());

    // When loading QtWebEngineWidgets, it creates a global share context on startup.
    // We have to account for this possibility by checking here for an existing
    // global share context
    auto globalShareContext = qt_gl_global_share_context();

#if !defined(DISABLE_QML)
    // Build a shared canvas / context for the Chromium processes
    if (!globalShareContext) {
        // Chromium rendering uses some GL functions that prevent nSight from capturing
        // frames, so we only create the shared context if nsight is NOT active.
        if (!nsightActive()) {
            // FIXME hack access to the internal share context for the Chromium helper
            // Normally we'd want to use QWebEngine::initialize(), but we can't because
            // our primary context is a QGLWidget, which can't easily be initialized to share
            // from a QOpenGLContext.
            //
            // So instead we create a new offscreen context to share with the QGLWidget,
            // and manually set THAT to be the shared context for the Chromium helper
            OffscreenGLCanvas* chromiumShareContext = new OffscreenGLCanvas();
            chromiumShareContext->setObjectName("ChromiumShareContext");
            auto format = QSurfaceFormat::defaultFormat();
#ifdef Q_OS_MAC
            // On mac, the primary shared OpenGL context must be a 3.2 core context,
            // or chromium flips out and spews error spam (but renders fine)
            format.setMajorVersion(3);
            format.setMinorVersion(2);
#endif
            chromiumShareContext->setFormat(format);
            chromiumShareContext->create();
            if (!chromiumShareContext->makeCurrent()) {
                qCWarning(interfaceapp, "Unable to make chromium shared context current");
            }
            globalShareContext = chromiumShareContext->getContext();
            qt_gl_set_global_share_context(globalShareContext);
            chromiumShareContext->doneCurrent();
        }
    }
#endif

    _glWidget->createContext(globalShareContext);

    if (!_glWidget->makeCurrent()) {
        qCWarning(interfaceapp, "Unable to make window context current");
    }

    // Populate the global OpenGL context based on the information for the primary window GL context
    gl::ContextInfo::get(true);

#if !defined(DISABLE_QML)
    QStringList chromiumFlags;
    // HACK: re-expose mic and camera to prevent crash on domain-change in chromium's media::FakeAudioInputStream::ReadAudioFromSource()
    // Bug 21993: disable microphone and camera input
    //chromiumFlags << "--use-fake-device-for-media-stream";

    // Disable signed distance field font rendering on ATI/AMD GPUs, due to
    // https://highfidelity.manuscript.com/f/cases/13677/Text-showing-up-white-on-Marketplace-app
    std::string vendor{ (const char*)glGetString(GL_VENDOR) };
    if ((vendor.find("AMD") != std::string::npos) || (vendor.find("ATI") != std::string::npos)) {
        chromiumFlags << "--disable-distance-field-text";
    }

    // Ensure all Qt webengine processes launched from us have the appropriate command line flags
    if (!chromiumFlags.empty()) {
        qputenv("QTWEBENGINE_CHROMIUM_FLAGS", chromiumFlags.join(' ').toLocal8Bit());
    }
#endif

    if (!globalShareContext) {
        globalShareContext = _glWidget->qglContext();
        qt_gl_set_global_share_context(globalShareContext);
    }

    // Build a shared canvas / context for the QML rendering
#if !defined(DISABLE_QML)
    {
        OffscreenGLCanvas* qmlShareContext = new OffscreenGLCanvas();
        qmlShareContext->setObjectName("QmlShareContext");
        qmlShareContext->create(globalShareContext);
        if (!qmlShareContext->makeCurrent()) {
            qCWarning(interfaceapp, "Unable to make QML shared context current");
        }
        OffscreenQmlSurface::setSharedContext(qmlShareContext->getContext());
        qmlShareContext->doneCurrent();
        if (!_glWidget->makeCurrent()) {
            qCWarning(interfaceapp, "Unable to make window context current");
        }
    }
#endif

    // Build an offscreen GL context for the main thread.
    _glWidget->makeCurrent();
    glClearColor(0.2f, 0.2f, 0.2f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    _glWidget->swapBuffers();

    _graphicsEngine->initializeGPU(_glWidget);
}

void Application::initializeRenderEngine() {
    // FIXME: on low end systems os the shaders take up to 1 minute to compile, so we pause the deadlock watchdog thread.
    DeadlockWatchdogThread::withPause([&] {
        _graphicsEngine->initializeRender();
        DependencyManager::get<Keyboard>()->registerKeyboardHighlighting();
    });
}

#if !defined(DISABLE_QML)
static const char* EXCLUSION_GROUP_KEY = "exclusionGroup";

static void addDisplayPluginToMenu(const DisplayPluginPointer& displayPlugin, int index, bool active) {
    auto menu = Menu::getInstance();
    QString name = displayPlugin->getName();
    auto grouping = displayPlugin->getGrouping();
    QString groupingMenu { "" };
    Q_ASSERT(!menu->menuItemExists(MenuOption::OutputMenu, name));

    // assign the meny grouping based on plugin grouping
    switch (grouping) {
        case Plugin::ADVANCED:
            groupingMenu = "Advanced";
            break;
        case Plugin::DEVELOPER:
            groupingMenu = "Developer";
            break;
        default:
            groupingMenu = "Standard";
            break;
    }

    static QActionGroup* displayPluginGroup = nullptr;
    if (!displayPluginGroup) {
        displayPluginGroup = new QActionGroup(menu);
        displayPluginGroup->setExclusive(true);
    }
    auto parent = menu->getMenu(MenuOption::OutputMenu);
    auto action = menu->addActionToQMenuAndActionHash(parent,
        name, QKeySequence(Qt::CTRL + (Qt::Key_0 + index)), qApp,
        SLOT(updateDisplayMode()),
        QAction::NoRole, Menu::UNSPECIFIED_POSITION, groupingMenu);

    action->setCheckable(true);
    action->setChecked(active);
    displayPluginGroup->addAction(action);

    action->setProperty(EXCLUSION_GROUP_KEY, QVariant::fromValue(displayPluginGroup));
    Q_ASSERT(menu->menuItemExists(MenuOption::OutputMenu, name));
}
#endif

void Application::initializeUi() {

    // Allow remote QML content from trusted sources ONLY
    {
        auto defaultUrlValidator = OffscreenQmlSurface::getUrlValidator();
        auto newValidator = [=](const QUrl& url) -> bool {
            QString allowlistPrefix = "[ALLOWLIST ENTITY SCRIPTS]";
            QList<QString> safeURLS = { "" };
            safeURLS += qEnvironmentVariable("EXTRA_ALLOWLIST").trimmed().split(QRegExp("\\s*,\\s*"), Qt::SkipEmptyParts);

            // PULL SAFEURLS FROM INTERFACE.JSON Settings

            QVariant raw = Setting::Handle<QVariant>("private/settingsSafeURLS").get();
            QStringList settingsSafeURLS = raw.toString().trimmed().split(QRegExp("\\s*[,\r\n]+\\s*"), Qt::SkipEmptyParts);
            safeURLS += settingsSafeURLS;

            // END PULL SAFEURLS FROM INTERFACE.JSON Settings

            if (QUrl(NetworkingConstants::OVERTE_COMMUNITY_APPLICATIONS).isParentOf(url)) {
                return true;
            } else {
                for (const auto& str : safeURLS) {
                    if (!str.isEmpty() && url.toString().endsWith(".qml") &&
                        url.toString().startsWith(str)) {
                        qCDebug(interfaceapp) << "Found matching url!" << url.host();
                        return true;
                    }
                }
            }

            qCDebug(interfaceapp) << "No matching url" << url.host();
            return defaultUrlValidator(url);
        };
        OffscreenQmlSurface::setUrlValidator(newValidator);
    }

    AddressBarDialog::registerType();
    ErrorDialog::registerType();
    LoginDialog::registerType();
    Tooltip::registerType();
    UpdateDialog::registerType();

    QmlContextCallback platformInfoCallback = [](QQmlContext* context) {
        context->setContextProperty("PlatformInfo", new PlatformInfoScriptingInterface());
    };
    OffscreenQmlSurface::addAllowlistContextHandler({
        QUrl{ "hifi/tablet/TabletAddressDialog.qml" },
        QUrl{ "hifi/Card.qml" },
        QUrl{ "hifi/Pal.qml" },
        QUrl{ "hifi/NameCard.qml" },
        }, platformInfoCallback);

    QmlContextCallback ttsCallback = [](QQmlContext* context) {
        context->setContextProperty("TextToSpeech", DependencyManager::get<TTSScriptingInterface>().data());
    };
    OffscreenQmlSurface::addAllowlistContextHandler({
        QUrl{ "hifi/tts/TTS.qml" }
    }, ttsCallback);
    qmlRegisterType<ResourceImageItem>("Hifi", 1, 0, "ResourceImageItem");
    qmlRegisterType<Preference>("Hifi", 1, 0, "Preference");
    qmlRegisterType<WebBrowserSuggestionsEngine>("HifiWeb", 1, 0, "WebBrowserSuggestionsEngine");

    {
        auto tabletScriptingInterface = DependencyManager::get<TabletScriptingInterface>();
        tabletScriptingInterface->getTablet(SYSTEM_TABLET);
    }

    auto offscreenUi = getOffscreenUI();
    connect(offscreenUi.data(), &hifi::qml::OffscreenSurface::rootContextCreated,
        this, &Application::onDesktopRootContextCreated);
    connect(offscreenUi.data(), &hifi::qml::OffscreenSurface::rootItemCreated,
        this, &Application::onDesktopRootItemCreated);

#if !defined(DISABLE_QML)
    offscreenUi->setProxyWindow(_window->windowHandle());
    // OffscreenUi is a subclass of OffscreenQmlSurface specifically designed to
    // support the window management and scripting proxies for VR use
    DeadlockWatchdogThread::withPause([&] {
        offscreenUi->createDesktop(PathUtils::qmlUrl("hifi/Desktop.qml"));
    });
    // FIXME either expose so that dialogs can set this themselves or
    // do better detection in the offscreen UI of what has focus
    offscreenUi->setNavigationFocused(false);
#else
    _window->setMenuBar(new Menu());
#endif

    setupPreferences();

#if !defined(DISABLE_QML)
    _glWidget->installEventFilter(offscreenUi.data());
    offscreenUi->setMouseTranslator([=](const QPointF& pt) {
        QPointF result = pt;
        auto displayPlugin = getActiveDisplayPlugin();
        if (displayPlugin->isHmd()) {
            getApplicationCompositor().handleRealMouseMoveEvent(false);
            auto resultVec = getApplicationCompositor().getReticlePosition();
            result = QPointF(resultVec.x, resultVec.y);
        }
        return result.toPoint();
    });

    // BUGZ-1365 - the root context should explicitly default to being unable to load local HTML content
    ContextAwareProfile::restrictContext(offscreenUi->getSurfaceContext(), true);
    offscreenUi->resume();
#endif
    connect(_window, &MainWindow::windowGeometryChanged, [this](const QRect& r){
        resizeGL();
        if (_touchscreenVirtualPadDevice) {
            _touchscreenVirtualPadDevice->resize();
        }
    });

    // This will set up the input plugins UI
    for(const auto& inputPlugin : PluginManager::getInstance()->getInputPlugins()) {
        if (KeyboardMouseDevice::NAME == inputPlugin->getName()) {
            _keyboardMouseDevice = std::dynamic_pointer_cast<KeyboardMouseDevice>(inputPlugin);
        }
        if (TouchscreenDevice::NAME == inputPlugin->getName()) {
            _touchscreenDevice = std::dynamic_pointer_cast<TouchscreenDevice>(inputPlugin);
        }
        if (TouchscreenVirtualPadDevice::NAME == inputPlugin->getName()) {
            _touchscreenVirtualPadDevice = std::dynamic_pointer_cast<TouchscreenVirtualPadDevice>(inputPlugin);
#if defined(ANDROID_APP_INTERFACE)
            auto& virtualPadManager = VirtualPad::Manager::instance();
            connect(&virtualPadManager, &VirtualPad::Manager::hapticFeedbackRequested,
                    this, [](int duration) {
                        AndroidHelper::instance().performHapticFeedback(duration);
                    });
#endif
        }
    }

    auto compositorHelper = DependencyManager::get<CompositorHelper>();
    connect(compositorHelper.data(), &CompositorHelper::allowMouseCaptureChanged, this, [=] {
        if (isHMDMode()) {
            auto compositorHelper = DependencyManager::get<CompositorHelper>(); // don't capture outer smartpointer
            showCursor(compositorHelper->getAllowMouseCapture() ?
                       Cursor::Manager::lookupIcon(_preferredCursor.get()) :
                       Cursor::Icon::SYSTEM);
        }
    });

#if !defined(DISABLE_QML)
    // Pre-create a couple of offscreen surfaces to speed up tablet UI
    auto offscreenSurfaceCache = DependencyManager::get<OffscreenQmlSurfaceCache>();
    offscreenSurfaceCache->setOnRootContextCreated([&](const QString& rootObject, QQmlContext* surfaceContext) {
        if (rootObject == TabletScriptingInterface::QML) {
            // in Qt 5.10.0 there is already an "Audio" object in the QML context
            // though I failed to find it (from QtMultimedia??). So..  let it be "AudioScriptingInterface"
            surfaceContext->setContextProperty("AudioScriptingInterface", DependencyManager::get<AudioScriptingInterface>().data());
            surfaceContext->setContextProperty("Account", AccountServicesScriptingInterface::getInstance()); // DEPRECATED - TO BE REMOVED
        }
    });

    offscreenSurfaceCache->reserve(TabletScriptingInterface::QML, 1);
    offscreenSurfaceCache->reserve(render::entities::WebEntityRenderer::QML, 2);
#endif

    flushMenuUpdates();

#if !defined(DISABLE_QML)
    // Now that the menu is instantiated, ensure the display plugin menu is properly updated
    {
        DisplayPluginList displayPlugins = PluginManager::getInstance()->getDisplayPlugins();
        // first sort the plugins into groupings: standard, advanced, developer
        std::stable_sort(displayPlugins.begin(), displayPlugins.end(),
            [](const DisplayPluginPointer& a, const DisplayPluginPointer& b) -> bool { return a->getGrouping() < b->getGrouping(); });
        int dpIndex = 1;
        // concatenate the groupings into a single list in the order: standard, advanced, developer
        for(const auto& displayPlugin : displayPlugins) {
            addDisplayPluginToMenu(displayPlugin, dpIndex, _displayPlugin == displayPlugin);
            dpIndex++;
        }

        // after all plugins have been added to the menu, add a separator to the menu
        auto parent = getPrimaryMenu()->getMenu(MenuOption::OutputMenu);
        parent->addSeparator();
    }
#endif


    // The display plugins are created before the menu now, so we need to do this here to hide the menu bar
    // now that it exists
    if (_window && _window->isFullScreen()) {
        setFullscreen(nullptr, true);
    }


    setIsInterstitialMode(true);

#if defined(DISABLE_QML) && defined(Q_OS_LINUX)
    resumeAfterLoginDialogActionTaken();
#endif
}

void Application::resizeGL() {
    PROFILE_RANGE(render, __FUNCTION__);
    if (nullptr == _displayPlugin) {
        return;
    }

    auto displayPlugin = getActiveDisplayPlugin();
    // Set the desired FBO texture size. If it hasn't changed, this does nothing.
    // Otherwise, it must rebuild the FBOs
    uvec2 framebufferSize = displayPlugin->getRecommendedRenderSize();
    uvec2 renderSize = uvec2(framebufferSize);
    if (_renderResolution != renderSize) {
        _renderResolution = renderSize;
        DependencyManager::get<FramebufferCache>()->setFrameBufferSize(fromGlm(renderSize));
    }

    // FIXME the aspect ratio for stereo displays is incorrect based on this.
    float aspectRatio = displayPlugin->getRecommendedAspectRatio();
    _myCamera.setProjection(glm::perspective(glm::radians(_fieldOfView.get()), aspectRatio,
                                             DEFAULT_NEAR_CLIP, DEFAULT_FAR_CLIP));
    // Possible change in aspect ratio
    {
        QMutexLocker viewLocker(&_viewMutex);
        _myCamera.loadViewFrustum(_viewFrustum);
    }

#if !defined(DISABLE_QML)
    getOffscreenUI()->resize(fromGlm(displayPlugin->getRecommendedUiSize()));
#endif
}

glm::uvec2 Application::getCanvasSize() const {
    return glm::uvec2(_glWidget->width(), _glWidget->height());
}

float Application::getRenderResolutionScale() const {
    return RenderScriptingInterface::getInstance()->getViewportResolutionScale();
}

void Application::updateRenderArgs(float deltaTime) {
    _graphicsEngine->editRenderArgs([this, deltaTime](AppRenderArgs& appRenderArgs) {
        PerformanceTimer perfTimer("editRenderArgs");
        appRenderArgs._headPose = getHMDSensorPose();

        auto myAvatar = getMyAvatar();

        // update the avatar with a fresh HMD pose
        {
            PROFILE_RANGE(render, "/updateAvatar");
            myAvatar->updateFromHMDSensorMatrix(appRenderArgs._headPose);
        }

        auto lodManager = DependencyManager::get<LODManager>();

        float sensorToWorldScale = getMyAvatar()->getSensorToWorldScale();
        appRenderArgs._sensorToWorldScale = sensorToWorldScale;
        appRenderArgs._sensorToWorld = getMyAvatar()->getSensorToWorldMatrix();
        {
            PROFILE_RANGE(render, "/buildFrustrumAndArgs");
            {
                QMutexLocker viewLocker(&_viewMutex);
                // adjust near clip plane to account for sensor scaling.
                auto adjustedProjection = glm::perspective(glm::radians(_fieldOfView.get()),
                    getActiveDisplayPlugin()->getRecommendedAspectRatio(),
                    DEFAULT_NEAR_CLIP * sensorToWorldScale,
                    DEFAULT_FAR_CLIP);
                _viewFrustum.setProjection(adjustedProjection);
                _viewFrustum.calculate();
            }
            appRenderArgs._renderArgs = RenderArgs(_graphicsEngine->getGPUContext(), lodManager->getVisibilityDistance(),
                lodManager->getBoundaryLevelAdjust(), lodManager->getLODFarHalfAngleTan(), lodManager->getLODNearHalfAngleTan(),
                lodManager->getLODFarDistance(), lodManager->getLODNearDistance(), RenderArgs::DEFAULT_RENDER_MODE,
                RenderArgs::MONO, RenderArgs::DEFERRED, RenderArgs::RENDER_DEBUG_NONE);
            appRenderArgs._renderArgs._scene = getMain3DScene();

            {
                QMutexLocker viewLocker(&_viewMutex);
                appRenderArgs._renderArgs.setViewFrustum(_viewFrustum);
            }
        }
        {
            PROFILE_RANGE(render, "/resizeGL");
            bool showWarnings = false;
            bool suppressShortTimings = false;
            auto menu = Menu::getInstance();
            if (menu) {
                suppressShortTimings = menu->isOptionChecked(MenuOption::SuppressShortTimings);
                showWarnings = menu->isOptionChecked(MenuOption::PipelineWarnings);
            }
            PerformanceWarning::setSuppressShortTimings(suppressShortTimings);
            PerformanceWarning warn(showWarnings, "Application::paintGL()");
            resizeGL();
        }

        this->updateCamera(appRenderArgs._renderArgs, deltaTime);
        appRenderArgs._eyeToWorld = _myCamera.getTransform();
        appRenderArgs._isStereo = false;

        {
            auto hmdInterface = DependencyManager::get<HMDScriptingInterface>();
            float ipdScale = hmdInterface->getIPDScale();

            // scale IPD by sensorToWorldScale, to make the world seem larger or smaller accordingly.
            ipdScale *= sensorToWorldScale;

            auto baseProjection = appRenderArgs._renderArgs.getViewFrustum().getProjection();

            if (getActiveDisplayPlugin()->isStereo()) {
                // Stereo modes will typically have a larger projection matrix overall,
                // so we ask for the 'mono' projection matrix, which for stereo and HMD
                // plugins will imply the combined projection for both eyes.
                //
                // This is properly implemented for the Oculus plugins, but for OpenVR
                // and Stereo displays I'm not sure how to get / calculate it, so we're
                // just relying on the left FOV in each case and hoping that the
                // overall culling margin of error doesn't cause popping in the
                // right eye.  There are FIXMEs in the relevant plugins
                _myCamera.setProjection(getActiveDisplayPlugin()->getCullingProjection(baseProjection));
                appRenderArgs._isStereo = true;

                auto& eyeOffsets = appRenderArgs._eyeOffsets;
                auto& eyeProjections = appRenderArgs._eyeProjections;

                // FIXME we probably don't need to set the projection matrix every frame,
                // only when the display plugin changes (or in non-HMD modes when the user
                // changes the FOV manually, which right now I don't think they can.
                for_each_eye([&](Eye eye) {
                    // Grab the translation
                    eyeOffsets[eye] = getActiveDisplayPlugin()->getEyeToHeadTransform(eye);
                    // Apply IPD scaling
                    eyeOffsets[eye][3][0] *= ipdScale;
                    eyeProjections[eye] = getActiveDisplayPlugin()->getEyeProjection(eye, baseProjection);
                });

                // Configure the type of display / stereo
                appRenderArgs._renderArgs._displayMode = (isHMDMode() ? RenderArgs::STEREO_HMD : RenderArgs::STEREO_MONITOR);
            }
        }

        appRenderArgs._renderArgs._stencilMaskMode = getActiveDisplayPlugin()->getStencilMaskMode();
        if (appRenderArgs._renderArgs._stencilMaskMode == StencilMaskMode::MESH) {
            appRenderArgs._renderArgs._stencilMaskOperator = getActiveDisplayPlugin()->getStencilMaskMeshOperator();
        }

        {
            QMutexLocker viewLocker(&_viewMutex);
            _myCamera.loadViewFrustum(_displayViewFrustum);
            appRenderArgs._view = glm::inverse(_displayViewFrustum.getView());
        }

        {
            QMutexLocker viewLocker(&_viewMutex);
            appRenderArgs._renderArgs.setViewFrustum(_displayViewFrustum);
        }


        // HACK
        // load the view frustum
        // FIXME: This preDisplayRender call is temporary until we create a separate render::scene for the mirror rendering.
        // Then we can move this logic into the Avatar::simulate call.
        myAvatar->preDisplaySide(&appRenderArgs._renderArgs);
    });
}
