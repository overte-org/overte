//
//  Application_UI.cpp
//  interface/src
//
//  Split from Application.cpp by HifiExperiments on 3/30/24
//  Created by Andrzej Kapolka on 5/10/13.
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors.
//  Copyright 2022-2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "Application.h"

#include <QtQml/QQmlContext>
#include <QStyle>
#include <QStyleFactory>
#if defined(Q_OS_WIN)
#include <windows.h>
#endif

#include <AddressManager.h>
#include <AnimationCacheScriptingInterface.h>
#include <audio/AudioScope.h>
#include <AudioScriptingInterface.h>
#include <AvatarBookmarks.h>
#include <display-plugins/CompositorHelper.h>
#include <FileDialogHelper.h>
#include <FSTReader.h>
#include <MainWindow.h>
#include <material-networking/TextureCacheScriptingInterface.h>
#include <Menu.h>
#include <MessagesClient.h>
#include <model-networking/ModelCacheScriptingInterface.h>
#include <OffscreenUi.h>
#include <plugins/InputConfiguration.h>
#include <plugins/PluginManager.h>
#include <plugins/PluginUtils.h>
#include <plugins/SteamClientPlugin.h>
#include <Preferences.h>
#include <procedural/MaterialCacheScriptingInterface.h>
#include <raypick/PointerScriptingInterface.h>
#include <recording/RecordingScriptingInterface.h>
#include <SandboxUtils.h>
#include <SceneScriptingInterface.h>
#include <ScriptEngines.h>
#include <scripting/AccountServicesScriptingInterface.h>
#include <scripting/AssetMappingsScriptingInterface.h>
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
#include <scripting/WindowScriptingInterface.h>
#ifndef Q_OS_ANDROID
#include <shared/FileLogger.h>
#endif
#include <shared/GlobalAppProperties.h>
#include <shared/StringHelpers.h>
#include <SoundCacheScriptingInterface.h>
#include <ui/AnimStats.h>
#include <ui/AvatarInputs.h>
#include <ui/DialogsManager.h>
#include <ui/DomainConnectionModel.h>
#include <ui/EntityScriptServerLogDialog.h>
#include <ui/Keyboard.h>
#include <ui/LogDialog.h>
#include <ui/LoginDialog.h>
#include <ui/OctreeStatsDialog.h>
#include <ui/OctreeStatsProvider.h>
#include <ui/Snapshot.h>
#include <ui/Stats.h>
#include <ui/TabletScriptingInterface.h>
#include <ui/ToolbarScriptingInterface.h>
#include <UserActivityLogger.h>
#include <UserActivityLoggerScriptingInterface.h>
#include <UsersScriptingInterface.h>

#include "AboutUtil.h"
#include "ArchiveDownloadInterface.h"
#include "AudioClient.h"
#include "GLCanvas.h"
#include "LocationBookmarks.h"
#include "LODManager.h"
#include "ResourceRequestObserver.h"
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
#include "SpeechRecognizer.h"
#endif

static const QString SYSTEM_TABLET = "com.highfidelity.interface.tablet.system";

static const QString STANDARD_TO_ACTION_MAPPING_NAME = "Standard to Action";
static const QString NO_MOVEMENT_MAPPING_NAME = "Standard to Action (No Movement)";
static const QString NO_MOVEMENT_MAPPING_JSON = PathUtils::resourcesPath() + "/controllers/standard_nomovement.json";

const QString DEFAULT_CURSOR_NAME = "SYSTEM";

static const QUrl AVATAR_INPUTS_BAR_QML = PathUtils::qmlUrl("AvatarInputsBar.qml");
static const QString INFO_HELP_PATH = "html/tabletHelp.html";


ui::Menu* Application::getPrimaryMenu() {
    auto appMenu = _window->menuBar();
    auto uiMenu = dynamic_cast<ui::Menu*>(appMenu);
    return uiMenu;
}

void Application::showDisplayPluginsTools(bool show) {
    DependencyManager::get<DialogsManager>()->hmdTools(show);
}

GLWidget* Application::getPrimaryWidget() {
    return _glWidget;
}

MainWindow* Application::getPrimaryWindow() {
    return getWindow();
}

QOpenGLContext* Application::getPrimaryContext() {
    return _glWidget->qglContext();
}

bool Application::isForeground() const {
    return _isForeground && !_window->isMinimized();
}

bool Application::hasFocus() const {
    bool result = (QApplication::activeWindow() != nullptr);


#if defined(Q_OS_WIN)
    // On Windows, QWidget::activateWindow() - as called in setFocus() - makes the application's taskbar icon flash but doesn't
    // take user focus away from their current window. So also check whether the application is the user's current foreground
    // window.
    result = result && (HWND)QApplication::activeWindow()->winId() == GetForegroundWindow();
#endif
    return result;
}

void Application::setFocus() {
    // Note: Windows doesn't allow a user focus to be taken away from another application. Instead, it changes the color of and
    // flashes the taskbar icon.
    auto window = qApp->getWindow();
    window->activateWindow();
}

void Application::raise() {
    auto windowState = qApp->getWindow()->windowState();
    if (windowState & Qt::WindowMinimized) {
        if (windowState & Qt::WindowMaximized) {
            qApp->getWindow()->showMaximized();
        } else if (windowState & Qt::WindowFullScreen) {
            qApp->getWindow()->showFullScreen();
        } else {
            qApp->getWindow()->showNormal();
        }
    }
    qApp->getWindow()->raise();
}

void Application::showCursor(const Cursor::Icon& cursor) {
    QMutexLocker locker(&_changeCursorLock);

    auto managedCursor = Cursor::Manager::instance().getCursor();
    auto curIcon = managedCursor->getIcon();
    if (curIcon != cursor) {
        managedCursor->setIcon(cursor);
        curIcon = cursor;
    }
    _desiredCursor = cursor == Cursor::Icon::SYSTEM ? Qt::ArrowCursor : Qt::BlankCursor;
    _cursorNeedsChanging = true;
}

CompositorHelper& Application::getApplicationCompositor() const {
    return *DependencyManager::get<CompositorHelper>();
}

PickRay Application::computePickRay(float x, float y) const {
    vec2 pickPoint { x, y };
    PickRay result;
    if (isHMDMode()) {
        getApplicationCompositor().computeHmdPickRay(pickPoint, result.origin, result.direction);
    } else {
        pickPoint /= getCanvasSize();
        if (_myCamera.getMode() == CameraMode::CAMERA_MODE_MIRROR) {
            pickPoint.x = 1.0f - pickPoint.x;
        }
        QMutexLocker viewLocker(&_viewMutex);
        _viewFrustum.computePickRay(pickPoint.x, pickPoint.y, result.origin, result.direction);
    }
    return result;
}

void Application::setupQmlSurface(QQmlContext* surfaceContext, bool setAdditionalContextProperties) {
    surfaceContext->setContextProperty("Users", DependencyManager::get<UsersScriptingInterface>().data());
    surfaceContext->setContextProperty("HMD", DependencyManager::get<HMDScriptingInterface>().data());
    surfaceContext->setContextProperty("UserActivityLogger", DependencyManager::get<UserActivityLoggerScriptingInterface>().data());
    surfaceContext->setContextProperty("Preferences", DependencyManager::get<Preferences>().data());
    surfaceContext->setContextProperty("Vec3", new Vec3());
    surfaceContext->setContextProperty("Quat", new Quat());
    surfaceContext->setContextProperty("MyAvatar", DependencyManager::get<AvatarManager>()->getMyAvatar().get());
    surfaceContext->setContextProperty("Entities", DependencyManager::get<EntityScriptingInterface>().data());
    surfaceContext->setContextProperty("Snapshot", DependencyManager::get<Snapshot>().data());
    surfaceContext->setContextProperty("KeyboardScriptingInterface", DependencyManager::get<KeyboardScriptingInterface>().data());

    if (setAdditionalContextProperties) {
        qDebug() << "setting additional context properties!";
        auto tabletScriptingInterface = DependencyManager::get<TabletScriptingInterface>();
        auto flags = tabletScriptingInterface->getFlags();

        surfaceContext->setContextProperty("offscreenFlags", flags);
        surfaceContext->setContextProperty("AddressManager", DependencyManager::get<AddressManager>().data());

        surfaceContext->setContextProperty("Settings", new QMLSettingsScriptingInterface(surfaceContext));
        surfaceContext->setContextProperty("MenuInterface", MenuScriptingInterface::getInstance());
        surfaceContext->setContextProperty("Performance", new PerformanceScriptingInterface());

        surfaceContext->setContextProperty("Account", AccountServicesScriptingInterface::getInstance()); // DEPRECATED - TO BE REMOVED
        surfaceContext->setContextProperty("GlobalServices", AccountServicesScriptingInterface::getInstance()); // DEPRECATED - TO BE REMOVED
        surfaceContext->setContextProperty("AccountServices", AccountServicesScriptingInterface::getInstance());

        // in Qt 5.10.0 there is already an "Audio" object in the QML context
        // though I failed to find it (from QtMultimedia??). So..  let it be "AudioScriptingInterface"
        surfaceContext->setContextProperty("AudioScriptingInterface", DependencyManager::get<AudioScriptingInterface>().data());

        surfaceContext->setContextProperty("AudioStats", DependencyManager::get<AudioClient>()->getStats().data());
        surfaceContext->setContextProperty("fileDialogHelper", new FileDialogHelper());
        surfaceContext->setContextProperty("ScriptDiscoveryService", DependencyManager::get<ScriptEngines>().data());
        surfaceContext->setContextProperty("Assets", DependencyManager::get<AssetMappingsScriptingInterface>().data());
        surfaceContext->setContextProperty("LODManager", DependencyManager::get<LODManager>().data());
        surfaceContext->setContextProperty("OctreeStats", DependencyManager::get<OctreeStatsProvider>().data());
        surfaceContext->setContextProperty("DCModel", DependencyManager::get<DomainConnectionModel>().data());
        surfaceContext->setContextProperty("AvatarInputs", AvatarInputs::getInstance());
        surfaceContext->setContextProperty("AvatarList", DependencyManager::get<AvatarManager>().data());
        surfaceContext->setContextProperty("DialogsManager", DialogsManagerScriptingInterface::getInstance());
        surfaceContext->setContextProperty("InputConfiguration", DependencyManager::get<InputConfiguration>().data());
        surfaceContext->setContextProperty("SoundCache", DependencyManager::get<SoundCacheScriptingInterface>().data());
        surfaceContext->setContextProperty("AvatarBookmarks", DependencyManager::get<AvatarBookmarks>().data());
        surfaceContext->setContextProperty("Render", RenderScriptingInterface::getInstance());
        surfaceContext->setContextProperty("Workload", qApp->getGameWorkload()._engine->getConfiguration().get());
        surfaceContext->setContextProperty("Controller", DependencyManager::get<controller::ScriptingInterface>().data());
        surfaceContext->setContextProperty("Pointers", DependencyManager::get<PointerScriptingInterface>().data());
        surfaceContext->setContextProperty("Window", DependencyManager::get<WindowScriptingInterface>().data());
        surfaceContext->setContextProperty("Reticle", qApp->getApplicationCompositor().getReticleInterface());
        surfaceContext->setContextProperty("About", AboutUtil::getInstance());
        surfaceContext->setContextProperty("HiFiAbout", AboutUtil::getInstance());  // Deprecated.
        surfaceContext->setContextProperty("ResourceRequestObserver", DependencyManager::get<ResourceRequestObserver>().data());
        surfaceContext->setContextProperty("PlatformInfo", PlatformInfoScriptingInterface::getInstance());
        surfaceContext->setContextProperty("ExternalResource", ExternalResource::getInstance());

        // This `module` context property is blank for the QML scripting interface so that we don't get log errors when importing
        // certain JS files from both scripts (in the JS context) and QML (in the QML context).
        surfaceContext->setContextProperty("module", "");
    }
}

void Application::setDesktopTabletBecomesToolbarSetting(bool value) {
    _desktopTabletBecomesToolbarSetting.set(value);
    updateSystemTabletMode();
}

void Application::setHmdTabletBecomesToolbarSetting(bool value) {
    _hmdTabletBecomesToolbarSetting.set(value);
    updateSystemTabletMode();
}

void Application::setMouseCaptureVR(bool value) {
    _defaultMouseCaptureVR.set(value);
    getApplicationCompositor().setEnableMouseCaptureVR(value);
}
bool Application::getMouseCaptureVR() {
    return _defaultMouseCaptureVR.get();
}

void Application::setMiniTabletEnabled(bool enabled) {
    _miniTabletEnabledSetting.set(enabled);
    emit miniTabletEnabledChanged(enabled);
}

void Application::setSettingConstrainToolbarPosition(bool setting) {
    _constrainToolbarPosition.set(setting);
    getOffscreenUI()->setConstrainToolbarToCenterX(setting);
}

void Application::setAwayStateWhenFocusLostInVREnabled(bool enabled) {
    _awayStateWhenFocusLostInVREnabled.set(enabled);
    emit awayStateWhenFocusLostInVRChanged(enabled);
}

QUuid Application::getTabletScreenID() const {
    auto HMD = DependencyManager::get<HMDScriptingInterface>();
    return HMD->getCurrentTabletScreenID();
}

QUuid Application::getTabletHomeButtonID() const {
    auto HMD = DependencyManager::get<HMDScriptingInterface>();
    return HMD->getCurrentHomeButtonID();
}

QUuid Application::getTabletFrameID() const {
    auto HMD = DependencyManager::get<HMDScriptingInterface>();
    return HMD->getCurrentTabletFrameID();
}

QVector<QUuid> Application::getTabletIDs() const {
    // Most important first.
    QVector<QUuid> result;
    auto HMD = DependencyManager::get<HMDScriptingInterface>();
    result << HMD->getCurrentTabletScreenID();
    result << HMD->getCurrentHomeButtonID();
    result << HMD->getCurrentTabletFrameID();
    return result;
}

void Application::confirmConnectWithoutAvatarEntities() {

    if (_confirmConnectWithoutAvatarEntitiesDialog) {
        // Dialog is already displayed.
        return;
    }

    if (!getMyAvatar()->hasAvatarEntities()) {
        // No avatar entities so continue with login.
        DependencyManager::get<NodeList>()->getDomainHandler().setCanConnectWithoutAvatarEntities(true);
        return;
    }

    QString continueMessage = "Your wearables will not display on this domain. Continue?";
    _confirmConnectWithoutAvatarEntitiesDialog = OffscreenUi::asyncQuestion("Continue Without Wearables", continueMessage,
        QMessageBox::Yes | QMessageBox::No);
    if (_confirmConnectWithoutAvatarEntitiesDialog->getDialogItem()) {
        QObject::connect(_confirmConnectWithoutAvatarEntitiesDialog, &ModalDialogListener::response, this, [=](QVariant answer) {
            QObject::disconnect(_confirmConnectWithoutAvatarEntitiesDialog, &ModalDialogListener::response, this, nullptr);
            _confirmConnectWithoutAvatarEntitiesDialog = nullptr;
            bool shouldConnect = (static_cast<QMessageBox::StandardButton>(answer.toInt()) == QMessageBox::Yes);
            DependencyManager::get<NodeList>()->getDomainHandler().setCanConnectWithoutAvatarEntities(shouldConnect);
        });
    }
}

void Application::createLoginDialog() {
    const glm::vec3 LOGIN_DIMENSIONS { 0.89f, 0.5f, 0.01f };
    const auto OFFSET = glm::vec2(0.7f, -0.1f);
    auto cameraPosition = _myCamera.getPosition();
    auto cameraOrientation = _myCamera.getOrientation();
    auto upVec = getMyAvatar()->getWorldOrientation() * Vectors::UNIT_Y;
    auto headLookVec = (cameraOrientation * Vectors::FRONT);
    // DEFAULT_DPI / tablet scale percentage
    const float DPI = 31.0f / (75.0f / 100.0f);
    auto offset = headLookVec * OFFSET.x;
    auto position = (cameraPosition + offset) + (upVec * OFFSET.y);

    EntityItemProperties properties;
    properties.setType(EntityTypes::Web);
    properties.setName("LoginDialogEntity");
    properties.setSourceUrl(LOGIN_DIALOG.toString());
    properties.setPosition(position);
    properties.setRotation(cameraOrientation);
    properties.setDimensions(LOGIN_DIMENSIONS);
    properties.setPrimitiveMode(PrimitiveMode::SOLID);
    properties.getGrab().setGrabbable(false);
    properties.setIgnorePickIntersection(false);
    properties.setAlpha(1.0f);
    properties.setDpi(DPI);
    properties.setVisible(true);

    auto entityScriptingInterface = DependencyManager::get<EntityScriptingInterface>();
    _loginDialogID = entityScriptingInterface->addEntityInternal(properties, entity::HostType::LOCAL);

    auto keyboard = DependencyManager::get<Keyboard>().data();
    if (!keyboard->getAnchorID().isNull() && !_loginDialogID.isNull()) {
        auto keyboardLocalOffset = cameraOrientation * glm::vec3(-0.4f * getMyAvatar()->getSensorToWorldScale(), -0.3f, 0.2f);

        EntityItemProperties properties;
        properties.setPosition(position + keyboardLocalOffset);
        properties.setRotation(cameraOrientation * Quaternions::Y_180);

        entityScriptingInterface->editEntity(keyboard->getAnchorID(), properties);
        keyboard->setResetKeyboardPositionOnRaise(false);
    }
    setKeyboardFocusEntity(_loginDialogID);
    emit loginDialogFocusEnabled();
    getApplicationCompositor().getReticleInterface()->setAllowMouseCapture(false);
    getApplicationCompositor().getReticleInterface()->setVisible(false);
    if (!_loginStateManager.isSetUp()) {
        _loginStateManager.setUp();
    }
}

void Application::updateLoginDialogPosition() {
    const float LOOK_AWAY_THRESHOLD_ANGLE = 70.0f;
    const auto OFFSET = glm::vec2(0.7f, -0.1f);

    auto entityScriptingInterface = DependencyManager::get<EntityScriptingInterface>();
    EntityPropertyFlags desiredProperties;
    desiredProperties += PROP_POSITION;
    auto properties = entityScriptingInterface->getEntityPropertiesInternal(_loginDialogID, desiredProperties, false);
    auto positionVec = properties.getPosition();
    auto cameraPositionVec = _myCamera.getPosition();
    auto cameraOrientation = cancelOutRollAndPitch(_myCamera.getOrientation());
    auto headLookVec = (cameraOrientation * Vectors::FRONT);
    auto entityToHeadVec = positionVec - cameraPositionVec;
    auto pointAngle = (glm::acos(glm::dot(glm::normalize(entityToHeadVec), glm::normalize(headLookVec))) * 180.0f / PI);
    auto upVec = getMyAvatar()->getWorldOrientation() * Vectors::UNIT_Y;
    auto offset = headLookVec * OFFSET.x;
    auto newPositionVec = (cameraPositionVec + offset) + (upVec * OFFSET.y);

    bool outOfBounds = glm::distance(positionVec, cameraPositionVec) > 1.0f;

    if (pointAngle > LOOK_AWAY_THRESHOLD_ANGLE || outOfBounds) {
        {
            EntityItemProperties properties;
            properties.setPosition(newPositionVec);
            properties.setRotation(cameraOrientation);
            entityScriptingInterface->editEntity(_loginDialogID, properties);
        }

        {
            glm::vec3 keyboardLocalOffset = cameraOrientation * glm::vec3(-0.4f * getMyAvatar()->getSensorToWorldScale(), -0.3f, 0.2f);
            glm::quat keyboardOrientation = cameraOrientation * glm::quat(glm::radians(glm::vec3(-30.0f, 180.0f, 0.0f)));

            EntityItemProperties properties;
            properties.setPosition(newPositionVec + keyboardLocalOffset);
            properties.setRotation(keyboardOrientation);
            entityScriptingInterface->editEntity(DependencyManager::get<Keyboard>()->getAnchorID(), properties);
        }
    }
}

void Application::createAvatarInputsBar() {
    const glm::vec3 LOCAL_POSITION { 0.0, 0.0, -1.0 };
    // DEFAULT_DPI / tablet scale percentage
    const float DPI = 31.0f / (75.0f / 100.0f);

    EntityItemProperties properties;
    properties.setType(EntityTypes::Web);
    properties.setName("AvatarInputsBarEntity");
    properties.setSourceUrl(AVATAR_INPUTS_BAR_QML.toString());
    properties.setParentID(getMyAvatar()->getSelfID());
    properties.setParentJointIndex(getMyAvatar()->getJointIndex("_CAMERA_MATRIX"));
    properties.setPosition(LOCAL_POSITION);
    properties.setLocalRotation(Quaternions::IDENTITY);
    //properties.setDimensions(LOGIN_DIMENSIONS);
    properties.setPrimitiveMode(PrimitiveMode::SOLID);
    properties.getGrab().setGrabbable(false);
    properties.setIgnorePickIntersection(false);
    properties.setAlpha(1.0f);
    properties.setDpi(DPI);
    properties.setVisible(true);

    auto entityScriptingInterface = DependencyManager::get<EntityScriptingInterface>();
    _avatarInputsBarID = entityScriptingInterface->addEntityInternal(properties, entity::HostType::LOCAL);
}

void Application::destroyAvatarInputsBar() {
    auto entityScriptingInterface = DependencyManager::get<EntityScriptingInterface>();
    if (!_avatarInputsBarID.isNull()) {
        entityScriptingInterface->deleteEntity(_avatarInputsBarID);
    }
}

void Application::showDialog(const QUrl& widgetUrl, const QUrl& tabletUrl, const QString& name) const {
    auto tablet = DependencyManager::get<TabletScriptingInterface>()->getTablet(SYSTEM_TABLET);
    auto hmd = DependencyManager::get<HMDScriptingInterface>();
    bool onTablet = false;

    if (!tablet->getToolbarMode()) {
        onTablet = tablet->pushOntoStack(tabletUrl);
        if (onTablet) {
            toggleTabletUI(true);
        }
    } else {
#if !defined(DISABLE_QML)
        getOffscreenUI()->show(widgetUrl, name);
#endif
    }
}

void Application::loadDialog() {
    ModalDialogListener* dlg = OffscreenUi::getOpenFileNameAsync(_glWidget, tr("Open Script"),
                                                                 getPreviousScriptLocation(),
                                                                 tr("JavaScript Files (*.js)"));
    connect(dlg, &ModalDialogListener::response, this, [=] (QVariant answer) {
        disconnect(dlg, &ModalDialogListener::response, this, nullptr);
        const QString& response = answer.toString();
        if (!response.isEmpty() && QFile(response).exists()) {
            setPreviousScriptLocation(QFileInfo(response).absolutePath());
            DependencyManager::get<ScriptEngines>()->loadScript(response, true, false, false, true);  // Don't load from cache
        }
    });
}

void Application::loadScriptURLDialog() const {
    ModalDialogListener* dlg = OffscreenUi::getTextAsync(OffscreenUi::ICON_NONE, "Open and Run Script", "Script URL");
    connect(dlg, &ModalDialogListener::response, this, [=] (QVariant response) {
        disconnect(dlg, &ModalDialogListener::response, this, nullptr);
        const QString& newScript = response.toString();
        if (QUrl(newScript).scheme() == "atp") {
            OffscreenUi::asyncWarning("Error Loading Script", "Cannot load client script over ATP");
        } else if (!newScript.isEmpty()) {
            DependencyManager::get<ScriptEngines>()->loadScript(newScript.trimmed());
        }
    });
}

void Application::toggleLogDialog() {
#ifndef ANDROID_APP_QUEST_INTERFACE
    if (getLoginDialogPoppedUp()) {
        return;
    }

    if (!_logDialog) {
        bool keepOnTop =_keepLogWindowOnTop.get();
#ifdef Q_OS_WIN
        _logDialog = new LogDialog(keepOnTop ? qApp->getWindow() : nullptr, getLogger());
#elif !defined(Q_OS_ANDROID)
        _logDialog = new LogDialog(nullptr, getLogger());

        if (keepOnTop) {
            Qt::WindowFlags flags = _logDialog->windowFlags() | Qt::Tool;
            _logDialog->setWindowFlags(flags);
        }
#else
        Q_UNUSED(keepOnTop)
#endif
    }

    if (_logDialog->isVisible()) {
        _logDialog->hide();
    } else {
        _logDialog->show();
    }
#endif
}

void Application::recreateLogWindow(int keepOnTop) {
    _keepLogWindowOnTop.set(keepOnTop != 0);
    if (_logDialog) {
        bool toggle = _logDialog->isVisible();
        _logDialog->close();
        _logDialog = nullptr;

        if (toggle) {
            toggleLogDialog();
        }
    }
}

void Application::toggleEntityScriptServerLogDialog() {
    if (! _entityScriptServerLogDialog) {
        _entityScriptServerLogDialog = new EntityScriptServerLogDialog(nullptr);
    }

    if (_entityScriptServerLogDialog->isVisible()) {
        _entityScriptServerLogDialog->hide();
    } else {
        _entityScriptServerLogDialog->show();
    }
}

void Application::showAssetServerWidget(QString filePath) {
    if (!DependencyManager::get<NodeList>()->getThisNodeCanWriteAssets() || getLoginDialogPoppedUp()) {
        return;
    }
    static const QUrl url { "hifi/AssetServer.qml" };

    auto startUpload = [=](QQmlContext* context, QObject* newObject){
        if (!filePath.isEmpty()) {
            emit uploadRequest(filePath);
        }
    };
    auto tabletScriptingInterface = DependencyManager::get<TabletScriptingInterface>();
    auto tablet = dynamic_cast<TabletProxy*>(tabletScriptingInterface->getTablet(SYSTEM_TABLET));
    auto hmd = DependencyManager::get<HMDScriptingInterface>();
    if (tablet->getToolbarMode()) {
        getOffscreenUI()->show(url, "AssetServer", startUpload);
    } else {
        if (!hmd->getShouldShowTablet() && !isHMDMode()) {
            getOffscreenUI()->show(url, "AssetServer", startUpload);
        } else {
            static const QUrl url("qrc:///qml/hifi/dialogs/TabletAssetServer.qml");
            if (!tablet->isPathLoaded(url)) {
                tablet->pushOntoStack(url);
            }
        }
    }

    startUpload(nullptr, nullptr);
}

void Application::loadAddAvatarBookmarkDialog() const {
    auto avatarBookmarks = DependencyManager::get<AvatarBookmarks>();
}

void Application::loadAvatarBrowser() const {
    auto tablet = dynamic_cast<TabletProxy*>(DependencyManager::get<TabletScriptingInterface>()->getTablet("com.highfidelity.interface.tablet.system"));
    // construct the url to the marketplace item
    QString url = MetaverseAPI::getCurrentMetaverseServerURL().toString() + "/marketplace?category=avatars";

    QString MARKETPLACES_INJECT_SCRIPT_PATH = "file:///" + qApp->applicationDirPath() + "/scripts/system/html/js/marketplacesInject.js";
    tablet->gotoWebScreen(url, MARKETPLACES_INJECT_SCRIPT_PATH);
    DependencyManager::get<HMDScriptingInterface>()->openTablet();
}

void Application::showLoginScreen() {
#if !defined(DISABLE_QML)
    auto accountManager = DependencyManager::get<AccountManager>();
    auto dialogsManager = DependencyManager::get<DialogsManager>();
    if (!accountManager->isLoggedIn()) {
        if (!isHMDMode()) {
            auto toolbar =  DependencyManager::get<ToolbarScriptingInterface>()->getToolbar("com.highfidelity.interface.toolbar.system");
            toolbar->writeProperty("visible", false);
        }
        _loginDialogPoppedUp = true;
        dialogsManager->showLoginDialog();
        emit loginDialogFocusEnabled();
        QJsonObject loginData = {};
        loginData["action"] = "login dialog popped up";
        UserActivityLogger::getInstance().logAction("encourageLoginDialog", loginData);
        _window->setWindowTitle("Overte");
    } else {
        resumeAfterLoginDialogActionTaken();
    }
    _loginDialogPoppedUp = !accountManager->isLoggedIn();
#else
    resumeAfterLoginDialogActionTaken();
#endif
}

void Application::showHelp() {
    static const QString HAND_CONTROLLER_NAME_VIVE = "vive";
    static const QString HAND_CONTROLLER_NAME_OCULUS_TOUCH = "oculus";
    static const QString HAND_CONTROLLER_NAME_WINDOWS_MR = "windowsMR";

    static const QString VIVE_PLUGIN_NAME = "HTC Vive";
    static const QString OCULUS_RIFT_PLUGIN_NAME = "Oculus Rift";
    static const QString WINDOWS_MR_PLUGIN_NAME = "WindowsMR";

    static const QString TAB_KEYBOARD_MOUSE = "kbm";
    static const QString TAB_GAMEPAD = "gamepad";
    static const QString TAB_HAND_CONTROLLERS = "handControllers";

    QString handControllerName;
    QString defaultTab = TAB_KEYBOARD_MOUSE;

    if (PluginUtils::isHMDAvailable(WINDOWS_MR_PLUGIN_NAME)) {
        defaultTab = TAB_HAND_CONTROLLERS;
        handControllerName = HAND_CONTROLLER_NAME_WINDOWS_MR;
    } else if (PluginUtils::isHMDAvailable(VIVE_PLUGIN_NAME)) {
        defaultTab = TAB_HAND_CONTROLLERS;
        handControllerName = HAND_CONTROLLER_NAME_VIVE;
    } else if (PluginUtils::isHMDAvailable(OCULUS_RIFT_PLUGIN_NAME)) {
        if (PluginUtils::isOculusTouchControllerAvailable()) {
            defaultTab = TAB_HAND_CONTROLLERS;
            handControllerName = HAND_CONTROLLER_NAME_OCULUS_TOUCH;
        } else if (PluginUtils::isXboxControllerAvailable()) {
            defaultTab = TAB_GAMEPAD;
        } else {
            defaultTab = TAB_KEYBOARD_MOUSE;
        }
    } else if (PluginUtils::isXboxControllerAvailable()) {
        defaultTab = TAB_GAMEPAD;
    } else {
        defaultTab = TAB_KEYBOARD_MOUSE;
    }

    QUrlQuery queryString;
    queryString.addQueryItem("handControllerName", handControllerName);
    queryString.addQueryItem("defaultTab", defaultTab);
    TabletProxy* tablet = dynamic_cast<TabletProxy*>(DependencyManager::get<TabletScriptingInterface>()->getTablet(SYSTEM_TABLET));
    tablet->gotoWebScreen(PathUtils::resourcesUrl() + INFO_HELP_PATH + "?" + queryString.toString());
    DependencyManager::get<HMDScriptingInterface>()->openTablet();
    //InfoView::show(INFO_HELP_PATH, false, queryString.toString());
}

void Application::updateSystemTabletMode() {
    if (_settingsLoaded && !getLoginDialogPoppedUp()) {
        qApp->setProperty(hifi::properties::HMD, isHMDMode());
        if (isHMDMode()) {
            DependencyManager::get<TabletScriptingInterface>()->setToolbarMode(getHmdTabletBecomesToolbarSetting());
        } else {
            DependencyManager::get<TabletScriptingInterface>()->setToolbarMode(getDesktopTabletBecomesToolbarSetting());
        }
    }
}

void Application::captureMouseChanged(bool captureMouse) {
    _captureMouse = captureMouse;
    if (_captureMouse) {
        _glWidget->setCursor(QCursor(Qt::BlankCursor));
    } else {
        _mouseCaptureTarget = QPointF(NAN, NAN);
        _glWidget->unsetCursor();
    }
}

void Application::toggleOverlays() {
    auto menu = Menu::getInstance();
    menu->setIsOptionChecked(MenuOption::Overlays, !menu->isOptionChecked(MenuOption::Overlays));
}

void Application::setOverlaysVisible(bool visible) {
    auto menu = Menu::getInstance();
    menu->setIsOptionChecked(MenuOption::Overlays, visible);
}

void Application::addAssetToWorldMessageClose() {
    // Clear messages, e.g., if Interface is being closed or domain changes.

    /*
    Call if user manually closes message box.
    Call if domain changes.
    Call if application is shutting down.

    Stop timers.
    Close the message box if open.
    Clear lists.
    */

    _addAssetToWorldInfoTimer.stop();
    _addAssetToWorldErrorTimer.stop();

    if (_addAssetToWorldMessageBox) {
        disconnect(_addAssetToWorldMessageBox);
        _addAssetToWorldMessageBox->setVisible(false);
        _addAssetToWorldMessageBox->deleteLater();
        _addAssetToWorldMessageBox = nullptr;
    }

    _addAssetToWorldInfoKeys.clear();
    _addAssetToWorldInfoMessages.clear();
}

void Application::loadLODToolsDialog() {
    auto tabletScriptingInterface = DependencyManager::get<TabletScriptingInterface>();
    auto tablet = dynamic_cast<TabletProxy*>(tabletScriptingInterface->getTablet(SYSTEM_TABLET));
    if (tablet->getToolbarMode() || (!tablet->getTabletRoot() && !isHMDMode())) {
        auto dialogsManager = DependencyManager::get<DialogsManager>();
        dialogsManager->lodTools();
    } else {
        tablet->pushOntoStack("hifi/dialogs/TabletLODTools.qml");
    }
}

void Application::loadEntityStatisticsDialog() {
    auto tabletScriptingInterface = DependencyManager::get<TabletScriptingInterface>();
    auto tablet = dynamic_cast<TabletProxy*>(tabletScriptingInterface->getTablet(SYSTEM_TABLET));
    if (tablet->getToolbarMode() || (!tablet->getTabletRoot() && !isHMDMode())) {
        auto dialogsManager = DependencyManager::get<DialogsManager>();
        dialogsManager->octreeStatsDetails();
    } else {
        tablet->pushOntoStack("hifi/dialogs/TabletEntityStatistics.qml");
    }
}

void Application::loadDomainConnectionDialog() {
    auto tabletScriptingInterface = DependencyManager::get<TabletScriptingInterface>();
    auto tablet = dynamic_cast<TabletProxy*>(tabletScriptingInterface->getTablet(SYSTEM_TABLET));
    if (tablet->getToolbarMode() || (!tablet->getTabletRoot() && !isHMDMode())) {
        auto dialogsManager = DependencyManager::get<DialogsManager>();
        dialogsManager->showDomainConnectionDialog();
    } else {
        tablet->pushOntoStack("hifi/dialogs/TabletDCDialog.qml");
    }
}

void Application::showScriptLogs() {
    QUrl defaultScriptsLoc = PathUtils::defaultScriptsLocation();
    defaultScriptsLoc.setPath(defaultScriptsLoc.path() + "developer/debugging/debugWindow.js");
    DependencyManager::get<ScriptEngines>()->loadScript(defaultScriptsLoc.toString());
}

void Application::setPreferredCursor(const QString& cursorName) {
    qCDebug(interfaceapp) << "setPreferredCursor" << cursorName;

    if (_displayPlugin && _displayPlugin->isHmd()) {
        _preferredCursor.set(cursorName.isEmpty() ? DEFAULT_CURSOR_NAME : cursorName);
    } else {
        _preferredCursor.set(cursorName.isEmpty() ? Cursor::Manager::getIconName(Cursor::Icon::SYSTEM) : cursorName);
    }

    showCursor(Cursor::Manager::lookupIcon(_preferredCursor.get()));
}

void Application::updateThemeColors() {
    // builtin style that exists on all platforms
    // NOTE: in Qt5 the Fusion style with a dark palette has
    // checkboxes with very low contrast, this was fixed in Qt6.5
    auto style = QStyleFactory::create("Fusion");

    QPalette palette = style->standardPalette();

    if (_darkTheme.get()) {
        palette.setColor(QPalette::Window,          QColor(48, 48, 48));
        palette.setColor(QPalette::WindowText,      QColor(224, 224, 224));

        palette.setColor(QPalette::Base,            QColor(40, 40, 40));
        palette.setColor(QPalette::AlternateBase,   QColor(44, 44, 44));

        palette.setColor(QPalette::Light,           QColor(72, 72, 72));
        palette.setColor(QPalette::Midlight,        QColor(64, 64, 64));
        palette.setColor(QPalette::Button,          QColor(40, 40, 40));
        palette.setColor(QPalette::Mid,             QColor(32, 32, 32));
        palette.setColor(QPalette::Dark,            QColor(20, 20, 20));
        palette.setColor(QPalette::Shadow,          QColor(0, 0, 0));

        palette.setColor(QPalette::Text,            QColor(224, 224, 224));
        palette.setColor(QPalette::ButtonText,      QColor(224, 224, 224));
        palette.setColor(QPalette::BrightText,      QColor(255, 255, 255));

        palette.setColor(QPalette::Highlight,       QColor(21, 83, 158));
        palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));

        palette.setColor(QPalette::Link,            QColor(95, 189, 252));
        palette.setColor(QPalette::LinkVisited,     QColor(192, 95, 252));
    }

    qApp->setStyle(style);
    qApp->setPalette(palette);
    qApp->getPrimaryMenu()->setPalette(palette); // weird Qt bug workaround
}

void Application::setDarkThemePreference(bool value) {
    bool previousValue = _darkTheme.get();

    if (value == previousValue) { return; }

    _darkTheme.set(value);
    updateThemeColors();
    emit darkThemePreferenceChanged(value);
}

void Application::showVRKeyboardForHudUI(bool show) {
    if (show) {
        DependencyManager::get<Keyboard>()->setRaised(true, true);
    } else {
        DependencyManager::get<Keyboard>()->setRaised(false);
    }
}

void Application::onDesktopRootItemCreated(QQuickItem* rootItem) {
    Stats::show();
    AnimStats::show();
    auto surfaceContext = getOffscreenUI()->getSurfaceContext();
    surfaceContext->setContextProperty("Stats", Stats::getInstance());
    surfaceContext->setContextProperty("AnimStats", AnimStats::getInstance());

#if !defined(Q_OS_ANDROID)
    auto offscreenUi = getOffscreenUI();
    auto qml = PathUtils::qmlUrl("AvatarInputsBar.qml");
    offscreenUi->show(qml, "AvatarInputsBar");
#endif
}

void Application::onDesktopRootContextCreated(QQmlContext* surfaceContext) {
    auto engine = surfaceContext->engine();
    // in Qt 5.10.0 there is already an "Audio" object in the QML context
    // though I failed to find it (from QtMultimedia??). So..  let it be "AudioScriptingInterface"
    surfaceContext->setContextProperty("AudioScriptingInterface", DependencyManager::get<AudioScriptingInterface>().data());

    surfaceContext->setContextProperty("AudioStats", DependencyManager::get<AudioClient>()->getStats().data());
    surfaceContext->setContextProperty("AudioScope", DependencyManager::get<AudioScope>().data());

    surfaceContext->setContextProperty("Controller", DependencyManager::get<controller::ScriptingInterface>().data());
    surfaceContext->setContextProperty("Entities", DependencyManager::get<EntityScriptingInterface>().data());
    surfaceContext->setContextProperty("Performance", new PerformanceScriptingInterface());
    _fileDownload = new ArchiveDownloadInterface(engine);
    surfaceContext->setContextProperty("File", _fileDownload);
    connect(_fileDownload, &ArchiveDownloadInterface::unzipResult, this, &Application::handleUnzip);
    surfaceContext->setContextProperty("MyAvatar", getMyAvatar().get());
    surfaceContext->setContextProperty("Messages", DependencyManager::get<MessagesClient>().data());
    surfaceContext->setContextProperty("Recording", DependencyManager::get<RecordingScriptingInterface>().data());
    surfaceContext->setContextProperty("Preferences", DependencyManager::get<Preferences>().data());
    surfaceContext->setContextProperty("AddressManager", DependencyManager::get<AddressManager>().data());
    surfaceContext->setContextProperty("FrameTimings", &_graphicsEngine->_frameTimingsScriptingInterface);
    surfaceContext->setContextProperty("Rates", new RatesScriptingInterface(this));

    surfaceContext->setContextProperty("TREE_SCALE", TREE_SCALE);
    // FIXME Quat and Vec3 won't work with QJSEngine used by QML
    surfaceContext->setContextProperty("Quat", new Quat());
    surfaceContext->setContextProperty("Vec3", new Vec3());
    surfaceContext->setContextProperty("Uuid", new ScriptUUID());
    surfaceContext->setContextProperty("Assets", DependencyManager::get<AssetMappingsScriptingInterface>().data());
    surfaceContext->setContextProperty("Keyboard", DependencyManager::get<KeyboardScriptingInterface>().data());

    surfaceContext->setContextProperty("AvatarList", DependencyManager::get<AvatarManager>().data());
    surfaceContext->setContextProperty("Users", DependencyManager::get<UsersScriptingInterface>().data());

    surfaceContext->setContextProperty("UserActivityLogger", DependencyManager::get<UserActivityLoggerScriptingInterface>().data());
    surfaceContext->setContextProperty("Camera", &_myCamera);

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    surfaceContext->setContextProperty("SpeechRecognizer", DependencyManager::get<SpeechRecognizer>().data());
#endif

    surfaceContext->setContextProperty("Overlays", &_overlays);
    surfaceContext->setContextProperty("Window", DependencyManager::get<WindowScriptingInterface>().data());
    surfaceContext->setContextProperty("Desktop", DependencyManager::get<DesktopScriptingInterface>().data());
    surfaceContext->setContextProperty("MenuInterface", MenuScriptingInterface::getInstance());
    surfaceContext->setContextProperty("Settings", new QMLSettingsScriptingInterface(surfaceContext));
    surfaceContext->setContextProperty("ScriptDiscoveryService", DependencyManager::get<ScriptEngines>().data());
    surfaceContext->setContextProperty("AvatarBookmarks", DependencyManager::get<AvatarBookmarks>().data());
    surfaceContext->setContextProperty("LocationBookmarks", DependencyManager::get<LocationBookmarks>().data());

    // Caches
    surfaceContext->setContextProperty("AnimationCache", DependencyManager::get<AnimationCacheScriptingInterface>().data());
    surfaceContext->setContextProperty("TextureCache", DependencyManager::get<TextureCacheScriptingInterface>().data());
    surfaceContext->setContextProperty("MaterialCache", DependencyManager::get<MaterialCacheScriptingInterface>().data());
    surfaceContext->setContextProperty("ModelCache", DependencyManager::get<ModelCacheScriptingInterface>().data());
    surfaceContext->setContextProperty("SoundCache", DependencyManager::get<SoundCacheScriptingInterface>().data());

    surfaceContext->setContextProperty("InputConfiguration", DependencyManager::get<InputConfiguration>().data());

    surfaceContext->setContextProperty("Account", AccountServicesScriptingInterface::getInstance()); // DEPRECATED - TO BE REMOVED
    surfaceContext->setContextProperty("GlobalServices", AccountServicesScriptingInterface::getInstance()); // DEPRECATED - TO BE REMOVED
    surfaceContext->setContextProperty("AccountServices", AccountServicesScriptingInterface::getInstance());

    surfaceContext->setContextProperty("DialogsManager", _dialogsManagerScriptingInterface);
    surfaceContext->setContextProperty("AvatarManager", DependencyManager::get<AvatarManager>().data());
    surfaceContext->setContextProperty("LODManager", DependencyManager::get<LODManager>().data());
    surfaceContext->setContextProperty("HMD", DependencyManager::get<HMDScriptingInterface>().data());
    surfaceContext->setContextProperty("Scene", DependencyManager::get<SceneScriptingInterface>().data());
    surfaceContext->setContextProperty("Render", RenderScriptingInterface::getInstance());
    surfaceContext->setContextProperty("PlatformInfo", PlatformInfoScriptingInterface::getInstance());
    surfaceContext->setContextProperty("Workload", _gameWorkload._engine->getConfiguration().get());
    surfaceContext->setContextProperty("Reticle", getApplicationCompositor().getReticleInterface());
    surfaceContext->setContextProperty("Snapshot", DependencyManager::get<Snapshot>().data());

    surfaceContext->setContextProperty("ApplicationCompositor", &getApplicationCompositor());

    surfaceContext->setContextProperty("AvatarInputs", AvatarInputs::getInstance());
    surfaceContext->setContextProperty("Selection", DependencyManager::get<SelectionScriptingInterface>().data());
    surfaceContext->setContextProperty("About", AboutUtil::getInstance());
    surfaceContext->setContextProperty("HiFiAbout", AboutUtil::getInstance());  // Deprecated
    surfaceContext->setContextProperty("ResourceRequestObserver", DependencyManager::get<ResourceRequestObserver>().data());
    surfaceContext->setContextProperty("ExternalResource", ExternalResource::getInstance());

    if (auto steamClient = PluginManager::getInstance()->getSteamClientPlugin()) {
        surfaceContext->setContextProperty("Steam", new SteamScriptingInterface(engine, steamClient.get()));
    }

    _window->setMenuBar(new Menu());
}

void Application::showDesktop() {}

void Application::notifyPacketVersionMismatch() {
    if (!_notifiedPacketVersionMismatchThisDomain && !isInterstitialMode()) {
        _notifiedPacketVersionMismatchThisDomain = true;

        QString message = "The location you are visiting is running an incompatible server version.\n";
        message += "Content may not display properly.";

        OffscreenUi::asyncWarning("", message);
    }
}

bool Application::askToSetAvatarUrl(const QString& url) {
    QUrl realUrl(url);
    if (realUrl.isLocalFile()) {
        OffscreenUi::asyncWarning("", "You can not use local files for avatar components.");
        return false;
    }

    // Download the FST file, to attempt to determine its model type
    QVariantHash fstMapping = FSTReader::downloadMapping(url);

    FSTReader::ModelType modelType = FSTReader::predictModelType(fstMapping);

    QString modelName = fstMapping["name"].toString();
    QString modelLicense = fstMapping["license"].toString();

    bool agreeToLicense = true; // assume true
    //create set avatar callback
    auto setAvatar = [=] (QString url, QString modelName) {
        ModalDialogListener* dlg = OffscreenUi::asyncQuestion("Set Avatar",
                                                              "Would you like to use '" + modelName + "' for your avatar?",
                                                              QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
        QObject::connect(dlg, &ModalDialogListener::response, this, [=] (QVariant answer) {
            QObject::disconnect(dlg, &ModalDialogListener::response, this, nullptr);

            bool ok = (QMessageBox::Ok == static_cast<QMessageBox::StandardButton>(answer.toInt()));
            if (ok) {
                getMyAvatar()->useFullAvatarURL(url, modelName);
                emit fullAvatarURLChanged(url, modelName);
            } else {
                qCDebug(interfaceapp) << "Declined to use the avatar";
            }
        });
    };

    if (!modelLicense.isEmpty()) {
        // word wrap the license text to fit in a reasonable shaped message box.
        const int MAX_CHARACTERS_PER_LINE = 90;
        modelLicense = simpleWordWrap(modelLicense, MAX_CHARACTERS_PER_LINE);

        ModalDialogListener* dlg = OffscreenUi::asyncQuestion("Avatar Usage License",
                                                              modelLicense + "\nDo you agree to these terms?",
                                                              QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        QObject::connect(dlg, &ModalDialogListener::response, this, [=, &agreeToLicense] (QVariant answer) {
            QObject::disconnect(dlg, &ModalDialogListener::response, this, nullptr);

            agreeToLicense = (static_cast<QMessageBox::StandardButton>(answer.toInt()) == QMessageBox::Yes);
            if (agreeToLicense) {
                switch (modelType) {
                    case FSTReader::HEAD_AND_BODY_MODEL: {
                    setAvatar(url, modelName);
                    break;
                }
                default:
                    OffscreenUi::asyncWarning("", modelName + "Does not support a head and body as required.");
                    break;
                }
            } else {
                qCDebug(interfaceapp) << "Declined to agree to avatar license";
            }

            //auto offscreenUi = getOffscreenUI();
        });
    } else {
        setAvatar(url, modelName);
    }

    return true;
}

bool Application::askToLoadScript(const QString& scriptFilenameOrURL) {
    QString shortName = scriptFilenameOrURL;

    QUrl scriptURL { scriptFilenameOrURL };

    if (scriptURL.host().endsWith(NetworkingConstants::HF_MARKETPLACE_CDN_HOSTNAME)) {
        int startIndex = shortName.lastIndexOf('/') + 1;
        int endIndex = shortName.lastIndexOf('?');
        shortName = shortName.mid(startIndex, endIndex - startIndex);
    }

#ifdef DISABLE_QML
    DependencyManager::get<ScriptEngines>()->loadScript(scriptFilenameOrURL);
#else
    QString message = "Would you like to run this script:\n" + shortName;
    ModalDialogListener* dlg = OffscreenUi::asyncQuestion(getWindow(), "Run Script", message,
                                                           QMessageBox::Yes | QMessageBox::No);

    QObject::connect(dlg, &ModalDialogListener::response, this, [=] (QVariant answer) {
        const QString& fileName = scriptFilenameOrURL;
        if (static_cast<QMessageBox::StandardButton>(answer.toInt()) == QMessageBox::Yes) {
            qCDebug(interfaceapp) << "Chose to run the script: " << fileName;
            DependencyManager::get<ScriptEngines>()->loadScript(fileName);
        } else {
            qCDebug(interfaceapp) << "Declined to run the script";
        }
        QObject::disconnect(dlg, &ModalDialogListener::response, this, nullptr);
    });
#endif
    return true;
}

bool Application::askToReplaceDomainContent(const QString& url) {
    QString methodDetails;
    const int MAX_CHARACTERS_PER_LINE = 90;
    if (DependencyManager::get<NodeList>()->getThisNodeCanReplaceContent()) {
        QUrl originURL { url };
        if (originURL.host().endsWith(NetworkingConstants::HF_MARKETPLACE_CDN_HOSTNAME)) {
            // Create a confirmation dialog when this call is made
            static const QString infoText = simpleWordWrap("Your domain's content will be replaced with a new content set. "
                "If you want to save what you have now, create a backup before proceeding. For more information about backing up "
                "and restoring content, visit the documentation page at: ", MAX_CHARACTERS_PER_LINE) +
                "\nhttps://docs.overte.org/host/maintain-domain/backup-domain.html";

            ModalDialogListener* dig = OffscreenUi::asyncQuestion("Are you sure you want to replace this domain's content set?",
                                                                  infoText, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

            QObject::connect(dig, &ModalDialogListener::response, this, [=] (QVariant answer) {
                QString details;
                if (static_cast<QMessageBox::StandardButton>(answer.toInt()) == QMessageBox::Yes) {
                    // Given confirmation, send request to domain server to replace content
                    replaceDomainContent(url, QString());
                    details = "SuccessfulRequestToReplaceContent";
                } else {
                    details = "UserDeclinedToReplaceContent";
                }
                QJsonObject messageProperties = {
                    { "status", details },
                    { "content_set_url", url }
                };
                UserActivityLogger::getInstance().logAction("replace_domain_content", messageProperties);
                QObject::disconnect(dig, &ModalDialogListener::response, this, nullptr);
            });
        } else {
            methodDetails = "ContentSetDidNotOriginateFromMarketplace";
            QJsonObject messageProperties = {
                { "status", methodDetails },
                { "content_set_url", url }
            };
            UserActivityLogger::getInstance().logAction("replace_domain_content", messageProperties);
        }
    } else {
            methodDetails = "UserDoesNotHavePermissionToReplaceContent";
            static const QString warningMessage = simpleWordWrap("The domain owner must enable 'Replace Content' "
                "permissions for you in this domain's server settings before you can continue.", MAX_CHARACTERS_PER_LINE);
            OffscreenUi::asyncWarning("You do not have permissions to replace domain content", warningMessage,
                                 QMessageBox::Ok, QMessageBox::Ok);

            QJsonObject messageProperties = {
                { "status", methodDetails },
                { "content_set_url", url }
            };
            UserActivityLogger::getInstance().logAction("replace_domain_content", messageProperties);
    }
    return true;
}

void Application::onDismissedLoginDialog() {
    _loginDialogPoppedUp = false;
    auto keyboard = DependencyManager::get<Keyboard>().data();
    keyboard->setResetKeyboardPositionOnRaise(true);
    if (!_loginDialogID.isNull()) {
        DependencyManager::get<EntityScriptingInterface>()->deleteEntity(_loginDialogID);
        _loginDialogID = QUuid();
        _loginStateManager.tearDown();
    }
    resumeAfterLoginDialogActionTaken();
}

bool Application::initMenu() {
    _isMenuInitialized = false;
    qApp->getWindow()->menuBar();
    return true;
}

void Application::pauseUntilLoginDetermined() {
    if (QThread::currentThread() != qApp->thread()) {
        QMetaObject::invokeMethod(this, "pauseUntilLoginDetermined");
        return;
    }

    auto myAvatar = getMyAvatar();
    _previousAvatarTargetScale = myAvatar->getTargetScale();
    _previousAvatarSkeletonModel = myAvatar->getSkeletonModelURL().toString();
    myAvatar->setTargetScale(1.0f);
    myAvatar->setSkeletonModelURLFromScript(myAvatar->defaultFullAvatarModelUrl().toString());
    myAvatar->setEnableMeshVisible(false);

    _controllerScriptingInterface->disableMapping(STANDARD_TO_ACTION_MAPPING_NAME);

    {
        auto userInputMapper = DependencyManager::get<UserInputMapper>();
        if (userInputMapper->loadMapping(NO_MOVEMENT_MAPPING_JSON)) {
            _controllerScriptingInterface->enableMapping(NO_MOVEMENT_MAPPING_NAME);
        }
    }

    const auto& nodeList = DependencyManager::get<NodeList>();
    // save interstitial mode setting until resuming.
    _interstitialModeEnabled = nodeList->getDomainHandler().getInterstitialModeEnabled();
    nodeList->getDomainHandler().setInterstitialModeEnabled(false);

    auto menu = Menu::getInstance();
    menu->getMenu("Edit")->setVisible(false);
    menu->getMenu("View")->setVisible(false);
    menu->getMenu("Navigate")->setVisible(false);
    menu->getMenu("Settings")->setVisible(false);
    _developerMenuVisible = menu->getMenu("Developer")->isVisible();
    menu->setIsOptionChecked(MenuOption::Stats, false);
    if (_developerMenuVisible) {
        menu->getMenu("Developer")->setVisible(false);
    }
    _previousCameraMode = _myCamera.getMode();
    _myCamera.setMode(CAMERA_MODE_FIRST_PERSON_LOOK_AT);
    cameraModeChanged();

    // disconnect domain handler.
    nodeList->getDomainHandler().disconnect("Pause until login determined");

    // From now on, it's permissible to call resumeAfterLoginDialogActionTaken()
    _resumeAfterLoginDialogActionTaken_SafeToRun = true;

    if (_resumeAfterLoginDialogActionTaken_WasPostponed) {
        // resumeAfterLoginDialogActionTaken() was already called, but it aborted. Now it's safe to call it again.
        resumeAfterLoginDialogActionTaken();
    }
}

void Application::resumeAfterLoginDialogActionTaken() {
    if (QThread::currentThread() != qApp->thread()) {
        QMetaObject::invokeMethod(this, "resumeAfterLoginDialogActionTaken");
        return;
    }

    if (!_resumeAfterLoginDialogActionTaken_SafeToRun) {
        _resumeAfterLoginDialogActionTaken_WasPostponed = true;
        return;
    }

#if !defined(DISABLE_QML)
    if (!isHMDMode() && getDesktopTabletBecomesToolbarSetting()) {
        auto toolbar = DependencyManager::get<ToolbarScriptingInterface>()->getToolbar("com.highfidelity.interface.toolbar.system");
        toolbar->writeProperty("visible", true);
    } else {
        getApplicationCompositor().getReticleInterface()->setAllowMouseCapture(true);
        getApplicationCompositor().getReticleInterface()->setVisible(true);
    }

    updateSystemTabletMode();
#endif

    {
        auto userInputMapper = DependencyManager::get<UserInputMapper>();
        userInputMapper->unloadMapping(NO_MOVEMENT_MAPPING_JSON);
        _controllerScriptingInterface->disableMapping(NO_MOVEMENT_MAPPING_NAME);
    }

    auto myAvatar = getMyAvatar();
    myAvatar->setTargetScale(_previousAvatarTargetScale);
    myAvatar->setSkeletonModelURLFromScript(_previousAvatarSkeletonModel);
    myAvatar->setEnableMeshVisible(true);

    _controllerScriptingInterface->enableMapping(STANDARD_TO_ACTION_MAPPING_NAME);

    const auto& nodeList = DependencyManager::get<NodeList>();
    nodeList->getDomainHandler().setInterstitialModeEnabled(_interstitialModeEnabled);
    {
        auto scriptEngines = DependencyManager::get<ScriptEngines>().data();
        // this will force the model the look at the correct directory (weird order of operations issue)
        scriptEngines->reloadLocalFiles();

        // if the --scripts command-line argument was used.
        if (_overrideDefaultScriptsLocation && _defaultScriptsLocation.exists()) {
            scriptEngines->loadDefaultScripts();
            scriptEngines->defaultScriptsLocationOverridden(true);
        } else {
            scriptEngines->loadScripts();
        }
    }

    auto accountManager = DependencyManager::get<AccountManager>();
    auto addressManager = DependencyManager::get<AddressManager>();

    // restart domain handler.
    nodeList->getDomainHandler().resetting();

    QVariant testProperty = property(hifi::properties::TEST);
    if (testProperty.isValid()) {
        const auto testScript = property(hifi::properties::TEST).toUrl();
        // Set last parameter to exit interface when the test script finishes, if so requested
        DependencyManager::get<ScriptEngines>()->loadScript(testScript, false, false, false, false, _quitWhenFinished);
        // This is done so we don't get a "connection time-out" message when we haven't passed in a URL.
        if (!_urlParam.isEmpty()) {
            auto reply = SandboxUtils::getStatus();
            connect(reply, &QNetworkReply::finished, this, [this, reply] { handleSandboxStatus(reply); });
        }
    } else {
        auto reply = SandboxUtils::getStatus();
        connect(reply, &QNetworkReply::finished, this, [this, reply] { handleSandboxStatus(reply); });
    }

    auto menu = Menu::getInstance();
    menu->getMenu("Edit")->setVisible(true);
    menu->getMenu("View")->setVisible(true);
    menu->getMenu("Navigate")->setVisible(true);
    menu->getMenu("Settings")->setVisible(true);
    menu->getMenu("Developer")->setVisible(_developerMenuVisible);
    _myCamera.setMode(_previousCameraMode);
    cameraModeChanged();
    _startUpFinished = true;
    getRefreshRateManager().setRefreshRateRegime(RefreshRateManager::RefreshRateRegime::FOCUS_ACTIVE);
}

QSharedPointer<OffscreenUi> Application::getOffscreenUI() {
#if !defined(DISABLE_QML)
    return DependencyManager::get<OffscreenUi>();
#else
    return nullptr;
#endif
}

void Application::updateDialogs(float deltaTime) const {
    PerformanceTimer perfTimer("updateDialogs");
    bool showWarnings = Menu::getInstance()->isOptionChecked(MenuOption::PipelineWarnings);
    PerformanceWarning warn(showWarnings, "Application::updateDialogs()");
    auto dialogsManager = DependencyManager::get<DialogsManager>();

    QPointer<OctreeStatsDialog> octreeStatsDialog = dialogsManager->getOctreeStatsDialog();
    if (octreeStatsDialog) {
        octreeStatsDialog->update();
    }
}

void Application::maybeToggleMenuVisible(QMouseEvent* event) const {
#ifndef Q_OS_MAC
    // If in full screen, and our main windows menu bar is hidden, and we're close to the top of the QMainWindow
    // then show the menubar.
    if (_window->isFullScreen()) {
        QMenuBar* menuBar = _window->menuBar();
        if (menuBar) {
            static const int MENU_TOGGLE_AREA = 10;
            if (!menuBar->isVisible()) {
                if (event->pos().y() <= MENU_TOGGLE_AREA) {
                    menuBar->setVisible(true);
                }
            }  else {
                if (event->pos().y() > MENU_TOGGLE_AREA) {
                    menuBar->setVisible(false);
                }
            }
        }
    }
#endif
}

void Application::toggleTabletUI(bool shouldOpen) const {
    auto hmd = DependencyManager::get<HMDScriptingInterface>();
    if (!(shouldOpen && hmd->getShouldShowTablet())) {
        auto HMD = DependencyManager::get<HMDScriptingInterface>();
        HMD->toggleShouldShowTablet();

        if (!HMD->getShouldShowTablet()) {
            DependencyManager::get<Keyboard>()->setRaised(false);
            _window->activateWindow();
            auto tablet = DependencyManager::get<TabletScriptingInterface>()->getTablet(SYSTEM_TABLET);
            tablet->unfocus();
        }
    }
}

bool Application::shouldCaptureMouse() const {
    return _captureMouse && _glWidget->isActiveWindow() && !ui::Menu::isSomeSubmenuShown();
}

void Application::checkChangeCursor() {
    QMutexLocker locker(&_changeCursorLock);
    if (_cursorNeedsChanging) {
#ifdef Q_OS_MAC
        auto cursorTarget = _window; // OSX doesn't seem to provide for hiding the cursor only on the GL widget
#else
        // On windows and linux, hiding the top level cursor also means it's invisible when hovering over the
        // window menu, which is a pain, so only hide it for the GL surface
        auto cursorTarget = _glWidget;
#endif
        cursorTarget->setCursor(_desiredCursor);

        _cursorNeedsChanging = false;
    }
}

void Application::addAssetToWorldInfo(QString modelName, QString infoText) {
    // Displays the most recent info message, subject to being overridden by error messages.

    if (_aboutToQuit) {
        return;
    }

    /*
    Cancel info timer if running.
    If list has an entry for modelName, delete it (just one).
    Append modelName, infoText to list.
    Display infoText in message box unless an error is being displayed (i.e., error timer is running).
    Show message box if not already visible.
    */

    _addAssetToWorldInfoTimer.stop();

    addAssetToWorldInfoClear(modelName);

    _addAssetToWorldInfoKeys.append(modelName);
    _addAssetToWorldInfoMessages.append(infoText);

    if (!_addAssetToWorldErrorTimer.isActive()) {
        if (!_addAssetToWorldMessageBox) {
            _addAssetToWorldMessageBox = getOffscreenUI()->createMessageBox(OffscreenUi::ICON_INFORMATION,
                "Downloading Model", "", QMessageBox::NoButton, QMessageBox::NoButton);
            connect(_addAssetToWorldMessageBox, SIGNAL(destroyed()), this, SLOT(onAssetToWorldMessageBoxClosed()));
        }

        _addAssetToWorldMessageBox->setProperty("text", "\n" + infoText);
        _addAssetToWorldMessageBox->setVisible(true);
    }
}

void Application::addAssetToWorldInfoClear(QString modelName) {
    // Clears modelName entry from message list without affecting message currently displayed.

    if (_aboutToQuit) {
        return;
    }

    /*
    Delete entry for modelName from list.
    */

    auto index = _addAssetToWorldInfoKeys.indexOf(modelName);
    if (index > -1) {
        _addAssetToWorldInfoKeys.removeAt(index);
        _addAssetToWorldInfoMessages.removeAt(index);
    }
}

void Application::addAssetToWorldInfoDone(QString modelName) {
    // Continues to display this message if the latest for a few seconds, then deletes it and displays the next latest.

    if (_aboutToQuit) {
        return;
    }

    /*
    Delete entry for modelName from list.
    (Re)start the info timer to update message box. ... onAddAssetToWorldInfoTimeout()
    */

    addAssetToWorldInfoClear(modelName);
    _addAssetToWorldInfoTimer.start();
}

void Application::addAssetToWorldError(QString modelName, QString errorText) {
    // Displays the most recent error message for a few seconds.

    if (_aboutToQuit) {
        return;
    }

    /*
    If list has an entry for modelName, delete it.
    Display errorText in message box.
    Show message box if not already visible.
    (Re)start error timer. ... onAddAssetToWorldErrorTimeout()
    */

    addAssetToWorldInfoClear(modelName);

    if (!_addAssetToWorldMessageBox) {
        _addAssetToWorldMessageBox = getOffscreenUI()->createMessageBox(OffscreenUi::ICON_INFORMATION,
            "Downloading Model", "", QMessageBox::NoButton, QMessageBox::NoButton);
        connect(_addAssetToWorldMessageBox, SIGNAL(destroyed()), this, SLOT(onAssetToWorldMessageBoxClosed()));
    }

    _addAssetToWorldMessageBox->setProperty("text", "\n" + errorText);
    _addAssetToWorldMessageBox->setVisible(true);

    _addAssetToWorldErrorTimer.start();
}
