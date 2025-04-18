//
//  Application_Setup.cpp
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

#include <functional>

#include <QDesktopServices>
#include <QFontDatabase>
#include <QtCore/QCommandLineParser>
#include <QtCore/QResource>
#include <QtQml/QQmlContext>
#include <QtQuick/QQuickWindow>

#include <AccountManager.h>
#include <AddressManager.h>
#include <AnimationCacheScriptingInterface.h>
#include <AvatarBookmarks.h>
#include <avatar/AvatarPackager.h>
#include <avatar/GrabManager.h>
#include <audio/AudioScope.h>
#include <AudioScriptingInterface.h>
#include <AutoUpdater.h>
#include <avatar/AvatarManager.h>
#include <BuildInfo.h>
#include <CameraRootTransformNode.h>
#include <crash-handler/CrashHandler.h>
#include <DebugDraw.h>
#include <DeferredLightingEffect.h>
#include <DesktopPreviewProvider.h>
#include <display-plugins/CompositorHelper.h>
#include <display-plugins/DisplayPlugin.h>
#include <DomainAccountManager.h>
#include <EntityScriptClient.h>
#include <EntityScriptServerLogClient.h>
#include <FingerprintUtils.h>
#include <FramebufferCache.h>
#include <gl/GLHelpers.h>
#include <GPUIdent.h>
#include <graphics-scripting/GraphicsScriptingInterface.h>
#include <hfm/ModelFormatRegistry.h>
#include <input-plugins/KeyboardMouseDevice.h>
#include <input-plugins/TouchscreenDevice.h>
#include <input-plugins/TouchscreenVirtualPadDevice.h>
#include <networking/CloseEventSender.h>
#include <MainWindow.h>
#include <material-networking/TextureCacheScriptingInterface.h>
#include <MessagesClient.h>
#include <Midi.h>
#include <model-networking/ModelCacheScriptingInterface.h>
#include <OffscreenUi.h>
#include <PickManager.h>
#include <plugins/InputConfiguration.h>
#include <plugins/OculusPlatformPlugin.h>
#include <plugins/PluginManager.h>
#include <plugins/SteamClientPlugin.h>
#include <PointerManager.h>
#include <Preferences.h>
#include <procedural/MaterialCacheScriptingInterface.h>
#include <procedural/ReferenceMaterial.h>
#include <raypick/MouseTransformNode.h>
#include <raypick/PickScriptingInterface.h>
#include <raypick/PointerScriptingInterface.h>
#include <raypick/RayPick.h>
#include <raypick/RayPickScriptingInterface.h>
#include <recording/ClipCache.h>
#include <recording/Deck.h>
#include <recording/Frame.h>
#include <recording/Recorder.h>
#include <recording/RecordingScriptingInterface.h>
#include <RenderableEntityItem.h>
#include <RenderableTextEntityItem.h>
#include <RenderableWebEntityItem.h>
#include <ResourceScriptingInterface.h>
#include <SceneScriptingInterface.h>
#include <ScriptEngines.h>
#include <scripting/Audio.h>
#include <scripting/AssetMappingsScriptingInterface.h>
#include <scripting/ControllerScriptingInterface.h>
#include <scripting/DesktopScriptingInterface.h>
#include <scripting/HMDScriptingInterface.h>
#include <scripting/KeyboardScriptingInterface.h>
#include <scripting/SelectionScriptingInterface.h>
#include <scripting/TestScriptingInterface.h>
#include <scripting/TTSScriptingInterface.h>
#include <scripting/WindowScriptingInterface.h>
#include <ShapeEntityItem.h>
#ifndef Q_OS_ANDROID
#include <shared/FileLogger.h>
#endif
#include <shared/GlobalAppProperties.h>
#include <shared/PlatformHelper.h>
#include <SoundCacheScriptingInterface.h>
#include <StatTracker.h>
#include <StencilMaskPass.h>
#include <ThreadHelpers.h>
#include <ui/DialogsManager.h>
#include <ui/DomainConnectionModel.h>
#include <ui/Keyboard.h>
#include <ui/LoginDialog.h>
#include <ui/OctreeStatsProvider.h>
#include <ui/OffscreenQmlSurfaceCache.h>
#include <ui/Snapshot.h>
#include <ui/StandAloneJSConsole.h>
#include <ui/TabletScriptingInterface.h>
#include <ui/ToolbarScriptingInterface.h>
#include <UserActivityLogger.h>
#include <UserActivityLoggerScriptingInterface.h>
#include <UsersScriptingInterface.h>
#include <VirtualPadManager.h>

#include "ApplicationEventHandler.h"
#include "ApplicationMeshProvider.h"
#include "AudioClient.h"
#include "CrashRecoveryHandler.h"
#include "DeadlockWatchdog.h"
#include "DiscordRichPresence.h"
#include "DiscoverabilityManager.h"
#include "FileDialogHelper.h"
#include "GLCanvas.h"
#include "InterfaceDynamicFactory.h"
#include "InterfaceLogging.h"
#include "InterfaceParentFinder.h"
#include "LocationBookmarks.h"
#include "LODManager.h"
#include "Menu.h"
#include "ResourceRequestObserver.h"
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
#include "SpeechRecognizer.h"
#endif
#include "Util.h"

#if defined(Q_OS_WIN)
#include <VersionHelpers.h>
#endif

#if defined(Q_OS_ANDROID)
#include "AndroidHelper.h"
#endif

using namespace std;

/*@jsdoc
 * <p>The <code>Controller.Hardware.Application</code> object has properties representing Interface's state. The property
 * values are integer IDs, uniquely identifying each output. <em>Read-only.</em></p>
 * <p>These states can be mapped to actions or functions or <code>Controller.Standard</code> items in a {@link RouteObject}
 * mapping (e.g., using the {@link RouteObject#when} method). Each data value is either <code>1.0</code> for "true" or
 * <code>0.0</code> for "false".</p>
 * <table>
 *   <thead>
 *     <tr><th>Property</th><th>Type</th><th>Data</th><th>Description</th></tr>
 *   </thead>
 *   <tbody>
 *     <tr><td><code>CameraFirstPerson</code></td><td>number</td><td>number</td><td>The camera is in first-person mode.
 *       <em>Legacy first person camera mode.</em></td></tr>
 *     <tr><td><code>CameraFirstPersonLookAt</code></td><td>number</td><td>number</td><td>The camera is in first-person mode.
 *       <em>Default first person camera mode.</em></td></tr>
 *     <tr><td><code>CameraThirdPerson</code></td><td>number</td><td>number</td><td>The camera is in third-person mode.
 *       <em>Legacy third person camera mode.</em></td></tr>
 *     <tr><td><code>CameraLookAt</code></td><td>number</td><td>number</td><td>The camera is in third-person mode.
 *       <em>Default third person camera mode.</em></td></tr>
 *     <tr><td><code>CameraFSM</code></td><td>number</td><td>number</td><td>The camera is in full screen mirror mode.
 *       <em>Legacy "look at myself" behavior.</em></td></tr>
 *     <tr><td><code>CameraSelfie</code></td><td>number</td><td>number</td><td>The camera is in selfie mode.
 *       <em>Default "look at myself" camera mode.</em></td></tr>
 *     <tr><td><code>CameraIndependent</code></td><td>number</td><td>number</td><td>The camera is in independent mode.</td></tr>
 *     <tr><td><code>CameraEntity</code></td><td>number</td><td>number</td><td>The camera is in entity mode.</td></tr>
 *     <tr><td><code>InHMD</code></td><td>number</td><td>number</td><td>The user is in HMD mode.</td></tr>
 *     <tr><td><code>CaptureMouse</code></td><td>number</td><td>number</td><td>The mouse is captured.  In this mode,
 *       the mouse is invisible and cannot leave the bounds of Interface, as long as Interface is the active window and
 *       no menu item is selected.</td></tr>
 *     <tr><td><code>AdvancedMovement</code></td><td>number</td><td>number</td><td>Advanced movement (walking) controls are
 *       enabled.</td></tr>
 *     <tr><td><code>StrafeEnabled</code></td><td>number</td><td>number</td><td>Strafing is enabled</td></tr>
 *     <tr><td><code>LeftHandDominant</code></td><td>number</td><td>number</td><td>Dominant hand set to left.</td></tr>
 *     <tr><td><code>RightHandDominant</code></td><td>number</td><td>number</td><td>Dominant hand set to right.</td></tr>
 *     <tr><td><code>SnapTurn</code></td><td>number</td><td>number</td><td>Snap turn is enabled.</td></tr>
 *     <tr><td><code>Grounded</code></td><td>number</td><td>number</td><td>The user's avatar is on the ground.</td></tr>
 *     <tr><td><code>NavigationFocused</code></td><td>number</td><td>number</td><td><em>Not used.</em></td></tr>
 *     <tr><td><code>PlatformWindows</code></td><td>number</td><td>number</td><td>The operating system is Windows.</td></tr>
 *     <tr><td><code>PlatformMac</code></td><td>number</td><td>number</td><td>The operating system is Mac.</td></tr>
 *     <tr><td><code>PlatformAndroid</code></td><td>number</td><td>number</td><td>The operating system is Android.</td></tr>
 *   </tbody>
 * </table>
 * @typedef {object} Controller.Hardware-Application
 */

static const QString STATE_IN_HMD = "InHMD";
static const QString STATE_CAMERA_FULL_SCREEN_MIRROR = "CameraFSM";
static const QString STATE_CAMERA_FIRST_PERSON = "CameraFirstPerson";
static const QString STATE_CAMERA_FIRST_PERSON_LOOK_AT = "CameraFirstPersonLookat";
static const QString STATE_CAMERA_THIRD_PERSON = "CameraThirdPerson";
static const QString STATE_CAMERA_ENTITY = "CameraEntity";
static const QString STATE_CAMERA_INDEPENDENT = "CameraIndependent";
static const QString STATE_CAMERA_LOOK_AT = "CameraLookAt";
static const QString STATE_CAMERA_SELFIE = "CameraSelfie";
static const QString STATE_SNAP_TURN = "SnapTurn";
static const QString STATE_ADVANCED_MOVEMENT_CONTROLS = "AdvancedMovement";
static const QString STATE_GROUNDED = "Grounded";
static const QString STATE_NAV_FOCUSED = "NavigationFocused";
static const QString STATE_PLATFORM_WINDOWS = "PlatformWindows";
static const QString STATE_PLATFORM_MAC = "PlatformMac";
static const QString STATE_PLATFORM_ANDROID = "PlatformAndroid";
static const QString STATE_LEFT_HAND_DOMINANT = "LeftHandDominant";
static const QString STATE_RIGHT_HAND_DOMINANT = "RightHandDominant";
static const QString STATE_STRAFE_ENABLED = "StrafeEnabled";
static const QString STATE_CAPTURE_MOUSE = "CaptureMouse";

static const QUrl AVATAR_INPUTS_BAR_QML = PathUtils::qmlUrl("AvatarInputsBar.qml");
static const QUrl BUBBLE_ICON_QML = PathUtils::qmlUrl("BubbleIcon.qml");

static const int ENTITY_SERVER_ADDED_TIMEOUT = 5000;

// For processing on QThreadPool, we target a number of threads after reserving some
// based on how many are being consumed by the application and the display plugin.  However,
// we will never drop below the 'min' value
static const int MIN_PROCESSING_THREAD_POOL_SIZE = 2;

#if !defined(Q_OS_ANDROID)
static const uint32_t MAX_CONCURRENT_RESOURCE_DOWNLOADS = 16;
#else
static const uint32_t MAX_CONCURRENT_RESOURCE_DOWNLOADS = 4;
#endif

#if defined(Q_OS_ANDROID)
static bool DISABLE_WATCHDOG = true;
#else
static const QString DISABLE_WATCHDOG_FLAG{ "HIFI_DISABLE_WATCHDOG" };
static bool DISABLE_WATCHDOG = nsightActive() || QProcessEnvironment::systemEnvironment().contains(DISABLE_WATCHDOG_FLAG);
#endif
static const int WATCHDOG_TIMER_TIMEOUT = 100;

#if defined(Q_OS_ANDROID)
static const QString TESTER_FILE = "/sdcard/_hifi_test_device.txt";
#endif

bool setupEssentials(const QCommandLineParser& parser, bool runningMarkerExisted) {
    const int listenPort = parser.isSet("listenPort") ? parser.value("listenPort").toInt() : INVALID_PORT;

    bool suppressPrompt = parser.isSet("suppress-settings-reset");

    // set the OCULUS_STORE property so the oculus plugin can know if we ran from the Oculus Store
    qApp->setProperty(hifi::properties::OCULUS_STORE, parser.isSet("oculus-store"));

    // emulate standalone device
    qApp->setProperty(hifi::properties::STANDALONE, parser.isSet("standalone"));

    // Ignore any previous crashes if running from command line with a test script.
    bool inTestMode = parser.isSet("testScript");

    bool previousSessionCrashed { false };
    if (!inTestMode) {
        // TODO: FIX
        previousSessionCrashed = CrashRecoveryHandler::checkForResetSettings(runningMarkerExisted, suppressPrompt);
    }

    // get dir to use for cache
    if (parser.isSet("cache")) {
        qApp->setProperty(hifi::properties::APP_LOCAL_DATA_PATH, parser.value("cache"));
    }

    {
        const QString resourcesBinaryFile = PathUtils::getRccPath();
        qCInfo(interfaceapp) << "Loading primary resources from" << resourcesBinaryFile;

        if (!QFile::exists(resourcesBinaryFile)) {
            throw std::runtime_error(QString("Unable to find primary resources from '%1'").arg(resourcesBinaryFile).toStdString());
        }
        if (!QResource::registerResource(resourcesBinaryFile)) {
            throw std::runtime_error(QString("Unable to load primary resources from '%1'").arg(resourcesBinaryFile).toStdString());
        }
    }

    DependencyManager::set<ScriptInitializers>();

    // Tell the plugin manager about our statically linked plugins
    auto pluginManager = PluginManager::getInstance();
    if (auto steamClient = pluginManager->getSteamClientPlugin()) {
        steamClient->init();
    }
    if (auto oculusPlatform = pluginManager->getOculusPlatformPlugin()) {
        oculusPlatform->init();
    }

    PROFILE_SET_THREAD_NAME("Main Thread");

#if defined(Q_OS_WIN)
    // Select appropriate audio DLL
    QString audioDLLPath = QCoreApplication::applicationDirPath();
    if (IsWindows8OrGreater()) {
        audioDLLPath += "/audioWin8";
    } else {
        audioDLLPath += "/audioWin7";
    }
    QCoreApplication::addLibraryPath(audioDLLPath);
#endif

    QString defaultScriptsOverrideOption = parser.value("defaultScriptsOverride");

    DependencyManager::registerInheritance<LimitedNodeList, NodeList>();
    DependencyManager::registerInheritance<AvatarHashMap, AvatarManager>();
    DependencyManager::registerInheritance<EntityDynamicFactoryInterface, InterfaceDynamicFactory>();
    DependencyManager::registerInheritance<SpatialParentFinder, InterfaceParentFinder>();

    // Set dependencies
    DependencyManager::set<PickManager>();
    DependencyManager::set<PointerManager>();
    DependencyManager::set<RayPickScriptingInterface>();
    DependencyManager::set<PointerScriptingInterface>();
    DependencyManager::set<PickScriptingInterface>();
    DependencyManager::set<Cursor::Manager>();
    DependencyManager::set<VirtualPad::Manager>();
    DependencyManager::set<DesktopPreviewProvider>();
#if defined(Q_OS_ANDROID)
    DependencyManager::set<AccountManager>(true); // use the default user agent getter
#else
    DependencyManager::set<AccountManager>(true, std::bind(&Application::getUserAgent, qApp));
#endif
    DependencyManager::set<DomainAccountManager>();
    DependencyManager::set<StatTracker>();
    DependencyManager::set<ScriptEngines>(ScriptManager::CLIENT_SCRIPT, defaultScriptsOverrideOption);
    DependencyManager::set<Preferences>();
    DependencyManager::set<recording::Deck>();
    DependencyManager::set<recording::Recorder>();
    DependencyManager::set<AddressManager>();
    DependencyManager::set<NodeList>(NodeType::Agent, listenPort);
    DependencyManager::set<recording::ClipCache>();
    DependencyManager::set<GeometryCache>();
    DependencyManager::set<ModelFormatRegistry>(); // ModelFormatRegistry must be defined before ModelCache. See the ModelCache constructor.
    DependencyManager::set<ModelCache>();
    DependencyManager::set<ModelCacheScriptingInterface>();
    DependencyManager::set<ScriptCache>();
    DependencyManager::set<SoundCache>();
    DependencyManager::set<SoundCacheScriptingInterface>();
    DependencyManager::set<AudioClient>();
    DependencyManager::set<AudioScope>();
    DependencyManager::set<DeferredLightingEffect>();
    DependencyManager::set<TextureCache>();
    DependencyManager::set<MaterialCache>();
    DependencyManager::set<TextureCacheScriptingInterface>();
    DependencyManager::set<MaterialCacheScriptingInterface>();
    DependencyManager::set<FramebufferCache>();
    DependencyManager::set<AnimationCache>();
    DependencyManager::set<AnimationCacheScriptingInterface>();
    DependencyManager::set<ModelBlender>();
    DependencyManager::set<UsersScriptingInterface>();
    DependencyManager::set<AvatarManager>();
    DependencyManager::set<LODManager>();
    DependencyManager::set<StandAloneJSConsole>();
    DependencyManager::set<DialogsManager>();
    DependencyManager::set<ResourceCacheSharedItems>();
    DependencyManager::set<DesktopScriptingInterface>();
    DependencyManager::set<EntityScriptingInterface>(true);
    DependencyManager::set<GraphicsScriptingInterface>();
    DependencyManager::registerInheritance<scriptable::ModelProviderFactory, ApplicationMeshProvider>();
    DependencyManager::set<ApplicationMeshProvider>();
    DependencyManager::set<RecordingScriptingInterface>();
    DependencyManager::set<WindowScriptingInterface>();
    DependencyManager::set<HMDScriptingInterface>();
    DependencyManager::set<ResourceScriptingInterface>();
    DependencyManager::set<TabletScriptingInterface>();
    DependencyManager::set<InputConfiguration>();
    DependencyManager::set<ToolbarScriptingInterface>();
    DependencyManager::set<UserActivityLoggerScriptingInterface>();
    DependencyManager::set<AssetMappingsScriptingInterface>();
    DependencyManager::set<DomainConnectionModel>();

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    DependencyManager::set<SpeechRecognizer>();
#endif
    DependencyManager::set<DiscoverabilityManager>();
    DependencyManager::set<SceneScriptingInterface>();
#if !defined(DISABLE_QML)
    DependencyManager::set<OffscreenUi>();
    {
        auto window = DependencyManager::get<OffscreenUi>()->getWindow();
        auto desktopScriptingInterface = DependencyManager::get<DesktopScriptingInterface>();
        QObject::connect(window, &QQuickWindow::focusObjectChanged, [desktopScriptingInterface](QObject *object) {
            if (object) {
                if (object->objectName() == QString("desktop")) {
                    emit desktopScriptingInterface->uiFocusChanged(false);
                    return;
                }
                // Signal with empty object name happens in addition to regular named ones and is not necessary here
                if (!object->objectName().isEmpty()) {
                    emit desktopScriptingInterface->uiFocusChanged(true);
                }
            }
        });
    }
#endif
    DependencyManager::set<Midi>();
    DependencyManager::set<PathUtils>();
    DependencyManager::set<InterfaceDynamicFactory>();
    DependencyManager::set<AudioInjectorManager>();
    DependencyManager::set<MessagesClient>();
    controller::StateController::setStateVariables({ { STATE_IN_HMD, STATE_CAMERA_FULL_SCREEN_MIRROR,
                    STATE_CAMERA_FIRST_PERSON, STATE_CAMERA_FIRST_PERSON_LOOK_AT, STATE_CAMERA_THIRD_PERSON,
                    STATE_CAMERA_ENTITY, STATE_CAMERA_INDEPENDENT, STATE_CAMERA_LOOK_AT, STATE_CAMERA_SELFIE, STATE_CAPTURE_MOUSE,
                    STATE_SNAP_TURN, STATE_ADVANCED_MOVEMENT_CONTROLS, STATE_GROUNDED, STATE_NAV_FOCUSED,
                    STATE_PLATFORM_WINDOWS, STATE_PLATFORM_MAC, STATE_PLATFORM_ANDROID, STATE_LEFT_HAND_DOMINANT, STATE_RIGHT_HAND_DOMINANT, STATE_STRAFE_ENABLED } });
    DependencyManager::set<UserInputMapper>();
    DependencyManager::set<controller::ScriptingInterface, ControllerScriptingInterface>();
    DependencyManager::set<InterfaceParentFinder>();
    DependencyManager::set<EntityTreeRenderer>(true, qApp, qApp);
    DependencyManager::set<CompositorHelper>();
    DependencyManager::set<OffscreenQmlSurfaceCache>();
    DependencyManager::set<EntityScriptClient>();

    DependencyManager::set<EntityScriptServerLogClient>();

    DependencyManager::set<OctreeStatsProvider>(nullptr, qApp->getOcteeSceneStats());
    DependencyManager::set<AvatarBookmarks>();
    DependencyManager::set<LocationBookmarks>();
    DependencyManager::set<Snapshot>();
    DependencyManager::set<CloseEventSender>();
    DependencyManager::set<ResourceManager>();
    DependencyManager::set<SelectionScriptingInterface>();
    DependencyManager::set<TTSScriptingInterface>();

    DependencyManager::set<ResourceRequestObserver>();
    DependencyManager::set<Keyboard>();
    DependencyManager::set<KeyboardScriptingInterface>();
    DependencyManager::set<GrabManager>();
    DependencyManager::set<AvatarPackager>();
    PlatformHelper::setup();

    QObject::connect(PlatformHelper::instance(), &PlatformHelper::systemWillWake, [] {
        QMetaObject::invokeMethod(DependencyManager::get<NodeList>().data(), "noteAwakening", Qt::QueuedConnection);
        QMetaObject::invokeMethod(DependencyManager::get<AudioClient>().data(), "noteAwakening", Qt::QueuedConnection);
    });

    QString setBookmarkValue = parser.value("setBookmark");
    if (!setBookmarkValue.isEmpty()) {
        // Bookmarks are expected to be in a name=url form.
        // An `=` character in the name or url is unsupported.
        auto parts = setBookmarkValue.split("=");
        if (parts.length() != 2) {
            qWarning() << "Malformed setBookmark argument: " << setBookmarkValue;
        } else {
            qDebug() << "Setting bookmark" << parts[0] << "to" << parts[1];
            DependencyManager::get<LocationBookmarks>()->insert(parts[0], parts[1]);
        }
    }

    return previousSessionCrashed;
}

void Application::initialize(const QCommandLineParser &parser) {
    //qCDebug(interfaceapp) << "Setting up essentials";
    setupEssentials(parser, _previousSessionCrashed);
    qCDebug(interfaceapp) << "Initializing application";

    _entitySimulation = std::make_shared<PhysicalEntitySimulation>();
    _physicsEngine = std::make_shared<PhysicsEngine>(Vectors::ZERO);
    _entityClipboard = std::make_shared<EntityTree>();
    _octreeProcessor = std::make_shared<OctreePacketProcessor>();
    _entityEditSender = std::make_shared<EntityEditPacketSender>();
    _graphicsEngine = std::make_shared<GraphicsEngine>();
    _applicationOverlay = std::make_shared<ApplicationOverlay>();
    _dialogsManagerScriptingInterface = new DialogsManagerScriptingInterface();

    auto steamClient = PluginManager::getInstance()->getSteamClientPlugin();
    setProperty(hifi::properties::STEAM, (steamClient && steamClient->isRunning()));

    constexpr auto INSTALLER_INI_NAME = "installer.ini";
    auto iniPath = QDir(applicationDirPath()).filePath(INSTALLER_INI_NAME);
    QFile installerFile { iniPath };
    std::unordered_map<QString, QString> installerKeyValues;
    if (installerFile.open(QIODevice::ReadOnly)) {
        while (!installerFile.atEnd()) {
            auto line = installerFile.readLine();
            if (!line.isEmpty()) {
                auto index = line.indexOf("=");
                if (index >= 0) {
                    installerKeyValues[line.mid(0, index).trimmed()] = line.mid(index + 1).trimmed();
                }
            }
        }
    }

    // In practice we shouldn't run across installs that don't have a known installer type.
    // Client or Client+Server installs should always have the installer.ini next to their
    // respective interface.exe, and Steam installs will be detected as such. If a user were
    // to delete the installer.ini, though, and as an example, we won't know the context of the
    // original install.
    constexpr auto INSTALLER_KEY_TYPE = "type";
    constexpr auto INSTALLER_KEY_CAMPAIGN = "campaign";
    constexpr auto INSTALLER_TYPE_UNKNOWN = "unknown";
    constexpr auto INSTALLER_TYPE_STEAM = "steam";

    auto typeIt = installerKeyValues.find(INSTALLER_KEY_TYPE);
    QString installerType = INSTALLER_TYPE_UNKNOWN;
    if (typeIt == installerKeyValues.end()) {
        if (property(hifi::properties::STEAM).toBool()) {
            installerType = INSTALLER_TYPE_STEAM;
        }
    } else {
        installerType = typeIt->second;
    }

    auto campaignIt = installerKeyValues.find(INSTALLER_KEY_CAMPAIGN);
    QString installerCampaign = campaignIt != installerKeyValues.end() ? campaignIt->second : "";

    qDebug() << "Detected installer type:" << installerType;
    qDebug() << "Detected installer campaign:" << installerCampaign;

    static int SEND_STATS_INTERVAL_MS;
    {
        if (parser.isSet("testScript")) {
            QString testScriptPath = parser.value("testScript");
            // If the URL scheme is http(s) or ftp, then use as is, else - treat it as a local file
            // This is done so as not break previous command line scripts
            if (testScriptPath.left(HIFI_URL_SCHEME_HTTP.length()) == HIFI_URL_SCHEME_HTTP ||
                testScriptPath.left(HIFI_URL_SCHEME_FTP.length()) == HIFI_URL_SCHEME_FTP) {

                setProperty(hifi::properties::TEST, QUrl::fromUserInput(testScriptPath));
            } else if (QFileInfo(testScriptPath).exists()) {
                setProperty(hifi::properties::TEST, QUrl::fromLocalFile(testScriptPath));
            }

            if (parser.isSet("quitWhenFinished")) {
                _quitWhenFinished = true;
            }
        }

        if (parser.isSet("testResultsLocation")) {
            // Set test snapshot location only if it is a writeable directory
            QString path = parser.value("testResultsLocation");

            QFileInfo fileInfo(path);
            if (fileInfo.isDir() && fileInfo.isWritable()) {
                TestScriptingInterface::getInstance()->setTestResultsLocation(path);
            }
        }
        _urlParam = parser.value("url");

        if (parser.isSet("disableWatchdog")) {
            DISABLE_WATCHDOG = true;
        }

        if (parser.isSet("system-cursor")) {
            _preferredCursor.set(Cursor::Manager::getIconName(Cursor::Icon::SYSTEM));
            _useSystemCursor = true;
        }

        if (parser.isSet("concurrent-downloads")) {
            bool success;
            uint32_t concurrentDownloads = parser.value("concurrent-downloads").toUInt(&success);
            if (!success) {
                concurrentDownloads = MAX_CONCURRENT_RESOURCE_DOWNLOADS;
            }
            ResourceCache::setRequestLimit(concurrentDownloads);
        }

        // perhaps override the avatar url.  Since we will test later for validity
        // we don't need to do so here.
        if (parser.isSet("avatarURL")) {
            _avatarOverrideUrl = QUrl::fromUserInput(parser.value("avatarURL"));
        }

        // If someone specifies both --avatarURL and --replaceAvatarURL,
        // the replaceAvatarURL wins.  So only set the _overrideUrl if this
        // does have a non-empty string.
        if (parser.isSet("replaceAvatarURL")) {
            QString replaceURL = parser.value("replaceAvatarURL");
            _avatarOverrideUrl = QUrl::fromUserInput(replaceURL);
            _saveAvatarOverrideUrl = true;
        }

#if !defined(Q_OS_ANDROID) && !defined(DISABLE_QML)
        const bool DISABLE_LOGIN_SCREEN = true; // Login screen is currently disabled
        // Do not show login dialog if requested not to on the command line
        if (DISABLE_LOGIN_SCREEN || parser.isSet("no-login-suggestion")) {
            _noLoginSuggestion = true;
        }
#endif

        if (parser.isSet("scripts")) {
            _defaultScriptsLocation.setPath(parser.value("scripts")); // Might need to be done in "main.cpp".
            _overrideDefaultScriptsLocation = true;
        } else {
            _overrideDefaultScriptsLocation = false;
        }

        // If launched from Steam, let it handle updates
        bool buildCanUpdate = BuildInfo::BUILD_TYPE == BuildInfo::BuildType::Stable
            || BuildInfo::BUILD_TYPE == BuildInfo::BuildType::Master;
        if (!parser.isSet("no-updater") && buildCanUpdate) {
            constexpr auto INSTALLER_TYPE_CLIENT_ONLY = "client_only";

            auto applicationUpdater = DependencyManager::set<AutoUpdater>();

            AutoUpdater::InstallerType type = installerType == INSTALLER_TYPE_CLIENT_ONLY
                ? AutoUpdater::InstallerType::CLIENT_ONLY : AutoUpdater::InstallerType::FULL;

            applicationUpdater->setInstallerType(type);
            applicationUpdater->setInstallerCampaign(installerCampaign);
            auto dialogsManager = DependencyManager::get<DialogsManager>();
            connect(applicationUpdater.data(), &AutoUpdater::newVersionIsAvailable, dialogsManager.data(), &DialogsManager::showUpdateDialog);
            applicationUpdater->checkForUpdate();
        }

        // setup the stats interval depending on if the 1s faster hearbeat was requested
        if (parser.isSet("fast-heartbeat")) {
            SEND_STATS_INTERVAL_MS = 1000;
        } else {
            SEND_STATS_INTERVAL_MS = 10000;
        }
    }

    {
        // identify gpu as early as possible to help identify OpenGL initialization errors.
        auto gpuIdent = GPUIdent::getInstance();
        auto &ch = CrashHandler::getInstance();

        ch.setAnnotation("sentry[contexts][gpu][name]", gpuIdent->getName().toStdString());
        ch.setAnnotation("sentry[contexts][gpu][version]", gpuIdent->getDriver().toStdString());
        ch.setAnnotation("gpu_memory", std::to_string(gpuIdent->getMemory()));
    }

    // make sure the debug draw singleton is initialized on the main thread.
    DebugDraw::getInstance().removeMarker("");

    PluginContainer* pluginContainer = dynamic_cast<PluginContainer*>(this); // set the container for any plugins that care
    PluginManager::getInstance()->setContainer(pluginContainer);

    QThreadPool::globalInstance()->setMaxThreadCount(MIN_PROCESSING_THREAD_POOL_SIZE);
    thread()->setPriority(QThread::HighPriority);
    thread()->setObjectName("Main Thread");

    setInstance(this);

    auto controllerScriptingInterface = DependencyManager::get<controller::ScriptingInterface>().data();
    _controllerScriptingInterface = dynamic_cast<ControllerScriptingInterface*>(controllerScriptingInterface);

    _entityClipboard->createRootElement();

#ifdef Q_OS_WIN
    installNativeEventFilter(&MyNativeEventFilter::getInstance());
#endif

    QFontDatabase::addApplicationFont(PathUtils::resourcesPath() + "styles/Inconsolata.otf");
    QFontDatabase::addApplicationFont(PathUtils::resourcesPath() + "fonts/fontawesome-webfont.ttf");
    QFontDatabase::addApplicationFont(PathUtils::resourcesPath() + "fonts/hifi-glyphs.ttf");
    QFontDatabase::addApplicationFont(PathUtils::resourcesPath() + "fonts/AnonymousPro-Regular.ttf");
    QFontDatabase::addApplicationFont(PathUtils::resourcesPath() + "fonts/Raleway-Light.ttf");
    QFontDatabase::addApplicationFont(PathUtils::resourcesPath() + "fonts/Raleway-Regular.ttf");
    QFontDatabase::addApplicationFont(PathUtils::resourcesPath() + "fonts/rawline-500.ttf");
    QFontDatabase::addApplicationFont(PathUtils::resourcesPath() + "fonts/Raleway-Bold.ttf");
    QFontDatabase::addApplicationFont(PathUtils::resourcesPath() + "fonts/Raleway-SemiBold.ttf");
    QFontDatabase::addApplicationFont(PathUtils::resourcesPath() + "fonts/Cairo-SemiBold.ttf");
    QFontDatabase::addApplicationFont(PathUtils::resourcesPath() + "fonts/FiraSans-SemiBold.ttf");
    QFontDatabase::addApplicationFont(PathUtils::resourcesPath() + "fonts/FiraSans-Regular.ttf");
    QFontDatabase::addApplicationFont(PathUtils::resourcesPath() + "fonts/FiraSans-Medium.ttf");
    _window->setWindowTitle("Overte");

    Model::setAbstractViewStateInterface(this); // The model class will sometimes need to know view state details from us

    auto nodeList = DependencyManager::get<NodeList>();
    nodeList->startThread();
    nodeList->setFlagTimeForConnectionStep(true);

    // move the AddressManager to the NodeList thread so that domain resets due to domain changes always occur
    // before we tell MyAvatar to go to a new location in the new domain
    auto addressManager = DependencyManager::get<AddressManager>();
    addressManager->moveToThread(nodeList->thread());

    // Set up a watchdog thread to intentionally crash the application on deadlocks
    if (!DISABLE_WATCHDOG) {
        auto deadlockWatchdogThread = new DeadlockWatchdogThread();
        deadlockWatchdogThread->setMainThreadID(QThread::currentThreadId());
        connect(deadlockWatchdogThread, &QThread::started, [] { setThreadName("DeadlockWatchdogThread"); });
        deadlockWatchdogThread->start();

        // Pause the deadlock watchdog when we sleep, or it might
        // trigger a false positive when we wake back up
        auto platformHelper = PlatformHelper::instance();

        connect(platformHelper, &PlatformHelper::systemWillSleep, [] {
            DeadlockWatchdogThread::pause();
        });

        connect(platformHelper, &PlatformHelper::systemWillWake, [] {
            DeadlockWatchdogThread::resume();
        });

        // Main thread timer to keep the watchdog updated
        QTimer* watchdogUpdateTimer = new QTimer(this);
        connect(watchdogUpdateTimer, &QTimer::timeout, [this] { updateHeartbeat(); });
        connect(this, &QCoreApplication::aboutToQuit, [watchdogUpdateTimer] {
            watchdogUpdateTimer->stop();
            watchdogUpdateTimer->deleteLater();
        });
        watchdogUpdateTimer->setSingleShot(false);
        watchdogUpdateTimer->setInterval(WATCHDOG_TIMER_TIMEOUT); // 100ms, Qt::CoarseTimer acceptable
        watchdogUpdateTimer->start();
    }

    // Set File Logger Session UUID
    auto avatarManager = DependencyManager::get<AvatarManager>();
    auto myAvatar = avatarManager ? avatarManager->getMyAvatar() : nullptr;
    if (avatarManager) {
        workload::SpacePointer space = getEntities()->getWorkloadSpace();
        avatarManager->setSpace(space);
    }

    auto accountManager = DependencyManager::get<AccountManager>();
    // set the account manager's root URL and trigger a login request if we don't have the access token
    accountManager->setIsAgent(true);
    accountManager->setAuthURL(MetaverseAPI::getCurrentMetaverseServerURL());
    if (!accountManager->hasKeyPair()) {
        accountManager->generateNewUserKeypair();
    }

#ifndef Q_OS_ANDROID
    _logger->setSessionID(accountManager->getSessionID());
#endif

    auto &ch = CrashHandler::getInstance();
    ch.setAnnotation("metaverse_session_id", accountManager->getSessionID().toString().toStdString());
    ch.setAnnotation("main_thread_id", std::to_string((size_t)QThread::currentThreadId()));

    if (steamClient) {
        qCDebug(interfaceapp) << "[VERSION] SteamVR buildID:" << steamClient->getSteamVRBuildID();
    }
    ch.setAnnotation("steam", property(hifi::properties::STEAM).toBool() ? "1" : "0");

    qCDebug(interfaceapp) << "[VERSION] Build sequence:" << qPrintable(applicationVersion());
    qCDebug(interfaceapp) << "[VERSION] MODIFIED_ORGANIZATION:" << BuildInfo::MODIFIED_ORGANIZATION;
    qCDebug(interfaceapp) << "[VERSION] VERSION:" << BuildInfo::VERSION;
    qCDebug(interfaceapp) << "[VERSION] BUILD_TYPE_STRING:" << BuildInfo::BUILD_TYPE_STRING;
    qCDebug(interfaceapp) << "[VERSION] BUILD_GLOBAL_SERVICES:" << BuildInfo::BUILD_GLOBAL_SERVICES;
#if USE_STABLE_GLOBAL_SERVICES
    qCDebug(interfaceapp) << "[VERSION] We will use STABLE global services.";
#else
    qCDebug(interfaceapp) << "[VERSION] We will use DEVELOPMENT global services.";
#endif

    updateHeartbeat();

    // Setup MessagesClient
    DependencyManager::get<MessagesClient>()->startThread();

    nodeList->getDomainHandler().setErrorDomainURL(QUrl(REDIRECT_HIFI_ADDRESS));

    // Inititalize sample before registering
    _sampleSound = DependencyManager::get<SoundCache>()->getSound(PathUtils::resourcesUrl("sounds/sample.wav"));

#ifdef _WIN32
    WSADATA WsaData;
    int wsaresult = WSAStartup(MAKEWORD(2, 2), &WsaData);
#endif

    // tell the NodeList instance who to tell the domain server we care about
    nodeList->addSetOfNodeTypesToNodeInterestSet(NodeSet() << NodeType::AudioMixer << NodeType::AvatarMixer
        << NodeType::EntityServer << NodeType::AssetServer << NodeType::MessagesMixer << NodeType::EntityScriptServer);

    // setDefaultFormat has no effect after the platform window has been created, so call it here.
    QSurfaceFormat::setDefaultFormat(getDefaultOpenGLSurfaceFormat());

    _glWidget = new GLCanvas();
    getApplicationCompositor().setRenderingWidget(_glWidget);
    _window->setCentralWidget(_glWidget);

    _window->restoreGeometry();
    _window->setVisible(true);

    _glWidget->setFocusPolicy(Qt::StrongFocus);
    _glWidget->setFocus();

    showCursor(Cursor::Manager::lookupIcon(_preferredCursor.get()));

    // enable mouse tracking; otherwise, we only get drag events
    _glWidget->setMouseTracking(true);
    // Make sure the window is set to the correct size by processing the pending events
    QCoreApplication::processEvents();

    // Create the main thread context, the GPU backend
    initializeGL();
    qCDebug(interfaceapp, "Initialized GL");

    // Initialize the display plugin architecture
    initializeDisplayPlugins();
    qCDebug(interfaceapp, "Initialized Display");

    if (_displayPlugin && !_displayPlugin->isHmd()) {
        showCursor(Cursor::Manager::lookupIcon(_preferredCursor.get()));
    }

    // An audio device changed signal received before the display plugins are set up will cause a crash,
    // so we defer the setup of the `scripting::Audio` class until this point
    DependencyManager::set<AudioScriptingInterface, scripting::Audio>();

    // Create the rendering engine.  This can be slow on some machines due to lots of
    // GPU pipeline creation.
    initializeRenderEngine();
    qCDebug(interfaceapp, "Initialized Render Engine.");

    _overlays.init(); // do this before scripts load

    // Initialize the user interface and menu system
    // Needs to happen AFTER the render engine initialization to access its configuration
    initializeUi();

    setupSignalsAndOperators();
    init();
    qCDebug(interfaceapp, "init() complete.");

    // create thread for parsing of octree data independent of the main network and rendering threads
    _octreeProcessor->initialize(_enableProcessOctreeThread);
    _entityEditSender->initialize(_enableProcessOctreeThread);

    // update before the first render
    update(0);

    // Make sure we don't time out during slow operations at startup
    updateHeartbeat();

    static const QString TESTER = "HIFI_TESTER";
    bool isTester = false;
#if defined (Q_OS_ANDROID)
    // Since we cannot set environment variables in Android we use a file presence
    // to denote that this is a testing device
    QFileInfo check_tester_file(TESTER_FILE);
    isTester = check_tester_file.exists() && check_tester_file.isFile();
#endif

    auto& userActivityLogger = UserActivityLogger::getInstance();
    if (userActivityLogger.isEnabled()) {
        // sessionRunTime will be reset soon by loadSettings. Grab it now to get previous session value.
        // The value will be 0 if the user blew away settings this session, which is both a feature and a bug.
        static const QString TESTER = "HIFI_TESTER";
        auto gpuIdent = GPUIdent::getInstance();
        auto glContextData = gl::ContextInfo::get();
        QJsonObject properties = {
            { "version", applicationVersion() },
            { "tester", QProcessEnvironment::systemEnvironment().contains(TESTER) || isTester },
            { "installer_campaign", installerCampaign },
            { "installer_type", installerType },
            { "build_type", BuildInfo::BUILD_TYPE_STRING },
            { "previousSessionCrashed", _previousSessionCrashed },
            { "previousSessionRuntime", (int)(_sessionRunTimer.elapsed() / MSECS_PER_SECOND) },
            { "cpu_architecture", QSysInfo::currentCpuArchitecture() },
            { "kernel_type", QSysInfo::kernelType() },
            { "kernel_version", QSysInfo::kernelVersion() },
            { "os_type", QSysInfo::productType() },
            { "os_version", QSysInfo::productVersion() },
            { "gpu_name", gpuIdent->getName() },
            { "gpu_driver", gpuIdent->getDriver() },
            { "gpu_memory", static_cast<qint64>(gpuIdent->getMemory()) },
            { "gl_version_int", glVersionToInteger(glContextData.version.c_str()) },
            { "gl_version", glContextData.version.c_str() },
            { "gl_vender", glContextData.vendor.c_str() },
            { "gl_sl_version", glContextData.shadingLanguageVersion.c_str() },
            { "gl_renderer", glContextData.renderer.c_str() },
            { "ideal_thread_count", QThread::idealThreadCount() }
        };
        auto macVersion = QSysInfo::macVersion();
        if (macVersion != QSysInfo::MV_None) {
            properties["os_osx_version"] = QSysInfo::macVersion();
        }
        auto windowsVersion = QSysInfo::windowsVersion();
        if (windowsVersion != QSysInfo::WV_None) {
            properties["os_win_version"] = QSysInfo::windowsVersion();
        }

        ProcessorInfo procInfo;
        if (getProcessorInfo(procInfo)) {
            properties["processor_core_count"] = procInfo.numProcessorCores;
            properties["logical_processor_count"] = procInfo.numLogicalProcessors;
            properties["processor_l1_cache_count"] = procInfo.numProcessorCachesL1;
            properties["processor_l2_cache_count"] = procInfo.numProcessorCachesL2;
            properties["processor_l3_cache_count"] = procInfo.numProcessorCachesL3;
        }

        properties["first_run"] = _firstRun.get();

        // add the user's machine ID to the launch event
        QString machineFingerPrint = uuidStringWithoutCurlyBraces(FingerprintUtils::getMachineFingerprint());
        properties["machine_fingerprint"] = machineFingerPrint;

        userActivityLogger.logAction("launch", properties);
    }

    _entityEditSender->setMyAvatar(myAvatar.get());

    // The entity octree will have to know about MyAvatar for the parentJointName import
    getEntities()->getTree()->setMyAvatar(myAvatar);
    _entityClipboard->setMyAvatar(myAvatar);

    // For now we're going to set the PPS for outbound packets to be super high, this is
    // probably not the right long term solution. But for now, we're going to do this to
    // allow you to move an entity around in your hand
    _entityEditSender->setPacketsPerSecond(3000); // super high!!

    // Make sure we don't time out during slow operations at startup
    updateHeartbeat();

    auto userInputMapper = DependencyManager::get<UserInputMapper>();
    _applicationStateDevice = userInputMapper->getStateDevice();

    _applicationStateDevice->setInputVariant(STATE_IN_HMD, []() -> float {
        return qApp->isHMDMode() ? 1 : 0;
    });
    _applicationStateDevice->setInputVariant(STATE_CAMERA_FULL_SCREEN_MIRROR, []() -> float {
        return qApp->getCamera().getMode() == CAMERA_MODE_MIRROR ? 1 : 0;
    });
    _applicationStateDevice->setInputVariant(STATE_CAMERA_FIRST_PERSON, []() -> float {
        return qApp->getCamera().getMode() == CAMERA_MODE_FIRST_PERSON ? 1 : 0;
    });
    _applicationStateDevice->setInputVariant(STATE_CAMERA_FIRST_PERSON_LOOK_AT, []() -> float {
        return qApp->getCamera().getMode() == CAMERA_MODE_FIRST_PERSON_LOOK_AT ? 1 : 0;
    });
    _applicationStateDevice->setInputVariant(STATE_CAMERA_THIRD_PERSON, []() -> float {
        return qApp->getCamera().getMode() == CAMERA_MODE_THIRD_PERSON ? 1 : 0;
    });
    _applicationStateDevice->setInputVariant(STATE_CAMERA_LOOK_AT, []() -> float {
        return qApp->getCamera().getMode() == CAMERA_MODE_LOOK_AT ? 1 : 0;
    });
    _applicationStateDevice->setInputVariant(STATE_CAMERA_SELFIE, []() -> float {
        return qApp->getCamera().getMode() == CAMERA_MODE_SELFIE ? 1 : 0;
    });
    _applicationStateDevice->setInputVariant(STATE_CAMERA_ENTITY, []() -> float {
        return qApp->getCamera().getMode() == CAMERA_MODE_ENTITY ? 1 : 0;
    });
    _applicationStateDevice->setInputVariant(STATE_CAMERA_INDEPENDENT, []() -> float {
        return qApp->getCamera().getMode() == CAMERA_MODE_INDEPENDENT ? 1 : 0;
    });
    _applicationStateDevice->setInputVariant(STATE_CAPTURE_MOUSE, []() -> float {
        return qApp->getCamera().getCaptureMouse() ? 1 : 0;
    });
    _applicationStateDevice->setInputVariant(STATE_SNAP_TURN, []() -> float {
        return qApp->getMyAvatar()->getSnapTurn() ? 1 : 0;
    });
    _applicationStateDevice->setInputVariant(STATE_ADVANCED_MOVEMENT_CONTROLS, []() -> float {
        return qApp->getMyAvatar()->useAdvancedMovementControls() ? 1 : 0;
    });
    _applicationStateDevice->setInputVariant(STATE_LEFT_HAND_DOMINANT, []() -> float {
        return qApp->getMyAvatar()->getDominantHand() == "left" ? 1 : 0;
    });
    _applicationStateDevice->setInputVariant(STATE_RIGHT_HAND_DOMINANT, []() -> float {
        return qApp->getMyAvatar()->getDominantHand() == "right" ? 1 : 0;
    });
    _applicationStateDevice->setInputVariant(STATE_STRAFE_ENABLED, []() -> float {
        return qApp->getMyAvatar()->getStrafeEnabled() ? 1 : 0;
    });

    _applicationStateDevice->setInputVariant(STATE_GROUNDED, []() -> float {
        return qApp->getMyAvatar()->getCharacterController()->onGround() ? 1 : 0;
    });
    _applicationStateDevice->setInputVariant(STATE_NAV_FOCUSED, [this]() -> float {
        auto offscreenUi = getOffscreenUI();
        return offscreenUi ? (offscreenUi->navigationFocused() ? 1 : 0) : 0;
    });
    _applicationStateDevice->setInputVariant(STATE_PLATFORM_WINDOWS, []() -> float {
#if defined(Q_OS_WIN)
        return 1;
#else
        return 0;
#endif
    });
    _applicationStateDevice->setInputVariant(STATE_PLATFORM_MAC, []() -> float {
#if defined(Q_OS_MAC)
        return 1;
#else
        return 0;
#endif
    });
    _applicationStateDevice->setInputVariant(STATE_PLATFORM_ANDROID, []() -> float {
#if defined(Q_OS_ANDROID)
        return 1 ;
#else
        return 0;
#endif
    });

    getRefreshRateManager().setRefreshRateRegime(RefreshRateManager::RefreshRateRegime::STARTUP);

    // Setup the _keyboardMouseDevice, _touchscreenDevice, _touchscreenVirtualPadDevice and the user input mapper with the default bindings
    userInputMapper->registerDevice(_keyboardMouseDevice->getInputDevice());
    // if the _touchscreenDevice is not supported it will not be registered
    if (_touchscreenDevice) {
        userInputMapper->registerDevice(_touchscreenDevice->getInputDevice());
    }
    if (_touchscreenVirtualPadDevice) {
        userInputMapper->registerDevice(_touchscreenVirtualPadDevice->getInputDevice());
    }

    // Make sure we don't time out during slow operations at startup
    updateHeartbeat();

    loadSettings(parser);

    updateVerboseLogging();

    setCachebustRequire();

    // Make sure we don't time out during slow operations at startup
    updateHeartbeat();

    QTimer* settingsTimer = new QTimer();
    int SAVE_SETTINGS_INTERVAL = 10 * MSECS_PER_SECOND; // Let's save every seconds for now
    settingsTimer->setSingleShot(false);
    settingsTimer->setInterval(SAVE_SETTINGS_INTERVAL); // 10s, Qt::CoarseTimer acceptable
    QObject::connect(settingsTimer, &QTimer::timeout, this, &Application::saveSettings);
    settingsTimer->start();

    if (Menu::getInstance()->isOptionChecked(MenuOption::FirstPersonLookAt)) {
        getMyAvatar()->setBoomLength(MyAvatar::ZOOM_MIN);  // So that camera doesn't auto-switch to third person.
    }

    {
        auto audioIO = DependencyManager::get<AudioClient>().data();
        // set the local loopback interface for local sounds
        AudioInjector::setLocalAudioInterface(audioIO);
        auto audioScriptingInterface = DependencyManager::get<AudioScriptingInterface>();
        audioScriptingInterface->setLocalAudioInterface(audioIO);

    }

    this->installEventFilter(this);

    Menu::getInstance()->setIsOptionChecked(MenuOption::ActionMotorControl, true);

// FIXME spacemouse code still needs cleanup
#if 0
    // the 3Dconnexion device wants to be initialized after a window is displayed.
    SpacemouseManager::getInstance().init();
#endif

    // Add periodic checks to send user activity data
    static int CHECK_NEARBY_AVATARS_INTERVAL_MS = 10000;
    static int NEARBY_AVATAR_RADIUS_METERS = 10;

    static glm::vec3 lastAvatarPosition = myAvatar->getWorldPosition();
    static glm::mat4 lastHMDHeadPose = getHMDSensorPose();
    static controller::Pose lastLeftHandPose = myAvatar->getLeftHandPose();
    static controller::Pose lastRightHandPose = myAvatar->getRightHandPose();

    // Periodically send fps as a user activity event
    QTimer* sendStatsTimer = new QTimer(this);
    sendStatsTimer->setInterval(SEND_STATS_INTERVAL_MS);  // 10s, Qt::CoarseTimer acceptable
    connect(sendStatsTimer, &QTimer::timeout, this, [this]() {

        QJsonObject properties = {};
        MemoryInfo memInfo;
        if (getMemoryInfo(memInfo)) {
            properties["system_memory_total"] = static_cast<qint64>(memInfo.totalMemoryBytes);
            properties["system_memory_used"] = static_cast<qint64>(memInfo.usedMemoryBytes);
            properties["process_memory_used"] = static_cast<qint64>(memInfo.processUsedMemoryBytes);
        }

        // content location and build info - useful for filtering stats
        auto addressManager = DependencyManager::get<AddressManager>();
        auto currentDomain = addressManager->currentShareableAddress(true).toString(); // domain only
        auto currentPath = addressManager->currentPath(true); // with orientation
        properties["current_domain"] = currentDomain;
        properties["current_path"] = currentPath;
        properties["build_version"] = BuildInfo::VERSION;

        auto displayPlugin = qApp->getActiveDisplayPlugin();

        properties["render_rate"] = getRenderLoopRate();
        properties["target_render_rate"] = getTargetRenderFrameRate();
        properties["present_rate"] = displayPlugin->presentRate();
        properties["new_frame_present_rate"] = displayPlugin->newFramePresentRate();
        properties["dropped_frame_rate"] = displayPlugin->droppedFrameRate();
        properties["stutter_rate"] = displayPlugin->stutterRate();
        properties["game_rate"] = getGameLoopRate();
        properties["has_async_reprojection"] = displayPlugin->hasAsyncReprojection();
        properties["hardware_stats"] = displayPlugin->getHardwareStats();

        // deadlock watchdog related stats
        properties["deadlock_watchdog_maxElapsed"] = (int)DeadlockWatchdogThread::_maxElapsed;
        properties["deadlock_watchdog_maxElapsedAverage"] = (int)DeadlockWatchdogThread::_maxElapsedAverage;

        auto nodeList = DependencyManager::get<NodeList>();
        properties["packet_rate_in"] = nodeList->getInboundPPS();
        properties["packet_rate_out"] = nodeList->getOutboundPPS();
        properties["kbps_in"] = nodeList->getInboundKbps();
        properties["kbps_out"] = nodeList->getOutboundKbps();

        SharedNodePointer entityServerNode = nodeList->soloNodeOfType(NodeType::EntityServer);
        SharedNodePointer audioMixerNode = nodeList->soloNodeOfType(NodeType::AudioMixer);
        SharedNodePointer avatarMixerNode = nodeList->soloNodeOfType(NodeType::AvatarMixer);
        SharedNodePointer assetServerNode = nodeList->soloNodeOfType(NodeType::AssetServer);
        SharedNodePointer messagesMixerNode = nodeList->soloNodeOfType(NodeType::MessagesMixer);
        properties["entity_ping"] = entityServerNode ? entityServerNode->getPingMs() : -1;
        properties["audio_ping"] = audioMixerNode ? audioMixerNode->getPingMs() : -1;
        properties["avatar_ping"] = avatarMixerNode ? avatarMixerNode->getPingMs() : -1;
        properties["asset_ping"] = assetServerNode ? assetServerNode->getPingMs() : -1;
        properties["messages_ping"] = messagesMixerNode ? messagesMixerNode->getPingMs() : -1;
        properties["atp_in_kbps"] = assetServerNode ? assetServerNode->getInboundKbps() : 0.0f;

        auto loadingRequests = ResourceCache::getLoadingRequests();

        QJsonArray loadingRequestsStats;
        for (const auto& requestPair : loadingRequests) {
            QJsonObject requestStats;
            requestStats["filename"] = requestPair.first->getURL().fileName();
            requestStats["received"] = requestPair.first->getBytesReceived();
            requestStats["total"] = requestPair.first->getBytesTotal();
            requestStats["attempts"] = (int)requestPair.first->getDownloadAttempts();
            loadingRequestsStats.append(requestStats);
        }

        properties["active_downloads"] = loadingRequests.size();
        properties["pending_downloads"] = (int)ResourceCache::getPendingRequestCount();
        properties["active_downloads_details"] = loadingRequestsStats;

        auto statTracker = DependencyManager::get<StatTracker>();

        properties["processing_resources"] = statTracker->getStat("Processing").toInt();
        properties["pending_processing_resources"] = statTracker->getStat("PendingProcessing").toInt();

        QJsonObject startedRequests;
        startedRequests["atp"] = statTracker->getStat(STAT_ATP_REQUEST_STARTED).toInt();
        startedRequests["http"] = statTracker->getStat(STAT_HTTP_REQUEST_STARTED).toInt();
        startedRequests["file"] = statTracker->getStat(STAT_FILE_REQUEST_STARTED).toInt();
        startedRequests["total"] = startedRequests["atp"].toInt() + startedRequests["http"].toInt()
            + startedRequests["file"].toInt();
        properties["started_requests"] = startedRequests;

        QJsonObject successfulRequests;
        successfulRequests["atp"] = statTracker->getStat(STAT_ATP_REQUEST_SUCCESS).toInt();
        successfulRequests["http"] = statTracker->getStat(STAT_HTTP_REQUEST_SUCCESS).toInt();
        successfulRequests["file"] = statTracker->getStat(STAT_FILE_REQUEST_SUCCESS).toInt();
        successfulRequests["total"] = successfulRequests["atp"].toInt() + successfulRequests["http"].toInt()
            + successfulRequests["file"].toInt();
        properties["successful_requests"] = successfulRequests;

        QJsonObject failedRequests;
        failedRequests["atp"] = statTracker->getStat(STAT_ATP_REQUEST_FAILED).toInt();
        failedRequests["http"] = statTracker->getStat(STAT_HTTP_REQUEST_FAILED).toInt();
        failedRequests["file"] = statTracker->getStat(STAT_FILE_REQUEST_FAILED).toInt();
        failedRequests["total"] = failedRequests["atp"].toInt() + failedRequests["http"].toInt()
            + failedRequests["file"].toInt();
        properties["failed_requests"] = failedRequests;

        QJsonObject cacheRequests;
        cacheRequests["atp"] = statTracker->getStat(STAT_ATP_REQUEST_CACHE).toInt();
        cacheRequests["http"] = statTracker->getStat(STAT_HTTP_REQUEST_CACHE).toInt();
        cacheRequests["total"] = cacheRequests["atp"].toInt() + cacheRequests["http"].toInt();
        properties["cache_requests"] = cacheRequests;

        QJsonObject atpMappingRequests;
        atpMappingRequests["started"] = statTracker->getStat(STAT_ATP_MAPPING_REQUEST_STARTED).toInt();
        atpMappingRequests["failed"] = statTracker->getStat(STAT_ATP_MAPPING_REQUEST_FAILED).toInt();
        atpMappingRequests["successful"] = statTracker->getStat(STAT_ATP_MAPPING_REQUEST_SUCCESS).toInt();
        properties["atp_mapping_requests"] = atpMappingRequests;

        properties["throttled"] = _displayPlugin ? _displayPlugin->isThrottled() : false;

        QJsonObject bytesDownloaded;
        auto atpBytes = statTracker->getStat(STAT_ATP_RESOURCE_TOTAL_BYTES).toLongLong();
        auto httpBytes = statTracker->getStat(STAT_HTTP_RESOURCE_TOTAL_BYTES).toLongLong();
        auto fileBytes = statTracker->getStat(STAT_FILE_RESOURCE_TOTAL_BYTES).toLongLong();
        bytesDownloaded["atp"] = atpBytes;
        bytesDownloaded["http"] = httpBytes;
        bytesDownloaded["file"] = fileBytes;
        bytesDownloaded["total"] = atpBytes + httpBytes + fileBytes;
        properties["bytes_downloaded"] = bytesDownloaded;

        auto myAvatar = getMyAvatar();
        glm::vec3 avatarPosition = myAvatar->getWorldPosition();
        properties["avatar_has_moved"] = lastAvatarPosition != avatarPosition;
        lastAvatarPosition = avatarPosition;

        auto entityScriptingInterface = DependencyManager::get<EntityScriptingInterface>();
        auto entityActivityTracking = entityScriptingInterface->getActivityTracking();
        entityScriptingInterface->resetActivityTracking();
        properties["added_entity_cnt"] = entityActivityTracking.addedEntityCount;
        properties["deleted_entity_cnt"] = entityActivityTracking.deletedEntityCount;
        properties["edited_entity_cnt"] = entityActivityTracking.editedEntityCount;

        NodeToOctreeSceneStats* octreeServerSceneStats = getOcteeSceneStats();
        unsigned long totalServerOctreeElements = 0;
        for (NodeToOctreeSceneStatsIterator i = octreeServerSceneStats->begin(); i != octreeServerSceneStats->end(); i++) {
            totalServerOctreeElements += i->second.getTotalElements();
        }

        properties["local_octree_elements"] = (qint64) OctreeElement::getInternalNodeCount();
        properties["server_octree_elements"] = (qint64) totalServerOctreeElements;

        properties["active_display_plugin"] = getActiveDisplayPlugin()->getName();
        properties["using_hmd"] = isHMDMode();

        auto contextInfo = gl::ContextInfo::get();
        properties["gl_info"] = QJsonObject{
            { "version", contextInfo.version.c_str() },
            { "sl_version", contextInfo.shadingLanguageVersion.c_str() },
            { "vendor", contextInfo.vendor.c_str() },
            { "renderer", contextInfo.renderer.c_str() },
        };
        properties["gpu_used_memory"] = (int)BYTES_TO_MB(gpu::Context::getUsedGPUMemSize());
        properties["gpu_free_memory"] = (int)BYTES_TO_MB(gpu::Context::getFreeGPUMemSize());
        properties["gpu_frame_time"] = (float)(qApp->getGPUContext()->getFrameTimerGPUAverage());
        properties["batch_frame_time"] = (float)(qApp->getGPUContext()->getFrameTimerBatchAverage());
        properties["ideal_thread_count"] = QThread::idealThreadCount();

        auto hmdHeadPose = getHMDSensorPose();
        properties["hmd_head_pose_changed"] = isHMDMode() && (hmdHeadPose != lastHMDHeadPose);
        lastHMDHeadPose = hmdHeadPose;

        auto leftHandPose = myAvatar->getLeftHandPose();
        auto rightHandPose = myAvatar->getRightHandPose();
        // controller::Pose considers two poses to be different if either are invalid. In our case, we actually
        // want to consider the pose to be unchanged if it was invalid and still is invalid, so we check that first.
        properties["hand_pose_changed"] =
            ((leftHandPose.valid || lastLeftHandPose.valid) && (leftHandPose != lastLeftHandPose))
            || ((rightHandPose.valid || lastRightHandPose.valid) && (rightHandPose != lastRightHandPose));
        lastLeftHandPose = leftHandPose;
        lastRightHandPose = rightHandPose;

        UserActivityLogger::getInstance().logAction("stats", properties);
    });
    sendStatsTimer->start();

    // Periodically check for count of nearby avatars
    static int lastCountOfNearbyAvatars = -1;
    QTimer* checkNearbyAvatarsTimer = new QTimer(this);
    checkNearbyAvatarsTimer->setInterval(CHECK_NEARBY_AVATARS_INTERVAL_MS); // 10 seconds, Qt::CoarseTimer ok
    connect(checkNearbyAvatarsTimer, &QTimer::timeout, this, []() {
        auto avatarManager = DependencyManager::get<AvatarManager>();
        int nearbyAvatars = avatarManager->numberOfAvatarsInRange(avatarManager->getMyAvatar()->getWorldPosition(),
                                                                  NEARBY_AVATAR_RADIUS_METERS) - 1;
        if (nearbyAvatars != lastCountOfNearbyAvatars) {
            lastCountOfNearbyAvatars = nearbyAvatars;
            UserActivityLogger::getInstance().logAction("nearby_avatars", { { "count", nearbyAvatars } });
        }
    });
    checkNearbyAvatarsTimer->start();

    // Track user activity event when we receive a mute packet
    auto onMutedByMixer = []() {
        UserActivityLogger::getInstance().logAction("received_mute_packet");
    };
    connect(DependencyManager::get<AudioClient>().data(), &AudioClient::mutedByMixer, this, onMutedByMixer);

    // Track when the address bar is opened
    auto onAddressBarShown = [this]() {
        // Record time
        UserActivityLogger::getInstance().logAction("opened_address_bar", { { "uptime_ms", _sessionRunTimer.elapsed() } });
    };
    connect(DependencyManager::get<DialogsManager>().data(), &DialogsManager::addressBarShown, this, onAddressBarShown);

    // Make sure we don't time out during slow operations at startup
    updateHeartbeat();

    auto entityScriptingInterface = DependencyManager::get<EntityScriptingInterface>();
    OctreeEditPacketSender* packetSender = entityScriptingInterface->getPacketSender();
    EntityEditPacketSender* entityPacketSender = static_cast<EntityEditPacketSender*>(packetSender);
    entityPacketSender->setMyAvatar(myAvatar.get());

    qCDebug(interfaceapp, "Startup time: %4.2f seconds.", (double)_sessionRunTimer.elapsed() / MSECS_PER_SECOND);

    updateSystemTabletMode();

    // Setup the mouse ray pick
    {
        auto mouseRayPick = std::make_shared<RayPick>(Vectors::ZERO, Vectors::UP, PickFilter(PickScriptingInterface::getPickEntities() | PickScriptingInterface::getPickLocalEntities()), 0.0f, 0.0f, true);
        mouseRayPick->parentTransform = std::make_shared<MouseTransformNode>();
        mouseRayPick->setJointState(PickQuery::JOINT_STATE_MOUSE);
        auto mouseRayPickID = DependencyManager::get<PickManager>()->addPick(PickQuery::Ray, mouseRayPick);
        DependencyManager::get<EntityTreeRenderer>()->setMouseRayPickID(mouseRayPickID);
    }

    // Setup the camera clipping ray pick
    {
        _prevCameraClippingEnabled = _cameraClippingEnabled.get();
        auto cameraRayPick = std::make_shared<RayPick>(Vectors::ZERO, -Vectors::UP,
                                                       PickFilter(PickScriptingInterface::getPickEntities() |
                                                                  PickScriptingInterface::getPickLocalEntities()),
                                                       MyAvatar::ZOOM_MAX, 0.0f, _prevCameraClippingEnabled);
        cameraRayPick->parentTransform = std::make_shared<CameraRootTransformNode>();
        _cameraClippingRayPickID = DependencyManager::get<PickManager>()->addPick(PickQuery::Ray, cameraRayPick);
    }

    // Preload Tablet sounds
    DependencyManager::get<EntityScriptingInterface>()->setEntityTree(qApp->getEntities()->getTree());
    DependencyManager::get<TabletScriptingInterface>()->preloadSounds();
    DependencyManager::get<Keyboard>()->createKeyboard();

    // Needs to happen later in the constructor as it depends on some other things being set up
    _discordPresence = new DiscordPresence();

    _pendingIdleEvent = false;
    _graphicsEngine->startup();

    qCDebug(interfaceapp) << "Directory Service session ID is" << uuidStringWithoutCurlyBraces(accountManager->getSessionID());

    pauseUntilLoginDetermined();
}

void Application::init() {
    // Make sure Login state is up to date
#if !defined(DISABLE_QML)
    DependencyManager::get<DialogsManager>()->toggleLoginDialog();
#endif
    DependencyManager::get<AvatarManager>()->init();

    _lastTimeUpdated.start();

    if (auto steamClient = PluginManager::getInstance()->getSteamClientPlugin()) {
        // when +connect_lobby in command line, join steam lobby
        const QString STEAM_LOBBY_COMMAND_LINE_KEY = "+connect_lobby";
        int lobbyIndex = arguments().indexOf(STEAM_LOBBY_COMMAND_LINE_KEY);
        if (lobbyIndex != -1) {
            QString lobbyId = arguments().value(lobbyIndex + 1);
            steamClient->joinLobby(lobbyId);
        }
    }

    qCDebug(interfaceapp) << "Loaded settings";

    // fire off an immediate domain-server check in now that settings are loaded
    QMetaObject::invokeMethod(DependencyManager::get<NodeList>().data(), "sendDomainServerCheckIn");

    // This allows collision to be set up properly for shape entities supported by GeometryCache.
    // This is before entity setup to ensure that it's ready for whenever instance collision is initialized.
    ShapeEntityItem::setShapeInfoCalulator(ShapeEntityItem::ShapeInfoCalculator(&shapeInfoCalculator));

    getEntities()->init();

    ObjectMotionState::setShapeManager(&_shapeManager);
    _physicsEngine->init();

    EntityTreePointer tree = getEntities()->getTree();
    _entitySimulation->init(tree, _physicsEngine, _entityEditSender.get());
    tree->setSimulation(_entitySimulation);

    _gameWorkload.startup(getEntities()->getWorkloadSpace(), _graphicsEngine->getRenderScene(), _entitySimulation);
    _entitySimulation->setWorkloadSpace(getEntities()->getWorkloadSpace());
}

void Application::setupSignalsAndOperators() {
    auto avatarManager = DependencyManager::get<AvatarManager>();
    auto myAvatar = avatarManager ? avatarManager->getMyAvatar() : nullptr;
    auto dialogsManager = DependencyManager::get<DialogsManager>();
    auto nodeList = DependencyManager::get<NodeList>();
    const DomainHandler& domainHandler = nodeList->getDomainHandler();

    // General
    {
        connect(this, SIGNAL(aboutToQuit()), this, SLOT(onAboutToQuit()));
        connect(this, &Application::applicationStateChanged, this, &Application::activeChanged);
        connect(_window, SIGNAL(windowMinimizedChanged(bool)), this, SLOT(windowMinimizedChanged(bool)));

        auto discoverabilityManager = DependencyManager::get<DiscoverabilityManager>();

        // setup a timer for domain-server check ins
        QTimer* domainCheckInTimer = new QTimer(this);
        connect(domainCheckInTimer, &QTimer::timeout, nodeList.data(), &NodeList::sendDomainServerCheckIn);
        domainCheckInTimer->start(DOMAIN_SERVER_CHECK_IN_MSECS);
        connect(this, &QCoreApplication::aboutToQuit, [domainCheckInTimer] {
            domainCheckInTimer->stop();
            domainCheckInTimer->deleteLater();
        });

        connect(&domainHandler, SIGNAL(domainURLChanged(QUrl)), SLOT(domainURLChanged(QUrl)));
        connect(&domainHandler, SIGNAL(redirectToErrorDomainURL(QUrl)), SLOT(goToErrorDomainURL(QUrl)));
        connect(&domainHandler, &DomainHandler::domainURLChanged, [](QUrl domainURL){
            auto &ch = CrashHandler::getInstance();
            ch.setAnnotation("domain", domainURL.toString().toStdString());
        });
        connect(&domainHandler, SIGNAL(resetting()), SLOT(resettingDomain()));
        connect(&domainHandler, SIGNAL(connectedToDomain(QUrl)), SLOT(updateWindowTitle()));
        connect(&domainHandler, SIGNAL(disconnectedFromDomain()), SLOT(updateWindowTitle()));
        connect(&domainHandler, &DomainHandler::disconnectedFromDomain, this, [this]() {
            auto entityScriptingInterface = DependencyManager::get<EntityScriptingInterface>();
            entityScriptingInterface->deleteEntity(getTabletScreenID());
            entityScriptingInterface->deleteEntity(getTabletHomeButtonID());
            entityScriptingInterface->deleteEntity(getTabletFrameID());
            _failedToConnectToEntityServer = false;
        });

        connect(&domainHandler, &DomainHandler::confirmConnectWithoutAvatarEntities,
            this, &Application::confirmConnectWithoutAvatarEntities);

        connect(&domainHandler, &DomainHandler::connectedToDomain, this, [this]() {
            if (!isServerlessMode()) {
                _entityServerConnectionTimer.setInterval(ENTITY_SERVER_ADDED_TIMEOUT);
                _entityServerConnectionTimer.start();
                _failedToConnectToEntityServer = false;
            }
        });
        // if we get a domain change, immediately attempt update location in directory server
        connect(&domainHandler, &DomainHandler::connectedToDomain, discoverabilityManager.data(),
                &DiscoverabilityManager::updateLocation);
        connect(&domainHandler, &DomainHandler::domainConnectionRefused, this, &Application::domainConnectionRefused);

        // We could clear ATP assets only when changing domains, but it's possible that the domain you are connected
        // to has gone down and switched to a new content set, so when you reconnect the cached ATP assets will no longer be valid.
        connect(&domainHandler, &DomainHandler::disconnectedFromDomain, DependencyManager::get<ScriptCache>().data(), &ScriptCache::clearATPScriptsFromCache);

        // you might think we could just do this in NodeList but we only want this connection for Interface
        connect(&domainHandler, &DomainHandler::limitOfSilentDomainCheckInsReached,
            nodeList.data(), [nodeList]() { nodeList->reset("Domain checkin limit"); });


        // update our location every 5 seconds in the directory server, assuming that we are authenticated with one
        const qint64 DATA_SERVER_LOCATION_CHANGE_UPDATE_MSECS = 5 * MSECS_PER_SECOND;
        connect(&_locationUpdateTimer, &QTimer::timeout, discoverabilityManager.data(), &DiscoverabilityManager::updateLocation);
        connect(&_locationUpdateTimer, &QTimer::timeout, DependencyManager::get<AddressManager>().data(),
                &AddressManager::storeCurrentAddress);
        _locationUpdateTimer.start(DATA_SERVER_LOCATION_CHANGE_UPDATE_MSECS);

        // send a location update immediately
        discoverabilityManager->updateLocation();

        connect(nodeList.data(), &NodeList::nodeAdded, this, &Application::nodeAdded);
        connect(nodeList.data(), &NodeList::nodeKilled, this, &Application::nodeKilled);
        connect(nodeList.data(), &NodeList::nodeActivated, this, &Application::nodeActivated);
        connect(nodeList.data(), &NodeList::uuidChanged, myAvatar.get(), &MyAvatar::setSessionUUID);
        connect(nodeList.data(), &NodeList::uuidChanged, this, &Application::setSessionUUID);
        connect(nodeList.data(), &NodeList::packetVersionMismatch, this, &Application::notifyPacketVersionMismatch);

        auto accountManager = DependencyManager::get<AccountManager>();
#if defined(Q_OS_ANDROID)
        connect(accountManager.data(), &AccountManager::authRequired, this, []() {
            auto addressManager = DependencyManager::get<AddressManager>();
            AndroidHelper::instance().showLoginDialog(addressManager->currentAddress());
        });
#else
        connect(accountManager.data(), &AccountManager::authRequired, dialogsManager.data(), &DialogsManager::showLoginDialog);
#endif
        connect(accountManager.data(), &AccountManager::usernameChanged, this, &Application::updateWindowTitle);

        auto domainAccountManager = DependencyManager::get<DomainAccountManager>();
        connect(domainAccountManager.data(), &DomainAccountManager::authRequired, dialogsManager.data(),
                &DialogsManager::showDomainLoginDialog);
        connect(domainAccountManager.data(), &DomainAccountManager::authRequired, this, &Application::updateWindowTitle);
        connect(domainAccountManager.data(), &DomainAccountManager::loginComplete, this, &Application::updateWindowTitle);
        // ####### TODO: Connect any other signals from domainAccountManager.

        auto addressManager = DependencyManager::get<AddressManager>();
        // use our MyAvatar position and quat for address manager path
        addressManager->setPositionGetter([] {
            auto avatarManager = DependencyManager::get<AvatarManager>();
            auto myAvatar = avatarManager ? avatarManager->getMyAvatar() : nullptr;
            return myAvatar ? myAvatar->getWorldFeetPosition() : Vectors::ZERO;
        });
        addressManager->setOrientationGetter([] {
            auto avatarManager = DependencyManager::get<AvatarManager>();
            auto myAvatar = avatarManager ? avatarManager->getMyAvatar() : nullptr;
            return myAvatar ? myAvatar->getWorldOrientation() : glm::quat();
        });

        connect(addressManager.data(), &AddressManager::hostChanged, this, &Application::updateWindowTitle);
        connect(this, &QCoreApplication::aboutToQuit, addressManager.data(), &AddressManager::storeCurrentAddress);

        {
            auto scriptEngines = DependencyManager::get<ScriptEngines>().data();
            scriptEngines->registerScriptInitializer([this](ScriptManagerPointer manager) {
                registerScriptEngineWithApplicationServices(manager);
            });

            connect(scriptEngines, &ScriptEngines::scriptCountChanged, this, [this] {
                auto scriptEngines = DependencyManager::get<ScriptEngines>();
                if (scriptEngines->getRunningScripts().isEmpty()) {
                    getMyAvatar()->clearScriptableSettings();
                }
            }, Qt::QueuedConnection);

            connect(scriptEngines, &ScriptEngines::scriptsReloading, this, [this] {
                getEntities()->reloadEntityScripts();
                loadAvatarScripts(getMyAvatar()->getScriptUrls());
            }, Qt::QueuedConnection);

            connect(scriptEngines, &ScriptEngines::scriptLoadError,
                this, [](const QString& filename, const QString& error) {
                OffscreenUi::asyncWarning(nullptr, "Error Loading Script", filename + " failed to load.");
            }, Qt::QueuedConnection);

            auto entityScriptServerLog = DependencyManager::get<EntityScriptServerLogClient>();
            connect(scriptEngines, &ScriptEngines::requestingEntityScriptServerLog,
                entityScriptServerLog.data(), &EntityScriptServerLogClient::requestMessagesForScriptEngines);
        }

        DependencyManager::get<PickManager>()->setShouldPickHUDOperator([]() { return DependencyManager::get<HMDScriptingInterface>()->isHMDMode(); });
        DependencyManager::get<PickManager>()->setCalculatePos2DFromHUDOperator([this](const glm::vec3& intersection) {
            const glm::vec2 MARGIN(25.0f);
            glm::vec2 maxPos = _controllerScriptingInterface->getViewportDimensions() - MARGIN;
            glm::vec2 pos2D = DependencyManager::get<HMDScriptingInterface>()->overlayFromWorldPoint(intersection);
            return glm::max(MARGIN, glm::min(pos2D, maxPos));
        });

        DependencyManager::get<UsersScriptingInterface>()->setKickConfirmationOperator([this] (const QUuid& nodeID, unsigned int banFlags) { userKickConfirmation(nodeID, banFlags); });

        FileDialogHelper::setOpenDirectoryOperator([this](const QString& path) { openDirectory(path); });
        QDesktopServices::setUrlHandler("file", this, "showUrlHandler");
        QDesktopServices::setUrlHandler("", this, "showUrlHandler");
        auto drives = QDir::drives();
        for (auto drive : drives) {
            QDesktopServices::setUrlHandler(QUrl(drive.absolutePath()).scheme(), this, "showUrlHandler");
        }

#if defined(Q_OS_ANDROID)
        connect(&AndroidHelper::instance(), &AndroidHelper::beforeEnterBackground, this, &Application::beforeEnterBackground);
        connect(&AndroidHelper::instance(), &AndroidHelper::enterBackground, this, &Application::enterBackground);
        connect(&AndroidHelper::instance(), &AndroidHelper::enterForeground, this, &Application::enterForeground);
        connect(&AndroidHelper::instance(), &AndroidHelper::toggleAwayMode, this, &Application::toggleAwayMode);
        AndroidHelper::instance().notifyLoadComplete();
#endif
    }

    // Make sure we don't time out during slow operations at startup
    updateHeartbeat();

    // Entities
    {
        connect(this, &Application::aboutToQuit, [this]() { setKeyboardFocusEntity(UNKNOWN_ENTITY_ID); });

        _entityServerConnectionTimer.setSingleShot(true);
        connect(&_entityServerConnectionTimer, &QTimer::timeout, this, &Application::setFailedToConnectToEntityServer);

        connect(_octreeProcessor.get(), &OctreePacketProcessor::packetVersionMismatch, this, &Application::notifyPacketVersionMismatch);

        // connect the _entityCollisionSystem to our EntityTreeRenderer since that's what handles running entity scripts
        connect(_entitySimulation.get(), &PhysicalEntitySimulation::entityCollisionWithEntity,
                getEntities().data(), &EntityTreeRenderer::entityCollisionWithEntity);

        auto entityScriptingInterface = DependencyManager::get<EntityScriptingInterface>();
        // connect the _entities (EntityTreeRenderer) to our script engine's EntityScriptingInterface for firing
        // of events related clicking, hovering over, and entering entities
        getEntities()->connectSignalsToSlots(entityScriptingInterface.data());

        EntityTreePointer tree = getEntities()->getTree();
        // Make sure any new sounds are loaded as soon as know about them.
        connect(tree.get(), &EntityTree::newCollisionSoundURL, this, [this](QUrl newURL, EntityItemID id) {
            getEntities()->setCollisionSound(id, DependencyManager::get<SoundCache>()->getSound(newURL));
        }, Qt::QueuedConnection);

        connect(entityScriptingInterface.data(), &EntityScriptingInterface::deletingEntity, this, [this](const EntityItemID& entityItemID) {
            if (entityItemID == _keyboardFocusedEntity.get()) {
                setKeyboardFocusEntity(UNKNOWN_ENTITY_ID);
            }
        }, Qt::QueuedConnection);

        EntityTree::setEntityClicksCapturedOperator([this] { return _controllerScriptingInterface->areEntityClicksCaptured(); });

        // If the user clicks on an object, we will check that it's a web surface, and if so, set the focus to it
        auto pointerManager = DependencyManager::get<PointerManager>().data();
        auto keyboardFocusOperator = [this](const QUuid& id, const PointerEvent& event) {
            if (event.shouldFocus()) {
                auto keyboard = DependencyManager::get<Keyboard>();
                if (getEntities()->wantsKeyboardFocus(id)) {
                    setKeyboardFocusEntity(id);
                } else if (!keyboard->containsID(id)) { // FIXME: this is a hack to make the keyboard work for now, since the keys would otherwise steal focus
                    setKeyboardFocusEntity(UNKNOWN_ENTITY_ID);
                }
            }
        };
        connect(pointerManager, &PointerManager::triggerBeginEntity, keyboardFocusOperator);
        connect(pointerManager, &PointerManager::triggerBeginOverlay, keyboardFocusOperator);

        getEntities()->setEntityLoadingPriorityFunction([this](const EntityItem& item) {
            if (item.getEntityHostType() == entity::HostType::AVATAR) {
                return item.isMyAvatarEntity() ? Avatar::MYAVATAR_ENTITY_LOADING_PRIORITY : Avatar::OTHERAVATAR_ENTITY_LOADING_PRIORITY;
            }

            const float maxSize = glm::compMax(item.getScaledDimensions());
            if (maxSize <= 0.0f) {
                return 0.0f;
            }

            const glm::vec3 itemPosition = item.getWorldPosition();
            const float distance = glm::distance(getMyAvatar()->getWorldPosition(), itemPosition);
            float result = atan2(maxSize, distance);
            bool isInView = true;
            {
                QMutexLocker viewLocker(&_viewMutex);
                isInView = _viewFrustum.sphereIntersectsKeyhole(itemPosition, maxSize);
            }
            if (!isInView) {
                const float OUT_OF_VIEW_PENALTY = -M_PI_2;
                result += OUT_OF_VIEW_PENALTY;
            }
            return result;
        });

        EntityTreeRenderer::setAddMaterialToEntityOperator([this](const QUuid& entityID, graphics::MaterialLayer material, const std::string& parentMaterialName) {
            if (_aboutToQuit) {
                return false;
            }

            auto renderable = getEntities()->renderableForEntityId(entityID);
            if (renderable) {
                renderable->addMaterial(material, parentMaterialName);
                return true;
            }

            return false;
        });
        EntityTreeRenderer::setRemoveMaterialFromEntityOperator([this](const QUuid& entityID, graphics::MaterialPointer material, const std::string& parentMaterialName) {
            if (_aboutToQuit) {
                return false;
            }

            auto renderable = getEntities()->renderableForEntityId(entityID);
            if (renderable) {
                renderable->removeMaterial(material, parentMaterialName);
                return true;
            }

            return false;
        });
        EntityTreeRenderer::setAddMaterialToAvatarOperator([](const QUuid& avatarID, graphics::MaterialLayer material, const std::string& parentMaterialName) {
            auto avatarManager = DependencyManager::get<AvatarManager>();
            auto avatar = static_pointer_cast<Avatar>(avatarManager->getAvatarBySessionID(avatarID));
            if (avatar) {
                avatar->addMaterial(material, parentMaterialName);
                return true;
            }
            return false;
        });
        EntityTreeRenderer::setRemoveMaterialFromAvatarOperator([](const QUuid& avatarID, graphics::MaterialPointer material, const std::string& parentMaterialName) {
            auto avatarManager = DependencyManager::get<AvatarManager>();
            auto avatar = static_pointer_cast<Avatar>(avatarManager->getAvatarBySessionID(avatarID));
            if (avatar) {
                avatar->removeMaterial(material, parentMaterialName);
                return true;
            }
            return false;
        });

        EntityTree::setGetUnscaledDimensionsForIDOperator([this](const QUuid& id) {
            if (_aboutToQuit) {
                return glm::vec3(1.0f);
            }

            auto entity = getEntities()->getEntity(id);
            if (entity) {
                return entity->getUnscaledDimensions();
            }

            auto avatarManager = DependencyManager::get<AvatarManager>();
            auto avatar = static_pointer_cast<Avatar>(avatarManager->getAvatarBySessionID(id));
            if (avatar) {
                return avatar->getSNScale();
            }
            return glm::vec3(1.0f);
        });

        ReferenceMaterial::setMaterialForUUIDOperator([this](const QUuid& entityID) -> graphics::MaterialPointer {
            if (_aboutToQuit) {
                return nullptr;
            }

            auto renderable = getEntities()->renderableForEntityId(entityID);
            if (renderable) {
                return renderable->getTopMaterial();
            }

            return nullptr;
        });

        Procedural::opaqueStencil = [](gpu::StatePointer state, bool useAA) {
            useAA ? PrepareStencil::testMaskDrawShape(*state) : PrepareStencil::testMaskDrawShapeNoAA(*state);
        };
        Procedural::transparentStencil = [](gpu::StatePointer state) { PrepareStencil::testMaskResetNoAA(*state); };

        EntityTree::setGetEntityObjectOperator([this](const QUuid& id) -> QObject* {
            auto entities = getEntities();
            if (auto entity = entities->renderableForEntityId(id)) {
                return qobject_cast<QObject*>(entity.get());
            }
            return nullptr;
        });

        EntityTree::setEmitScriptEventOperator([this](const QUuid& id, const QVariant& message) {
            auto entities = getEntities();
            if (auto entity = entities->renderableForEntityId(id)) {
                entity->emitScriptEvent(message);
            }
        });

        EntityTree::setTextSizeOperator([this](const QUuid& id, const QString& text) {
            auto entities = getEntities();
            if (auto entity = entities->renderableForEntityId(id)) {
                if (auto renderable = std::dynamic_pointer_cast<render::entities::TextEntityRenderer>(entity)) {
                    return renderable->textSize(text);
                }
            }
            return QSizeF(0.0f, 0.0f);
        });

        Texture::setUnboundTextureForUUIDOperator([this](const QUuid& entityID) -> gpu::TexturePointer {
            if (_aboutToQuit) {
                return nullptr;
            }

            auto renderable = getEntities()->renderableForEntityId(entityID);
            if (renderable) {
                return renderable->getTexture();
            }

            return nullptr;
        });

        EntityTreeRenderer::setEntitiesShouldFadeFunction([this]() {
            SharedNodePointer entityServerNode = DependencyManager::get<NodeList>()->soloNodeOfType(NodeType::EntityServer);
            return entityServerNode && !isPhysicsEnabled();
        });

        DependencyManager::get<EntityTreeRenderer>()->setMouseRayPickResultOperator([](unsigned int rayPickID) {
            RayToEntityIntersectionResult entityResult;
            entityResult.intersects = false;
            auto pickResult = DependencyManager::get<PickManager>()->getPrevPickResultTyped<RayPickResult>(rayPickID);
            if (pickResult) {
                entityResult.intersects = pickResult->type != IntersectionType::NONE;
                if (entityResult.intersects) {
                    entityResult.intersection = pickResult->intersection;
                    entityResult.distance = pickResult->distance;
                    entityResult.surfaceNormal = pickResult->surfaceNormal;
                    entityResult.entityID = pickResult->objectID;
                    entityResult.extraInfo = pickResult->extraInfo;
                }
            }
            return entityResult;
        });
        DependencyManager::get<EntityTreeRenderer>()->setSetPrecisionPickingOperator([](unsigned int rayPickID, bool value) {
            DependencyManager::get<PickManager>()->setPrecisionPicking(rayPickID, value);
        });

        BillboardModeHelpers::setBillboardRotationOperator([](const glm::vec3& position, const glm::quat& rotation,
                                                              BillboardMode billboardMode, const glm::vec3& frustumPos, bool rotate90x) {
            const glm::quat ROTATE_90X = glm::angleAxis(-(float)M_PI_2, Vectors::RIGHT);
            if (billboardMode == BillboardMode::YAW) {
                //rotate about vertical to face the camera
                glm::vec3 dPosition = frustumPos - position;
                // If x and z are 0, atan(x, z) is undefined, so default to 0 degrees
                float yawRotation = dPosition.x == 0.0f && dPosition.z == 0.0f ? 0.0f : glm::atan(dPosition.x, dPosition.z);
                glm::quat result = glm::quat(glm::vec3(0.0f, yawRotation, 0.0f)) * rotation;
                if (rotate90x) {
                    result *= ROTATE_90X;
                }
                return result;
            } else if (billboardMode == BillboardMode::FULL) {
                // use the referencial from the avatar, y isn't always up
                glm::vec3 avatarUP = DependencyManager::get<AvatarManager>()->getMyAvatar()->getWorldOrientation() * Vectors::UP;
                // check to see if glm::lookAt will work / using glm::lookAt variable name
                glm::highp_vec3 s(glm::cross(position - frustumPos, avatarUP));

                // make sure s is not NaN for any component
                if (glm::length2(s) > 0.0f) {
                    glm::quat result = glm::conjugate(glm::toQuat(glm::lookAt(frustumPos, position, avatarUP))) * rotation;
                    if (rotate90x) {
                        result *= ROTATE_90X;
                    }
                    return result;
                }
            }
            return rotation;
        });
        BillboardModeHelpers::setPrimaryViewFrustumPositionOperator([this]() {
            ViewFrustum viewFrustum;
            copyViewFrustum(viewFrustum);
            return viewFrustum.getPosition();
        });

        render::entities::WebEntityRenderer::setAcquireWebSurfaceOperator([=](const QString& url, bool htmlContent, QSharedPointer<OffscreenQmlSurface>& webSurface, bool& cachedWebSurface) {
            bool isTablet = url == TabletScriptingInterface::QML;
            if (htmlContent) {
                webSurface = DependencyManager::get<OffscreenQmlSurfaceCache>()->acquire(render::entities::WebEntityRenderer::QML);
                cachedWebSurface = true;
                auto rootItemLoadedFunctor = [url, webSurface] {
                    webSurface->getRootItem()->setProperty(render::entities::WebEntityRenderer::URL_PROPERTY, url);
                };
                if (webSurface->getRootItem()) {
                    rootItemLoadedFunctor();
                } else {
                    QObject::connect(webSurface.data(), &hifi::qml::OffscreenSurface::rootContextCreated, rootItemLoadedFunctor);
                }
                auto surfaceContext = webSurface->getSurfaceContext();
                surfaceContext->setContextProperty("KeyboardScriptingInterface", DependencyManager::get<KeyboardScriptingInterface>().data());
            } else {
                // FIXME: the tablet should use the OffscreenQmlSurfaceCache
                webSurface = QSharedPointer<OffscreenQmlSurface>(new OffscreenQmlSurface(), [](OffscreenQmlSurface* webSurface) {
                    AbstractViewStateInterface::instance()->sendLambdaEvent([webSurface] {
                        // WebEngineView may run other threads (wasapi), so they must be deleted for a clean shutdown
                        // if the application has already stopped its event loop, delete must be explicit
                        delete webSurface;
                    });
                });
                auto rootItemLoadedFunctor = [webSurface, url, isTablet] {
                    Application::setupQmlSurface(webSurface->getSurfaceContext(), isTablet || url == LOGIN_DIALOG.toString() || url == AVATAR_INPUTS_BAR_QML.toString() ||
                       url == BUBBLE_ICON_QML.toString());
                };
                if (webSurface->getRootItem()) {
                    rootItemLoadedFunctor();
                } else {
                    QObject::connect(webSurface.data(), &hifi::qml::OffscreenSurface::rootContextCreated, rootItemLoadedFunctor);
                }
                webSurface->load(url);
                cachedWebSurface = false;
            }
            const uint8_t DEFAULT_MAX_FPS = 10;
            const uint8_t TABLET_FPS = 90;
            webSurface->setMaxFps(isTablet ? TABLET_FPS : DEFAULT_MAX_FPS);
        });
        render::entities::WebEntityRenderer::setReleaseWebSurfaceOperator([=](QSharedPointer<OffscreenQmlSurface>& webSurface, bool& cachedWebSurface, std::vector<QMetaObject::Connection>& connections) {
            QQuickItem* rootItem = webSurface->getRootItem();

            // Fix for crash in QtWebEngineCore when rapidly switching domains
            // Call stop on the QWebEngineView before destroying OffscreenQMLSurface.
            if (rootItem && !cachedWebSurface) {
                // stop loading
                QMetaObject::invokeMethod(rootItem, "stop");
            }

            webSurface->pause();

            for (auto& connection : connections) {
                QObject::disconnect(connection);
            }
            connections.clear();

            // If the web surface was fetched out of the cache, release it back into the cache
            if (cachedWebSurface) {
                // If it's going back into the cache make sure to explicitly set the URL to a blank page
                // in order to stop any resource consumption or audio related to the page.
                if (rootItem) {
                    rootItem->setProperty("url", "about:blank");
                }
                auto offscreenCache = DependencyManager::get<OffscreenQmlSurfaceCache>();
                if (offscreenCache) {
                    offscreenCache->release(render::entities::WebEntityRenderer::QML, webSurface);
                }
                cachedWebSurface = false;
            }
            webSurface.reset();
        });
    }

    // Make sure we don't time out during slow operations at startup
    updateHeartbeat();

    // Plugins
    {
        connect(PluginManager::getInstance().data(), &PluginManager::inputDeviceRunningChanged,
            _controllerScriptingInterface, &controller::ScriptingInterface::updateRunningInputDevices);

        connect(this, &Application::activeDisplayPluginChanged, this, &Application::updateThreadPoolCount);
        if (_useSystemCursor) {
            connect(this, &Application::activeDisplayPluginChanged, this, [=](){
                qApp->setProperty(hifi::properties::HMD, qApp->isHMDMode());
                auto displayPlugin = qApp->getActiveDisplayPlugin();

                if (displayPlugin->isHmd()) {
                    if (_preferredCursor.get() == Cursor::Manager::getIconName(Cursor::Icon::RETICLE)) {
                        setPreferredCursor(Cursor::Manager::getIconName(Cursor::Icon::RETICLE));
                    } else {
                        setPreferredCursor(Cursor::Manager::getIconName(Cursor::Icon::ARROW));
                    }
                } else {
                    setPreferredCursor(Cursor::Manager::getIconName(Cursor::Icon::SYSTEM));
                }

                auto &ch = CrashHandler::getInstance();
                ch.setAnnotation("display_plugin", displayPlugin->getName().toStdString());
                ch.setAnnotation("hmd", displayPlugin->isHmd() ? "1" : "0");
            });
        }
        connect(this, &Application::activeDisplayPluginChanged, this, &Application::updateSystemTabletMode);
        connect(this, &Application::activeDisplayPluginChanged, this, [&](){
            if (getLoginDialogPoppedUp()) {
                auto dialogsManager = DependencyManager::get<DialogsManager>();
                auto keyboard = DependencyManager::get<Keyboard>();
                if (_firstRun.get()) {
                    // display mode changed.  Don't allow auto-switch to work after this session.
                    _firstRun.set(false);
                }
                if (isHMDMode()) {
                    emit loginDialogFocusDisabled();
                    dialogsManager->hideLoginDialog();
                    createLoginDialog();
                } else {
                    DependencyManager::get<EntityScriptingInterface>()->deleteEntity(_loginDialogID);
                    _loginDialogID = QUuid();
                    _loginStateManager.tearDown();
                    dialogsManager->showLoginDialog();
                    emit loginDialogFocusEnabled();
                }
            }
        });

        // Setup the userInputMapper with the actions
        auto userInputMapper = DependencyManager::get<UserInputMapper>();
        connect(userInputMapper.data(), &UserInputMapper::actionEvent, [this](int action, float state) {
            using namespace controller;
            auto tabletScriptingInterface = DependencyManager::get<TabletScriptingInterface>();
            QSharedPointer<scripting::Audio> audioScriptingInterface = qSharedPointerDynamicCast<scripting::Audio>(DependencyManager::get<AudioScriptingInterface>());
            {
                auto actionEnum = static_cast<Action>(action);
                int key = Qt::Key_unknown;
                static int lastKey = Qt::Key_unknown;
                bool navAxis = false;
                switch (actionEnum) {
                    case Action::TOGGLE_PUSHTOTALK:
                        if (audioScriptingInterface) {
                            if (state > 0.0f) {
                                audioScriptingInterface->setPushingToTalk(true);
                            } else if (state <= 0.0f) {
                                audioScriptingInterface->setPushingToTalk(false);
                            }
                        }
                        break;

                    case Action::UI_NAV_VERTICAL:
                        navAxis = true;
                        if (state > 0.0f) {
                            key = Qt::Key_Up;
                        } else if (state < 0.0f) {
                            key = Qt::Key_Down;
                        }
                        break;

                    case Action::UI_NAV_LATERAL:
                        navAxis = true;
                        if (state > 0.0f) {
                            key = Qt::Key_Right;
                        } else if (state < 0.0f) {
                            key = Qt::Key_Left;
                        }
                        break;

                    case Action::UI_NAV_GROUP:
                        navAxis = true;
                        if (state > 0.0f) {
                            key = Qt::Key_Tab;
                        } else if (state < 0.0f) {
                            key = Qt::Key_Backtab;
                        }
                        break;

                    case Action::UI_NAV_BACK:
                        key = Qt::Key_Escape;
                        break;

                    case Action::UI_NAV_SELECT:
                        key = Qt::Key_Return;
                        break;
                    default:
                        break;
                }

                auto window = tabletScriptingInterface->getTabletWindow();
                if (navAxis && window) {
                    if (lastKey != Qt::Key_unknown) {
                        QKeyEvent event(QEvent::KeyRelease, lastKey, Qt::NoModifier);
                        sendEvent(window, &event);
                        lastKey = Qt::Key_unknown;
                    }

                    if (key != Qt::Key_unknown) {
                        QKeyEvent event(QEvent::KeyPress, key, Qt::NoModifier);
                        sendEvent(window, &event);
                        tabletScriptingInterface->processEvent(&event);
                        lastKey = key;
                    }
                } else if (key != Qt::Key_unknown && window) {
                    if (state) {
                        QKeyEvent event(QEvent::KeyPress, key, Qt::NoModifier);
                        sendEvent(window, &event);
                        tabletScriptingInterface->processEvent(&event);
                    } else {
                        QKeyEvent event(QEvent::KeyRelease, key, Qt::NoModifier);
                        sendEvent(window, &event);
                    }
                    return;
                }
            }

            if (action == controller::toInt(controller::Action::RETICLE_CLICK)) {
                auto reticlePos = getApplicationCompositor().getReticlePosition();
                QPoint localPos(reticlePos.x, reticlePos.y); // both hmd and desktop already handle this in our coordinates.
                if (state) {
                    QMouseEvent mousePress(QEvent::MouseButtonPress, localPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                    sendEvent(_glWidget, &mousePress);
                    _reticleClickPressed = true;
                } else {
                    QMouseEvent mouseRelease(QEvent::MouseButtonRelease, localPos, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
                    sendEvent(_glWidget, &mouseRelease);
                    _reticleClickPressed = false;
                }
                return; // nothing else to do
            }

            if (state) {
                if (action == controller::toInt(controller::Action::TOGGLE_MUTE)) {
                    auto audioClient = DependencyManager::get<AudioClient>();
                    audioClient->setMuted(!audioClient->isMuted());
                } else if (action == controller::toInt(controller::Action::CYCLE_CAMERA)) {
                    cycleCamera();
                } else if (action == controller::toInt(controller::Action::CONTEXT_MENU) && !isInterstitialMode()) {
                    toggleTabletUI();
                } else if (action == controller::toInt(controller::Action::RETICLE_X)) {
                    auto oldPos = getApplicationCompositor().getReticlePosition();
                    getApplicationCompositor().setReticlePosition({ oldPos.x + state, oldPos.y });
                } else if (action == controller::toInt(controller::Action::RETICLE_Y)) {
                    auto oldPos = getApplicationCompositor().getReticlePosition();
                    getApplicationCompositor().setReticlePosition({ oldPos.x, oldPos.y + state });
                } else if (action == controller::toInt(controller::Action::TOGGLE_OVERLAY)) {
                    toggleOverlays();
                }
            }
        });
    }

    // Make sure we don't time out during slow operations at startup
    updateHeartbeat();

    // UI
    {
        auto offscreenUi = getOffscreenUI().data();
        connect(offscreenUi, &OffscreenUi::desktopReady, [this]() {
            // Now that we've loaded the menu and thus switched to the previous display plugin
            // we can unlock the desktop repositioning code, since all the positions will be
            // relative to the desktop size for this plugin
            auto offscreenUi = getOffscreenUI();
            auto desktop = offscreenUi->getDesktop();
            if (desktop) {
                desktop->setProperty("repositionLocked", false);
            }
        });

#if defined(Q_OS_ANDROID) || defined(DISABLE_QML)
        connect(offscreenUi, &OffscreenUi::keyboardFocusActive, [this]() {
            resumeAfterLoginDialogActionTaken();
        });
#else
        connect(offscreenUi, &OffscreenUi::keyboardFocusActive, [this]() {
            // Do not show login dialog if requested not to on the command line
            if (!_noLoginSuggestion) {
                showLoginScreen();
            }
            resumeAfterLoginDialogActionTaken();
        });
#endif

        // Monitor model assets (e.g., from Clara.io) added to the world that may need resizing.
        static const int ADD_ASSET_TO_WORLD_TIMER_INTERVAL_MS = 1000;
        _addAssetToWorldResizeTimer.setInterval(ADD_ASSET_TO_WORLD_TIMER_INTERVAL_MS); // 1s, Qt::CoarseTimer acceptable
        connect(&_addAssetToWorldResizeTimer, &QTimer::timeout, this, &Application::addAssetToWorldCheckModelSize);

        // Auto-update and close adding asset to world info message box.
        static const int ADD_ASSET_TO_WORLD_INFO_TIMEOUT_MS = 5000;
        _addAssetToWorldInfoTimer.setInterval(ADD_ASSET_TO_WORLD_INFO_TIMEOUT_MS); // 5s, Qt::CoarseTimer acceptable
        _addAssetToWorldInfoTimer.setSingleShot(true);
        connect(&_addAssetToWorldInfoTimer, &QTimer::timeout, this, &Application::addAssetToWorldInfoTimeout);
        static const int ADD_ASSET_TO_WORLD_ERROR_TIMEOUT_MS = 8000;
        _addAssetToWorldErrorTimer.setInterval(ADD_ASSET_TO_WORLD_ERROR_TIMEOUT_MS); // 8s, Qt::CoarseTimer acceptable
        _addAssetToWorldErrorTimer.setSingleShot(true);
        connect(&_addAssetToWorldErrorTimer, &QTimer::timeout, this, &Application::addAssetToWorldErrorTimeout);

        connect(this, &QCoreApplication::aboutToQuit, this, &Application::addAssetToWorldMessageClose);
        connect(&domainHandler, &DomainHandler::domainURLChanged, this, &Application::addAssetToWorldMessageClose);
        connect(&domainHandler, &DomainHandler::redirectToErrorDomainURL, this, &Application::addAssetToWorldMessageClose);
    }

    // Make sure we don't time out during slow operations at startup
    updateHeartbeat();

    // Avatar
    {
        // Save avatar location immediately after a teleport.
        connect(myAvatar.get(), &MyAvatar::positionGoneTo,
            DependencyManager::get<AddressManager>().data(), &AddressManager::storeCurrentAddress);

        connect(myAvatar.get(), &MyAvatar::positionGoneTo, this, [this] {
            if (!_physicsEnabled) {
                // when we arrive somewhere without physics enabled --> startSafeLanding
                _octreeProcessor->startSafeLanding();
            }
        }, Qt::QueuedConnection);

        connect(myAvatar.get(), &MyAvatar::skeletonModelURLChanged, [](){
            QUrl avatarURL = qApp->getMyAvatar()->getSkeletonModelURL();
            auto &ch = CrashHandler::getInstance();
            ch.setAnnotation("avatar", avatarURL.toString().toStdString());
        });

        // FIXME -- I'm a little concerned about this.
        connect(myAvatar->getSkeletonModel().get(), &SkeletonModel::skeletonLoaded,
            this, &Application::checkSkeleton, Qt::QueuedConnection);

        connect(myAvatar.get(), &MyAvatar::newCollisionSoundURL, this, [this](QUrl newURL) {
            if (auto avatar = getMyAvatar()) {
                auto sound = DependencyManager::get<SoundCache>()->getSound(newURL);
                avatar->setCollisionSound(sound);
            }
        }, Qt::QueuedConnection);
    }

    // Make sure we don't time out during slow operations at startup
    updateHeartbeat();

    // Camera
    {
        connect(&_myCamera, &Camera::modeUpdated, this, &Application::cameraModeChanged);
        connect(&_myCamera, &Camera::captureMouseChanged, this, &Application::captureMouseChanged);
    }

    // Make sure we don't time out during slow operations at startup
    updateHeartbeat();

    // Audio
    {
        auto audioIO = DependencyManager::get<AudioClient>().data();
        audioIO->setPositionGetter([] {
            auto avatarManager = DependencyManager::get<AvatarManager>();
            auto myAvatar = avatarManager ? avatarManager->getMyAvatar() : nullptr;

            return myAvatar ? myAvatar->getPositionForAudio() : Vectors::ZERO;
        });
        audioIO->setOrientationGetter([] {
            auto avatarManager = DependencyManager::get<AvatarManager>();
            auto myAvatar = avatarManager ? avatarManager->getMyAvatar() : nullptr;

            return myAvatar ? myAvatar->getOrientationForAudio() : Quaternions::IDENTITY;
        });

        recording::Frame::registerFrameHandler(AudioConstants::getAudioFrameName(), [&audioIO](recording::Frame::ConstPointer frame) {
            audioIO->handleRecordedAudioInput(frame->data);
        });

        connect(audioIO, &AudioClient::inputReceived, [](const QByteArray& audio) {
            static auto recorder = DependencyManager::get<recording::Recorder>();
            if (recorder->isRecording()) {
                static const recording::FrameType AUDIO_FRAME_TYPE = recording::Frame::registerFrameType(AudioConstants::getAudioFrameName());
                recorder->recordFrame(AUDIO_FRAME_TYPE, audio);
            }
        });
        audioIO->startThread();

        auto audioScriptingInterface = DependencyManager::get<AudioScriptingInterface>().data();
        connect(audioIO, &AudioClient::mutedByMixer, audioScriptingInterface, &AudioScriptingInterface::mutedByMixer);
        connect(audioIO, &AudioClient::receivedFirstPacket, audioScriptingInterface, &AudioScriptingInterface::receivedFirstPacket);
        connect(audioIO, &AudioClient::disconnected, audioScriptingInterface, &AudioScriptingInterface::disconnected);
        connect(audioIO, &AudioClient::noiseGateOpened, audioScriptingInterface, &AudioScriptingInterface::noiseGateOpened);
        connect(audioIO, &AudioClient::noiseGateClosed, audioScriptingInterface, &AudioScriptingInterface::noiseGateClosed);
        connect(audioIO, &AudioClient::inputReceived, audioScriptingInterface, &AudioScriptingInterface::inputReceived);
        connect(audioIO, &AudioClient::muteEnvironmentRequested, [](glm::vec3 position, float radius) {
            auto audioClient = DependencyManager::get<AudioClient>();
            auto audioScriptingInterface = DependencyManager::get<AudioScriptingInterface>();
            auto myAvatarPosition = DependencyManager::get<AvatarManager>()->getMyAvatar()->getWorldPosition();
            float distance = glm::distance(myAvatarPosition, position);

            if (distance < radius) {
                audioClient->setMuted(true);
                audioScriptingInterface->environmentMuted();
            }
        });
        QSharedPointer<scripting::Audio> scriptingAudioSharedPointer = qSharedPointerDynamicCast<scripting::Audio>(DependencyManager::get<AudioScriptingInterface>());
        if (scriptingAudioSharedPointer) {
            connect(this, &Application::activeDisplayPluginChanged,
                scriptingAudioSharedPointer.data(), &scripting::Audio::onContextChanged);
        }
        dynamic_cast<scripting::Audio*>(audioScriptingInterface)->onContextChanged();
    }
}
