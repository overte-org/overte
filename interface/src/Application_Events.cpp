//
//  Application_Events.cpp
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

#include <QtCore/QMimeData>

#include <controllers/InputRecorder.h>
#include <display-plugins/CompositorHelper.h>
#include <graphics/RenderEventHandler.h>
#include <input-plugins/KeyboardMouseDevice.h>
#include <input-plugins/TouchscreenDevice.h>
#include <input-plugins/TouchscreenVirtualPadDevice.h>
#include <MainWindow.h>
#include <OffscreenUi.h>
#include <plugins/PluginManager.h>
#include <ScriptEngines.h>
#include <scripting/Audio.h>
#include <scripting/ControllerScriptingInterface.h>
#include <shared/FileUtils.h>
#include <ui/DialogsManager.h>

#include "AudioClient.h"
#include "GLCanvas.h"
#include "Menu.h"

#if defined(Q_OS_ANDROID)
#include "AndroidHelper.h"
#endif

Q_LOGGING_CATEGORY(trace_app_input_mouse, "trace.app.input.mouse")

static const unsigned int THROTTLED_SIM_FRAMERATE = 15;
static const int THROTTLED_SIM_FRAME_PERIOD_MS = MSECS_PER_SECOND / THROTTLED_SIM_FRAMERATE;

class LambdaEvent : public QEvent {
    std::function<void()> _fun;
public:
    LambdaEvent(const std::function<void()> & fun) :
        QEvent(static_cast<QEvent::Type>(ApplicationEvent::Lambda)), _fun(fun) {}
    LambdaEvent(std::function<void()> && fun) :
        QEvent(static_cast<QEvent::Type>(ApplicationEvent::Lambda)), _fun(fun) {}
    void call() const { _fun(); }
};

bool Application::notify(QObject* object, QEvent* event) {
    if (thread() == QThread::currentThread()) {
        PROFILE_RANGE_IF_LONGER(app, "notify", 2)
        return QApplication::notify(object, event);
    }

    return QApplication::notify(object, event);
}

static inline bool isKeyEvent(QEvent::Type type) {
    return type == QEvent::KeyPress || type == QEvent::KeyRelease;
}

bool Application::event(QEvent* event) {
    if (_aboutToQuit) {
        return false;
    }

    // This helps avoid deadlock issue early during Application initialization
    if (!_isMenuInitialized) {
        return QApplication::event(event);
    }

    if (!Menu::getInstance()) {
        return false;
    }

    if ((event->type() == QEvent::InputMethod || event->type() == QEvent::InputMethodQuery) && handleInputMethodEventForFocusedEntity(event)) {
        return true;
    }

    // Allow focused Entities to handle keyboard input
    if (isKeyEvent(event->type()) && handleKeyEventForFocusedEntity(event)) {
        return true;
    }

    int type = event->type();
    switch (type) {
        case ApplicationEvent::Lambda:
            static_cast<LambdaEvent*>(event)->call();
            return true;

        // Explicit idle keeps the idle running at a lower interval, but without any rendering
        // see (windowMinimizedChanged)
        case ApplicationEvent::Idle:
            idle();

#ifdef DEBUG_EVENT_QUEUE_DEPTH
            // The event queue may very well grow beyond 400, so
            // this code should only be enabled on local builds
            {
                int count = ::hifi::qt::getEventQueueSize(QThread::currentThread());
                if (count > 400) {
                    ::hifi::qt::dumpEventQueue(QThread::currentThread());
                }
            }
#endif // DEBUG_EVENT_QUEUE_DEPTH

            _pendingIdleEvent.store(false);

            return true;

        case QEvent::MouseMove:
            mouseMoveEvent(static_cast<QMouseEvent*>(event));
            return true;
        case QEvent::MouseButtonPress:
            mousePressEvent(static_cast<QMouseEvent*>(event));
            return true;
        case QEvent::MouseButtonDblClick:
            mouseDoublePressEvent(static_cast<QMouseEvent*>(event));
            return true;
        case QEvent::MouseButtonRelease:
            mouseReleaseEvent(static_cast<QMouseEvent*>(event));
            return true;
        case QEvent::KeyPress:
            keyPressEvent(static_cast<QKeyEvent*>(event));
            return true;
        case QEvent::KeyRelease:
            keyReleaseEvent(static_cast<QKeyEvent*>(event));
            return true;
        case QEvent::FocusOut:
            focusOutEvent(static_cast<QFocusEvent*>(event));
            return true;
        case QEvent::FocusIn:
        { //testing to see if we can set focus when focus is not set to root window.
            _glWidget->activateWindow();
            _glWidget->setFocus();
            return true;
        }

        case QEvent::TouchBegin:
            touchBeginEvent(static_cast<QTouchEvent*>(event));
            event->accept();
            return true;
        case QEvent::TouchEnd:
            touchEndEvent(static_cast<QTouchEvent*>(event));
            return true;
        case QEvent::TouchUpdate:
            touchUpdateEvent(static_cast<QTouchEvent*>(event));
            return true;
        case QEvent::Gesture:
            touchGestureEvent((QGestureEvent*)event);
            return true;
        case QEvent::Wheel:
            wheelEvent(static_cast<QWheelEvent*>(event));
            return true;
        case QEvent::Drop:
            dropEvent(static_cast<QDropEvent*>(event));
            return true;

        case QEvent::FileOpen:
            if (handleFileOpenEvent(static_cast<QFileOpenEvent*>(event))) {
                return true;
            }
            break;

        default:
            break;
    }

    return QApplication::event(event);
}

bool Application::eventFilter(QObject* object, QEvent* event) {
    auto eventType = event->type();

    if (_aboutToQuit && eventType != QEvent::DeferredDelete && eventType != QEvent::Destroy) {
        return true;
    }

#if defined(Q_OS_MAC)
    // On Mac OS, Cmd+LeftClick is treated as a RightClick (more specifically, it seems to
    // be Cmd+RightClick without the modifier being dropped). Starting in Qt 5.12, only
    // on Mac, the MouseButtonRelease event for these mouse presses is sent to the top
    // level QWidgetWindow, but are not propagated further. This means that the Application
    // will see a MouseButtonPress, but no MouseButtonRelease, causing the client to get
    // stuck in "mouse-look." The cause of the problem is in the way QWidgetWindow processes
    // events where QMouseEvent::button() is not equal to QMouseEvent::buttons(). In this case
    // QMouseEvent::button() is Qt::RightButton, while QMouseEvent::buttons() is (correctly?)
    // Qt::LeftButton.
    //
    // The change here gets around this problem by capturing these
    // pseudo-RightClicks, and re-emitting them as "pure" RightClicks, where
    // QMouseEvent::button() == QMouseEvent::buttons() == Qt::RightButton.
    //
    if (eventType == QEvent::MouseButtonPress) {
        auto mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton
            && mouseEvent->buttons() == Qt::LeftButton
            && mouseEvent->modifiers() == Qt::MetaModifier) {

            QMouseEvent* newEvent = new QMouseEvent(
                    QEvent::MouseButtonPress, mouseEvent->localPos(), mouseEvent->windowPos(),
                    mouseEvent->screenPos(), Qt::RightButton, Qt::MouseButtons(Qt::RightButton),
                    mouseEvent->modifiers());
            QApplication::postEvent(object, newEvent);
            return true;
        }
    }
#endif

    if (eventType == QEvent::KeyPress || eventType == QEvent::KeyRelease || eventType == QEvent::MouseMove) {
        getRefreshRateManager().resetInactiveTimer();
    }

    if (event->type() == QEvent::Leave) {
        getApplicationCompositor().handleLeaveEvent();
    }

    if (event->type() == QEvent::ShortcutOverride) {
#if !defined(DISABLE_QML)
        if (getOffscreenUI()->shouldSwallowShortcut(event)) {
            event->accept();
            return true;
        }
#endif

        // Filter out captured keys before they're used for shortcut actions.
        if (_controllerScriptingInterface->isKeyCaptured(static_cast<QKeyEvent*>(event))) {
            event->accept();
            return true;
        }
    }

    if (event->type() == QEvent::WindowStateChange) {
        if (getWindow()->windowState() & Qt::WindowMinimized) {
            getRefreshRateManager().setRefreshRateRegime(RefreshRateManager::RefreshRateRegime::MINIMIZED);
        } else {
            auto* windowStateChangeEvent = static_cast<QWindowStateChangeEvent*>(event);
            if (windowStateChangeEvent->oldState() & Qt::WindowMinimized) {
                getRefreshRateManager().setRefreshRateRegime(RefreshRateManager::RefreshRateRegime::FOCUS_ACTIVE);
                getRefreshRateManager().resetInactiveTimer();
            }
        }
    }

    return false;
}

void Application::postLambdaEvent(const std::function<void()>& f) {
    if (this->thread() == QThread::currentThread()) {
        f();
    } else {
        QCoreApplication::postEvent(this, new LambdaEvent(f));
    }
}

void Application::sendLambdaEvent(const std::function<void()>& f) {
    if (this->thread() == QThread::currentThread()) {
        f();
    } else {
        LambdaEvent event(f);
        QCoreApplication::sendEvent(this, &event);
    }
}

void Application::pushPostUpdateLambda(void* key, const std::function<void()>& func) {
    std::unique_lock<std::mutex> guard(_postUpdateLambdasLock);
    _postUpdateLambdas[key] = func;
}

// thread-safe
void Application::onPresent(quint32 frameCount) {
    bool expected = false;
    if (_pendingIdleEvent.compare_exchange_strong(expected, true)) {
        postEvent(this, new QEvent((QEvent::Type)ApplicationEvent::Idle), Qt::HighEventPriority);
    }
    expected = false;
    if (_graphicsEngine->checkPendingRenderEvent() && !isAboutToQuit()) {
        postEvent(_graphicsEngine->_renderEventHandler, new QEvent((QEvent::Type)ApplicationEvent::Render));
    }
}

void Application::activeChanged(Qt::ApplicationState state) {
    switch (state) {
        case Qt::ApplicationActive:
            _isForeground = true;
            if (!_aboutToQuit && _startUpFinished) {
                getRefreshRateManager().setRefreshRateRegime(RefreshRateManager::RefreshRateRegime::FOCUS_ACTIVE);
            }
            break;

        case Qt::ApplicationSuspended:
            break;
        case Qt::ApplicationHidden:
            break;
        case Qt::ApplicationInactive:
            if (!_aboutToQuit && _startUpFinished) {
                getRefreshRateManager().setRefreshRateRegime(RefreshRateManager::RefreshRateRegime::UNFOCUS);
            }
            break;
        default:
            _isForeground = false;
            break;
    }
}

void Application::windowMinimizedChanged(bool minimized) {
    // initialize the _minimizedWindowTimer
    static std::once_flag once;
    std::call_once(once, [&] {
        connect(&_minimizedWindowTimer, &QTimer::timeout, this, [] {
            QCoreApplication::postEvent(QCoreApplication::instance(), new QEvent(static_cast<QEvent::Type>(Idle)), Qt::HighEventPriority);
        });
    });

    // avoid rendering to the display plugin but continue posting Idle events,
    // so that physics continues to simulate and the deadlock watchdog knows we're alive
    if (!minimized && !getActiveDisplayPlugin()->isActive()) {
        _minimizedWindowTimer.stop();
        getActiveDisplayPlugin()->activate();
    } else if (minimized && getActiveDisplayPlugin()->isActive()) {
        getActiveDisplayPlugin()->deactivate();
        _minimizedWindowTimer.start(THROTTLED_SIM_FRAME_PERIOD_MS);
    }
}

void Application::keyPressEvent(QKeyEvent* event) {
    if (!event->isAutoRepeat()) {
        _keysPressed.insert(event->key(), *event);
    }

    _controllerScriptingInterface->emitKeyPressEvent(event); // send events to any registered scripts
    // if one of our scripts have asked to capture this event, then stop processing it
    if (_controllerScriptingInterface->isKeyCaptured(event) || isInterstitialMode()) {
        return;
    }

    if (hasFocus() && getLoginDialogPoppedUp()) {
        if (_keyboardMouseDevice->isActive()) {
            _keyboardMouseDevice->keyReleaseEvent(event);
        }

        bool isControlOrCommand = event->modifiers().testFlag(Qt::ControlModifier);
        bool isOption = event->modifiers().testFlag(Qt::AltModifier);
        switch (event->key()) {
            case Qt::Key_4:
            case Qt::Key_5:
            case Qt::Key_6:
            case Qt::Key_7:
                if (isControlOrCommand || isOption) {
                    unsigned int index = static_cast<unsigned int>(event->key() - Qt::Key_1);
                    const auto& displayPlugins = PluginManager::getInstance()->getDisplayPlugins();
                    if (index < displayPlugins.size()) {
                        auto targetPlugin = displayPlugins.at(index);
                        QString targetName = targetPlugin->getName();
                        auto menu = Menu::getInstance();
                        QAction* action = menu->getActionForOption(targetName);
                        if (action && !action->isChecked()) {
                            action->trigger();
                        }
                    }
                }
                break;
        }
    } else if (hasFocus()) {
        if (_keyboardMouseDevice->isActive()) {
            _keyboardMouseDevice->keyPressEvent(event);
        }

        bool isShifted = event->modifiers().testFlag(Qt::ShiftModifier);
        bool isControlOrCommand = event->modifiers().testFlag(Qt::ControlModifier);
        bool isMetaOrMacControl = event->modifiers().testFlag(Qt::MetaModifier);
        bool isOption = event->modifiers().testFlag(Qt::AltModifier);
        switch (event->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                if (isOption) {
                    if (_window->isFullScreen()) {
                        unsetFullscreen();
                    } else {
                        setFullscreen(nullptr);
                    }
                }
                break;

            case Qt::Key_1: {
                Menu* menu = Menu::getInstance();
                menu->triggerOption(MenuOption::FirstPersonLookAt);
                break;
            }
            case Qt::Key_2: {
                Menu* menu = Menu::getInstance();
                menu->triggerOption(MenuOption::SelfieCamera);
                break;
            }
            case Qt::Key_3: {
                Menu* menu = Menu::getInstance();
                menu->triggerOption(MenuOption::LookAtCamera);
                break;
            }
            case Qt::Key_4:
            case Qt::Key_5:
            case Qt::Key_6:
            case Qt::Key_7:
                if (isControlOrCommand || isOption) {
                    unsigned int index = static_cast<unsigned int>(event->key() - Qt::Key_1);
                    const auto& displayPlugins = PluginManager::getInstance()->getDisplayPlugins();
                    if (index < displayPlugins.size()) {
                        auto targetPlugin = displayPlugins.at(index);
                        QString targetName = targetPlugin->getName();
                        auto menu = Menu::getInstance();
                        QAction* action = menu->getActionForOption(targetName);
                        if (action && !action->isChecked()) {
                            action->trigger();
                        }
                    }
                }
                break;

            case Qt::Key_G:
                if (isShifted && isControlOrCommand && isOption && isMetaOrMacControl) {
                    static const QString HIFI_FRAMES_FOLDER_VAR = "HIFI_FRAMES_FOLDER";
                    static const QString GPU_FRAME_FOLDER = QProcessEnvironment::systemEnvironment().contains(HIFI_FRAMES_FOLDER_VAR)
                        ? QProcessEnvironment::systemEnvironment().value(HIFI_FRAMES_FOLDER_VAR)
                        : "hifiFrames";
                    static QString GPU_FRAME_TEMPLATE = GPU_FRAME_FOLDER + "/{DATE}_{TIME}";
                    QString fullPath = FileUtils::computeDocumentPath(FileUtils::replaceDateTimeTokens(GPU_FRAME_TEMPLATE));
                    if (FileUtils::canCreateFile(fullPath)) {
                        getActiveDisplayPlugin()->captureFrame(fullPath.toStdString());
                    }
                }
                break;
            case Qt::Key_X:
                if (isShifted && isControlOrCommand) {
                    auto offscreenUi = getOffscreenUI();
                    offscreenUi->togglePinned();
                    //offscreenUi->getSurfaceContext()->engine()->clearComponentCache();
                    //OffscreenUi::information("Debugging", "Component cache cleared");
                    // placeholder for dialogs being converted to QML.
                }
                break;

            case Qt::Key_Y:
                if (isShifted && isControlOrCommand) {
                    getActiveDisplayPlugin()->cycleDebugOutput();
                }
                break;

            case Qt::Key_B:
                if (isOption) {
                    controller::InputRecorder* inputRecorder = controller::InputRecorder::getInstance();
                    inputRecorder->stopPlayback();
                }
                break;

            case Qt::Key_L:
                if (isShifted && isControlOrCommand) {
                    Menu::getInstance()->triggerOption(MenuOption::Log);
                } else if (isControlOrCommand) {
                    auto dialogsManager = DependencyManager::get<DialogsManager>();
                    dialogsManager->toggleAddressBar();
                }
                break;

            case Qt::Key_R:
                if (isControlOrCommand && !event->isAutoRepeat()) {
                    DependencyManager::get<ScriptEngines>()->reloadAllScripts();
                    getOffscreenUI()->clearCache();
                }
                break;

            case Qt::Key_Asterisk:
                Menu::getInstance()->triggerOption(MenuOption::DefaultSkybox);
                break;

            case Qt::Key_M:
                if (isControlOrCommand) {
                    auto audioClient = DependencyManager::get<AudioClient>();
                    audioClient->setMuted(!audioClient->isMuted());
                    QSharedPointer<scripting::Audio> audioScriptingInterface = qSharedPointerDynamicCast<scripting::Audio>(DependencyManager::get<AudioScriptingInterface>());
                    if (audioScriptingInterface && audioScriptingInterface->getPTT()) {
                       audioScriptingInterface->setPushingToTalk(!audioClient->isMuted());
                    }
                }
                break;

            case Qt::Key_S:
                if (isShifted && isControlOrCommand && !isOption) {
                    Menu::getInstance()->triggerOption(MenuOption::SuppressShortTimings);
                }
                break;

            case Qt::Key_Apostrophe: {
                if (isControlOrCommand) {
                    auto cursor = Cursor::Manager::instance().getCursor();
                    auto curIcon = cursor->getIcon();
                    if (curIcon == Cursor::Icon::DEFAULT) {
                        showCursor(Cursor::Icon::RETICLE);
                    } else if (curIcon == Cursor::Icon::RETICLE) {
                        showCursor(Cursor::Icon::SYSTEM);
                    } else if (curIcon == Cursor::Icon::SYSTEM) {
                        showCursor(Cursor::Icon::LINK);
                    } else {
                        showCursor(Cursor::Icon::DEFAULT);
                    }
                } else if (!event->isAutoRepeat()){
                    resetSensors(true);
                }
                break;
            }

            case Qt::Key_Backslash:
                Menu::getInstance()->triggerOption(MenuOption::Chat);
                break;

            case Qt::Key_Slash:
                Menu::getInstance()->triggerOption(MenuOption::Stats);
                break;

            case Qt::Key_Plus: {
                if (isControlOrCommand && event->modifiers().testFlag(Qt::KeypadModifier)) {
                    auto& cursorManager = Cursor::Manager::instance();
                    cursorManager.setScale(cursorManager.getScale() * 1.1f);
                } else {
                    getMyAvatar()->increaseSize();
                }
                break;
            }

            case Qt::Key_Minus: {
                if (isControlOrCommand && event->modifiers().testFlag(Qt::KeypadModifier)) {
                    auto& cursorManager = Cursor::Manager::instance();
                    cursorManager.setScale(cursorManager.getScale() / 1.1f);
                } else {
                    getMyAvatar()->decreaseSize();
                }
                break;
            }

            case Qt::Key_Equal:
                getMyAvatar()->resetSize();
                break;
            case Qt::Key_Escape: {
                getActiveDisplayPlugin()->abandonCalibration();
                break;
            }

            default:
                event->ignore();
                break;
        }
    }
}

void Application::keyReleaseEvent(QKeyEvent* event) {
    if (!event->isAutoRepeat()) {
        _keysPressed.remove(event->key());
    }

#if defined(Q_OS_ANDROID)
    if (event->key() == Qt::Key_Back) {
        event->accept();
        AndroidHelper::instance().requestActivity("Home", false);
    }
#endif
    _controllerScriptingInterface->emitKeyReleaseEvent(event); // send events to any registered scripts

    // if one of our scripts have asked to capture this event, then stop processing it
    if (_controllerScriptingInterface->isKeyCaptured(event)) {
        return;
    }

    if (_keyboardMouseDevice->isActive()) {
        _keyboardMouseDevice->keyReleaseEvent(event);
    }

}

void Application::focusOutEvent(QFocusEvent* event) {
    const auto& inputPlugins = PluginManager::getInstance()->getInputPlugins();
    for(const auto& inputPlugin : inputPlugins) {
        if (inputPlugin->isActive()) {
            inputPlugin->pluginFocusOutEvent();
        }
    }
// FIXME spacemouse code still needs cleanup
#if 0
    //SpacemouseDevice::getInstance().focusOutEvent();
    //SpacemouseManager::getInstance().getDevice()->focusOutEvent();
    SpacemouseManager::getInstance().ManagerFocusOutEvent();
#endif

    synthesizeKeyReleasEvents();
}

void Application::synthesizeKeyReleasEvents() {
    // synthesize events for keys currently pressed, since we may not get their release events
    // Because our key event handlers may manipulate _keysPressed, lets swap the keys pressed into a local copy,
    // clearing the existing list.
    QHash<int, QKeyEvent> keysPressed;
    std::swap(keysPressed, _keysPressed);
    for (auto& ev : keysPressed) {
        QKeyEvent synthesizedEvent { QKeyEvent::KeyRelease, ev.key(), Qt::NoModifier, ev.text() };
        keyReleaseEvent(&synthesizedEvent);
    }
}

void Application::mouseMoveEvent(QMouseEvent* event) {
    PROFILE_RANGE(app_input_mouse, __FUNCTION__);

    if (_ignoreMouseMove) {
        _ignoreMouseMove = false;
        return;
    }

    maybeToggleMenuVisible(event);

    auto& compositor = getApplicationCompositor();
    // if this is a real mouse event, and we're in HMD mode, then we should use it to move the
    // compositor reticle
    // handleRealMouseMoveEvent() will return true, if we shouldn't process the event further
    if (!compositor.fakeEventActive() && compositor.handleRealMouseMoveEvent()) {
        return; // bail
    }

#if !defined(DISABLE_QML)
    auto offscreenUi = getOffscreenUI();
    auto eventPosition = compositor.getMouseEventPosition(event);
    QPointF transformedPos = offscreenUi ? offscreenUi->mapToVirtualScreen(eventPosition) : QPointF();
#else
    QPointF transformedPos;
#endif
    auto button = event->button();
    auto buttons = event->buttons();
    // Determine if the ReticleClick Action is 1 and if so, fake include the LeftMouseButton
    if (_reticleClickPressed) {
        if (button == Qt::NoButton) {
            button = Qt::LeftButton;
        }
        buttons |= Qt::LeftButton;
    }

    QMouseEvent mappedEvent(event->type(),
        transformedPos,
        event->screenPos(), button,
        buttons, event->modifiers());

    if (compositor.getReticleVisible() || !isHMDMode() || !compositor.getReticleOverDesktop() ||
        getOverlays().getOverlayAtPoint(glm::vec2(transformedPos.x(), transformedPos.y())) != UNKNOWN_ENTITY_ID) {
        getEntities()->mouseMoveEvent(&mappedEvent);
    }

    _controllerScriptingInterface->emitMouseMoveEvent(&mappedEvent); // send events to any registered scripts

    // if one of our scripts have asked to capture this event, then stop processing it
    if (_controllerScriptingInterface->isMouseCaptured()) {
        return;
    }

    if (_keyboardMouseDevice->isActive()) {
        _keyboardMouseDevice->mouseMoveEvent(event, _captureMouse, _mouseCaptureTarget);
    }
}

void Application::mousePressEvent(QMouseEvent* event) {
#if !defined(DISABLE_QML)
    auto offscreenUi = getOffscreenUI();
    // If we get a mouse press event it means it wasn't consumed by the offscreen UI,
    // hence, we should defocus all of the offscreen UI windows, in order to allow
    // keyboard shortcuts not to be swallowed by them.  In particular, WebEngineViews
    // will consume all keyboard events.
    offscreenUi->unfocusWindows();

    auto eventPosition = getApplicationCompositor().getMouseEventPosition(event);
    QPointF transformedPos = offscreenUi->mapToVirtualScreen(eventPosition);
#else
    QPointF transformedPos;
#endif

    QMouseEvent mappedEvent(event->type(), transformedPos, event->screenPos(), event->button(), event->buttons(), event->modifiers());
    QUuid result = getEntities()->mousePressEvent(&mappedEvent);
    setKeyboardFocusEntity(getEntities()->wantsKeyboardFocus(result) ? result : UNKNOWN_ENTITY_ID);

    _controllerScriptingInterface->emitMousePressEvent(&mappedEvent); // send events to any registered scripts

    // if one of our scripts have asked to capture this event, then stop processing it
    if (_controllerScriptingInterface->isMouseCaptured()) {
        return;
    }

#if defined(Q_OS_MAC)
    // Fix for OSX right click dragging on window when coming from a native window
    bool isFocussed = hasFocus();
    if (!isFocussed && event->button() == Qt::MouseButton::RightButton) {
        setFocus();
        isFocussed = true;
    }

    if (isFocussed) {
#else
    if (hasFocus()) {
#endif
        if (_keyboardMouseDevice->isActive()) {
            _keyboardMouseDevice->mousePressEvent(event);
        }
    }
}

void Application::mouseDoublePressEvent(QMouseEvent* event) {
#if !defined(DISABLE_QML)
    auto offscreenUi = getOffscreenUI();
    auto eventPosition = getApplicationCompositor().getMouseEventPosition(event);
    QPointF transformedPos = offscreenUi->mapToVirtualScreen(eventPosition);
#else
    QPointF transformedPos;
#endif
    QMouseEvent mappedEvent(event->type(),
        transformedPos,
        event->screenPos(), event->button(),
        event->buttons(), event->modifiers());
    getEntities()->mouseDoublePressEvent(&mappedEvent);

    // if one of our scripts have asked to capture this event, then stop processing it
    if (_controllerScriptingInterface->isMouseCaptured()) {
        return;
    }

    _controllerScriptingInterface->emitMouseDoublePressEvent(event);
}

void Application::mouseReleaseEvent(QMouseEvent* event) {
#if !defined(DISABLE_QML)
    auto offscreenUi = getOffscreenUI();
    auto eventPosition = getApplicationCompositor().getMouseEventPosition(event);
    QPointF transformedPos = offscreenUi->mapToVirtualScreen(eventPosition);
#else
    QPointF transformedPos;
#endif
    QMouseEvent mappedEvent(event->type(),
        transformedPos,
        event->screenPos(), event->button(),
        event->buttons(), event->modifiers());

    getEntities()->mouseReleaseEvent(&mappedEvent);

    _controllerScriptingInterface->emitMouseReleaseEvent(&mappedEvent); // send events to any registered scripts

    // if one of our scripts have asked to capture this event, then stop processing it
    if (_controllerScriptingInterface->isMouseCaptured()) {
        return;
    }

    if (hasFocus()) {
        if (_keyboardMouseDevice->isActive()) {
            _keyboardMouseDevice->mouseReleaseEvent(event);
        }
    }
}

void Application::touchBeginEvent(QTouchEvent* event) {
    TouchEvent thisEvent(*event); // on touch begin, we don't compare to last event
    _controllerScriptingInterface->emitTouchBeginEvent(thisEvent); // send events to any registered scripts

    _lastTouchEvent = thisEvent; // and we reset our last event to this event before we call our update
    touchUpdateEvent(event);

    // if one of our scripts have asked to capture this event, then stop processing it
    if (_controllerScriptingInterface->isTouchCaptured()) {
        return;
    }

    if (_keyboardMouseDevice->isActive()) {
        _keyboardMouseDevice->touchBeginEvent(event);
    }
    if (_touchscreenDevice && _touchscreenDevice->isActive()) {
        _touchscreenDevice->touchBeginEvent(event);
    }
    if (_touchscreenVirtualPadDevice && _touchscreenVirtualPadDevice->isActive()) {
        _touchscreenVirtualPadDevice->touchBeginEvent(event);
    }

}

void Application::touchEndEvent(QTouchEvent* event) {
    TouchEvent thisEvent(*event, _lastTouchEvent);
    _controllerScriptingInterface->emitTouchEndEvent(thisEvent); // send events to any registered scripts
    _lastTouchEvent = thisEvent;

    // if one of our scripts have asked to capture this event, then stop processing it
    if (_controllerScriptingInterface->isTouchCaptured()) {
        return;
    }

    if (_keyboardMouseDevice->isActive()) {
        _keyboardMouseDevice->touchEndEvent(event);
    }
    if (_touchscreenDevice && _touchscreenDevice->isActive()) {
        _touchscreenDevice->touchEndEvent(event);
    }
    if (_touchscreenVirtualPadDevice && _touchscreenVirtualPadDevice->isActive()) {
        _touchscreenVirtualPadDevice->touchEndEvent(event);
    }
    // put any application specific touch behavior below here..
}

void Application::touchUpdateEvent(QTouchEvent* event) {
    if (event->type() == QEvent::TouchUpdate) {
        TouchEvent thisEvent(*event, _lastTouchEvent);
        _controllerScriptingInterface->emitTouchUpdateEvent(thisEvent); // send events to any registered scripts
        _lastTouchEvent = thisEvent;
    }

    // if one of our scripts have asked to capture this event, then stop processing it
    if (_controllerScriptingInterface->isTouchCaptured()) {
        return;
    }

    if (_keyboardMouseDevice->isActive()) {
        _keyboardMouseDevice->touchUpdateEvent(event);
    }
    if (_touchscreenDevice && _touchscreenDevice->isActive()) {
        _touchscreenDevice->touchUpdateEvent(event);
    }
    if (_touchscreenVirtualPadDevice && _touchscreenVirtualPadDevice->isActive()) {
        _touchscreenVirtualPadDevice->touchUpdateEvent(event);
    }
}

void Application::touchGestureEvent(QGestureEvent* event) {
    if (_keyboardMouseDevice->isActive()) {
        _keyboardMouseDevice->touchGestureEvent(event);
    }
    if (_touchscreenDevice && _touchscreenDevice->isActive()) {
        _touchscreenDevice->touchGestureEvent(event);
    }
    if (_touchscreenVirtualPadDevice && _touchscreenVirtualPadDevice->isActive()) {
        _touchscreenVirtualPadDevice->touchGestureEvent(event);
    }
}

void Application::wheelEvent(QWheelEvent* event) const {
    _controllerScriptingInterface->emitWheelEvent(event); // send events to any registered scripts

    // if one of our scripts have asked to capture this event, then stop processing it
    if (_controllerScriptingInterface->isWheelCaptured() || getLoginDialogPoppedUp()) {
        return;
    }

    if (_keyboardMouseDevice->isActive()) {
        _keyboardMouseDevice->wheelEvent(event);
    }
}

void Application::dropEvent(QDropEvent *event) {
    const QMimeData* mimeData = event->mimeData();
    for (auto& url : mimeData->urls()) {
        QString urlString = url.toString();
        if (acceptURL(urlString, true)) {
            event->acceptProposedAction();
        }
    }
}

bool Application::handleInputMethodEventForFocusedEntity(QEvent* event) {
    if (_keyboardFocusedEntity.get() != UNKNOWN_ENTITY_ID) {
        switch (event->type()) {
            case QEvent::InputMethod:
            case QEvent::InputMethodQuery:
            {
                auto eventHandler = getEntities()->getEventHandler(_keyboardFocusedEntity.get());
                if (eventHandler) {
                    event->setAccepted(false);
                    QCoreApplication::sendEvent(eventHandler, event);
                    if (event->isAccepted()) {
                        _lastAcceptedKeyPress = usecTimestampNow();
                        return true;
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    return false;
}

bool Application::handleKeyEventForFocusedEntity(QEvent* event) {
    if (_keyboardFocusedEntity.get() != UNKNOWN_ENTITY_ID) {
        switch (event->type()) {
            case QEvent::KeyPress:
            case QEvent::KeyRelease:
                {
                    auto eventHandler = getEntities()->getEventHandler(_keyboardFocusedEntity.get());
                    if (eventHandler) {
                        event->setAccepted(false);
                        QCoreApplication::sendEvent(eventHandler, event);
                        if (event->isAccepted()) {
                            _lastAcceptedKeyPress = usecTimestampNow();
                            return true;
                        }
                    }
                    break;
                }
            default:
                break;
        }
    }

    return false;
}

bool Application::handleFileOpenEvent(QFileOpenEvent* fileEvent) {
    QUrl url = fileEvent->url();
    if (!url.isEmpty()) {
        QString urlString = url.toString();
        if (canAcceptURL(urlString)) {
            return acceptURL(urlString);
        }
    }
    return false;
}
