//
//  Application.h
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

#ifndef hifi_Application_h
#define hifi_Application_h

#include <QtWidgets/QApplication>

#include <AbstractScriptingServicesInterface.h>
#include <AbstractUriHandler.h>
#include <AbstractViewStateInterface.h>
#include <AvatarHashMap.h>
#include <EntityEditPacketSender.h>
#include <ModerationFlags.h>
#include <OctreeQuery.h>
#include <PhysicsEngine.h>
#include <ShapeManager.h>
#include <TouchEvent.h>
#include <ui-plugins/PluginContainer.h>

#include "avatar/MyAvatar.h"
#include "ConnectionMonitor.h"
#include "CursorManager.h"
#include "FancyCamera.h"
#include "graphics/GraphicsEngine.h"
#include "LoginStateManager.h"
#include "octree/OctreePacketProcessor.h"
#include "PerformanceManager.h"
#include "RefreshRateManager.h"
#include "scripting/DialogsManagerScriptingInterface.h"
#include "ui/ApplicationOverlay.h"
#include "ui/OverlayConductor.h"
#include "ui/overlays/Overlays.h"
#include "VisionSqueeze.h"
#include "workload/GameWorkload.h"

class ArchiveDownloadInterface;
class AudioInjector;
class CompositorHelper;
class ControllerScriptingInterface;
class DiscordPresence;
class EntityScriptServerLogDialog;
class FileLogger;
class GLCanvas;
class KeyboardMouseDevice;
class LogDialog;
class MainWindow;
class ModalDialogListener;
class OffscreenUi;
class TouchscreenDevice;
class TouchscreenVirtualPadDevice;
class QCommandLineParser;
class QGestureEvent;
class QQmlContext;
class QQuickItem;

namespace controller {
    class StateController;
}

class Application;
#if defined(qApp)
#undef qApp
#endif
#define qApp (static_cast<Application*>(QCoreApplication::instance()))

class Application : public QApplication,
                    public AbstractViewStateInterface,
                    public AbstractScriptingServicesInterface,
                    public AbstractUriHandler,
                    public PluginContainer
{
    Q_OBJECT

public:
    Application(
        int& argc, char** argv,
        const QCommandLineParser& parser,
        QElapsedTimer& startup_time
    );
    ~Application();

    /**
     * @brief Initialize everything
     *
     * This is a QApplication, and for Qt reasons it's desirable to create this object
     * as early as possible. Without that some Qt functions don't work, like QCoreApplication::applicationDirPath()
     *
     * So we keep the constructor as minimal as possible, and do the rest of the work in
     * this function.
     */
    void initialize(const QCommandLineParser &parser);

    MainWindow* getWindow() const { return _window; }
    virtual QThread* getMainThread() override { return thread(); }

    bool isAboutToQuit() const { return _aboutToQuit; }
    bool isServerlessMode() const;
    bool isInterstitialMode() const { return _interstitialMode; }
    bool failedToConnectToEntityServer() const { return _failedToConnectToEntityServer; }

    void setPreviousSessionCrashed(bool value) { _previousSessionCrashed = value; }

    // Return an HTTP User-Agent string with OS and device information.
    Q_INVOKABLE QString getUserAgent();

    PerformanceManager& getPerformanceManager() { return _performanceManager; }
    RefreshRateManager& getRefreshRateManager() { return _refreshRateManager; }
    float getGameLoopRate() const { return _gameLoopCounter.rate(); }

#ifndef Q_OS_ANDROID
    FileLogger* getLogger() const { return _logger; }
#endif

    QString getPreviousScriptLocation() { return _previousScriptLocation.get(); }
    void setPreviousScriptLocation(const QString& previousScriptLocation) { _previousScriptLocation.set(previousScriptLocation); }

    void replaceDomainContent(const QString& url, const QString& itemName);

    void openDirectory(const QString& path);

    void overrideEntry() { _overrideEntry = true; }
    void forceDisplayName(const QString& displayName) { getMyAvatar()->setDisplayName(displayName); }
    void forceLoginWithTokens(const QString& tokens);
    void setConfigFileURL(const QString& fileUrl);

    void loadAvatarScripts(const QVector<QString>& urls);
    void unloadAvatarScripts();

    Q_INVOKABLE void copyToClipboard(const QString& text);

    Q_INVOKABLE void setMinimumGPUTextureMemStabilityCount(int stabilityCount) { _minimumGPUTextureMemSizeStabilityCount = stabilityCount; }

    virtual ControllerScriptingInterface* getControllerScriptingInterface() { return _controllerScriptingInterface; }
    virtual void registerScriptEngineWithApplicationServices(ScriptManagerPointer& scriptManager) override;

    // used by preferences and HMDScriptingInterface...
    VisionSqueeze& getVisionSqueeze() { return _visionSqueeze; }


    // UI
    virtual ui::Menu* getPrimaryMenu() override;
    virtual void showDisplayPluginsTools(bool show) override;
    virtual GLWidget* getPrimaryWidget() override;
    virtual MainWindow* getPrimaryWindow() override;
    virtual QOpenGLContext* getPrimaryContext() override;
    virtual bool isForeground() const override;

    bool hasFocus() const;
    void setFocus();
    void raise();

    void showCursor(const Cursor::Icon& cursor);

    Overlays& getOverlays() { return _overlays; }
    ApplicationOverlay& getApplicationOverlay() { return *_applicationOverlay; }
    const ApplicationOverlay& getApplicationOverlay() const { return *_applicationOverlay; }
    CompositorHelper& getApplicationCompositor() const;

    virtual PickRay computePickRay(float x, float y) const override;

    static void setupQmlSurface(QQmlContext* surfaceContext, bool setAdditionalContextProperties);

    float getHMDTabletScale() { return _hmdTabletScale.get(); }
    void setHMDTabletScale(float hmdTabletScale) { _hmdTabletScale.set(hmdTabletScale); }
    float getDesktopTabletScale() { return _desktopTabletScale.get(); }
    void setDesktopTabletScale(float desktopTabletScale) { _desktopTabletScale.set(desktopTabletScale); }

    bool getDesktopTabletBecomesToolbarSetting() { return _desktopTabletBecomesToolbarSetting.get(); }
    void setDesktopTabletBecomesToolbarSetting(bool value);
    bool getHmdTabletBecomesToolbarSetting() { return _hmdTabletBecomesToolbarSetting.get(); }
    void setHmdTabletBecomesToolbarSetting(bool value);

    bool getLogWindowOnTopSetting() { return _keepLogWindowOnTop.get(); }
    void setLogWindowOnTopSetting(bool keepOnTop) { _keepLogWindowOnTop.set(keepOnTop); }
    bool getPreferStylusOverLaser() { return _preferStylusOverLaserSetting.get(); }
    void setPreferStylusOverLaser(bool value) { _preferStylusOverLaserSetting.set(value); }
    bool getPreferAvatarFingerOverStylus() { return _preferAvatarFingerOverStylusSetting.get(); }
    void setPreferAvatarFingerOverStylus(bool value) { _preferAvatarFingerOverStylusSetting.set(value); }
    
    void setMouseCaptureVR(bool value);
    bool getMouseCaptureVR();

    bool getMiniTabletEnabled() { return _miniTabletEnabledSetting.get(); }
    void setMiniTabletEnabled(bool enabled);

    float getSettingConstrainToolbarPosition() { return _constrainToolbarPosition.get(); }
    void setSettingConstrainToolbarPosition(bool setting);

    float getAwayStateWhenFocusLostInVREnabled() { return _awayStateWhenFocusLostInVREnabled.get(); }
    void setAwayStateWhenFocusLostInVREnabled(bool setting);

    QUuid getTabletScreenID() const;
    QUuid getTabletHomeButtonID() const;
    QUuid getTabletFrameID() const;
    QVector<QUuid> getTabletIDs() const;

    void confirmConnectWithoutAvatarEntities();

    bool getLoginDialogPoppedUp() const { return _loginDialogPoppedUp; }
    void createLoginDialog();
    void updateLoginDialogPosition();

    void createAvatarInputsBar();
    void destroyAvatarInputsBar();


    // Plugins
    /**
     * @brief Initialize the plugin manager
     *
     * This both does the initial startup and parses arguments. This
     * is necessary because the plugin manager's options must be set
     * before any usage of it is made, or they won't apply.
     *
     * @param parser
     */
    void initializePluginManager(const QCommandLineParser& parser);

    virtual void requestReset() override { resetSensors(false); }
    virtual bool makeRenderingContextCurrent() override { return true; }

    static void shutdownPlugins();

    void initializeDisplayPlugins();
    virtual DisplayPluginPointer getActiveDisplayPlugin() const override;
    void setActiveDisplayPlugin(const QString& pluginName);

    glm::uvec2 getUiSize() const;
    QRect getRecommendedHUDRect() const;
    glm::vec2 getDeviceSize() const;
    bool isThrottleRendering() const;
    float getTargetRenderFrameRate() const;  // frames/second

    // Check if a headset is connected
    bool hasRiftControllers();
    bool hasViveControllers();

    // the isHMDMode is true whenever we use the interface from an HMD and not a standard flat display
    // rendering of several elements depend on that
    // TODO: carry that information on the Camera as a setting
    virtual bool isHMDMode() const override;
    glm::mat4 getHMDSensorPose() const;
    glm::mat4 getEyeOffset(int eye) const;
    glm::mat4 getEyeProjection(int eye) const;


    // Graphics
    void initializeGL();
    void initializeRenderEngine();
    void initializeUi();

    void resizeGL();

    glm::uvec2 getCanvasSize() const;
    float getRenderResolutionScale() const;

    render::ScenePointer getMain3DScene() override { return _graphicsEngine->getRenderScene(); }
    render::EnginePointer getRenderEngine() override { return _graphicsEngine->getRenderEngine(); }
    gpu::ContextPointer getGPUContext() const { return _graphicsEngine->getGPUContext(); }
    float getRenderLoopRate() const { return _graphicsEngine->getRenderLoopRate(); }


    // Events
    bool notify(QObject*, QEvent*) override;
    bool event(QEvent* event) override;
    bool eventFilter(QObject* object, QEvent* event) override;

    void postLambdaEvent(const std::function<void()>& f) override;
    void sendLambdaEvent(const std::function<void()>& f) override;
    virtual void pushPostUpdateLambda(void* key, const std::function<void()>& func) override;


    // Camera
    Camera& getCamera() { return _myCamera; }
    const Camera& getCamera() const { return _myCamera; }
    // Represents the current view frustum of the avatar.
    void copyViewFrustum(ViewFrustum& viewOut) const;
    // Represents the view frustum of the current rendering pass,
    // which might be different from the viewFrustum, i.e. shadowmap
    // passes, mirror window passes, etc
    void copyDisplayViewFrustum(ViewFrustum& viewOut) const;

    void updateCamera(RenderArgs& renderArgs, float deltaTime);
    void updateSecondaryCameraViewFrustum();

    const ConicalViewFrustums& getConicalViews() const override { return _conicalViews; }

    float getFieldOfView() { return _fieldOfView.get(); }
    void setFieldOfView(float fov);

    bool getCameraClippingEnabled() { return _cameraClippingEnabled.get(); }
    void setCameraClippingEnabled(bool enabled);

    void updateMyAvatarLookAtPosition(float deltaTime);


    // Entities
    QSharedPointer<EntityTreeRenderer> getEntities() const { return DependencyManager::get<EntityTreeRenderer>(); }
    EntityTreePointer getEntityClipboard() const { return _entityClipboard; }
    const OctreePacketProcessor& getOctreePacketProcessor() const { return *_octreeProcessor; }
    std::shared_ptr<EntityEditPacketSender> getEntityEditPacketSender() { return _entityEditSender; }

    void setMaxOctreePacketsPerSecond(int maxOctreePPS);
    int getMaxOctreePacketsPerSecond() const { return _maxOctreePPS; }
    bool isMissingSequenceNumbers() { return _isMissingSequenceNumbers; }

    NodeToOctreeSceneStats* getOcteeSceneStats() { return _octreeProcessor->getOctreeSceneStats(); }


    // Assets
    virtual bool canAcceptURL(const QString& url) const override;
    virtual bool acceptURL(const QString& url, bool defaultUpload = false) override;


    // Physics
    bool isPhysicsEnabled() const { return _physicsEnabled; }
    PhysicsEnginePointer getPhysicsEngine() { return _physicsEngine; }
    const GameWorkload& getGameWorkload() const { return _gameWorkload; }

    float getNumCollisionObjects() const { return _physicsEngine ? _physicsEngine->getNumCollisionObjects() : 0; }
    void saveNextPhysicsStats(QString filename) { _physicsEngine->saveNextPhysicsStats(filename); }


    // Avatar
    virtual glm::vec3 getAvatarPosition() const override { return getMyAvatar()->getWorldPosition(); }

    void clearAvatarOverrideUrl() { _avatarOverrideUrl = QUrl(); _saveAvatarOverrideUrl = false; }
    QUrl getAvatarOverrideUrl() { return _avatarOverrideUrl; }
    bool getSaveAvatarOverrideUrl() { return _saveAvatarOverrideUrl; }

    int getOtherAvatarsReplicaCount() { return DependencyManager::get<AvatarHashMap>()->getReplicaCount(); }
    void setOtherAvatarsReplicaCount(int count) { DependencyManager::get<AvatarHashMap>()->setReplicaCount(count); }


    // Snapshots
    using SnapshotOperator = std::tuple<std::function<void(const QImage&)>, float, bool>;
    void addSnapshotOperator(const SnapshotOperator& snapshotOperator);
    bool takeSnapshotOperators(std::queue<SnapshotOperator>& snapshotOperators);

    void takeSnapshot(bool notify, bool includeAnimated = false, float aspectRatio = 0.0f, const QString& filename = QString());
    void takeSecondaryCameraSnapshot(const bool& notify, const QString& filename = QString());
    void takeSecondaryCamera360Snapshot(const glm::vec3& cameraPosition,
                                        const bool& cubemapOutputFormat,
                                        const bool& notify,
                                        const QString& filename = QString());
    void shareSnapshot(const QString& filename, const QUrl& href = QUrl(""));


#if defined(Q_OS_ANDROID)
    void beforeEnterBackground();
    void enterBackground();
    void enterForeground();
    void toggleAwayMode();
#endif

signals:
    void svoImportRequested(const QString& url);

    void fullAvatarURLChanged(const QString& newValue, const QString& modelName);

    void beforeAboutToQuit();
    void activeDisplayPluginChanged();

    void uploadRequest(QString path);

    void interstitialModeChanged(bool isInInterstitialMode);

    void loginDialogFocusEnabled();
    void loginDialogFocusDisabled();

    void miniTabletEnabledChanged(bool enabled);
    void awayStateWhenFocusLostInVRChanged(bool enabled);

    void darkThemePreferenceChanged(bool useDarkTheme);

public slots:
    void updateThreadPoolCount() const;

    static void gotoTutorial();
    void goToErrorDomainURL(QUrl errorDomainURL);
    void handleLocalServerConnection() const;
    void readArgumentsFromLocalSocket() const;
    void showUrlHandler(const QUrl& url);

#if (PR_BUILD || DEV_BUILD)
    void sendWrongProtocolVersionsSignature(bool checked) { ::sendWrongProtocolVersionsSignature(checked); }
#endif

    void hmdVisibleChanged(bool visible);

    void reloadResourceCaches();

    void updateHeartbeat() const;

    static void deadlockApplication();
    static void unresponsiveApplication();  // cause main thread to be unresponsive for 35 seconds
    void crashOnShutdown(); // used to test "shutdown" crash annotation.

    void rotationModeChanged() const;

    void setIsServerlessMode(bool serverlessDomain);
    std::map<QString, QString> prepareServerlessDomainContents(QUrl domainURL, QByteArray data);

    void loadServerlessDomain(QUrl domainURL);
    void loadErrorDomain(QUrl domainURL);
    void setIsInterstitialMode(bool interstitialMode);

    void updateVerboseLogging();

    void setCachebustRequire();

    QString getGraphicsCardType();
    bool gpuTextureMemSizeStable();

    Q_INVOKABLE SharedSoundPointer getSampleSound() const { return _sampleSound; }

    static void runTests();

    // UI
    void showDialog(const QUrl& widgetUrl, const QUrl& tabletUrl, const QString& name) const;
    Q_INVOKABLE void loadDialog();
    Q_INVOKABLE void loadScriptURLDialog() const;
    void toggleLogDialog();
    void recreateLogWindow(int);
    void toggleEntityScriptServerLogDialog();
    Q_INVOKABLE void showAssetServerWidget(QString filePath = "");
    Q_INVOKABLE void loadAddAvatarBookmarkDialog() const;
    Q_INVOKABLE void loadAvatarBrowser() const;

    void showLoginScreen();
    static void showHelp();

    void updateSystemTabletMode();

    void captureMouseChanged(bool captureMouse);
    void toggleOverlays();
    void setOverlaysVisible(bool visible);
    Q_INVOKABLE void centerUI() { _overlayConductor.centerUI(); }

    void addAssetToWorldMessageClose();

    void loadLODToolsDialog();
    void loadEntityStatisticsDialog();
    void loadDomainConnectionDialog();
    void showScriptLogs();

    const QString getPreferredCursor() const { return _preferredCursor.get(); }
    void setPreferredCursor(const QString& cursor);

    bool getDarkThemePreference() const { return _darkTheme.get(); }
    void setDarkThemePreference(bool value);

    /**
     * @brief Shows/hides VR keyboard input for Overlay windows
     *
     * This is used by QML scripts to show and hide VR keyboard. Unlike JS API Keyboard.raised = true,
     * with showVRKeyboardForHudUI the input is passed to the active window on the overlay first.
     *
     * @param show
     * If set to true, then keyboard is shown, for false it's hidden.
     */
    Q_INVOKABLE void showVRKeyboardForHudUI(bool show);


    // Plugins
    void resetSensors(bool andReload = false);


    // Entities
    QVector<EntityItemID> pasteEntities(const QString& entityHostType, float x, float y, float z);
    bool exportEntities(const QString& filename, const QVector<QUuid>& entityIDs, const glm::vec3* givenOffset = nullptr);
    bool exportEntities(const QString& filename, float x, float y, float z, float scale);
    bool importEntities(const QString& url, const bool isObservable = true, const qint64 callerId = -1);

    void setKeyboardFocusHighlight(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& dimensions);
    QUuid getKeyboardFocusEntity() const { return _keyboardFocusedEntity.get(); }  // thread-safe
    void setKeyboardFocusEntity(const QUuid& id);


    // Assets
    void addAssetToWorldFromURL(QString url);
    void addAssetToWorldFromURLRequestFinished();
    void addAssetToWorld(QString filePath, QString zipFile, bool isZip = false);
    void addAssetToWorldUnzipFailure(QString filePath);
    void addAssetToWorldWithNewMapping(QString filePath, QString mapping, int copy, bool isZip = false);
    void addAssetToWorldUpload(QString filePath, QString mapping, bool isZip = false);
    void addAssetToWorldSetMapping(QString filePath, QString mapping, QString hash, bool isZip = false);
    void addAssetToWorldAddEntity(QString filePath, QString mapping);

    void handleUnzip(QString sourceFile, QStringList destinationFile, bool autoAdd, bool isZip);

    ArchiveDownloadInterface* getFileDownloadInterface() { return _fileDownload; }

    static void packageModel();


    // Camera
    void cycleCamera();
    void cameraModeChanged();
    void cameraMenuChanged();

    void changeViewAsNeeded(float boomLength);


    // Physics
    void resetPhysicsReadyInformation();


private slots:
    void onAboutToQuit();

    void loadSettings(const QCommandLineParser& parser);
    void saveSettings() const;
    void setFailedToConnectToEntityServer() { _failedToConnectToEntityServer = true; }

    bool acceptSnapshot(const QString& urlString);

    void setSessionUUID(const QUuid& sessionUUID) const;

    void domainURLChanged(QUrl domainURL);
    void domainConnectionRefused(const QString& reasonMessage, int reason, const QString& extraInfo);

    void updateWindowTitle() const;
    void nodeAdded(SharedNodePointer node);
    void nodeActivated(SharedNodePointer node);
    void nodeKilled(SharedNodePointer node);

    void handleSandboxStatus(QNetworkReply* reply);


    // UI
    void onDesktopRootItemCreated(QQuickItem* qmlContext);
    void onDesktopRootContextCreated(QQmlContext* qmlContext);
    void showDesktop();

    void notifyPacketVersionMismatch();

    bool askToSetAvatarUrl(const QString& url);
    bool askToLoadScript(const QString& scriptFilenameOrURL);
    bool askToReplaceDomainContent(const QString& url);

    void onDismissedLoginDialog();


    // Entities
    void clearDomainOctreeDetails(bool clearAll = true);
    void resettingDomain();


    // Events
    void onPresent(quint32 frameCount);

    void activeChanged(Qt::ApplicationState state);
    void windowMinimizedChanged(bool minimized);


    // Plugins
    void updateDisplayMode();
    void switchDisplayMode();
    void setDisplayPlugin(DisplayPluginPointer newPlugin);


    // Assets
    void addAssetToWorldCheckModelSize();
    void onAssetToWorldMessageBoxClosed();
    void addAssetToWorldInfoTimeout();
    void addAssetToWorldErrorTimeout();


    // Physics
    void setShowBulletWireframe(bool value) { _physicsEngine->setShowBulletWireframe(value); }
    void setShowBulletAABBs(bool value) { _physicsEngine->setShowBulletAABBs(value); }
    void setShowBulletContactPoints(bool value) { _physicsEngine->setShowBulletContactPoints(value); }
    void setShowBulletConstraints(bool value) { _physicsEngine->setShowBulletConstraints(value); }
    void setShowBulletConstraintLimits(bool value) { _physicsEngine->setShowBulletConstraintLimits(value); }

    void setShowTrackedObjects(bool value) { _showTrackedObjects = value; }

private:
    void cleanupBeforeQuit();

    void idle();
    void update(float deltaTime);
    void updateLOD(float deltaTime) const;
    void updateThreads(float deltaTime);

    void userKickConfirmation(const QUuid& nodeID, unsigned int banFlags = ModerationFlags::getDefaultBanFlags());


    // Setup
    void init();
    void setupSignalsAndOperators();


    // UI
    bool initMenu();
    void pauseUntilLoginDetermined();
    void resumeAfterLoginDialogActionTaken();

    static QSharedPointer<OffscreenUi> getOffscreenUI();

    void updateDialogs(float deltaTime) const;

    void maybeToggleMenuVisible(QMouseEvent* event) const;
    void toggleTabletUI(bool shouldOpen = false) const;

    bool shouldCaptureMouse() const;
    void checkChangeCursor();

    void addAssetToWorldInfo(QString modelName, QString infoText);
    void addAssetToWorldInfoClear(QString modelName);
    void addAssetToWorldInfoDone(QString modelName);
    void addAssetToWorldError(QString modelName, QString errorText);


    // Events
    void resizeEvent(QResizeEvent* size) { resizeGL(); }

    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);

    void focusOutEvent(QFocusEvent* event);
    void synthesizeKeyReleasEvents();

    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseDoublePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    void touchBeginEvent(QTouchEvent* event);
    void touchEndEvent(QTouchEvent* event);
    void touchUpdateEvent(QTouchEvent* event);
    void touchGestureEvent(QGestureEvent* event);

    void wheelEvent(QWheelEvent* event) const;
    void dropEvent(QDropEvent* event);
    static void dragEnterEvent(QDragEnterEvent* event) { event->acceptProposedAction(); }

    bool handleInputMethodEventForFocusedEntity(QEvent* event);
    bool handleKeyEventForFocusedEntity(QEvent* event);
    bool handleFileOpenEvent(QFileOpenEvent* event);

    void processDriverBlocklistReply(const QString& fullDriverToTest, const QString& os, const QString& vendor, const QString& renderer, const QString& api,
        const QString& driver);


    // Entities
    void queryOctree(NodeType_t serverType, PacketType packetType);

    int sendNackPackets();


    // Graphics
    void updateRenderArgs(float deltaTime);


    // Plugins
    void startHMDStandBySession();
    void endHMDSession();


    // Assets
    bool importJSONFromURL(const QString& urlString);
    bool importSVOFromURL(const QString& urlString);
    bool importFromZIP(const QString& filePath);
    bool importImage(const QString& urlString);


    // Avatars
    std::shared_ptr<MyAvatar> getMyAvatar() const;
    void checkSkeleton() const;

    void queryAvatars();


    // Physics
    void tryToEnablePhysics();


    // Member Variables
    // The window needs to be initialized early as other initializers try to access it
    MainWindow* _window;
    // _isMenuInitialized: used to initialize menu early enough before it's needed by other
    // initializers. Fixes a deadlock issue with recent Qt versions.
    bool _isMenuInitialized;

    bool _startUpFinished { false };
    bool _quitWhenFinished { false };
    bool _crashOnShutdown { false };
    bool _aboutToQuit { false };
    bool _previousSessionCrashed { false };

    QUrl _urlParam;
    bool _overrideEntry { false };
    bool _settingsLoaded { false };

    bool _domainLoadingInProgress { false };
    bool _interstitialMode { false };
    bool _interstitialModeEnabled { false };

    PerformanceManager _performanceManager;
    RefreshRateManager _refreshRateManager;
    GameWorkload _gameWorkload;

    ConnectionMonitor _connectionMonitor;

#ifndef Q_OS_ANDROID
    FileLogger* _logger { nullptr };
#endif

    QElapsedTimer& _sessionRunTimer;
    QElapsedTimer _lastTimeUpdated;
    QTimer _locationUpdateTimer;
    QTimer _minimizedWindowTimer;
    // Frame Rate Measurement
    RateCounter<500> _gameLoopCounter;

    using SteadyClock = std::chrono::steady_clock;
    using TimePoint = SteadyClock::time_point;
    TimePoint _queryExpiry;

    quint64 _lastNackTime;
    quint64 _lastSendDownstreamAudioStats;

    bool _notifiedPacketVersionMismatchThisDomain { false };

    QDir _defaultScriptsLocation;
    // If above is only set by parameter, below is unnecessary.
    bool _overrideDefaultScriptsLocation;

    qint64 _gpuTextureMemSizeStabilityCount { 0 };
    qint64 _gpuTextureMemSizeAtLastCheck { 0 };
    int _minimumGPUTextureMemSizeStabilityCount { 30 };

    SharedSoundPointer _sampleSound { nullptr };

    VisionSqueeze _visionSqueeze;

    DiscordPresence* _discordPresence { nullptr };

    Setting::Handle<bool> _firstRun;
    Setting::Handle<QString> _previousScriptLocation;


    // UI
    GLCanvas* _glWidget { nullptr };

    Overlays _overlays;
    std::shared_ptr<ApplicationOverlay> _applicationOverlay;
    OverlayConductor _overlayConductor;

    mutable QRecursiveMutex _changeCursorLock;
    Qt::CursorShape _desiredCursor { Qt::BlankCursor };
    bool _cursorNeedsChanging { false };
    bool _useSystemCursor { false };

    DialogsManagerScriptingInterface* _dialogsManagerScriptingInterface;

    QPointer<LogDialog> _logDialog;
    QPointer<EntityScriptServerLogDialog> _entityScriptServerLogDialog;
    ModalDialogListener* _confirmConnectWithoutAvatarEntitiesDialog { nullptr };

    QUuid _loginDialogID;
    QUuid _avatarInputsBarID;
    LoginStateManager _loginStateManager;

    QQuickItem* _addAssetToWorldMessageBox { nullptr };
    QStringList _addAssetToWorldInfoKeys;  // Model name
    QStringList _addAssetToWorldInfoMessages;  // Info message
    QTimer _addAssetToWorldInfoTimer;
    QTimer _addAssetToWorldErrorTimer;

    ArchiveDownloadInterface* _fileDownload;

    bool _loginDialogPoppedUp { false };
    bool _developerMenuVisible { false };
    bool _noLoginSuggestion { false };

    bool _resumeAfterLoginDialogActionTaken_WasPostponed { false };
    bool _resumeAfterLoginDialogActionTaken_SafeToRun { false };

    bool _isForeground { true }; // starts out assumed to be in foreground

    Setting::Handle<float> _hmdTabletScale;
    Setting::Handle<float> _desktopTabletScale;
    Setting::Handle<bool> _desktopTabletBecomesToolbarSetting;
    Setting::Handle<bool> _hmdTabletBecomesToolbarSetting;
    Setting::Handle<bool> _preferStylusOverLaserSetting;
    Setting::Handle<bool> _preferAvatarFingerOverStylusSetting;
    Setting::Handle<bool> _defaultMouseCaptureVR;
    Setting::Handle<bool> _constrainToolbarPosition;
    Setting::Handle<bool> _awayStateWhenFocusLostInVREnabled;
    Setting::Handle<QString> _preferredCursor;
    // TODO Qt6: Qt5 doesn't have anything for system theme preferences, Qt6.5+ does
    Setting::Handle<bool> _darkTheme;
    Setting::Handle<bool> _miniTabletEnabledSetting;
    Setting::Handle<bool> _keepLogWindowOnTop { "keepLogWindowOnTop", false };

    void updateThemeColors();


    // Plugins
    DisplayPluginPointer _displayPlugin;
    mutable std::mutex _displayPluginLock;

    std::shared_ptr<controller::StateController> _applicationStateDevice; // Default ApplicationDevice reflecting the state of different properties of the session
    std::shared_ptr<KeyboardMouseDevice> _keyboardMouseDevice;   // Default input device, the good old keyboard mouse and maybe touchpad
    std::shared_ptr<TouchscreenDevice> _touchscreenDevice;   // the good old touchscreen
    std::shared_ptr<TouchscreenVirtualPadDevice> _touchscreenVirtualPadDevice;
    bool _keyboardDeviceHasFocus { true };

    ControllerScriptingInterface* _controllerScriptingInterface { nullptr };

    DisplayPluginPointer _autoSwitchDisplayModeSupportedHMDPlugin { nullptr };
    QString _autoSwitchDisplayModeSupportedHMDPluginName;
    bool _previousHMDWornStatus { false };


    // Events
    QHash<int, QKeyEvent> _keysPressed;
    TouchEvent _lastTouchEvent;
    quint64 _lastAcceptedKeyPress { 0 };

    ThreadSafeValueCache<EntityItemID> _keyboardFocusedEntity;

    bool _reticleClickPressed { false };
    bool _keyboardFocusWaitingOnRenderable { false };

    bool _captureMouse { false };
    bool _ignoreMouseMove { false };
    QPointF _mouseCaptureTarget { NAN, NAN };

    std::map<void*, std::function<void()>> _postUpdateLambdas;
    std::mutex _postUpdateLambdasLock;

    std::atomic<bool> _pendingIdleEvent { true };


    // Entities
    EntityTreePointer _entityClipboard;

    std::shared_ptr<OctreePacketProcessor> _octreeProcessor;
    std::shared_ptr<EntityEditPacketSender> _entityEditSender;
    OctreeQuery _octreeQuery { true };  // NodeData derived class for querying octee cells from octree servers
    bool _enableProcessOctreeThread { true };

    uint32_t _fullSceneCounterAtLastPhysicsCheck { 0 };     // _fullSceneReceivedCounter last time we checked physics ready

    mutable QTimer _entityServerConnectionTimer;

    QUuid _keyboardFocusHighlightID;

    bool _failedToConnectToEntityServer { false };
    bool _isMissingSequenceNumbers { false };

    Setting::Handle<int> _maxOctreePacketsPerSecond;
    int _maxOctreePPS { DEFAULT_MAX_OCTREE_PPS };


    // Assets
    typedef bool (Application::*AcceptURLMethod)(const QString&);
    static const std::vector<std::pair<QString, AcceptURLMethod>> _acceptedExtensions;

    QTimer _addAssetToWorldResizeTimer;
    QHash<QUuid, int> _addAssetToWorldResizeList;


    // Camera
    FancyCamera _myCamera;  // My view onto the world

    CameraMode _previousCameraMode;
    glm::vec3 _thirdPersonHMDCameraBoom { 0.0f, 0.0f, -1.0f };
    bool _thirdPersonHMDCameraBoomValid { true };
    float _scaleMirror { 1.0f };
    float _mirrorYawOffset { 0.0f };
    float _raiseMirror { 0.0f };

    mutable QRecursiveMutex _viewMutex;
    ViewFrustum _viewFrustum;  // current state of view frustum, perspective, orientation, etc.
    ViewFrustum _displayViewFrustum;

    ConicalViewFrustums _conicalViews;
    ConicalViewFrustums _lastQueriedViews;  // last views used to query servers

    Setting::Handle<float> _fieldOfView;
    Setting::Handle<float> _cameraClippingEnabled;

    bool _prevCameraClippingEnabled { false };
    unsigned int _cameraClippingRayPickID;


    // Graphics
    std::shared_ptr<GraphicsEngine> _graphicsEngine;
    glm::uvec2 _renderResolution;

    Setting::Handle<QString> _prevCheckedDriver { "prevCheckedDriver", "" };
    bool _isGLInitialized { false };


    // Avatars
    QString _previousAvatarSkeletonModel;
    float _previousAvatarTargetScale;

    QUrl _avatarOverrideUrl;
    bool _saveAvatarOverrideUrl { false };


    // Physics
    ShapeManager _shapeManager;
    PhysicalEntitySimulationPointer _entitySimulation;
    PhysicsEnginePointer _physicsEngine;

    bool _physicsEnabled { false };
    // This is needed so that physics do not get re-enabled before safe landing starts when moving from
    // serverless to domain server.
    // It's set to true by Application::clearDomainOctreeData and is cleared by Application::setIsServerlessMode
    bool _waitForServerlessToBeSet { true };

    bool _showTrackedObjects { false };
    bool _prevShowTrackedObjects { false };


    // Snapshots
    std::mutex _snapshotMutex;
    std::queue<SnapshotOperator> _snapshotOperators;
    bool _hasPrimarySnapshot { false };
};
#endif // hifi_Application_h
