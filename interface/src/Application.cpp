//
//  Application.cpp
//  interface/src
//
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

#include <QDesktopWidget>
#include <QDesktopServices>
#include <QtGui/QClipboard>
#include <QtNetwork/QLocalSocket>
#include <QtNetwork/QLocalServer>
#include <QtQuick/QQuickWindow>
#include <QWidget>

#include <AccountManager.h>
#include <AddressManager.h>
#include <AnimationCacheScriptingInterface.h>
#include <AnimDebugDraw.h>
#include <AvatarBookmarks.h>
#include <audio/AudioScope.h>
#include <avatar/AvatarManager.h>
#include <avatar/AvatarPackager.h>
#include <avatar/GrabManager.h>
#include <BuildInfo.h>
#include <controllers/ScriptingInterface.h>
#include <controllers/UserInputMapper.h>
#include <CrashHelpers.h>
#include <DebugDraw.h>
#include <DesktopPreviewProvider.h>
#include <display-plugins/CompositorHelper.h>
#include <display-plugins/DisplayPlugin.h>
#include <DomainAccountManager.h>
#include <EntityScriptServerLogClient.h>
#include <FramebufferCache.h>
#include <gl/GLHelpers.h>
#include <GPUIdent.h>
#include <graphics-scripting/GraphicsScriptingInterface.h>
#include <hfm/ModelFormatRegistry.h>
#include <input-plugins/InputPlugin.h>
#include <input-plugins/KeyboardMouseDevice.h>
#include <LocationScriptingInterface.h>
#include <LogHandler.h>
#include <MainWindow.h>
#include <MessagesClient.h>
#include <material-networking/TextureCacheScriptingInterface.h>
#include <model-networking/ModelCacheScriptingInterface.h>
#include <networking/CloseEventSender.h>
#include <OffscreenUi.h>
#include <PickManager.h>
#include <platform/Platform.h>
#include <platform/PlatformKeys.h>
#include <platform/backend/PlatformInstance.h>
#include <plugins/OculusPlatformPlugin.h>
#include <plugins/PluginManager.h>
#include <plugins/PluginUtils.h>
#include <plugins/SteamClientPlugin.h>
#include <Preferences.h>
#include <procedural/MaterialCacheScriptingInterface.h>
#include <QmlFragmentClass.h>
#include <QmlWebWindowClass.h>
#include <QmlWindowClass.h>
#include <raypick/PickScriptingInterface.h>
#include <raypick/PointerScriptingInterface.h>
#include <raypick/RayPickScriptingInterface.h>
#include <recording/RecordingScriptingInterface.h>
#include <render/EngineStats.h>
#include <ResourceScriptingInterface.h>
#include <SandboxUtils.h>
#include <SceneScriptingInterface.h>
#include <ScriptEngines.h>
#include <scripting/AccountServicesScriptingInterface.h>
#include <scripting/Audio.h>
#include <scripting/ClipboardScriptingInterface.h>
#include <scripting/ControllerScriptingInterface.h>
#include <scripting/DesktopScriptingInterface.h>
#include <scripting/HMDScriptingInterface.h>
#include <scripting/KeyboardScriptingInterface.h>
#include <scripting/MenuScriptingInterface.h>
#include <scripting/PerformanceScriptingInterface.h>
#include <scripting/PlatformInfoScriptingInterface.h>
#include <scripting/RatesScriptingInterface.h>
#include <scripting/RenderScriptingInterface.h>
#include <scripting/SelectionScriptingInterface.h>
#include <scripting/SettingsScriptingInterface.h>
#include <scripting/TestScriptingInterface.h>
#include <scripting/WindowScriptingInterface.h>
#ifndef Q_OS_ANDROID
#include <shared/FileLogger.h>
#endif
#include <shared/GlobalAppProperties.h>
#include <shared/PlatformHelper.h>
#include <shared/QtHelpers.h>
#include <SoundCacheScriptingInterface.h>
#include <StatTracker.h>
#include <ui/AvatarInputs.h>
#include <ui/AnimStats.h>
#include <ui/TabletScriptingInterface.h>
#include <ui/Keyboard.h>
#include <ui/OctreeStatsProvider.h>
#include <ui/OffscreenQmlSurfaceCache.h>
#include <ui/Snapshot.h>
#include <ui/SnapshotAnimated.h>
#include <ui/StandAloneJSConsole.h>
#include <ui/Stats.h>
#include <ui/ToolbarScriptingInterface.h>
#include <UserActivityLogger.h>
#include <UserActivityLoggerScriptingInterface.h>
#include <UsersScriptingInterface.h>

#include "AboutUtil.h"
#include "ApplicationEventHandler.h"
#include "AudioClient.h"
#include "DeadlockWatchdog.h"
#include "GLCanvas.h"
#include "LocationBookmarks.h"
#include "LODManager.h"
#include "Menu.h"
#include "ResourceRequestObserver.h"
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
#include "SpeechRecognizer.h"
#endif
#include "Util.h"

#if defined(Q_OS_WIN)
#include "WindowsSystemInfo.h"

// On Windows PC, NVidia Optimus laptop, we want to enable NVIDIA GPU
// FIXME seems to be broken.
extern "C" {
 _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif

#if defined(Q_OS_MAC)
// On Mac OS, disable App Nap to prevent audio glitches while running in the background
#include "AppNapDisabler.h"
static AppNapDisabler appNapDisabler;   // disabled, while in scope
#endif

#if defined(Q_OS_ANDROID)
#include <android/log.h>
#endif

// For processing on QThreadPool, we target a number of threads after reserving some
// based on how many are being consumed by the application and the display plugin.  However,
// we will never drop below the 'min' value
static const int MIN_PROCESSING_THREAD_POOL_SIZE = 2;
static const int ENTITY_SERVER_CONNECTION_TIMEOUT = 5000;

const float DEFAULT_HMD_TABLET_SCALE_PERCENT = 60.0f;
const float DEFAULT_DESKTOP_TABLET_SCALE_PERCENT = 75.0f;
const bool DEFAULT_DESKTOP_TABLET_BECOMES_TOOLBAR = true;
const bool DEFAULT_HMD_TABLET_BECOMES_TOOLBAR = false;
const bool DEFAULT_PREFER_STYLUS_OVER_LASER = false;
const bool DEFAULT_PREFER_AVATAR_FINGER_OVER_STYLUS = false;
const bool DEFAULT_MOUSE_CAPTURE_VR = false;
const bool DEFAULT_SHOW_GRAPHICS_ICON = true;
const bool DEFAULT_MINI_TABLET_ENABLED = false;
const bool DEFAULT_AWAY_STATE_WHEN_FOCUS_LOST_IN_VR_ENABLED = true;

static const quint64 TOO_LONG_SINCE_LAST_SEND_DOWNSTREAM_AUDIO_STATS = 1 * USECS_PER_SECOND;

static const QString DESKTOP_LOCATION = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
static const QString KEEP_ME_LOGGED_IN_SETTING_NAME = "keepMeLoggedIn";
const QString DEFAULT_CURSOR_NAME = "SYSTEM";

Setting::Handle<int> sessionRunTime { "sessionRunTime", 0 };

void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message) {
    QString logMessage = LogHandler::getInstance().printMessage((LogMsgType) type, context, message);

    if (!logMessage.isEmpty()) {
#ifdef Q_OS_ANDROID
        const char * local=logMessage.toStdString().c_str();
        switch (type) {
            case QtDebugMsg:
                __android_log_write(ANDROID_LOG_DEBUG,"Interface",local);
                break;
            case QtInfoMsg:
                __android_log_write(ANDROID_LOG_INFO,"Interface",local);
                break;
            case QtWarningMsg:
                __android_log_write(ANDROID_LOG_WARN,"Interface",local);
                break;
            case QtCriticalMsg:
                __android_log_write(ANDROID_LOG_ERROR,"Interface",local);
                break;
            case QtFatalMsg:
            default:
                __android_log_write(ANDROID_LOG_FATAL,"Interface",local);
                abort();
        }
#else
        qApp->getLogger()->addMessage(qPrintable(logMessage));
#endif
    }
}

Application::Application(
    int& argc, char** argv,
    const QCommandLineParser& parser,
    QElapsedTimer& startupTimer
) :
    QApplication(argc, argv),
    _window(new MainWindow(desktop())),
    // Menu needs to be initialized before other initializers. Otherwise deadlock happens on qApp->getWindow()->menuBar().
    _isMenuInitialized(initMenu()),
#ifndef Q_OS_ANDROID
    _logger(new FileLogger(this)),
#endif
    _sessionRunTimer(startupTimer),
    _lastNackTime(usecTimestampNow()),
    _lastSendDownstreamAudioStats(usecTimestampNow()),
    _firstRun(Settings::firstRun, true),
    _previousScriptLocation("LastScriptLocation", DESKTOP_LOCATION),
    // UI
    _hmdTabletScale("hmdTabletScale", DEFAULT_HMD_TABLET_SCALE_PERCENT),
    _desktopTabletScale("desktopTabletScale", DEFAULT_DESKTOP_TABLET_SCALE_PERCENT),
    _desktopTabletBecomesToolbarSetting("desktopTabletBecomesToolbar", DEFAULT_DESKTOP_TABLET_BECOMES_TOOLBAR),
    _hmdTabletBecomesToolbarSetting("hmdTabletBecomesToolbar", DEFAULT_HMD_TABLET_BECOMES_TOOLBAR),
    _preferStylusOverLaserSetting("preferStylusOverLaser", DEFAULT_PREFER_STYLUS_OVER_LASER),
    _preferAvatarFingerOverStylusSetting("preferAvatarFingerOverStylus", DEFAULT_PREFER_AVATAR_FINGER_OVER_STYLUS),
    _defaultMouseCaptureVR("defaultMouseCaptureVR", DEFAULT_MOUSE_CAPTURE_VR),
    _constrainToolbarPosition("toolbar/constrainToolbarToCenterX", true),
    _awayStateWhenFocusLostInVREnabled("awayStateWhenFocusLostInVREnabled", DEFAULT_AWAY_STATE_WHEN_FOCUS_LOST_IN_VR_ENABLED),
    _preferredCursor("preferredCursor", DEFAULT_CURSOR_NAME),
    _darkTheme("darkTheme", true),
    _miniTabletEnabledSetting("miniTabletEnabled", DEFAULT_MINI_TABLET_ENABLED),
    // Entities
    _maxOctreePacketsPerSecond("maxOctreePPS", DEFAULT_MAX_OCTREE_PPS),
    _maxOctreePPS(_maxOctreePacketsPerSecond.get()),
    // Camera
    _fieldOfView("fieldOfView", DEFAULT_FIELD_OF_VIEW_DEGREES),
    _cameraClippingEnabled("cameraClippingEnabled", false)
{
    setProperty(hifi::properties::CRASHED, _previousSessionCrashed);

    LogHandler::getInstance().moveToThread(thread());
    LogHandler::getInstance().setupRepeatedMessageFlusher();
    qInstallMessageHandler(messageHandler);

    DependencyManager::set<PathUtils>();
}

Application::~Application() {
    // remove avatars from physics engine
    if (auto avatarManager = DependencyManager::get<AvatarManager>()) {
        // AvatarManager may not yet exist in case of an early exit

        avatarManager->clearOtherAvatars();
        auto myCharacterController = getMyAvatar()->getCharacterController();
        myCharacterController->clearDetailedMotionStates();

        PhysicsEngine::Transaction transaction;
        avatarManager->buildPhysicsTransaction(transaction);
        _physicsEngine->processTransaction(transaction);
        avatarManager->handleProcessedPhysicsTransaction(transaction);
        avatarManager->deleteAllAvatars();
    }

    if (_physicsEngine) {
        _physicsEngine->setCharacterController(nullptr);
    }

    // the _shapeManager should have zero references
    _shapeManager.collectGarbage();
    assert(_shapeManager.getNumShapes() == 0);

    if (_graphicsEngine) {
        // shutdown graphics engine
        _graphicsEngine->shutdown();
    }

    _gameWorkload.shutdown();

    DependencyManager::destroy<Preferences>();
    PlatformHelper::shutdown();

    if (_entityClipboard) {
        _entityClipboard->eraseAllOctreeElements();
        _entityClipboard.reset();
    }

    if (_octreeProcessor) {
        _octreeProcessor->terminate();
    }

    if (_entityEditSender) {
        _entityEditSender->terminate();
    }

    if (auto pluginManager = PluginManager::getInstance()) {
        if (auto steamClient = pluginManager->getSteamClientPlugin()) {
            steamClient->shutdown();
        }

        if (auto oculusPlatform = pluginManager->getOculusPlatformPlugin()) {
            oculusPlatform->shutdown();
        }
    }

    DependencyManager::destroy<PluginManager>();

    DependencyManager::destroy<CompositorHelper>(); // must be destroyed before the FramebufferCache

    DependencyManager::destroy<SoundCacheScriptingInterface>();

    DependencyManager::destroy<AudioInjectorManager>();
    DependencyManager::destroy<AvatarManager>();
    DependencyManager::destroy<AnimationCacheScriptingInterface>();
    DependencyManager::destroy<AnimationCache>();
    DependencyManager::destroy<FramebufferCache>();
    DependencyManager::destroy<MaterialCacheScriptingInterface>();
    DependencyManager::destroy<MaterialCache>();
    DependencyManager::destroy<TextureCacheScriptingInterface>();
    DependencyManager::destroy<TextureCache>();
    DependencyManager::destroy<ModelCacheScriptingInterface>();
    DependencyManager::destroy<ModelCache>();
    DependencyManager::destroy<ModelFormatRegistry>();
    DependencyManager::destroy<ScriptCache>();
    DependencyManager::destroy<SoundCacheScriptingInterface>();
    DependencyManager::destroy<SoundCache>();
    DependencyManager::destroy<OctreeStatsProvider>();
    DependencyManager::destroy<GeometryCache>();

    if (auto resourceManager = DependencyManager::get<ResourceManager>()) {
        resourceManager->cleanup();
    }

    // remove the NodeList from the DependencyManager
    DependencyManager::destroy<NodeList>();

#if 0
    ConnexionClient::getInstance().destroy();
#endif
    // The window takes ownership of the menu, so this has the side effect of destroying it.
    _window->setMenuBar(nullptr);

    _window->deleteLater();

    // make sure that the quit event has finished sending before we take the application down
    if (auto closeEventSender = DependencyManager::get<CloseEventSender>()) {
        while (!closeEventSender->hasFinishedQuitEvent() && !closeEventSender->hasTimedOutQuitEvent()) {
            // sleep a little so we're not spinning at 100%
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        // quit the thread used by the closure event sender
        closeEventSender->thread()->quit();
    }

    // Can't log to file past this point, FileLogger about to be deleted
    qInstallMessageHandler(LogHandler::verboseMessageHandler);

#ifdef Q_OS_MAC
    // 26 Feb 2021 - Tried re-enabling this call but OSX still crashes on exit.
    //
    // 10/16/2019 - Disabling this call. This causes known crashes (A), and it is not
    // fully understood whether it might cause other unknown crashes (B).
    //
    // (A) Although we try to shutdown the ScriptEngine threads in onAboutToQuit, there is
    // currently no guarantee that they have stopped. Waiting on them to stop has so far appeared to
    // never return on Mac, causing the application to hang on shutdown. Because ScriptEngines
    // may still be running, they may end up receiving events that are triggered from this processEvents call,
    // and then try to access resources that are no longer available at this point in time.
    // If the ScriptEngine threads were fully destroyed before getting here, this would
    // not be an issue.
    //
    // (B) It seems likely that a bunch of potential event handlers are dependent on Application
    // and other common dependencies to be available and not destroyed or in the middle of being
    // destroyed.


    // Clear the event queue before application is totally destructed.
    // This will drain the messasge queue of pending "deleteLaters" queued up
    // during shutdown of the script engines.
    // We do this here because there is a possiblty that [NSApplication terminate:]
    // will be called during processEvents which will invoke all static destructors.
    // We want to postpone this utill the last possible moment.
    //QCoreApplication::processEvents();
#endif
}

bool Application::isServerlessMode() const {
    auto tree = getEntities()->getTree();
    if (tree) {
        return tree->isServerlessMode();
    }

    return false;
}

QString Application::getUserAgent() {
    if (QThread::currentThread() != thread()) {
        QString userAgent;

        BLOCKING_INVOKE_METHOD(this, "getUserAgent", Q_RETURN_ARG(QString, userAgent));

        return userAgent;
    }

    QString userAgent = NetworkingConstants::OVERTE_USER_AGENT + "/" + BuildInfo::VERSION + "; "
        + QSysInfo::productType() + " " + QSysInfo::productVersion() + ")";

    auto formatPluginName = [](QString name) -> QString { return name.trimmed().replace(" ", "-");  };

    // For each plugin, add to userAgent
    const auto& displayPlugins = PluginManager::getInstance()->getDisplayPlugins();
    for (const auto& dp : displayPlugins) {
        if (dp->isActive() && dp->isHmd()) {
            userAgent += " " + formatPluginName(dp->getName());
        }
    }
    const auto& inputPlugins = PluginManager::getInstance()->getInputPlugins();
    for (const auto& ip : inputPlugins) {
        if (ip->isActive()) {
            userAgent += " " + formatPluginName(ip->getName());
        }
    }
    // for codecs, we include all of them, even if not active
    const auto& codecPlugins = PluginManager::getInstance()->getCodecPlugins();
    for (const auto& cp : codecPlugins) {
        userAgent += " " + formatPluginName(cp->getName());
    }

    return userAgent;
}

static const QString CONTENT_SET_NAME_QUERY_PARAM = "name";
void Application::replaceDomainContent(const QString& url, const QString& itemName) {
    qCDebug(interfaceapp) << "Attempting to replace domain content";
    QUrl msgUrl(url);
    QUrlQuery urlQuery(msgUrl.query());
    urlQuery.addQueryItem(CONTENT_SET_NAME_QUERY_PARAM, itemName);
    msgUrl.setQuery(urlQuery.query(QUrl::QUrl::FullyEncoded));
    QByteArray urlData(msgUrl.toString(QUrl::QUrl::FullyEncoded).toUtf8());
    auto limitedNodeList = DependencyManager::get<NodeList>();
    const auto& domainHandler = limitedNodeList->getDomainHandler();

    auto octreeFilePacket = NLPacket::create(PacketType::DomainContentReplacementFromUrl, urlData.size(), true);
    octreeFilePacket->write(urlData);
    limitedNodeList->sendPacket(std::move(octreeFilePacket), domainHandler.getSockAddr());

    auto addressManager = DependencyManager::get<AddressManager>();
    addressManager->handleLookupString(DOMAIN_SPAWNING_POINT);
    QString newHomeAddress = addressManager->getHost() + DOMAIN_SPAWNING_POINT;
    qCDebug(interfaceapp) << "Setting new home bookmark to: " << newHomeAddress;
    DependencyManager::get<LocationBookmarks>()->setHomeLocationToAddress(newHomeAddress);
}

void Application::openDirectory(const QString& path) {
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, "openDirectory", Q_ARG(const QString&, path));
        return;
    }

    QString dirPath = path;
#if defined(Q_OS_WIN)
    const QString FILE_SCHEME = "file:///";
#else
    const QString FILE_SCHEME = "file://";
#endif
    if (dirPath.startsWith(FILE_SCHEME)) {
        dirPath.remove(0, FILE_SCHEME.length());
    }
    QFileInfo fileInfo(dirPath);
    if (fileInfo.isDir()) {
        auto scheme = QUrl(path).scheme();
        QDesktopServices::unsetUrlHandler(scheme);
        QDesktopServices::openUrl(path);
        QDesktopServices::setUrlHandler(scheme, this, "showUrlHandler");
    }
}

void Application::forceLoginWithTokens(const QString& tokens) {
    DependencyManager::get<AccountManager>()->setAccessTokens(tokens);
    Setting::Handle<bool>(KEEP_ME_LOGGED_IN_SETTING_NAME, true).set(true);
}

void Application::setConfigFileURL(const QString& fileUrl) {
    DependencyManager::get<AccountManager>()->setConfigFileURL(fileUrl);
}

void Application::loadAvatarScripts(const QVector<QString>& urls) {
    auto scriptEngines = DependencyManager::get<ScriptEngines>();
    auto runningScripts = scriptEngines->getRunningScripts();
    for (auto url : urls) {
        int index = runningScripts.indexOf(url);
        if (index < 0) {
            auto scriptEnginePointer = scriptEngines->loadScript(url, false);
            if (scriptEnginePointer) {
                scriptEnginePointer->setType(ScriptManager::Type::AVATAR);
            }
        }
    }
}

void Application::unloadAvatarScripts() {
    auto scriptEngines = DependencyManager::get<ScriptEngines>();
    auto urls = scriptEngines->getRunningScripts();
    for (auto url : urls) {
        auto scriptEngine = scriptEngines->getScriptEngine(url);
        if (scriptEngine->getType() == ScriptManager::Type::AVATAR) {
            scriptEngines->stopScript(url, false);
        }
    }
}

void Application::copyToClipboard(const QString& text) {
    if (QThread::currentThread() != qApp->thread()) {
        QMetaObject::invokeMethod(this, "copyToClipboard");
        return;
    }

    // assume that the address is being copied because the user wants a shareable address
    QApplication::clipboard()->setText(text);
}

void Application::registerScriptEngineWithApplicationServices(ScriptManagerPointer& scriptManager) {
    auto scriptEngine = scriptManager->engine();
    scriptManager->setEmitScriptUpdatesFunction([this]() {
        SharedNodePointer entityServerNode = DependencyManager::get<NodeList>()->soloNodeOfType(NodeType::EntityServer);
        return !entityServerNode || isPhysicsEnabled();
    });

    // setup the packet sender of the script engine's scripting interfaces so
    // we can use the same ones from the application.
    auto entityScriptingInterface = DependencyManager::get<EntityScriptingInterface>();
    entityScriptingInterface->setPacketSender(_entityEditSender.get());
    entityScriptingInterface->setEntityTree(getEntities()->getTree());

    if (property(hifi::properties::TEST).isValid()) {
        scriptEngine->registerGlobalObject("Test", TestScriptingInterface::getInstance());
    }

    scriptEngine->registerGlobalObject("PlatformInfo", PlatformInfoScriptingInterface::getInstance());
    scriptEngine->registerGlobalObject("Rates", new RatesScriptingInterface(this));

    scriptEngine->registerGlobalObject("AvatarList", DependencyManager::get<AvatarManager>().data());

    scriptEngine->registerGlobalObject("Camera", &_myCamera);

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    scriptEngine->registerGlobalObject("SpeechRecognizer", DependencyManager::get<SpeechRecognizer>().data());
#endif

    ClipboardScriptingInterface* clipboardScriptable = new ClipboardScriptingInterface();
    scriptEngine->registerGlobalObject("Clipboard", clipboardScriptable);
    connect(scriptManager.get(), &ScriptManager::finished, clipboardScriptable, &ClipboardScriptingInterface::deleteLater);

    scriptEngine->registerGlobalObject("Overlays", &_overlays);

    bool clientScript = scriptManager->isClientScript();

#if !defined(DISABLE_QML)
    scriptEngine->registerGlobalObject("OffscreenFlags", getOffscreenUI()->getFlags());
    if (clientScript) {
        scriptEngine->registerGlobalObject("Desktop", DependencyManager::get<DesktopScriptingInterface>().data());
    } else {
        auto desktopScriptingInterface = new DesktopScriptingInterface(nullptr, true);
        scriptEngine->registerGlobalObject("Desktop", desktopScriptingInterface);
        if (QThread::currentThread() != thread()) {
            desktopScriptingInterface->moveToThread(thread());
        }
    }
#endif

    scriptEngine->registerGlobalObject("Toolbars", DependencyManager::get<ToolbarScriptingInterface>().data());

    scriptEngine->registerGlobalObject("Tablet", DependencyManager::get<TabletScriptingInterface>().data());
    // FIXME remove these deprecated names for the tablet scripting interface
    scriptEngine->registerGlobalObject("tabletInterface", DependencyManager::get<TabletScriptingInterface>().data());

    auto toolbarScriptingInterface = DependencyManager::get<ToolbarScriptingInterface>().data();
    DependencyManager::get<TabletScriptingInterface>().data()->setToolbarScriptingInterface(toolbarScriptingInterface);

    scriptEngine->registerGlobalObject("Window", DependencyManager::get<WindowScriptingInterface>().data());
    scriptEngine->registerGetterSetter("location", LocationScriptingInterface::locationGetter,
                        LocationScriptingInterface::locationSetter, "Window");
    // register `location` on the global object.
    scriptEngine->registerGetterSetter("location", LocationScriptingInterface::locationGetter,
                                       LocationScriptingInterface::locationSetter);

    scriptEngine->registerFunction("OverlayWindow", clientScript ? QmlWindowClass::constructor : QmlWindowClass::restricted_constructor);
#if !defined(Q_OS_ANDROID) && !defined(DISABLE_QML)
    scriptEngine->registerFunction("OverlayWebWindow", clientScript ? QmlWebWindowClass::constructor : QmlWebWindowClass::restricted_constructor);
#endif
    scriptEngine->registerFunction("QmlFragment", clientScript ? QmlFragmentClass::constructor : QmlFragmentClass::restricted_constructor);

    scriptEngine->registerGlobalObject("Menu", MenuScriptingInterface::getInstance());
    scriptEngine->registerGlobalObject("DesktopPreviewProvider", DependencyManager::get<DesktopPreviewProvider>().data());
#if !defined(DISABLE_QML)
    scriptEngine->registerGlobalObject("Stats", Stats::getInstance());
#endif
    scriptEngine->registerGlobalObject("Settings", SettingsScriptingInterface::getInstance());
    scriptEngine->registerGlobalObject("Snapshot", DependencyManager::get<Snapshot>().data());
    scriptEngine->registerGlobalObject("AudioStats", DependencyManager::get<AudioClient>()->getStats().data());
    scriptEngine->registerGlobalObject("AudioScope", DependencyManager::get<AudioScope>().data());
    scriptEngine->registerGlobalObject("AvatarBookmarks", DependencyManager::get<AvatarBookmarks>().data());
    scriptEngine->registerGlobalObject("LocationBookmarks", DependencyManager::get<LocationBookmarks>().data());

    scriptEngine->registerGlobalObject("RayPick", DependencyManager::get<RayPickScriptingInterface>().data());
    scriptEngine->registerGlobalObject("Picks", DependencyManager::get<PickScriptingInterface>().data());
    scriptEngine->registerGlobalObject("Pointers", DependencyManager::get<PointerScriptingInterface>().data());

    // Caches
    scriptEngine->registerGlobalObject("AnimationCache", DependencyManager::get<AnimationCacheScriptingInterface>().data());
    scriptEngine->registerGlobalObject("TextureCache", DependencyManager::get<TextureCacheScriptingInterface>().data());
    scriptEngine->registerGlobalObject("MaterialCache", DependencyManager::get<MaterialCacheScriptingInterface>().data());
    scriptEngine->registerGlobalObject("ModelCache", DependencyManager::get<ModelCacheScriptingInterface>().data());
    scriptEngine->registerGlobalObject("SoundCache", DependencyManager::get<SoundCacheScriptingInterface>().data());

    scriptEngine->registerGlobalObject("DialogsManager", _dialogsManagerScriptingInterface);

    scriptEngine->registerGlobalObject("Account", AccountServicesScriptingInterface::getInstance()); // DEPRECATED - TO BE REMOVED
    scriptEngine->registerGlobalObject("GlobalServices", AccountServicesScriptingInterface::getInstance()); // DEPRECATED - TO BE REMOVED
    scriptEngine->registerGlobalObject("AccountServices", AccountServicesScriptingInterface::getInstance());

    scriptEngine->registerGlobalObject("AvatarManager", DependencyManager::get<AvatarManager>().data());

    scriptEngine->registerGlobalObject("LODManager", DependencyManager::get<LODManager>().data());

    scriptEngine->registerGlobalObject("Keyboard", DependencyManager::get<KeyboardScriptingInterface>().data());
    scriptEngine->registerGlobalObject("Performance", new PerformanceScriptingInterface());

    scriptEngine->registerGlobalObject("Paths", DependencyManager::get<PathUtils>().data());

    scriptEngine->registerGlobalObject("HMD", DependencyManager::get<HMDScriptingInterface>().data());
    scriptEngine->registerFunction("HMD", "getHUDLookAtPosition2D", HMDScriptingInterface::getHUDLookAtPosition2D, 0);
    scriptEngine->registerFunction("HMD", "getHUDLookAtPosition3D", HMDScriptingInterface::getHUDLookAtPosition3D, 0);

    scriptEngine->registerGlobalObject("Scene", DependencyManager::get<SceneScriptingInterface>().data());
    scriptEngine->registerGlobalObject("Render", RenderScriptingInterface::getInstance());
    scriptEngine->registerGlobalObject("Workload", _gameWorkload._engine->getConfiguration().get());

    scriptEngine->registerGlobalObject("Graphics", DependencyManager::get<GraphicsScriptingInterface>().data());

    scriptEngine->registerGlobalObject("ScriptDiscoveryService", DependencyManager::get<ScriptEngines>().data());
    scriptEngine->registerGlobalObject("Reticle", getApplicationCompositor().getReticleInterface());

    scriptEngine->registerGlobalObject("UserActivityLogger", DependencyManager::get<UserActivityLoggerScriptingInterface>().data());
    scriptEngine->registerGlobalObject("Users", DependencyManager::get<UsersScriptingInterface>().data());

    if (auto steamClient = PluginManager::getInstance()->getSteamClientPlugin()) {
        scriptEngine->registerGlobalObject("Steam", new SteamScriptingInterface(scriptManager.get(), steamClient.get()));
    }
    auto scriptingInterface = DependencyManager::get<controller::ScriptingInterface>();
    scriptEngine->registerGlobalObject("Controller", scriptingInterface.data());

    {
        auto connection = std::make_shared<QMetaObject::Connection>();
        *connection = scriptManager->connect(scriptManager.get(), &ScriptManager::scriptEnding, [this, scriptManager, connection]() {
            // Request removal of controller routes with callbacks to a given script engine
            auto userInputMapper = DependencyManager::get<UserInputMapper>();
            // scheduleScriptEndpointCleanup will have the last instance of shared pointer to script manager
            // so script manager will get deleted as soon as cleanup is done
            userInputMapper->scheduleScriptEndpointCleanup(scriptManager);
            QObject::disconnect(*connection);
        });
    }

    UserInputMapper::registerControllerTypes(scriptEngine.get());

    auto recordingInterface = DependencyManager::get<RecordingScriptingInterface>();
    scriptEngine->registerGlobalObject("Recording", recordingInterface.data());

    auto entityScriptServerLog = DependencyManager::get<EntityScriptServerLogClient>();
    scriptEngine->registerGlobalObject("EntityScriptServerLog", entityScriptServerLog.data());
    scriptEngine->registerGlobalObject("AvatarInputs", AvatarInputs::getInstance());
    scriptEngine->registerGlobalObject("Selection", DependencyManager::get<SelectionScriptingInterface>().data());
    scriptEngine->registerGlobalObject("AddressManager", DependencyManager::get<AddressManager>().data());
    scriptEngine->registerGlobalObject("About", AboutUtil::getInstance());
    scriptEngine->registerGlobalObject("HifiAbout", AboutUtil::getInstance());  // Deprecated.
    scriptEngine->registerGlobalObject("ResourceRequestObserver", DependencyManager::get<ResourceRequestObserver>().data());

    // connect this script engines printedMessage signal to the global ScriptEngines these various messages
    auto scriptEngines = DependencyManager::get<ScriptEngines>().data();
    connect(scriptManager.get(), &ScriptManager::printedMessage, scriptEngines, &ScriptEngines::onPrintedMessage);
    connect(scriptManager.get(), &ScriptManager::errorMessage, scriptEngines, &ScriptEngines::onErrorMessage);
    connect(scriptManager.get(), &ScriptManager::warningMessage, scriptEngines, &ScriptEngines::onWarningMessage);
    connect(scriptManager.get(), &ScriptManager::infoMessage, scriptEngines, &ScriptEngines::onInfoMessage);
    connect(scriptManager.get(), &ScriptManager::clearDebugWindow, scriptEngines, &ScriptEngines::onClearDebugWindow);
}

// Snapshots
void Application::addSnapshotOperator(const SnapshotOperator& snapshotOperator) {
    std::lock_guard<std::mutex> lock(_snapshotMutex);
    _snapshotOperators.push(snapshotOperator);
    _hasPrimarySnapshot = _hasPrimarySnapshot || std::get<2>(snapshotOperator);
}

bool Application::takeSnapshotOperators(std::queue<SnapshotOperator>& snapshotOperators) {
    std::lock_guard<std::mutex> lock(_snapshotMutex);
    bool hasPrimarySnapshot = _hasPrimarySnapshot;
    _hasPrimarySnapshot = false;
    _snapshotOperators.swap(snapshotOperators);
    return hasPrimarySnapshot;
}

void Application::takeSnapshot(bool notify, bool includeAnimated, float aspectRatio, const QString& filename) {
    addSnapshotOperator(std::make_tuple([notify, includeAnimated, aspectRatio, filename](const QImage& snapshot) {
        qApp->postLambdaEvent([snapshot, notify, includeAnimated, aspectRatio, filename] {
            QString path = DependencyManager::get<Snapshot>()->saveSnapshot(snapshot, filename, TestScriptingInterface::getInstance()->getTestResultsLocation());

            // If we're not doing an animated snapshot as well...
            if (!includeAnimated) {
                if (!path.isEmpty()) {
                    // Tell the dependency manager that the capture of the still snapshot has taken place.
                    emit DependencyManager::get<WindowScriptingInterface>()->stillSnapshotTaken(path, notify);
                }
            } else if (!SnapshotAnimated::isAlreadyTakingSnapshotAnimated()) {
                // Get an animated GIF snapshot and save it
                SnapshotAnimated::saveSnapshotAnimated(path, aspectRatio, DependencyManager::get<WindowScriptingInterface>());
            }
        });
    }, aspectRatio, true));
}

void Application::takeSecondaryCameraSnapshot(const bool& notify, const QString& filename) {
    addSnapshotOperator(std::make_tuple([notify, filename](const QImage& snapshot) {
        qApp->postLambdaEvent([snapshot, notify, filename] {
            QString snapshotPath = DependencyManager::get<Snapshot>()->saveSnapshot(snapshot, filename, TestScriptingInterface::getInstance()->getTestResultsLocation());

            emit DependencyManager::get<WindowScriptingInterface>()->stillSnapshotTaken(snapshotPath, notify);
        });
    }, 0.0f, false));
}

void Application::takeSecondaryCamera360Snapshot(const glm::vec3& cameraPosition, const bool& cubemapOutputFormat, const bool& notify, const QString& filename) {
    postLambdaEvent([notify, filename, cubemapOutputFormat, cameraPosition] {
        DependencyManager::get<Snapshot>()->save360Snapshot(cameraPosition, cubemapOutputFormat, notify, filename);
    });
}

void Application::shareSnapshot(const QString& path, const QUrl& href) {
    postLambdaEvent([path, href] {
        // not much to do here, everything is done in snapshot code...
        DependencyManager::get<Snapshot>()->uploadSnapshot(path, href);
    });
}

#if defined(Q_OS_ANDROID)
void Application::beforeEnterBackground() {
    auto nodeList = DependencyManager::get<NodeList>();
    nodeList->setSendDomainServerCheckInEnabled(false);
    nodeList->reset("Entering background", true);
    clearDomainOctreeDetails();
}

void Application::enterBackground() {
    QMetaObject::invokeMethod(DependencyManager::get<AudioClient>().data(),
                              "stop", Qt::BlockingQueuedConnection);
// Quest only supports one plugin which can't be deactivated currently
#if !defined(ANDROID_APP_QUEST_INTERFACE)
    if (getActiveDisplayPlugin()->isActive()) {
        getActiveDisplayPlugin()->deactivate();
    }
#endif
}

void Application::enterForeground() {
    QMetaObject::invokeMethod(DependencyManager::get<AudioClient>().data(),
                                  "start", Qt::BlockingQueuedConnection);
// Quest only supports one plugin which can't be deactivated currently
#if !defined(ANDROID_APP_QUEST_INTERFACE)
    if (!getActiveDisplayPlugin() || getActiveDisplayPlugin()->isActive() || !getActiveDisplayPlugin()->activate()) {
        qWarning() << "Could not re-activate display plugin";
    }
#endif
    auto nodeList = DependencyManager::get<NodeList>();
    nodeList->setSendDomainServerCheckInEnabled(true);
}

void Application::toggleAwayMode(){
    QKeyEvent event = QKeyEvent (QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QCoreApplication::sendEvent (this, &event);
}
#endif

// FIXME?  perhaps two, one for the main thread and one for the offscreen UI rendering thread?
static const int UI_RESERVED_THREADS = 1;
// Windows won't let you have all the cores
static const int OS_RESERVED_THREADS = 1;
void Application::updateThreadPoolCount() const {
    auto reservedThreads = UI_RESERVED_THREADS + OS_RESERVED_THREADS + _displayPlugin->getRequiredThreadCount();
    auto availableThreads = QThread::idealThreadCount() - reservedThreads;
    auto threadPoolSize = std::max(MIN_PROCESSING_THREAD_POOL_SIZE, availableThreads);
    qCDebug(interfaceapp) << "Ideal Thread Count " << QThread::idealThreadCount();
    qCDebug(interfaceapp) << "Reserved threads " << reservedThreads;
    qCDebug(interfaceapp) << "Setting thread pool size to " << threadPoolSize;
    QThreadPool::globalInstance()->setMaxThreadCount(threadPoolSize);
}

void Application::gotoTutorial() {
    const QString TUTORIAL_ADDRESS = "file:///~/serverless/tutorial.json";
    DependencyManager::get<AddressManager>()->handleLookupString(TUTORIAL_ADDRESS);
}

void Application::goToErrorDomainURL(QUrl errorDomainURL) {
    // disable physics until we have enough information about our new location to not cause craziness.
    setIsServerlessMode(errorDomainURL.scheme() != URL_SCHEME_OVERTE);
    if (isServerlessMode()) {
        loadErrorDomain(errorDomainURL);
    }
    updateWindowTitle();
}

void Application::handleLocalServerConnection() const {
    auto server = qobject_cast<QLocalServer*>(sender());
    Q_ASSERT(server != nullptr);

    qCDebug(interfaceapp) << "Got connection on local server from additional instance - waiting for parameters";

    auto socket = server->nextPendingConnection();

    connect(socket, &QLocalSocket::readyRead, this, &Application::readArgumentsFromLocalSocket);

    qApp->getWindow()->raise();
    qApp->getWindow()->activateWindow();
}

void Application::readArgumentsFromLocalSocket() const {
    auto socket = qobject_cast<QLocalSocket*>(sender());
    Q_ASSERT(socket != nullptr);

    auto message = socket->readAll();
    socket->deleteLater();

    qCDebug(interfaceapp) << "Read from connection: " << message;

    // If we received a message, try to open it as a URL
    if (message.length() > 0) {
        DependencyManager::get<AddressManager>()->handleLookupString(QString::fromUtf8(message));
    }
}

void Application::showUrlHandler(const QUrl& url) {
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, "showUrlHandler", Q_ARG(const QUrl&, url));
        return;
    }

    ModalDialogListener* dlg = OffscreenUi::asyncQuestion("Confirm openUrl", "Do you recognize this path or code and want to open or execute it: " + url.toDisplayString());
    QObject::connect(dlg, &ModalDialogListener::response, this, [=](QVariant answer) {
        QObject::disconnect(dlg, &ModalDialogListener::response, this, nullptr);
        if (QMessageBox::Yes == static_cast<QMessageBox::StandardButton>(answer.toInt())) {
            // Unset the handler, open the URL, and the reset the handler
            QDesktopServices::unsetUrlHandler(url.scheme());
            QDesktopServices::openUrl(url);
            QDesktopServices::setUrlHandler(url.scheme(), this, "showUrlHandler");
        }
    });
}

void Application::hmdVisibleChanged(bool visible) {
    // TODO
    // calling start and stop will change audio input and ouput to default audio devices.
    // we need to add a pause/unpause functionality to AudioClient for this to work properly
#if 0
    if (visible) {
        QMetaObject::invokeMethod(DependencyManager::get<AudioClient>().data(), "start", Qt::QueuedConnection);
    } else {
        QMetaObject::invokeMethod(DependencyManager::get<AudioClient>().data(), "stop", Qt::QueuedConnection);
    }
#endif
}

void Application::reloadResourceCaches() {
    resetPhysicsReadyInformation();

    // Query the octree to refresh everything in view
    _queryExpiry = SteadyClock::now();
    _octreeQuery.incrementConnectionID();

    queryOctree(NodeType::EntityServer, PacketType::EntityQuery);

    getMyAvatar()->prepareAvatarEntityDataForReload();
    // Clear the entities and their renderables
    getEntities()->clear();

    DependencyManager::get<AssetClient>()->clearCache();
    //It's already cleared in reloadAllScripts so I'm not sure this is necessary.
    //DependencyManager::get<ScriptCache>()->clearCache();

    // Clear all the resource caches
    DependencyManager::get<ResourceCacheSharedItems>()->clear();
    DependencyManager::get<AnimationCache>()->refreshAll();
    DependencyManager::get<SoundCache>()->refreshAll();
    DependencyManager::get<MaterialCache>()->refreshAll();
    DependencyManager::get<ModelCache>()->refreshAll();
    ShaderCache::instance().refreshAll();
    DependencyManager::get<TextureCache>()->refreshAll();
    DependencyManager::get<recording::ClipCache>()->refreshAll();

    DependencyManager::get<NodeList>()->reset("Reloading resources");  // Force redownload of .fst models

    DependencyManager::get<ScriptEngines>()->reloadAllScripts();
    getOffscreenUI()->clearCache();

    DependencyManager::get<Keyboard>()->createKeyboard();

    getMyAvatar()->resetFullAvatarURL();
}

void Application::updateHeartbeat() const {
    DeadlockWatchdogThread::updateHeartbeat();
}

void Application::deadlockApplication() {
    qCDebug(interfaceapp) << "Intentionally deadlocked Interface";
    // Using a loop that will *technically* eventually exit (in ~600 billion years)
    // to avoid compiler warnings about a loop that will never exit
    for (uint64_t i = 1; i != 0; ++i) {
        QThread::sleep(1);
    }
}

// cause main thread to be unresponsive for 35 seconds
void Application::unresponsiveApplication() {
    // to avoid compiler warnings about a loop that will never exit
    uint64_t start = usecTimestampNow();
    uint64_t UNRESPONSIVE_FOR_SECONDS = 35;
    uint64_t UNRESPONSIVE_FOR_USECS = UNRESPONSIVE_FOR_SECONDS * USECS_PER_SECOND;
    qCDebug(interfaceapp) << "Intentionally cause Interface to be unresponsive for " << UNRESPONSIVE_FOR_SECONDS << " seconds";
    while (usecTimestampNow() - start < UNRESPONSIVE_FOR_USECS) {
        QThread::sleep(1);
    }
}

// used to test "shutdown" crash annotation.
void Application::crashOnShutdown() {
    qDebug() << "crashOnShutdown(), ON PURPOSE!";
    _crashOnShutdown = true;
    quit();
}

void Application::rotationModeChanged() const {
    if (!Menu::getInstance()->isOptionChecked(MenuOption::CenterPlayerInView)) {
        getMyAvatar()->setHeadPitch(0);
    }
}

void Application::setIsServerlessMode(bool serverlessDomain) {
    DependencyManager::get<NodeList>()->setSendDomainServerCheckInEnabled(!serverlessDomain);
    auto tree = getEntities()->getTree();
    if (tree) {
        tree->setIsServerlessMode(serverlessDomain);
        _waitForServerlessToBeSet = false;
    }
}

std::map<QString, QString> Application::prepareServerlessDomainContents(QUrl domainURL, QByteArray data) {
    QUuid serverlessSessionID = QUuid::createUuid();
    getMyAvatar()->setSessionUUID(serverlessSessionID);
    auto nodeList = DependencyManager::get<NodeList>();
    nodeList->setSessionUUID(serverlessSessionID);

    // there is no domain-server to tell us our permissions, so enable all
    NodePermissions permissions;
    permissions.setAll(true);
    nodeList->setPermissions(permissions);

    // FIXME: Lock the main tree and import directly into it.
    EntityTreePointer tmpTree(std::make_shared<EntityTree>());
    tmpTree->setIsServerlessMode(true);
    tmpTree->createRootElement();
    auto myAvatar = getMyAvatar();
    tmpTree->setMyAvatar(myAvatar);
    bool success = tmpTree->readFromByteArray(domainURL.toString(), data);
    if (success) {
        tmpTree->reaverageOctreeElements();
        tmpTree->sendEntities(_entityEditSender.get(), getEntities()->getTree(), "domain", 0, 0, 0);
    }
    std::map<QString, QString> namedPaths = tmpTree->getNamedPaths();

    // we must manually eraseAllOctreeElements(false) else the tmpTree will mem-leak
    tmpTree->eraseAllOctreeElements(false);

    return namedPaths;
}

void Application::loadServerlessDomain(QUrl domainURL) {
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, "loadServerlessDomain", Q_ARG(QUrl, domainURL));
        return;
    }

    if (domainURL.isEmpty()) {
        return;
    }

    QString trimmedUrl = domainURL.toString().trimmed();
    bool DEFAULT_IS_OBSERVABLE = true;
    const qint64 DEFAULT_CALLER_ID = -1;
    auto request = DependencyManager::get<ResourceManager>()->createResourceRequest(
        this, trimmedUrl, DEFAULT_IS_OBSERVABLE, DEFAULT_CALLER_ID, "Application::loadServerlessDomain");

    if (!request) {
        return;
    }

    connect(request, &ResourceRequest::finished, this, [=]() {
        if (request->getResult() == ResourceRequest::Success) {
            auto namedPaths = prepareServerlessDomainContents(domainURL, request->getData());
            auto nodeList = DependencyManager::get<NodeList>();
            nodeList->getDomainHandler().connectedToServerless(namedPaths);
            _octreeProcessor->getFullSceneReceivedCounter()++;
        }
        request->deleteLater();
    });
    request->send();
}

void Application::loadErrorDomain(QUrl domainURL) {
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, "loadErrorDomain", Q_ARG(QUrl, domainURL));
        return;
    }

    loadServerlessDomain(domainURL);
}

void Application::setIsInterstitialMode(bool interstitialMode) {
    bool enableInterstitial = DependencyManager::get<NodeList>()->getDomainHandler().getInterstitialModeEnabled();
    if (enableInterstitial) {
        if (_interstitialMode != interstitialMode) {
            _interstitialMode = interstitialMode;
            emit interstitialModeChanged(_interstitialMode);

            DependencyManager::get<AudioClient>()->setAudioPaused(_interstitialMode);
            DependencyManager::get<AvatarManager>()->setMyAvatarDataPacketsPaused(_interstitialMode);
        }
    }
}

void Application::updateVerboseLogging() {
    auto menu = Menu::getInstance();
    if (!menu) {
        return;
    }
    bool enable = menu->isOptionChecked(MenuOption::VerboseLogging);

    QString rules =
        "hifi.*.info=%1\n"
        "hifi.audio-stream.debug=false\n"
        "hifi.audio-stream.info=false";
    rules = rules.arg(enable ? "true" : "false");
    QLoggingCategory::setFilterRules(rules);
}

static const QString CACHEBUST_SCRIPT_REQUIRE_SETTING_NAME = "cachebustScriptRequire";
void Application::setCachebustRequire() {
    auto menu = Menu::getInstance();
    if (!menu) {
        return;
    }
    bool enable = menu->isOptionChecked(MenuOption::CachebustRequire);

    Setting::Handle<bool>{ CACHEBUST_SCRIPT_REQUIRE_SETTING_NAME, false }.set(enable);
}

QString Application::getGraphicsCardType() {
    return GPUIdent::getInstance()->getName();
}

bool Application::gpuTextureMemSizeStable() {
    auto renderConfig = qApp->getRenderEngine()->getConfiguration();
    auto renderStats = renderConfig->getConfig<render::EngineStats>("Stats");

    qint64 textureResourceGPUMemSize = renderStats->textureResourceGPUMemSize;
    qint64 texturePopulatedGPUMemSize = renderStats->textureResourcePopulatedGPUMemSize;
    qint64 textureTransferSize = renderStats->texturePendingGPUTransferSize;

    if (_gpuTextureMemSizeAtLastCheck == textureResourceGPUMemSize) {
        _gpuTextureMemSizeStabilityCount++;
    } else {
        _gpuTextureMemSizeStabilityCount = 0;
    }
    _gpuTextureMemSizeAtLastCheck = textureResourceGPUMemSize;

    if (_gpuTextureMemSizeStabilityCount >= _minimumGPUTextureMemSizeStabilityCount) {
        return (textureResourceGPUMemSize == texturePopulatedGPUMemSize) && (textureTransferSize == 0);
    }
    return false;
}

void Application::runTests() {
    runTimingTests();
    runUnitTests();
}

void Application::resetPhysicsReadyInformation() {
    // we've changed domains or cleared out caches or something.  we no longer know enough about the
    // collision information of nearby entities to make running bullet be safe.
    _octreeProcessor->getFullSceneReceivedCounter() = 0;
    _fullSceneCounterAtLastPhysicsCheck = 0;
    _gpuTextureMemSizeStabilityCount = 0;
    _gpuTextureMemSizeAtLastCheck = 0;
    _physicsEnabled = false;
    _octreeProcessor->stopSafeLanding();
}

static const QString ACTIVE_DISPLAY_PLUGIN_SETTING_NAME = "activeDisplayPlugin";
void Application::onAboutToQuit() {
    auto &ch = CrashHandler::getInstance();
    ch.setAnnotation("shutdown", "1");

    // quickly save AvatarEntityData before the EntityTree is dismantled
    getMyAvatar()->saveAvatarEntityDataToSettings();

    emit beforeAboutToQuit();

    if (getLoginDialogPoppedUp() && _firstRun.get()) {
        _firstRun.set(false);
    }

    for(const auto& inputPlugin : PluginManager::getInstance()->getInputPlugins()) {
        if (inputPlugin->isActive()) {
            inputPlugin->deactivate();
        }
    }

    // The active display plugin needs to be loaded before the menu system is active,
    // so its persisted explicitly here
    Setting::Handle<QString>{ ACTIVE_DISPLAY_PLUGIN_SETTING_NAME }.set(getActiveDisplayPlugin()->getName());

    getActiveDisplayPlugin()->deactivate();
    if (_autoSwitchDisplayModeSupportedHMDPlugin
        && _autoSwitchDisplayModeSupportedHMDPlugin->isSessionActive()) {
        _autoSwitchDisplayModeSupportedHMDPlugin->endSession();
    }
    // use the CloseEventSender via a QThread to send an event that says the user asked for the app to close
    DependencyManager::get<CloseEventSender>()->startThread();

    // Hide Running Scripts dialog so that it gets destroyed in an orderly manner; prevents warnings at shutdown.
#if !defined(DISABLE_QML)
    getOffscreenUI()->hide("RunningScripts");
#endif

    _aboutToQuit = true;

    cleanupBeforeQuit();

    if (_crashOnShutdown) {
        // triggered by crash menu
        crash::nullDeref();
    }

    getRefreshRateManager().setRefreshRateRegime(RefreshRateManager::RefreshRateRegime::SHUTDOWN);
}

void Application::loadSettings(const QCommandLineParser& parser) {

    sessionRunTime.set(0); // Just clean living. We're about to saveSettings, which will update value.
    DependencyManager::get<AudioClient>()->loadSettings();
    DependencyManager::get<LODManager>()->loadSettings();

    auto menu = Menu::getInstance();
    menu->loadSettings();

    // override the menu option show overlays to always be true on startup
    menu->setIsOptionChecked(MenuOption::Overlays, true);

    // If there is a preferred plugin, we probably messed it up with the menu settings, so fix it.
    auto pluginManager = PluginManager::getInstance();
    auto plugins = pluginManager->getPreferredDisplayPlugins();
    if (plugins.size() > 0) {
        for (auto plugin : plugins) {
            if (auto action = menu->getActionForOption(plugin->getName())) {
                action->setChecked(true);
                action->trigger();
                // Find and activated highest priority plugin, bail for the rest
                break;
            }
        }
    }

    bool isFirstPerson = false;
    if (parser.isSet("no-launcher")) {
        const auto& displayPlugins = pluginManager->getDisplayPlugins();
        for (const auto& plugin : displayPlugins) {
            if (!plugin->isHmd()) {
                if (auto action = menu->getActionForOption(plugin->getName())) {
                    action->setChecked(true);
                    action->trigger();
                    break;
                }
            }
        }
        isFirstPerson = (qApp->isHMDMode());
    } else {
        if (_firstRun.get()) {
            // If this is our first run, and no preferred devices were set, default to
            // an HMD device if available.
            const auto& displayPlugins = pluginManager->getDisplayPlugins();
            for (const auto& plugin : displayPlugins) {
                if (plugin->isHmd()) {
                    if (auto action = menu->getActionForOption(plugin->getName())) {
                        action->setChecked(true);
                        action->trigger();
                        break;
                    }
                }
            }
            isFirstPerson = (qApp->isHMDMode());
        } else {
            // if this is not the first run, the camera will be initialized differently depending on user settings
            if (qApp->isHMDMode()) {
                // if the HMD is active, use first-person camera, unless the appropriate setting is checked
                isFirstPerson = menu->isOptionChecked(MenuOption::FirstPersonHMD);
            } else {
                // if HMD is not active, only use first person if the menu option is checked
                isFirstPerson = menu->isOptionChecked(MenuOption::FirstPersonLookAt);
            }
        }
    }

    // Load settings of the RenderScritpingInterface
    // Do that explicitely before being used
    RenderScriptingInterface::getInstance()->loadSettings();

    // Setup the PerformanceManager which will enforce the several settings to match the Preset
    // On the first run, the Preset is evaluated from the
    getPerformanceManager().setupPerformancePresetSettings(_firstRun.get());

    // finish initializing the camera, based on everything we checked above. Third person camera will be used if no settings
    // dictated that we should be in first person
    Menu::getInstance()->setIsOptionChecked(MenuOption::FirstPersonLookAt, isFirstPerson);
    Menu::getInstance()->setIsOptionChecked(MenuOption::ThirdPerson, !isFirstPerson);
    _myCamera.setMode((isFirstPerson) ? CAMERA_MODE_FIRST_PERSON_LOOK_AT : CAMERA_MODE_LOOK_AT);
    cameraMenuChanged();

    const auto& inputs = pluginManager->getInputPlugins();
    for (const auto& plugin : inputs) {
        if (!plugin->isActive()) {
            plugin->activate();
        }
    }

    QSharedPointer<scripting::Audio> audioScriptingInterface = qSharedPointerDynamicCast<scripting::Audio>(DependencyManager::get<AudioScriptingInterface>());
    if (audioScriptingInterface) {
        audioScriptingInterface->loadData();
    }

    getMyAvatar()->loadData();

    auto bucketEnum = QMetaEnum::fromType<ExternalResource::Bucket>();
    auto externalResource = ExternalResource::getInstance();

    for (int i = 0; i < bucketEnum.keyCount(); i++) {
        const char* keyName = bucketEnum.key(i);
        QString setting("ExternalResource/");
        setting += keyName;
        auto bucket = static_cast<ExternalResource::Bucket>(bucketEnum.keyToValue(keyName));
        Setting::Handle<QString> url(setting, externalResource->getBase(bucket));
        externalResource->setBase(bucket, url.get());
    }

    // the setter function isn't called, so update the theme colors now
    updateThemeColors();

    _settingsLoaded = true;
}

void Application::saveSettings() const {
    sessionRunTime.set(_sessionRunTimer.elapsed() / MSECS_PER_SECOND);
    DependencyManager::get<AudioClient>()->saveSettings();
    DependencyManager::get<LODManager>()->saveSettings();

    QSharedPointer<scripting::Audio> audioScriptingInterface = qSharedPointerDynamicCast<scripting::Audio>(DependencyManager::get<AudioScriptingInterface>());
    if (audioScriptingInterface) {
        audioScriptingInterface->saveData();
    }

    Menu::getInstance()->saveSettings();
    getMyAvatar()->saveData();
    PluginManager::getInstance()->saveSettings();

    // Don't save external resource paths until such time as there's UI to select or set alternatives. Otherwise new default
    // values won't be used unless Interface.json entries are manually remove or Interface.json is deleted.
    /*
    auto bucketEnum = QMetaEnum::fromType<ExternalResource::Bucket>();
    auto externalResource = ExternalResource::getInstance();

    for (int i = 0; i < bucketEnum.keyCount(); i++) {
        const char* keyName = bucketEnum.key(i);
        QString setting("ExternalResource/");
        setting += keyName;
        auto bucket = static_cast<ExternalResource::Bucket>(bucketEnum.keyToValue(keyName));
        Setting::Handle<QString> url(setting, externalResource->getBase(bucket));
        url.set(externalResource->getBase(bucket));
    }
    */
}

// This is currently not used, but could be invoked if the user wants to go to the place embedded in an
// Interface-taken snapshot. (It was developed for drag and drop, before we had asset-server loading or in-world browsers.)
bool Application::acceptSnapshot(const QString& urlString) {
    QUrl url(urlString);
    QString snapshotPath = url.toLocalFile();

    SnapshotMetaData* snapshotData = DependencyManager::get<Snapshot>()->parseSnapshotData(snapshotPath);
    if (snapshotData) {
        if (!snapshotData->getURL().toString().isEmpty()) {
            DependencyManager::get<AddressManager>()->handleLookupString(snapshotData->getURL().toString());
        }
    } else {
        OffscreenUi::asyncWarning("", "No location details were found in the file\n" +
                             snapshotPath + "\nTry dragging in an authentic Hifi snapshot.");
    }
    return true;
}

void Application::setSessionUUID(const QUuid& sessionUUID) const {
    Physics::setSessionUUID(sessionUUID);
}

void Application::domainURLChanged(QUrl domainURL) {
    // disable physics until we have enough information about our new location to not cause craziness.
    setIsServerlessMode(domainURL.scheme() != URL_SCHEME_OVERTE);
    if (isServerlessMode()) {
        loadServerlessDomain(domainURL);
    }
    updateWindowTitle();
}

void Application::domainConnectionRefused(const QString& reasonMessage, int reasonCodeInt, const QString& extraInfo) {
    DomainHandler::ConnectionRefusedReason reasonCode = static_cast<DomainHandler::ConnectionRefusedReason>(reasonCodeInt);

    if (reasonCode == DomainHandler::ConnectionRefusedReason::TooManyUsers && !extraInfo.isEmpty()) {
        DependencyManager::get<AddressManager>()->handleLookupString(extraInfo);
        return;
    }

    switch (reasonCode) {
        case DomainHandler::ConnectionRefusedReason::ProtocolMismatch:
        case DomainHandler::ConnectionRefusedReason::TooManyUsers:
        case DomainHandler::ConnectionRefusedReason::Unknown: {
            QString message = "Unable to connect to the location you are visiting.\n";
            message += reasonMessage;
            OffscreenUi::asyncWarning("", message);
            getMyAvatar()->setWorldVelocity(glm::vec3(0.0f));
            break;
        }
        default:
            // nothing to do.
            break;
    }
}

void Application::updateWindowTitle() const {
    auto nodeList = DependencyManager::get<NodeList>();
    auto accountManager = DependencyManager::get<AccountManager>();
    auto domainAccountManager = DependencyManager::get<DomainAccountManager>();
    auto isInErrorState = nodeList->getDomainHandler().isInErrorState();
    bool isMetaverseLoggedIn = accountManager->isLoggedIn();
    bool hasDomainLogIn = domainAccountManager->hasLogIn();
    bool isDomainLoggedIn = domainAccountManager->isLoggedIn();
    QString authedDomainName = domainAccountManager->getAuthedDomainName();

    QString buildVersion = " - Overte - " +
                           (BuildInfo::BUILD_TYPE == BuildInfo::BuildType::Stable ? QString("Version") : QString("Build")) +
                           " " + applicationVersion();

    QString connectionStatus = isInErrorState                               ? " (ERROR CONNECTING)"
                               : nodeList->getDomainHandler().isConnected() ? ""
                                                                            : " (NOT CONNECTED)";

    QString metaverseUsername = accountManager->getAccountInfo().getUsername();
    QString domainUsername = domainAccountManager->getUsername();

    auto& ch = CrashHandler::getInstance();
    ch.setAnnotation("sentry[user][username]", metaverseUsername.toStdString());

    QString currentPlaceName;
    if (isServerlessMode()) {
        if (isInErrorState) {
            currentPlaceName = "Serverless: " + nodeList->getDomainHandler().getErrorDomainURL().toString();
        } else {
            currentPlaceName = "Serverless: " + DependencyManager::get<AddressManager>()->getDomainURL().toString();
        }
    } else {
        currentPlaceName = DependencyManager::get<AddressManager>()->getDomainURL().host();
        if (currentPlaceName.isEmpty()) {
            currentPlaceName = nodeList->getDomainHandler().getHostname();
        }
    }

    QString metaverseDetails;
    if (isMetaverseLoggedIn) {
        metaverseDetails = " (Directory Services: Connected to " + MetaverseAPI::getCurrentMetaverseServerURL().toString() +
                           " as " + metaverseUsername + ")";
    } else {
        metaverseDetails = " (Directory Services: Not Logged In)";
    }

    QString domainDetails;
    if (hasDomainLogIn) {
        if (currentPlaceName == authedDomainName && isDomainLoggedIn) {
            domainDetails = " (Domain: Logged in as " + domainUsername + ")";
        } else {
            domainDetails = " (Domain: Not Logged In)";
        }
    } else {
        domainDetails = "";
    }

    QString title = currentPlaceName + connectionStatus + metaverseDetails + domainDetails + buildVersion;

#ifndef WIN32
    // crashes with vs2013/win32
    qCDebug(interfaceapp, "Application title set to: %s", title.toStdString().c_str());
#endif
    _window->setWindowTitle(title);

    // updateTitleWindow gets called whenever there's a change regarding the domain, so rather
    // than placing this within domainURLChanged, it's placed here to cover the other potential cases.
    DependencyManager::get<MessagesClient>()->sendLocalMessage("Toolbar-DomainChanged", "");
}

void Application::nodeAdded(SharedNodePointer node) {
    if (node->getType() == NodeType::EntityServer) {
        if (_failedToConnectToEntityServer && !_entityServerConnectionTimer.isActive()) {
            _octreeProcessor->stopSafeLanding();
            _failedToConnectToEntityServer = false;
        } else if (_entityServerConnectionTimer.isActive()) {
            _entityServerConnectionTimer.stop();
        }
        _octreeProcessor->startSafeLanding();
        _entityServerConnectionTimer.setInterval(ENTITY_SERVER_CONNECTION_TIMEOUT);
        _entityServerConnectionTimer.start();
    }
}

void Application::nodeActivated(SharedNodePointer node) {
    if (node->getType() == NodeType::AssetServer) {
        // asset server just connected - check if we have the asset browser showing

#if !defined(DISABLE_QML)
        auto offscreenUi = getOffscreenUI();
        if (offscreenUi) {
            auto nodeList = DependencyManager::get<NodeList>();

            if (nodeList->getThisNodeCanWriteAssets()) {
                // call reload on the shown asset browser dialog to get the mappings (if permissions allow)
                auto assetDialog = offscreenUi ? offscreenUi->getRootItem()->findChild<QQuickItem*>("AssetServer") : nullptr;
                if (assetDialog) {
                    QMetaObject::invokeMethod(assetDialog, "reload");
                }
            } else {
                // we switched to an Asset Server that we can't modify, hide the Asset Browser
                offscreenUi->hide("AssetServer");
            }
        }
#endif
    }

    // If we get a new EntityServer activated, reset lastQueried time
    // so we will do a proper query during update
    if (node->getType() == NodeType::EntityServer) {
        _queryExpiry = SteadyClock::now();
        _octreeQuery.incrementConnectionID();

        if  (!_failedToConnectToEntityServer) {
            _entityServerConnectionTimer.stop();
        }
    }

    if (node->getType() == NodeType::AudioMixer && !isInterstitialMode()) {
        DependencyManager::get<AudioClient>()->negotiateAudioFormat();
    }

    if (node->getType() == NodeType::AvatarMixer) {
        _queryExpiry = SteadyClock::now();

        // new avatar mixer, send off our identity packet on next update loop
        // Reset skeletonModelUrl if the last server modified our choice.
        // Override the avatar url (but not model name) here too.
        if (_avatarOverrideUrl.isValid()) {
            getMyAvatar()->useFullAvatarURL(_avatarOverrideUrl);
        }

        if (getMyAvatar()->getFullAvatarURLFromPreferences() != getMyAvatar()->getSkeletonModelURL()) {
            getMyAvatar()->resetFullAvatarURL();
        }
        getMyAvatar()->markIdentityDataChanged();
        getMyAvatar()->resetLastSent();

        if (!isInterstitialMode()) {
            // transmit a "sendAll" packet to the AvatarMixer we just connected to.
            getMyAvatar()->sendAvatarDataPacket(true);
        }
    }
}

void Application::nodeKilled(SharedNodePointer node) {
    // These are here because connecting NodeList::nodeKilled to OctreePacketProcessor::nodeKilled doesn't work:
    // OctreePacketProcessor::nodeKilled is not being called when NodeList::nodeKilled is emitted.
    // This may have to do with GenericThread::threadRoutine() blocking the QThread event loop

    _octreeProcessor->nodeKilled(node);

    _entityEditSender->nodeKilled(node);

    if (node->getType() == NodeType::AudioMixer) {
        QMetaObject::invokeMethod(DependencyManager::get<AudioClient>().data(), "audioMixerKilled");
    } else if (node->getType() == NodeType::EntityServer) {
        // we lost an entity server, clear all of the domain octree details
        clearDomainOctreeDetails(false);
    } else if (node->getType() == NodeType::AssetServer) {
        // asset server going away - check if we have the asset browser showing

#if !defined(DISABLE_QML)
        auto offscreenUi = getOffscreenUI();
        auto assetDialog = offscreenUi ? offscreenUi->getRootItem()->findChild<QQuickItem*>("AssetServer") : nullptr;

        if (assetDialog) {
            // call reload on the shown asset browser dialog
            QMetaObject::invokeMethod(assetDialog, "clear");
        }
#endif
    }
}

void Application::handleSandboxStatus(QNetworkReply* reply) {
    PROFILE_RANGE(render, __FUNCTION__);

    bool sandboxIsRunning = SandboxUtils::readStatus(reply->readAll());

    enum HandControllerType {
        Vive,
        Oculus
    };
    static const std::map<HandControllerType, int> MIN_CONTENT_VERSION = {
        { Vive, 1 },
        { Oculus, 27 }
    };

    // Get sandbox content set version
    auto acDirPath = PathUtils::getAppDataPath() + "../../" + BuildInfo::MODIFIED_ORGANIZATION + "/assignment-client/";
    auto contentVersionPath = acDirPath + "content-version.txt";
    qCDebug(interfaceapp) << "Checking " << contentVersionPath << " for content version";
    int contentVersion = 0;
    QFile contentVersionFile(contentVersionPath);
    if (contentVersionFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString line = contentVersionFile.readAll();
        contentVersion = line.toInt(); // returns 0 if conversion fails
    }

    // Get controller availability
#ifdef ANDROID_APP_QUEST_INTERFACE
    bool hasHandControllers = true;
#else
    bool hasHandControllers = false;
    if (PluginUtils::isViveControllerAvailable() || PluginUtils::isOculusTouchControllerAvailable()) {
        hasHandControllers = true;
    }
#endif

    // Check HMD use (may be technically available without being in use)
    bool hasHMD = PluginUtils::isHMDAvailable();
    bool isUsingHMD = _displayPlugin->isHmd();
    bool isUsingHMDAndHandControllers = hasHMD && hasHandControllers && isUsingHMD;

    qCDebug(interfaceapp) << "HMD:" << hasHMD << ", Hand Controllers: " << hasHandControllers << ", Using HMD: " << isUsingHMDAndHandControllers;

    QString addressLookupString;

    // when --url in command line, teleport to location
    if (!_urlParam.isEmpty()) { // Not sure if format supported by isValid().
        if (_urlParam.scheme() == URL_SCHEME_OVERTEAPP) {
            Setting::Handle<QVariant>("startUpApp").set(_urlParam.path());
        } else {
            addressLookupString = _urlParam.toString();
        }
    }

    static const QString SENT_TO_PREVIOUS_LOCATION = "previous_location";
    static const QString SENT_TO_ENTRY = "entry";

    QString sentTo;

    // If this is a first run we short-circuit the address passed in
    if (_firstRun.get()) {
        if (!BuildInfo::PRELOADED_STARTUP_LOCATION.isEmpty()) {
            DependencyManager::get<LocationBookmarks>()->setHomeLocationToAddress(NetworkingConstants::DEFAULT_OVERTE_ADDRESS);
            Menu::getInstance()->triggerOption(MenuOption::HomeLocation);
        }

        if (!_overrideEntry) {
            DependencyManager::get<AddressManager>()->goToEntry();
            sentTo = SENT_TO_ENTRY;
        } else {
            DependencyManager::get<AddressManager>()->loadSettings(addressLookupString);
            sentTo = SENT_TO_PREVIOUS_LOCATION;
        }
       _firstRun.set(false);
    } else {
        QString goingTo = "";
        if (addressLookupString.isEmpty()) {
            if (Menu::getInstance()->isOptionChecked(MenuOption::HomeLocation)) {
                auto locationBookmarks = DependencyManager::get<LocationBookmarks>();
                addressLookupString = locationBookmarks->addressForBookmark(LocationBookmarks::HOME_BOOKMARK);
                goingTo = "home location";
            } else {
                goingTo = "previous location";
            }
        }
        qCDebug(interfaceapp) << "Not first run... going to" << qPrintable(!goingTo.isEmpty() ? goingTo : addressLookupString);
        DependencyManager::get<AddressManager>()->loadSettings(addressLookupString);
        sentTo = SENT_TO_PREVIOUS_LOCATION;
    }

    UserActivityLogger::getInstance().logAction("startup_sent_to", {
        { "sent_to", sentTo },
        { "sandbox_is_running", sandboxIsRunning },
        { "has_hmd", hasHMD },
        { "has_hand_controllers", hasHandControllers },
        { "is_using_hmd", isUsingHMD },
        { "is_using_hmd_and_hand_controllers", isUsingHMDAndHandControllers },
        { "content_version", contentVersion }
    });

    _connectionMonitor.init();
}

void Application::cleanupBeforeQuit() {
    // add a logline indicating if QTWEBENGINE_REMOTE_DEBUGGING is set or not
    QString webengineRemoteDebugging = QProcessEnvironment::systemEnvironment().value("QTWEBENGINE_REMOTE_DEBUGGING", "false");
    qCDebug(interfaceapp) << "QTWEBENGINE_REMOTE_DEBUGGING =" << webengineRemoteDebugging;

    DependencyManager::prepareToExit();

    if (tracing::enabled()) {
        auto tracer = DependencyManager::get<tracing::Tracer>();
        tracer->stopTracing();
        auto outputFile = property(hifi::properties::TRACING).toString();
        tracer->serialize(outputFile);
    }

    // Stop third party processes so that they're not left running in the event of a subsequent shutdown crash.
    AnimDebugDraw::getInstance().shutdown();

    // FIXME: once we move to shared pointer for the INputDevice we shoud remove this naked delete:
    _applicationStateDevice.reset();

    {
        if (_keyboardFocusHighlightID != UNKNOWN_ENTITY_ID) {
            DependencyManager::get<EntityScriptingInterface>()->deleteEntity(_keyboardFocusHighlightID);
            _keyboardFocusHighlightID = UNKNOWN_ENTITY_ID;
        }
    }

    {
        auto nodeList = DependencyManager::get<NodeList>();

        // send the domain a disconnect packet, force stoppage of domain-server check-ins
        nodeList->getDomainHandler().disconnect("Quitting");
        nodeList->setIsShuttingDown(true);

        // tell the packet receiver we're shutting down, so it can drop packets
        nodeList->getPacketReceiver().setShouldDropPackets(true);
    }

    getEntities()->shutdown(); // tell the entities system we're shutting down, so it will stop running scripts

    // Clear any queued processing (I/O, FBX/OBJ/Texture parsing)
    QThreadPool::globalInstance()->clear();
    QThreadPool::globalInstance()->waitForDone();

    DependencyManager::destroy<RecordingScriptingInterface>();

    // FIXME: Something is still holding on to the ScriptEnginePointers contained in ScriptEngines, and they hold backpointers to ScriptEngines,
    // so this doesn't shut down properly
    DependencyManager::get<ScriptEngines>()->shutdownScripting(); // stop all currently running global scripts
    // These classes hold ScriptEnginePointers, so they must be destroyed before ScriptEngines
    // Must be done after shutdownScripting in case any scripts try to access these things
    {
        DependencyManager::destroy<StandAloneJSConsole>();
        EntityTreePointer tree = getEntities()->getTree();
        tree->setSimulation(nullptr);
        DependencyManager::destroy<EntityTreeRenderer>();
    }
    DependencyManager::destroy<ScriptEngines>();

    bool keepMeLoggedIn = Setting::Handle<bool>(KEEP_ME_LOGGED_IN_SETTING_NAME, false).get();
    if (!keepMeLoggedIn) {
        DependencyManager::get<AccountManager>()->removeAccountFromFile();
    }
    // ####### TODO

    _displayPlugin.reset();
    PluginManager::getInstance()->shutdown();

    // Cleanup all overlays after the scripts, as scripts might add more
    _overlays.cleanupAllOverlays();

    // first stop all timers directly or by invokeMethod
    // depending on what thread they run in
    _locationUpdateTimer.stop();
    _window->saveGeometry();

    // stop QML
    DependencyManager::destroy<TabletScriptingInterface>();
    DependencyManager::destroy<ToolbarScriptingInterface>();
    DependencyManager::destroy<OffscreenUi>();

    DependencyManager::destroy<OffscreenQmlSurfaceCache>();

    // destroy Audio so it and its threads have a chance to go down safely
    // this must happen after QML, as there are unexplained audio crashes originating in qtwebengine
    AudioInjector::setLocalAudioInterface(nullptr);
    DependencyManager::destroy<AudioClient>();
    DependencyManager::destroy<AudioScriptingInterface>();

    // The PointerManager must be destroyed before the PickManager because when a Pointer is deleted,
    // it accesses the PickManager to delete its associated Pick
    DependencyManager::destroy<PointerManager>();
    DependencyManager::destroy<PickManager>();
    DependencyManager::destroy<KeyboardScriptingInterface>();
    DependencyManager::destroy<Keyboard>();
    DependencyManager::destroy<AvatarPackager>();

    qCDebug(interfaceapp) << "Application::cleanupBeforeQuit() complete";
}


static const float FOCUS_HIGHLIGHT_EXPANSION_FACTOR = 1.05f;
void Application::idle() {
    PerformanceTimer perfTimer("idle");

#if !defined(DISABLE_QML)
    auto offscreenUi = getOffscreenUI();

    // These tasks need to be done on our first idle, because we don't want the showing of
    // overlay subwindows to do a showDesktop() until after the first time through
    static bool firstIdle = true;
    if (firstIdle) {
        firstIdle = false;
        connect(offscreenUi.data(), &OffscreenUi::showDesktop, this, &Application::showDesktop);
    }
#endif

#ifdef Q_OS_WIN
    {
        // If tracing is enabled then monitor the CPU in a separate thread
        static std::once_flag once;
        std::call_once(once, [&] {
            if (trace_app().isDebugEnabled()) {
                QThread* cpuMonitorThread = new QThread(qApp);
                cpuMonitorThread->setObjectName("cpuMonitorThread");
                QObject::connect(cpuMonitorThread, &QThread::started, [this] { setupCpuMonitorThread(); });
                QObject::connect(qApp, &QCoreApplication::aboutToQuit, cpuMonitorThread, &QThread::quit);
                cpuMonitorThread->start();
            }
        });
    }
#endif

    auto displayPlugin = getActiveDisplayPlugin();
#if !defined(DISABLE_QML)
    if (displayPlugin) {
        auto uiSize = displayPlugin->getRecommendedUiSize();
        // Bit of a hack since there's no device pixel ratio change event I can find.
        if (offscreenUi->size() != fromGlm(uiSize)) {
            qCDebug(interfaceapp) << "Device pixel ratio changed, triggering resize to " << uiSize;
            offscreenUi->resize(fromGlm(uiSize));
        }
    }
#endif

    if (displayPlugin) {
        PROFILE_COUNTER_IF_CHANGED(app, "present", float, displayPlugin->presentRate());
    }
    PROFILE_COUNTER_IF_CHANGED(app, "renderLoopRate", float, getRenderLoopRate());
    PROFILE_COUNTER_IF_CHANGED(app, "currentDownloads", uint32_t, ResourceCache::getLoadingRequests().length());
    PROFILE_COUNTER_IF_CHANGED(app, "pendingDownloads", uint32_t, ResourceCache::getPendingRequestCount());
    PROFILE_COUNTER_IF_CHANGED(app, "currentProcessing", int, DependencyManager::get<StatTracker>()->getStat("Processing").toInt());
    PROFILE_COUNTER_IF_CHANGED(app, "pendingProcessing", int, DependencyManager::get<StatTracker>()->getStat("PendingProcessing").toInt());
    auto renderConfig = _graphicsEngine->getRenderEngine()->getConfiguration();
    PROFILE_COUNTER_IF_CHANGED(render, "gpuTime", float, (float)_graphicsEngine->getGPUContext()->getFrameTimerGPUAverage());

    PROFILE_RANGE(app, __FUNCTION__);

    if (auto steamClient = PluginManager::getInstance()->getSteamClientPlugin()) {
        steamClient->runCallbacks();
    }

    if (auto oculusPlugin = PluginManager::getInstance()->getOculusPlatformPlugin()) {
        oculusPlugin->handleOVREvents();
    }

    float secondsSinceLastUpdate = (float)_lastTimeUpdated.nsecsElapsed() / NSECS_PER_MSEC / MSECS_PER_SECOND;
    _lastTimeUpdated.start();

#if !defined(DISABLE_QML)
    // If the offscreen Ui has something active that is NOT the root, then assume it has keyboard focus.
    if (offscreenUi && offscreenUi->getWindow()) {
        auto activeFocusItem = offscreenUi->getWindow()->activeFocusItem();
        if (_keyboardDeviceHasFocus && (activeFocusItem != NULL && activeFocusItem != offscreenUi->getRootItem())) {
            _keyboardMouseDevice->pluginFocusOutEvent();
            _keyboardDeviceHasFocus = false;
            synthesizeKeyReleasEvents();
        } else if (activeFocusItem == offscreenUi->getRootItem()) {
            _keyboardDeviceHasFocus = true;
        }
    }
#endif

    checkChangeCursor();

#if !defined(DISABLE_QML)
    auto stats = Stats::getInstance();
    if (stats) {
        stats->updateStats();
    }
    auto animStats = AnimStats::getInstance();
    if (animStats) {
        animStats->updateStats();
    }
#endif

    // Normally we check PipelineWarnings, but since idle will often take more than 10ms we only show these idle timing
    // details if we're in ExtraDebugging mode. However, the ::update() and its subcomponents will show their timing
    // details normally.
#ifdef Q_OS_ANDROID
    bool showWarnings = false;
#else
    bool showWarnings = getLogger()->extraDebugging();
#endif
    PerformanceWarning warn(showWarnings, "idle()");

    {
        _gameWorkload.updateViews(_viewFrustum, getMyAvatar()->getHeadPosition());
        _gameWorkload._engine->run();
    }
    {
        PerformanceTimer perfTimer("update");
        PerformanceWarning warn(showWarnings, "Application::idle()... update()");
        static const float BIGGEST_DELTA_TIME_SECS = 0.25f;
        update(glm::clamp(secondsSinceLastUpdate, 0.0f, BIGGEST_DELTA_TIME_SECS));
    }

    { // Update keyboard focus highlight
        if (!_keyboardFocusedEntity.get().isInvalidID()) {
            const quint64 LOSE_FOCUS_AFTER_ELAPSED_TIME = 30 * USECS_PER_SECOND; // if idle for 30 seconds, drop focus
            quint64 elapsedSinceAcceptedKeyPress = usecTimestampNow() - _lastAcceptedKeyPress;
            if (elapsedSinceAcceptedKeyPress > LOSE_FOCUS_AFTER_ELAPSED_TIME) {
                setKeyboardFocusEntity(UNKNOWN_ENTITY_ID);
            } else {
                if (auto entity = getEntities()->getTree()->findEntityByID(_keyboardFocusedEntity.get())) {
                    EntityItemProperties properties;
                    properties.setPosition(entity->getWorldPosition());
                    properties.setRotation(entity->getWorldOrientation());
                    properties.setDimensions(entity->getScaledDimensions() * FOCUS_HIGHLIGHT_EXPANSION_FACTOR);
                    DependencyManager::get<EntityScriptingInterface>()->editEntity(_keyboardFocusHighlightID, properties);
                }
            }
        }
    }

    {
        if (_keyboardFocusWaitingOnRenderable && getEntities()->renderableForEntityId(_keyboardFocusedEntity.get())) {
            QUuid entityId = _keyboardFocusedEntity.get();
            setKeyboardFocusEntity(UNKNOWN_ENTITY_ID);
            _keyboardFocusWaitingOnRenderable = false;
            setKeyboardFocusEntity(entityId);
        }
    }

    {
        PerformanceTimer perfTimer("pluginIdle");
        PerformanceWarning warn(showWarnings, "Application::idle()... pluginIdle()");
        getActiveDisplayPlugin()->idle();
        const auto& inputPlugins = PluginManager::getInstance()->getInputPlugins();
        for(const auto& inputPlugin : inputPlugins) {
            if (inputPlugin->isActive()) {
                inputPlugin->idle();
            }
        }
    }

    _overlayConductor.update(secondsSinceLastUpdate);

    _gameLoopCounter.increment();

    // Perform one-time startup checks in case we need to show warnings
    {
        static std::once_flag once;
        std::call_once(once, [this] {
            const QString& bookmarksError = DependencyManager::get<AvatarBookmarks>()->getBookmarkError();
            if (!bookmarksError.isEmpty()) {
                OffscreenUi::asyncWarning("Avatar Bookmarks Error", "JSON parse error: " + bookmarksError, QMessageBox::Ok, QMessageBox::Ok);
            }

            QString os = platform::getComputer()[platform::keys::computer::OS].dump().c_str();
            os = os.replace("\"", "");
            GPUIdent* gpuIdent = GPUIdent::getInstance();
            QString vendor = platform::Instance::findGPUVendorInDescription(gpuIdent->getName().toStdString());
            QString renderer = gl::ContextInfo::get().renderer.c_str();
            QString api = _graphicsEngine->getGPUContext()->getBackendVersion().c_str();
            QString driver = gpuIdent->getDriver();
            QString fullDriverToTest = os + " " + vendor + " " + renderer + " " + api + " " + driver;
            if (fullDriverToTest != _prevCheckedDriver.get()) {
                QNetworkAccessManager& networkAccessManager = NetworkAccessManager::getInstance();
                QNetworkRequest request(QUrl("https://mv.overte.org/gpu_driver_blocklist.json"));
                request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
                request.setHeader(QNetworkRequest::UserAgentHeader, NetworkingConstants::OVERTE_USER_AGENT);
                QNetworkReply* reply = networkAccessManager.get(request);
                auto onFinished = std::bind(&Application::processDriverBlocklistReply, this, fullDriverToTest, os, vendor, renderer, api, driver.replace(" ", "."));
                connect(reply, &QNetworkReply::finished, this, onFinished);
            }
        });
    }
}

void Application::update(float deltaTime) {
    PROFILE_RANGE_EX(app, __FUNCTION__, 0xffff0000, (uint64_t)_graphicsEngine->_renderFrameCount + 1);

    if (_aboutToQuit) {
        return;
    }

    if (!_physicsEnabled) {
        if (!_domainLoadingInProgress) {
            PROFILE_ASYNC_BEGIN(app, "Scene Loading", "");
            _domainLoadingInProgress = true;
        }

        // we haven't yet enabled physics.  we wait until we think we have all the collision information
        // for nearby entities before starting bullet up.
        if (isServerlessMode() && !_waitForServerlessToBeSet) {
            tryToEnablePhysics();
        } else if (_failedToConnectToEntityServer) {
            if (_octreeProcessor->safeLandingIsActive()) {
                _octreeProcessor->stopSafeLanding();
            }
        } else {
            _octreeProcessor->updateSafeLanding();
            if (_octreeProcessor->safeLandingIsComplete()) {
                tryToEnablePhysics();
            }
        }
    } else if (_domainLoadingInProgress) {
        _domainLoadingInProgress = false;
        PROFILE_ASYNC_END(app, "Scene Loading", "");
    }

     if (shouldCaptureMouse()) {
        QPoint point = _glWidget->mapToGlobal(_glWidget->geometry().center());
        if (QCursor::pos() != point) {
            _mouseCaptureTarget = point;
            _ignoreMouseMove = true;
            if (_captureMouse) {
                _keyboardMouseDevice->updateMousePositionForCapture(QCursor::pos(), _mouseCaptureTarget);
            }
            QCursor::setPos(point);
        }
    }

    auto myAvatar = getMyAvatar();
    {
        PerformanceTimer perfTimer("devices");
        auto userInputMapper = DependencyManager::get<UserInputMapper>();

        controller::HmdAvatarAlignmentType hmdAvatarAlignmentType;
        if (myAvatar->getHmdAvatarAlignmentType() == "eyes") {
            hmdAvatarAlignmentType = controller::HmdAvatarAlignmentType::Eyes;
        } else {
            hmdAvatarAlignmentType = controller::HmdAvatarAlignmentType::Head;
        }

        controller::InputCalibrationData calibrationData = {
            myAvatar->getSensorToWorldMatrix(),
            createMatFromQuatAndPos(myAvatar->getWorldOrientation(), myAvatar->getWorldPosition()),
            myAvatar->getHMDSensorMatrix(),
            myAvatar->getCenterEyeCalibrationMat(),
            myAvatar->getHeadCalibrationMat(),
            myAvatar->getSpine2CalibrationMat(),
            myAvatar->getHipsCalibrationMat(),
            myAvatar->getLeftFootCalibrationMat(),
            myAvatar->getRightFootCalibrationMat(),
            myAvatar->getRightArmCalibrationMat(),
            myAvatar->getLeftArmCalibrationMat(),
            myAvatar->getRightHandCalibrationMat(),
            myAvatar->getLeftHandCalibrationMat(),
            hmdAvatarAlignmentType
        };

        InputPluginPointer keyboardMousePlugin;
        for(const auto& inputPlugin : PluginManager::getInstance()->getInputPlugins()) {
            if (inputPlugin->getName() == KeyboardMouseDevice::NAME) {
                keyboardMousePlugin = inputPlugin;
            } else if (inputPlugin->isActive()) {
                inputPlugin->pluginUpdate(deltaTime, calibrationData);
            }
        }

        userInputMapper->setInputCalibrationData(calibrationData);
        userInputMapper->update(deltaTime);

        if (keyboardMousePlugin && keyboardMousePlugin->isActive()) {
            keyboardMousePlugin->pluginUpdate(deltaTime, calibrationData);
        }
        // Transfer the user inputs to the driveKeys
        // FIXME can we drop drive keys and just have the avatar read the action states directly?
        myAvatar->clearDriveKeys();
        if (_myCamera.getMode() != CAMERA_MODE_INDEPENDENT && !isInterstitialMode()) {
            if (!_controllerScriptingInterface->areActionsCaptured() && _myCamera.getMode() != CAMERA_MODE_MIRROR) {
                myAvatar->setDriveKey(MyAvatar::TRANSLATE_Z, -1.0f * userInputMapper->getActionState(controller::Action::TRANSLATE_Z));
                myAvatar->setDriveKey(MyAvatar::TRANSLATE_Y, userInputMapper->getActionState(controller::Action::TRANSLATE_Y));
                myAvatar->setDriveKey(MyAvatar::TRANSLATE_X, userInputMapper->getActionState(controller::Action::TRANSLATE_X));
                if (deltaTime > FLT_EPSILON && userInputMapper->getActionState(controller::Action::TRANSLATE_CAMERA_Z)  == 0.0f) {
                    myAvatar->setDriveKey(MyAvatar::PITCH, -1.0f * userInputMapper->getActionState(controller::Action::PITCH));
                    myAvatar->setDriveKey(MyAvatar::YAW, -1.0f * userInputMapper->getActionState(controller::Action::YAW));
                    myAvatar->setDriveKey(MyAvatar::DELTA_PITCH, -_myCamera.getSensitivity() * userInputMapper->getActionState(controller::Action::DELTA_PITCH));
                    myAvatar->setDriveKey(MyAvatar::DELTA_YAW, -_myCamera.getSensitivity() * userInputMapper->getActionState(controller::Action::DELTA_YAW));
                    myAvatar->setDriveKey(MyAvatar::STEP_YAW, -1.0f * userInputMapper->getActionState(controller::Action::STEP_YAW));
                }
            }
            myAvatar->setDriveKey(MyAvatar::ZOOM, userInputMapper->getActionState(controller::Action::TRANSLATE_CAMERA_Z));
        }

        myAvatar->setSprintMode((bool)userInputMapper->getActionState(controller::Action::SPRINT));
        static const std::vector<controller::Action> avatarControllerActions = {
            controller::Action::LEFT_HAND,
            controller::Action::RIGHT_HAND,
            controller::Action::LEFT_FOOT,
            controller::Action::RIGHT_FOOT,
            controller::Action::HIPS,
            controller::Action::SPINE2,
            controller::Action::HEAD,
            controller::Action::LEFT_HAND_THUMB1,
            controller::Action::LEFT_HAND_THUMB2,
            controller::Action::LEFT_HAND_THUMB3,
            controller::Action::LEFT_HAND_THUMB4,
            controller::Action::LEFT_HAND_INDEX1,
            controller::Action::LEFT_HAND_INDEX2,
            controller::Action::LEFT_HAND_INDEX3,
            controller::Action::LEFT_HAND_INDEX4,
            controller::Action::LEFT_HAND_MIDDLE1,
            controller::Action::LEFT_HAND_MIDDLE2,
            controller::Action::LEFT_HAND_MIDDLE3,
            controller::Action::LEFT_HAND_MIDDLE4,
            controller::Action::LEFT_HAND_RING1,
            controller::Action::LEFT_HAND_RING2,
            controller::Action::LEFT_HAND_RING3,
            controller::Action::LEFT_HAND_RING4,
            controller::Action::LEFT_HAND_PINKY1,
            controller::Action::LEFT_HAND_PINKY2,
            controller::Action::LEFT_HAND_PINKY3,
            controller::Action::LEFT_HAND_PINKY4,
            controller::Action::RIGHT_HAND_THUMB1,
            controller::Action::RIGHT_HAND_THUMB2,
            controller::Action::RIGHT_HAND_THUMB3,
            controller::Action::RIGHT_HAND_THUMB4,
            controller::Action::RIGHT_HAND_INDEX1,
            controller::Action::RIGHT_HAND_INDEX2,
            controller::Action::RIGHT_HAND_INDEX3,
            controller::Action::RIGHT_HAND_INDEX4,
            controller::Action::RIGHT_HAND_MIDDLE1,
            controller::Action::RIGHT_HAND_MIDDLE2,
            controller::Action::RIGHT_HAND_MIDDLE3,
            controller::Action::RIGHT_HAND_MIDDLE4,
            controller::Action::RIGHT_HAND_RING1,
            controller::Action::RIGHT_HAND_RING2,
            controller::Action::RIGHT_HAND_RING3,
            controller::Action::RIGHT_HAND_RING4,
            controller::Action::RIGHT_HAND_PINKY1,
            controller::Action::RIGHT_HAND_PINKY2,
            controller::Action::RIGHT_HAND_PINKY3,
            controller::Action::RIGHT_HAND_PINKY4,
            controller::Action::LEFT_ARM,
            controller::Action::RIGHT_ARM,
            controller::Action::LEFT_SHOULDER,
            controller::Action::RIGHT_SHOULDER,
            controller::Action::LEFT_FORE_ARM,
            controller::Action::RIGHT_FORE_ARM,
            controller::Action::LEFT_LEG,
            controller::Action::RIGHT_LEG,
            controller::Action::LEFT_UP_LEG,
            controller::Action::RIGHT_UP_LEG,
            controller::Action::LEFT_TOE_BASE,
            controller::Action::RIGHT_TOE_BASE,
            controller::Action::LEFT_EYE,
            controller::Action::RIGHT_EYE

        };

        // copy controller poses from userInputMapper to myAvatar.
        glm::mat4 myAvatarMatrix = createMatFromQuatAndPos(myAvatar->getWorldOrientation(), myAvatar->getWorldPosition());
        glm::mat4 worldToSensorMatrix = glm::inverse(myAvatar->getSensorToWorldMatrix());
        glm::mat4 avatarToSensorMatrix = worldToSensorMatrix * myAvatarMatrix;
        for (auto& action : avatarControllerActions) {
            controller::Pose pose = userInputMapper->getPoseState(action);
            myAvatar->setControllerPoseInSensorFrame(action, pose.transform(avatarToSensorMatrix));
        }

        static const std::vector<QString> trackedObjectStringLiterals = {
            QStringLiteral("_TrackedObject00"), QStringLiteral("_TrackedObject01"), QStringLiteral("_TrackedObject02"), QStringLiteral("_TrackedObject03"),
            QStringLiteral("_TrackedObject04"), QStringLiteral("_TrackedObject05"), QStringLiteral("_TrackedObject06"), QStringLiteral("_TrackedObject07"),
            QStringLiteral("_TrackedObject08"), QStringLiteral("_TrackedObject09"), QStringLiteral("_TrackedObject10"), QStringLiteral("_TrackedObject11"),
            QStringLiteral("_TrackedObject12"), QStringLiteral("_TrackedObject13"), QStringLiteral("_TrackedObject14"), QStringLiteral("_TrackedObject15")
        };

        // Controlled by the Developer > Avatar > Show Tracked Objects menu.
        if (_showTrackedObjects) {
            static const std::vector<controller::Action> trackedObjectActions = {
                controller::Action::TRACKED_OBJECT_00, controller::Action::TRACKED_OBJECT_01, controller::Action::TRACKED_OBJECT_02, controller::Action::TRACKED_OBJECT_03,
                controller::Action::TRACKED_OBJECT_04, controller::Action::TRACKED_OBJECT_05, controller::Action::TRACKED_OBJECT_06, controller::Action::TRACKED_OBJECT_07,
                controller::Action::TRACKED_OBJECT_08, controller::Action::TRACKED_OBJECT_09, controller::Action::TRACKED_OBJECT_10, controller::Action::TRACKED_OBJECT_11,
                controller::Action::TRACKED_OBJECT_12, controller::Action::TRACKED_OBJECT_13, controller::Action::TRACKED_OBJECT_14, controller::Action::TRACKED_OBJECT_15
            };

            int i = 0;
            glm::vec4 BLUE(0.0f, 0.0f, 1.0f, 1.0f);
            for (auto& action : trackedObjectActions) {
                controller::Pose pose = userInputMapper->getPoseState(action);
                if (pose.valid) {
                    glm::vec3 pos = transformPoint(myAvatarMatrix, pose.translation);
                    glm::quat rot = glmExtractRotation(myAvatarMatrix) * pose.rotation;
                    DebugDraw::getInstance().addMarker(trackedObjectStringLiterals[i], rot, pos, BLUE);
                } else {
                    DebugDraw::getInstance().removeMarker(trackedObjectStringLiterals[i]);
                }
                i++;
            }
        } else if (_prevShowTrackedObjects) {
            for (auto& key : trackedObjectStringLiterals) {
                DebugDraw::getInstance().removeMarker(key);
            }
        }
        _prevShowTrackedObjects = _showTrackedObjects;
    }

    updateThreads(deltaTime); // If running non-threaded, then give the threads some time to process...
    updateDialogs(deltaTime); // update various stats dialogs if present

    auto grabManager = DependencyManager::get<GrabManager>();
    grabManager->simulateGrabs();

    // TODO: break these out into distinct perfTimers when they prove interesting
    {
        PROFILE_RANGE(app, "PickManager");
        PerformanceTimer perfTimer("pickManager");
        DependencyManager::get<PickManager>()->update();
    }

    {
        PROFILE_RANGE(app, "PointerManager");
        PerformanceTimer perfTimer("pointerManager");
        DependencyManager::get<PointerManager>()->update();
    }

    QSharedPointer<AvatarManager> avatarManager = DependencyManager::get<AvatarManager>();

    {
        PROFILE_RANGE(simulation_physics, "Simulation");
        PerformanceTimer perfTimer("simulation");

        getEntities()->preUpdate();
        _entitySimulation->removeDeadEntities();

        auto t0 = std::chrono::high_resolution_clock::now();
        auto t1 = t0;
        {
            PROFILE_RANGE(simulation_physics, "PrePhysics");
            PerformanceTimer perfTimer("prePhysics)");
            {
                PROFILE_RANGE(simulation_physics, "Entities");
                PhysicsEngine::Transaction transaction;
                _entitySimulation->buildPhysicsTransaction(transaction);
                _physicsEngine->processTransaction(transaction);
                _entitySimulation->handleProcessedPhysicsTransaction(transaction);
            }

            t1 = std::chrono::high_resolution_clock::now();

            {
                PROFILE_RANGE(simulation_physics, "Avatars");
                PhysicsEngine::Transaction transaction;
                avatarManager->buildPhysicsTransaction(transaction);
                _physicsEngine->processTransaction(transaction);
                avatarManager->handleProcessedPhysicsTransaction(transaction);

                myAvatar->prepareForPhysicsSimulation();
                myAvatar->getCharacterController()->preSimulation();
            }
        }

        if (_physicsEnabled) {
            {
                PROFILE_RANGE(simulation_physics, "PrepareActions");
                _entitySimulation->applyDynamicChanges();
                _physicsEngine->forEachDynamic([&](EntityDynamicPointer dynamic) {
                    dynamic->prepareForPhysicsSimulation();
                });
            }
            auto t2 = std::chrono::high_resolution_clock::now();
            {
                PROFILE_RANGE(simulation_physics, "StepPhysics");
                PerformanceTimer perfTimer("stepPhysics");
                getEntities()->getTree()->withWriteLock([&] {
                    _physicsEngine->stepSimulation();
                });
            }
            auto t3 = std::chrono::high_resolution_clock::now();
            {
                if (_physicsEngine->hasOutgoingChanges()) {
                    {
                        PROFILE_RANGE(simulation_physics, "PostPhysics");
                        PerformanceTimer perfTimer("postPhysics");
                        // grab the collision events BEFORE handleChangedMotionStates() because at this point
                        // we have a better idea of which objects we own or should own.
                        auto& collisionEvents = _physicsEngine->getCollisionEvents();

                        getEntities()->getTree()->withWriteLock([&] {
                            PROFILE_RANGE(simulation_physics, "HandleChanges");
                            PerformanceTimer perfTimer("handleChanges");

                            const VectorOfMotionStates& outgoingChanges = _physicsEngine->getChangedMotionStates();
                            _entitySimulation->handleChangedMotionStates(outgoingChanges);
                            avatarManager->handleChangedMotionStates(outgoingChanges);

                            const VectorOfMotionStates& deactivations = _physicsEngine->getDeactivatedMotionStates();
                            _entitySimulation->handleDeactivatedMotionStates(deactivations);
                        });

                        // handleCollisionEvents() AFTER handleChangedMotionStates()
                        {
                            PROFILE_RANGE(simulation_physics, "CollisionEvents");
                            avatarManager->handleCollisionEvents(collisionEvents);
                            // Collision events (and their scripts) must not be handled when we're locked, above. (That would risk
                            // deadlock.)
                            _entitySimulation->handleCollisionEvents(collisionEvents);
                        }

                        {
                            PROFILE_RANGE(simulation_physics, "MyAvatar");
                            myAvatar->getCharacterController()->postSimulation();
                            myAvatar->harvestResultsFromPhysicsSimulation(deltaTime);
                        }

                        if (PerformanceTimer::isActive() &&
                                Menu::getInstance()->isOptionChecked(MenuOption::DisplayDebugTimingDetails) &&
                                Menu::getInstance()->isOptionChecked(MenuOption::ExpandPhysicsTiming)) {
                            _physicsEngine->harvestPerformanceStats();
                        }
                        // NOTE: the PhysicsEngine stats are written to stdout NOT to Qt log framework
                        _physicsEngine->dumpStatsIfNecessary();
                    }
                    auto t4 = std::chrono::high_resolution_clock::now();

                    // NOTE: the getEntities()->update() call below will wait for lock
                    // and will provide non-physical entity motion
                    getEntities()->update(true); // update the models...

                    auto t5 = std::chrono::high_resolution_clock::now();

                    workload::Timings timings(6);
                    timings[0] = t1 - t0; // prePhysics entities
                    timings[1] = t2 - t1; // prePhysics avatars
                    timings[2] = t3 - t2; // stepPhysics
                    timings[3] = t4 - t3; // postPhysics
                    timings[4] = t5 - t4; // non-physical kinematics
                    timings[5] = workload::Timing_ns((int32_t)(NSECS_PER_SECOND * deltaTime)); // game loop duration
                    _gameWorkload.updateSimulationTimings(timings);
                }
            }
        } else {
            // update the rendering without any simulation
            getEntities()->update(false);
        }
        // remove recently dead avatarEntities
        SetOfEntities deadAvatarEntities;
        _entitySimulation->takeDeadAvatarEntities(deadAvatarEntities);
        avatarManager->removeDeadAvatarEntities(deadAvatarEntities);
    }

    // AvatarManager update
    {
        {
            PROFILE_RANGE(simulation, "OtherAvatars");
            PerformanceTimer perfTimer("otherAvatars");
            avatarManager->updateOtherAvatars(deltaTime);
        }

        {
            PROFILE_RANGE(simulation, "MyAvatar");
            PerformanceTimer perfTimer("MyAvatar");
            qApp->updateMyAvatarLookAtPosition(deltaTime);
            avatarManager->updateMyAvatar(deltaTime);
        }
    }

    bool showWarnings = Menu::getInstance()->isOptionChecked(MenuOption::PipelineWarnings);
    PerformanceWarning warn(showWarnings, "Application::update()");

    updateLOD(deltaTime);

    if (!_loginDialogID.isNull()) {
        _loginStateManager.update(getMyAvatar()->getDominantHand(), _loginDialogID);
        updateLoginDialogPosition();
    }

    {
        PROFILE_RANGE_EX(app, "Overlays", 0xffff0000, (uint64_t)getActiveDisplayPlugin()->presentCount());
        PerformanceTimer perfTimer("overlays");
        _overlays.update(deltaTime);
    }

    // Update _viewFrustum with latest camera and view frustum data...
    // NOTE: we get this from the view frustum, to make it simpler, since the
    // loadViewFrumstum() method will get the correct details from the camera
    // We could optimize this to not actually load the viewFrustum, since we don't
    // actually need to calculate the view frustum planes to send these details
    // to the server.
    {
        QMutexLocker viewLocker(&_viewMutex);
        _myCamera.loadViewFrustum(_viewFrustum);

        _conicalViews.clear();
        _conicalViews.push_back(_viewFrustum);
        // TODO: Fix this by modeling the way the secondary camera works on how the main camera works
        // ie. Use a camera object stored in the game logic and informs the Engine on where the secondary
        // camera should be.
        updateSecondaryCameraViewFrustum();
    }

    quint64 now = usecTimestampNow();

    // Update my voxel servers with my current voxel query...
    {
        PROFILE_RANGE_EX(app, "QueryOctree", 0xffff0000, (uint64_t)getActiveDisplayPlugin()->presentCount());
        PerformanceTimer perfTimer("queryOctree");
        QMutexLocker viewLocker(&_viewMutex);

        bool viewIsDifferentEnough = false;
        if (_conicalViews.size() == _lastQueriedViews.size()) {
            for (size_t i = 0; i < _conicalViews.size(); ++i) {
                if (!_conicalViews[i].isVerySimilar(_lastQueriedViews[i])) {
                    viewIsDifferentEnough = true;
                    break;
                }
            }
        } else {
            viewIsDifferentEnough = true;
        }


        // if it's been a while since our last query or the view has significantly changed then send a query, otherwise suppress it
        static const std::chrono::seconds MIN_PERIOD_BETWEEN_QUERIES { 3 };
        auto now = SteadyClock::now();
        if (now > _queryExpiry || viewIsDifferentEnough) {
            if (DependencyManager::get<SceneScriptingInterface>()->shouldRenderEntities()) {
                queryOctree(NodeType::EntityServer, PacketType::EntityQuery);
            }
            queryAvatars();

            _lastQueriedViews = _conicalViews;
            _queryExpiry = now + MIN_PERIOD_BETWEEN_QUERIES;
        }
    }

    // sent nack packets containing missing sequence numbers of received packets from nodes
    {
        quint64 sinceLastNack = now - _lastNackTime;
        const quint64 TOO_LONG_SINCE_LAST_NACK = 1 * USECS_PER_SECOND;
        if (sinceLastNack > TOO_LONG_SINCE_LAST_NACK) {
            _lastNackTime = now;
            sendNackPackets();
        }
    }

    // send packet containing downstream audio stats to the AudioMixer
    {
        quint64 sinceLastNack = now - _lastSendDownstreamAudioStats;
        if (sinceLastNack > TOO_LONG_SINCE_LAST_SEND_DOWNSTREAM_AUDIO_STATS && !isInterstitialMode()) {
            _lastSendDownstreamAudioStats = now;

            QMetaObject::invokeMethod(DependencyManager::get<AudioClient>().data(), "sendDownstreamAudioStatsPacket", Qt::QueuedConnection);
        }
    }

    {
        PerformanceTimer perfTimer("avatarManager/postUpdate");
        avatarManager->postUpdate(deltaTime, getMain3DScene());
    }

    {
        PROFILE_RANGE_EX(app, "PostUpdateLambdas", 0xffff0000, (uint64_t)0);
        PerformanceTimer perfTimer("postUpdateLambdas");
        std::unique_lock<std::mutex> guard(_postUpdateLambdasLock);
        for (auto& iter : _postUpdateLambdas) {
            iter.second();
        }
        _postUpdateLambdas.clear();
    }


    updateRenderArgs(deltaTime);

    {
        PerformanceTimer perfTimer("AnimDebugDraw");
        AnimDebugDraw::getInstance().update();
    }

    { // Game loop is done, mark the end of the frame for the scene transactions and the render loop to take over
        PerformanceTimer perfTimer("enqueueFrame");
        getMain3DScene()->enqueueFrame();
    }

    // If the display plugin is inactive then the frames won't be processed so process them here.
    if (!getActiveDisplayPlugin()->isActive()) {
        getMain3DScene()->processTransactionQueue();
    }

    // decide if the sensorToWorldMatrix is changing in a way that warrents squeezing the edges of the view down
    if (getActiveDisplayPlugin()->isHmd()) {
        PerformanceTimer perfTimer("squeezeVision");
        _visionSqueeze.updateVisionSqueeze(myAvatar->getSensorToWorldMatrix(), deltaTime);
    }
}


void Application::updateLOD(float deltaTime) const {
    PerformanceTimer perfTimer("LOD");
    // adjust it unless we were asked to disable this feature, or if we're currently in throttleRendering mode
    if (!isThrottleRendering()) {
        float presentTime = getActiveDisplayPlugin()->getAveragePresentTime();
        float engineRunTime = (float)(_graphicsEngine->getRenderEngine()->getConfiguration().get()->getCPURunTime());
        float gpuTime = getGPUContext()->getFrameTimerGPUAverage();
        float batchTime = getGPUContext()->getFrameTimerBatchAverage();
        auto lodManager = DependencyManager::get<LODManager>();
        lodManager->setRenderTimes(presentTime, engineRunTime, batchTime, gpuTime);
        lodManager->autoAdjustLOD(deltaTime);
    } else {
        DependencyManager::get<LODManager>()->resetLODAdjust();
    }
}

void Application::updateThreads(float deltaTime) {
    PerformanceTimer perfTimer("updateThreads");
    bool showWarnings = Menu::getInstance()->isOptionChecked(MenuOption::PipelineWarnings);
    PerformanceWarning warn(showWarnings, "Application::updateThreads()");

    // parse voxel packets
    if (!_enableProcessOctreeThread) {
        _octreeProcessor->threadRoutine();
        _entityEditSender->threadRoutine();
    }
}

void Application::userKickConfirmation(const QUuid& nodeID, unsigned int banFlags) {
    auto avatarHashMap = DependencyManager::get<AvatarHashMap>();
    auto avatar = avatarHashMap->getAvatarBySessionID(nodeID);

    QString userName;

    if (avatar) {
        userName = avatar->getSessionDisplayName();
    } else {
        userName = nodeID.toString();
    }

    QString kickMessage = "Do you wish to kick " + userName + " from your domain";
    ModalDialogListener* dlg = OffscreenUi::asyncQuestion("Kick User", kickMessage,
                                                          QMessageBox::Yes | QMessageBox::No);

    if (dlg->getDialogItem()) {

        QObject::connect(dlg, &ModalDialogListener::response, this, [=] (QVariant answer) {
            QObject::disconnect(dlg, &ModalDialogListener::response, this, nullptr);

            bool yes = (static_cast<QMessageBox::StandardButton>(answer.toInt()) == QMessageBox::Yes);
            // ask the NodeList to kick the user with the given session ID

            if (yes) {
                DependencyManager::get<NodeList>()->kickNodeBySessionID(nodeID, banFlags);
            }

            DependencyManager::get<UsersScriptingInterface>()->setWaitForKickResponse(false);
        });
        DependencyManager::get<UsersScriptingInterface>()->setWaitForKickResponse(true);
    }
}

std::shared_ptr<MyAvatar> Application::getMyAvatar() const {
    return DependencyManager::get<AvatarManager>()->getMyAvatar();
}

void Application::checkSkeleton() const {
    if (getMyAvatar()->getSkeletonModel()->isLoaded() && !getMyAvatar()->getSkeletonModel()->hasSkeleton()) {
        qCDebug(interfaceapp) << "MyAvatar model has no skeleton";

        QString message = "Your selected avatar body has no skeleton.\n\nThe default body will be loaded...";
        OffscreenUi::asyncWarning("", message);

        getMyAvatar()->useFullAvatarURL(AvatarData::defaultFullAvatarModelUrl(), DEFAULT_FULL_AVATAR_MODEL_NAME);
    } else {
        _physicsEngine->setCharacterController(getMyAvatar()->getCharacterController());
    }
}

void Application::queryAvatars() {
    if (!isInterstitialMode()) {
        auto avatarPacket = NLPacket::create(PacketType::AvatarQuery);
        auto destinationBuffer = reinterpret_cast<unsigned char*>(avatarPacket->getPayload());
        unsigned char* bufferStart = destinationBuffer;

        uint8_t numFrustums = (uint8_t)_conicalViews.size();
        memcpy(destinationBuffer, &numFrustums, sizeof(numFrustums));
        destinationBuffer += sizeof(numFrustums);

        for (const auto& view : _conicalViews) {
            destinationBuffer += view.serialize(destinationBuffer);
        }

        avatarPacket->setPayloadSize(destinationBuffer - bufferStart);

        DependencyManager::get<NodeList>()->broadcastToNodes(std::move(avatarPacket), NodeSet() << NodeType::AvatarMixer);
    }
}

void Application::tryToEnablePhysics() {
    bool enableInterstitial = DependencyManager::get<NodeList>()->getDomainHandler().getInterstitialModeEnabled();

    if (gpuTextureMemSizeStable() || !enableInterstitial) {
        _fullSceneCounterAtLastPhysicsCheck = _octreeProcessor->getFullSceneReceivedCounter();
        _lastQueriedViews.clear();  // Force new view.

        // process octree stats packets are sent in between full sends of a scene (this isn't currently true).
        // We keep physics disabled until we've received a full scene and everything near the avatar in that
        // scene is ready to compute its collision shape.
        auto myAvatar = getMyAvatar();
        if (myAvatar->isReadyForPhysics()) {
            myAvatar->getCharacterController()->setPhysicsEngine(_physicsEngine);
            _octreeProcessor->resetSafeLanding();
            _physicsEnabled = true;
            setIsInterstitialMode(false);
            myAvatar->updateMotionBehaviorFromMenu();
        }
    }
}
