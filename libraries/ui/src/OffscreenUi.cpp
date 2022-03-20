//
//  OffscreenUi.cpp
//  interface/src/render-utils
//
//  Created by Bradley Austin Davis on 2015-04-04
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "OffscreenUi.h"

#include <QtCore/QVariant>
#include <QtGui/QGuiApplication>
#include <QtQuick/QQuickWindow>
#include <QtQml/QtQml>

#include <shared/QtHelpers.h>
#include <gl/GLHelpers.h>

#include <AbstractUriHandler.h>
#include <AccountManager.h>
#include <DependencyManager.h>

#include "ui/TabletScriptingInterface.h"
#include "FileDialogHelper.h"
#include "VrMenu.h"

#include "ui/Logging.h"
#include "ui/ToolbarScriptingInterface.h"

#include <PointerManager.h>
#include "MainWindow.h"

/*@jsdoc
 * The <code>OffscreenFlags</code> API enables gamepad joystick navigation of UI.
 *
 * <p><em>This API currently has no effect and is not used.</em></p>
 *
 * @namespace OffscreenFlags
 *
 * @hifi-interface
 * @hifi-client-entity
 * @hifi-avatar
 *
 * @property {boolean} navigationFocused - <code>true</code> if UI has joystick navigation focus, <code>false</code> if it
 *     doesn't.
 * @property {boolean} navigationFocusDisabled - <code>true</code> if UI joystick navigation focus is disabled,
 *     <code>false</code> if it isn't.
 */

// Needs to match the constants in resources/qml/Global.js
class OffscreenFlags : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool navigationFocused READ isNavigationFocused WRITE setNavigationFocused NOTIFY navigationFocusedChanged)

    // Allow scripts that are doing their own navigation support to disable navigation focus (i.e. handControllerPointer.js)
    Q_PROPERTY(bool navigationFocusDisabled READ isNavigationFocusDisabled WRITE setNavigationFocusDisabled NOTIFY navigationFocusDisabledChanged)

public:

    OffscreenFlags(QObject* parent = nullptr) : QObject(parent) {}
    bool isNavigationFocused() const { return _navigationFocused; }
    void setNavigationFocused(bool focused) {
        if (_navigationFocused != focused) {
            _navigationFocused = focused;
            emit navigationFocusedChanged();
        }
    }

    bool isNavigationFocusDisabled() const { return _navigationFocusDisabled; }
    void setNavigationFocusDisabled(bool disabled) {
        if (_navigationFocusDisabled != disabled) {
            _navigationFocusDisabled = disabled;
            emit navigationFocusDisabledChanged();
        }
    }

signals:

    /*@jsdoc
     * Triggered when the value of the <code>navigationFocused</code> property changes.
     * @function OffscreenFlags.navigationFocusedChanged
     * @returns {Signal}
     */
    void navigationFocusedChanged();

    /*@jsdoc
     * Triggered when the value of the <code>navigationFocusDisabled</code> property changes.
     * @function OffscreenFlags.navigationFocusDisabledChanged
     * @returns {Signal}
     */
    void navigationFocusDisabledChanged();

private:
    bool _navigationFocused { false };
    bool _navigationFocusDisabled{ false };
};

static OffscreenFlags* offscreenFlags { nullptr };

// This hack allows the QML UI to work with keys that are also bound as
// shortcuts at the application level.  However, it seems as though the
// bound actions are still getting triggered.  At least for backspace.
// Not sure why.
//
// However, the problem may go away once we switch to the new menu system,
// so I think it's OK for the time being.
bool OffscreenUi::shouldSwallowShortcut(QEvent* event) {
    Q_ASSERT(event->type() == QEvent::ShortcutOverride);
    auto window = getWindow();
    if (window) {
        QObject* focusObject = getWindow()->focusObject();
        if (focusObject != getWindow() && focusObject != getRootItem()) {
            event->accept();
            return true;
        }
    }
    return false;
}

static QTouchDevice _touchDevice;
OffscreenUi::OffscreenUi() {
    static std::once_flag once;
    std::call_once(once, [&] {
        _touchDevice.setCapabilities(QTouchDevice::Position);
        _touchDevice.setType(QTouchDevice::TouchScreen);
        _touchDevice.setName("OffscreenUiTouchDevice");
        _touchDevice.setMaximumTouchPoints(4);
    });

    auto pointerManager = DependencyManager::get<PointerManager>();
    connect(pointerManager.data(), &PointerManager::hoverBeginHUD, this, &OffscreenUi::hoverBeginEvent);
    connect(pointerManager.data(), &PointerManager::hoverContinueHUD, this, &OffscreenUi::handlePointerEvent);
    connect(pointerManager.data(), &PointerManager::hoverEndHUD, this, &OffscreenUi::hoverEndEvent);
    connect(pointerManager.data(), &PointerManager::triggerBeginHUD, this, &OffscreenUi::handlePointerEvent);
    connect(pointerManager.data(), &PointerManager::triggerContinueHUD, this, &OffscreenUi::handlePointerEvent);
    connect(pointerManager.data(), &PointerManager::triggerEndHUD, this, &OffscreenUi::handlePointerEvent);
}

void OffscreenUi::hoverBeginEvent(const PointerEvent& event) {
    OffscreenQmlSurface::hoverBeginEvent(event, _touchDevice);
}

void OffscreenUi::hoverEndEvent(const PointerEvent& event) {
    OffscreenQmlSurface::hoverEndEvent(event, _touchDevice);
}

void OffscreenUi::handlePointerEvent(const PointerEvent& event) {
    OffscreenQmlSurface::handlePointerEvent(event, _touchDevice);
}

QObject* OffscreenUi::getFlags() {
    return offscreenFlags;
}

void OffscreenUi::removeModalDialog(QObject* modal) {
    if (modal) {
        _modalDialogListeners.removeOne(modal);
        modal->deleteLater();
    }
}

void OffscreenUi::onRootContextCreated(QQmlContext* qmlContext) {
    OffscreenQmlSurface::onRootContextCreated(qmlContext);
    qmlContext->setContextProperty("OffscreenUi", this);
    qmlContext->setContextProperty("offscreenFlags", offscreenFlags = new OffscreenFlags());
    qmlContext->setContextProperty("fileDialogHelper", new FileDialogHelper());
#ifdef DEBUG
    qmlContext->setContextProperty("DebugQML", QVariant(true));
#else
    qmlContext->setContextProperty("DebugQML", QVariant(false));
#endif

}

void OffscreenUi::show(const QUrl& url, const QString& name, std::function<void(QQmlContext*, QObject*)> f) {
    emit showDesktop();
    QQuickItem* item = getRootItem()->findChild<QQuickItem*>(name);
    // First load?
    if (!item) {
        load(url, f);
        item = getRootItem()->findChild<QQuickItem*>(name);
    }

    if (item) {
        QQmlProperty(item, OFFSCREEN_VISIBILITY_PROPERTY).write(true);
    }
}

void OffscreenUi::hideDesktopWindows() {
    if (_desktop) {
        if (QThread::currentThread() != thread()) {
            BLOCKING_INVOKE_METHOD(this, "hideDesktopWindows");
        }
        QMetaObject::invokeMethod(_desktop, "hideDesktopWindows");
    }
}

void OffscreenUi::toggle(const QUrl& url, const QString& name, std::function<void(QQmlContext*, QObject*)> f) {
    QQuickItem* item = getRootItem()->findChild<QQuickItem*>(name);
    if (!item) {
        show(url, name, f);
        return;
    }

    // Already loaded, so just flip the bit
    QQmlProperty shownProperty(item, OFFSCREEN_VISIBILITY_PROPERTY);
    shownProperty.write(!shownProperty.read().toBool());
}

bool OffscreenUi::isPointOnDesktopWindow(QVariant point) {
    if (_desktop) {
        QVariant result;
        BLOCKING_INVOKE_METHOD(_desktop, "isPointOnWindow",
                               Q_RETURN_ARG(QVariant, result),
                               Q_ARG(QVariant, point));
        return result.toBool();
    }
    return false;
}

void OffscreenUi::hide(const QString& name) {
    auto rootItem = getRootItem();
    if (rootItem) {
        QQuickItem* item = rootItem->findChild<QQuickItem*>(name);
        if (item) {
            QQmlProperty(item, OFFSCREEN_VISIBILITY_PROPERTY).write(false);
        }
    }
}

bool OffscreenUi::isVisible(const QString& name) {
    auto rootItem = getRootItem();
    if (rootItem) {
        QQuickItem* item = rootItem->findChild<QQuickItem*>(name);
        if (item) {
            return QQmlProperty(item, OFFSCREEN_VISIBILITY_PROPERTY).read().toBool();
        }
    }
    return false;
}

class MessageBoxListener : public ModalDialogListener {
    Q_OBJECT

    friend class OffscreenUi;
    MessageBoxListener(QQuickItem* messageBox) : ModalDialogListener(messageBox) {
        if (_finished) {
            return;
        }
        connect(_dialog, SIGNAL(selected(int)), this, SLOT(onSelected(int)));
    }

    virtual QMessageBox::StandardButton waitForButtonResult() {
        ModalDialogListener::waitForResult();
        return static_cast<QMessageBox::StandardButton>(_result.toInt());
    }

protected slots:
    virtual void onDestroyed() override {
        ModalDialogListener::onDestroyed();
        onSelected(QMessageBox::NoButton);
    }

private slots:
    void onSelected(int button) {
        _result = button;
        _finished = true;
        auto offscreenUi = DependencyManager::get<OffscreenUi>();
        emit response(_result);
        if (!offscreenUi.isNull()) {
            offscreenUi->removeModalDialog(qobject_cast<QObject*>(this));
        }
        disconnect(_dialog);
    }
};

QQuickItem* OffscreenUi::createMessageBox(Icon icon, const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton) {
    QVariantMap map;
    map.insert("title", title);
    map.insert("text", text);
    map.insert("icon", icon);
    map.insert("buttons", buttons.operator int());
    map.insert("defaultButton", defaultButton);
    QVariant result;
    bool invokeResult = false;
    auto tabletScriptingInterface = DependencyManager::get<TabletScriptingInterface>();
    TabletProxy* tablet = dynamic_cast<TabletProxy*>(tabletScriptingInterface->getTablet("com.highfidelity.interface.tablet.system"));
    if (tablet->getToolbarMode() && _desktop) {
       invokeResult =  QMetaObject::invokeMethod(_desktop, "messageBox",
                                  Q_RETURN_ARG(QVariant, result),
                                  Q_ARG(QVariant, QVariant::fromValue(map)));
    } else if (QQuickItem* tabletRoot = tablet->getTabletRoot()) {
        invokeResult =  QMetaObject::invokeMethod(tabletRoot, "messageBox",
                                                  Q_RETURN_ARG(QVariant, result),
                                                  Q_ARG(QVariant, QVariant::fromValue(map)));
        emit tabletScriptingInterface->tabletNotification();
    }

    if (!invokeResult) {
        qWarning() << "Failed to create message box";
        return nullptr;
    }
    return qvariant_cast<QQuickItem*>(result);
}

int OffscreenUi::waitForMessageBoxResult(QQuickItem* messageBox) {
    if (!messageBox) {
        return QMessageBox::NoButton;
    }

    return MessageBoxListener(messageBox).waitForButtonResult();
}


QMessageBox::StandardButton OffscreenUi::messageBox(Icon icon, const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton) {
    if (QThread::currentThread() != thread()) {
        QMessageBox::StandardButton result = QMessageBox::StandardButton::NoButton;
        BLOCKING_INVOKE_METHOD(this, "messageBox",
            Q_RETURN_ARG(QMessageBox::StandardButton, result),
            Q_ARG(Icon, icon),
            Q_ARG(QString, title),
            Q_ARG(QString, text),
            Q_ARG(QMessageBox::StandardButtons, buttons),
            Q_ARG(QMessageBox::StandardButton, defaultButton));
        return result;
    }

    return static_cast<QMessageBox::StandardButton>(waitForMessageBoxResult(createMessageBox(icon, title, text, buttons, defaultButton)));
}

ModalDialogListener* OffscreenUi::asyncMessageBox(Icon icon, const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton) {
    if (QThread::currentThread() != thread()) {
        ModalDialogListener* ret;
        BLOCKING_INVOKE_METHOD(this, "asyncMessageBox",
                               Q_RETURN_ARG(ModalDialogListener*, ret),
                               Q_ARG(Icon, icon),
                               Q_ARG(QString, title),
                               Q_ARG(QString, text),
                               Q_ARG(QMessageBox::StandardButtons, buttons),
                               Q_ARG(QMessageBox::StandardButton, defaultButton));
        return ret;
    }

    MessageBoxListener* messageBoxListener = new MessageBoxListener(createMessageBox(icon, title, text, buttons, defaultButton));
    QObject* modalDialog = qobject_cast<QObject*>(messageBoxListener);
    _modalDialogListeners.push_back(modalDialog);
    return messageBoxListener;
}

QMessageBox::StandardButton OffscreenUi::critical(const QString& title, const QString& text,
    QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton) {
    return DependencyManager::get<OffscreenUi>()->messageBox(OffscreenUi::Icon::ICON_CRITICAL, title, text, buttons, defaultButton);
}
QMessageBox::StandardButton OffscreenUi::information(const QString& title, const QString& text,
    QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton) {
    return DependencyManager::get<OffscreenUi>()->messageBox(OffscreenUi::Icon::ICON_INFORMATION, title, text, buttons, defaultButton);
}

ModalDialogListener* OffscreenUi::asyncCritical(const QString& title, const QString& text,
    QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton) {
    return DependencyManager::get<OffscreenUi>()->asyncMessageBox(OffscreenUi::Icon::ICON_CRITICAL, title, text, buttons, defaultButton);
}

ModalDialogListener* OffscreenUi::asyncInformation(const QString& title, const QString& text,
    QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton) {
    return DependencyManager::get<OffscreenUi>()->asyncMessageBox(OffscreenUi::Icon::ICON_INFORMATION, title, text, buttons, defaultButton);
}

QMessageBox::StandardButton OffscreenUi::question(const QString& title, const QString& text,
    QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton) {
    return DependencyManager::get<OffscreenUi>()->messageBox(OffscreenUi::Icon::ICON_QUESTION, title, text, buttons, defaultButton);
}

ModalDialogListener *OffscreenUi::asyncQuestion(const QString& title, const QString& text,
    QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton) {
    return DependencyManager::get<OffscreenUi>()->asyncMessageBox(OffscreenUi::Icon::ICON_QUESTION, title, text, buttons, defaultButton);
}

QMessageBox::StandardButton OffscreenUi::warning(const QString& title, const QString& text,
    QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton) {
    return DependencyManager::get<OffscreenUi>()->messageBox(OffscreenUi::Icon::ICON_WARNING, title, text, buttons, defaultButton);
}

ModalDialogListener* OffscreenUi::asyncWarning(const QString& title, const QString& text,
    QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton) {
    return DependencyManager::get<OffscreenUi>()->asyncMessageBox(OffscreenUi::Icon::ICON_WARNING, title, text, buttons, defaultButton);
}


class InputDialogListener : public ModalDialogListener {
    Q_OBJECT

    friend class OffscreenUi;
    InputDialogListener(QQuickItem* queryBox) : ModalDialogListener(queryBox) {
        if (_finished) {
            return;
        }
        connect(_dialog, SIGNAL(selected(QVariant)), this, SLOT(onSelected(const QVariant&)));
        connect(_dialog, SIGNAL(canceled()), this, SLOT(onSelected()));
    }

private slots:
    void onSelected(const QVariant& result = "") {
        _result = result;
        auto offscreenUi = DependencyManager::get<OffscreenUi>();
        emit response(_result);
        offscreenUi->removeModalDialog(qobject_cast<QObject*>(this));
        _finished = true;
        disconnect(_dialog);
    }
};

QString OffscreenUi::getText(const Icon icon, const QString& title, const QString& label, const QString& text, bool* ok) {
    if (ok) { *ok = false; }
    QVariant result = DependencyManager::get<OffscreenUi>()->inputDialog(icon, title, label, text).toString();
    if (ok && result.isValid()) {
        *ok = true;
    }
    return result.toString();
}

QString OffscreenUi::getItem(const Icon icon, const QString& title, const QString& label, const QStringList& items,
    int current, bool editable, bool* ok) {

    if (ok) {
        *ok = false;
    }

    auto offscreenUi = DependencyManager::get<OffscreenUi>();
    auto inputDialog = offscreenUi->createInputDialog(icon, title, label, current);
    if (!inputDialog) {
        return QString();
    }
    inputDialog->setProperty("items", items);
    inputDialog->setProperty("editable", editable);

    QVariant result = offscreenUi->waitForInputDialogResult(inputDialog);
    if (!result.isValid()) {
        return QString();
    }

    if (ok) {
        *ok = true;
    }
    return result.toString();
}

ModalDialogListener* OffscreenUi::getTextAsync(const Icon icon, const QString& title, const QString& label, const QString& text) {
    return DependencyManager::get<OffscreenUi>()->inputDialogAsync(icon, title, label, text);
}

ModalDialogListener* OffscreenUi::getItemAsync(const Icon icon, const QString& title, const QString& label, const QStringList& items,
    int current, bool editable) {

    auto offscreenUi = DependencyManager::get<OffscreenUi>();
    auto inputDialog = offscreenUi->createInputDialog(icon, title, label, current);
    if (!inputDialog) {
        return nullptr;
    }
    inputDialog->setProperty("items", items);
    inputDialog->setProperty("editable", editable);

    InputDialogListener* inputDialogListener = new InputDialogListener(inputDialog);
    offscreenUi->getModalDialogListeners().push_back(qobject_cast<QObject*>(inputDialogListener));

    return inputDialogListener;
}

QVariant OffscreenUi::inputDialog(const Icon icon, const QString& title, const QString& label, const QVariant& current) {
    if (QThread::currentThread() != thread()) {
        QVariant result;
        BLOCKING_INVOKE_METHOD(this, "inputDialog",
            Q_RETURN_ARG(QVariant, result),
            Q_ARG(Icon, icon),
            Q_ARG(QString, title),
            Q_ARG(QString, label),
            Q_ARG(QVariant, current));
        return result;
    }

    return waitForInputDialogResult(createInputDialog(icon, title, label, current));
}

ModalDialogListener* OffscreenUi::inputDialogAsync(const Icon icon, const QString& title, const QString& label, const QVariant& current) {
    if (QThread::currentThread() != thread()) {
        ModalDialogListener* ret;
        BLOCKING_INVOKE_METHOD(this, "inputDialogAsync",
            Q_RETURN_ARG(ModalDialogListener*, ret),
            Q_ARG(Icon, icon),
            Q_ARG(QString, title),
            Q_ARG(QString, label),
            Q_ARG(QVariant, current));
        return ret;
    }

    InputDialogListener* inputDialogListener = new InputDialogListener(createInputDialog(icon, title, label, current));
    QObject* inputDialog = qobject_cast<QObject*>(inputDialogListener);
    _modalDialogListeners.push_back(inputDialog);
    return inputDialogListener;
}

QVariant OffscreenUi::customInputDialog(const Icon icon, const QString& title, const QVariantMap& config) {
    if (QThread::currentThread() != thread()) {
        QVariant result;
        BLOCKING_INVOKE_METHOD(this, "customInputDialog",
                                  Q_RETURN_ARG(QVariant, result),
                                  Q_ARG(Icon, icon),
                                  Q_ARG(QString, title),
                                  Q_ARG(QVariantMap, config));
        return result;
    }

    QVariant result = waitForInputDialogResult(createCustomInputDialog(icon, title, config));
    if (result.isValid()) {
        // We get a JSON encoded result, so we unpack it into a QVariant wrapping a QVariantMap
        result = QVariant(QJsonDocument::fromJson(result.toString().toUtf8()).object().toVariantMap());
    }

    return result;
}

ModalDialogListener* OffscreenUi::customInputDialogAsync(const Icon icon, const QString& title, const QVariantMap& config) {
    if (QThread::currentThread() != thread()) {
        ModalDialogListener* ret;
        BLOCKING_INVOKE_METHOD(this, "customInputDialogAsync",
                               Q_RETURN_ARG(ModalDialogListener*, ret),
                               Q_ARG(Icon, icon),
                               Q_ARG(QString, title),
                               Q_ARG(QVariantMap, config));
        return ret;
    }

    InputDialogListener* inputDialogListener = new InputDialogListener(createCustomInputDialog(icon, title, config));
    QObject* inputDialog = qobject_cast<QObject*>(inputDialogListener);
    _modalDialogListeners.push_back(inputDialog);
    return inputDialogListener;
}

void OffscreenUi::togglePinned() {
    bool invokeResult = _desktop && QMetaObject::invokeMethod(_desktop, "togglePinned");
    if (!invokeResult) {
        qWarning() << "Failed to toggle window visibility";
    }
}

void OffscreenUi::setPinned(bool pinned) {
    bool invokeResult = _desktop && QMetaObject::invokeMethod(_desktop, "setPinned", Q_ARG(QVariant, pinned));
    if (!invokeResult) {
        qWarning() << "Failed to set window visibility";
    }
}

void OffscreenUi::setConstrainToolbarToCenterX(bool constrained) {
    bool invokeResult = _desktop && QMetaObject::invokeMethod(_desktop, "setConstrainToolbarToCenterX", Q_ARG(QVariant, constrained));
    if (!invokeResult) {
        qWarning() << "Failed to set toolbar constraint";
    }
}

void OffscreenUi::addMenuInitializer(std::function<void(VrMenu*)> f) {
    if (!_vrMenu) {
        _queuedMenuInitializers.push_back(f);
        return;
    }
    f(_vrMenu);
}

QQuickItem* OffscreenUi::createInputDialog(const Icon icon, const QString& title, const QString& label,
    const QVariant& current) {

    QVariantMap map;
    map.insert("title", title);
    map.insert("icon", icon);
    map.insert("label", label);
    map.insert("current", current);
    QVariant result;

    auto tabletScriptingInterface = DependencyManager::get<TabletScriptingInterface>();
    TabletProxy* tablet = dynamic_cast<TabletProxy*>(tabletScriptingInterface->getTablet("com.highfidelity.interface.tablet.system"));

    bool invokeResult = false;
    if (tablet->getToolbarMode() && _desktop) {
        invokeResult = QMetaObject::invokeMethod(_desktop, "inputDialog",
                                                 Q_RETURN_ARG(QVariant, result),
                                                 Q_ARG(QVariant, QVariant::fromValue(map)));
    } else if (QQuickItem* tabletRoot = tablet->getTabletRoot()) {
        invokeResult = QMetaObject::invokeMethod(tabletRoot, "inputDialog",
                                                 Q_RETURN_ARG(QVariant, result),
                                                 Q_ARG(QVariant, QVariant::fromValue(map)));
        emit tabletScriptingInterface->tabletNotification();
    }

    if (!invokeResult) {
        qWarning() << "Failed to create message box";
        return nullptr;
    }

    return qvariant_cast<QQuickItem*>(result);
}

QQuickItem* OffscreenUi::createCustomInputDialog(const Icon icon, const QString& title, const QVariantMap& config) {
    QVariantMap map = config;
    map.insert("title", title);
    map.insert("icon", icon);
    QVariant result;
    auto tabletScriptingInterface = DependencyManager::get<TabletScriptingInterface>();
    TabletProxy* tablet = dynamic_cast<TabletProxy*>(tabletScriptingInterface->getTablet("com.highfidelity.interface.tablet.system"));

    bool invokeResult = false;
    if (tablet->getToolbarMode() && _desktop) {
        invokeResult = QMetaObject::invokeMethod(_desktop, "inputDialog",
                                                 Q_RETURN_ARG(QVariant, result),
                                                 Q_ARG(QVariant, QVariant::fromValue(map)));
    } else if (QQuickItem* tabletRoot = tablet->getTabletRoot()) {
        invokeResult = QMetaObject::invokeMethod(tabletRoot, "inputDialog",
                                                 Q_RETURN_ARG(QVariant, result),
                                                 Q_ARG(QVariant, QVariant::fromValue(map)));
        emit tabletScriptingInterface->tabletNotification();
    }

    if (!invokeResult) {
        qWarning() << "Failed to create custom message box";
        return nullptr;
    }

    return qvariant_cast<QQuickItem*>(result);
}

QVariant OffscreenUi::waitForInputDialogResult(QQuickItem* inputDialog) {
    if (!inputDialog) {
        return QVariant();
    }
    return InputDialogListener(inputDialog).waitForResult();
}

bool OffscreenUi::navigationFocused() {
    return !offscreenFlags->isNavigationFocusDisabled() && offscreenFlags->isNavigationFocused();
}

void OffscreenUi::setNavigationFocused(bool focused) {
    if (offscreenFlags) {
        offscreenFlags->setNavigationFocused(focused);
    }
}

// FIXME HACK....
// This hack is an attempt to work around the 'offscreen UI can't gain keyboard focus' bug
// https://app.asana.com/0/27650181942747/83176475832393
// The problem seems related to https://bugreports.qt.io/browse/QTBUG-50309
//
// The workaround seems to be to give some other window (same process or another process doesn't seem to matter)
// focus and then put focus back on the interface main window.
//
// If I could reliably reproduce this bug I could eventually track down what state change is occuring
// during the process of the main window losing and then gaining focus, but failing that, here's a
// brute force way of triggering that state change at application start in a way that should be nearly
// imperceptible to the user.
class KeyboardFocusHack : public QObject {
    Q_OBJECT
public:
    KeyboardFocusHack() {
        Q_ASSERT(_mainWindow);
        QTimer::singleShot(200, [=] {
            _window = new QWindow();
            _window->setFlags(Qt::FramelessWindowHint);
            _window->setGeometry(_mainWindow->x(), _mainWindow->y(), 10, 10);
            _window->show();
            _window->requestActivate();
            QTimer::singleShot(200, [=] {
                _window->hide();
                _window->deleteLater();
                _window = nullptr;
                _mainWindow->requestActivate();
                emit keyboardFocusActive();
                this->deleteLater();
            });
        });
    }

signals:
    void keyboardFocusActive();

private:
    QWindow* const _mainWindow { MainWindow::findMainWindow() };
    QWindow* _window { nullptr };
};

void OffscreenUi::createDesktop(const QUrl& url) {
    if (_desktop) {
        qCDebug(uiLogging) << "Desktop already created";
        return;
    }

    load(url, [=](QQmlContext* context, QObject* newObject) {
        Q_UNUSED(context)
        _desktop = static_cast<QQuickItem*>(newObject);
        getSurfaceContext()->setContextProperty("desktop", _desktop);

        _vrMenu = new VrMenu(this);
        for (const auto& menuInitializer : _queuedMenuInitializers) {
            menuInitializer(_vrMenu);
        }


        auto toolbarScriptingInterface = DependencyManager::get<ToolbarScriptingInterface>();
        connect(_desktop, SIGNAL(toolbarVisibleChanged(bool, QString)), toolbarScriptingInterface.data(), SIGNAL(toolbarVisibleChanged(bool, QString)));

        auto keyboardFocus = new KeyboardFocusHack();
        connect(_desktop, SIGNAL(showDesktop()), this, SIGNAL(showDesktop()));
        emit desktopReady();
        connect(keyboardFocus, SIGNAL(keyboardFocusActive()), this, SIGNAL(keyboardFocusActive()));
    });
}

QQuickItem* OffscreenUi::getDesktop() {
    return _desktop;
}

QObject* OffscreenUi::getRootMenu() {
    return getRootItem()->findChild<QObject*>("rootMenu");
}

void OffscreenUi::unfocusWindows() {
    bool invokeResult = _desktop && QMetaObject::invokeMethod(_desktop, "unfocusWindows");
    Q_ASSERT(invokeResult);
}


class FileDialogListener : public ModalDialogListener {
    Q_OBJECT

    friend class OffscreenUi;
    FileDialogListener(QQuickItem* messageBox) : ModalDialogListener(messageBox) {
        if (_finished) {
            return;
        }
        connect(_dialog, SIGNAL(selectedFile(QVariant)), this, SLOT(onSelectedFile(QVariant)));
        connect(_dialog, SIGNAL(canceled()), this, SLOT(onSelectedFile()));
    }

private slots:
    void onSelectedFile(QVariant file = "") {
        _result = file.toUrl().toLocalFile();
        _finished = true;
        auto offscreenUi = DependencyManager::get<OffscreenUi>();
        emit response(_result);
        offscreenUi->removeModalDialog(qobject_cast<QObject*>(this));
        disconnect(_dialog);
    }
};


QString OffscreenUi::fileDialog(const QVariantMap& properties) {
    QVariant buildDialogResult;
    bool invokeResult = false;
    auto tabletScriptingInterface = DependencyManager::get<TabletScriptingInterface>();
    TabletProxy* tablet = dynamic_cast<TabletProxy*>(tabletScriptingInterface->getTablet("com.highfidelity.interface.tablet.system"));
    if (tablet->getToolbarMode() && _desktop) {
       invokeResult =  QMetaObject::invokeMethod(_desktop, "fileDialog",
                                  Q_RETURN_ARG(QVariant, buildDialogResult),
                                  Q_ARG(QVariant, QVariant::fromValue(properties)));
    } else if (QQuickItem* tabletRoot = tablet->getTabletRoot()) {
        invokeResult =  QMetaObject::invokeMethod(tabletRoot, "fileDialog",
                                  Q_RETURN_ARG(QVariant, buildDialogResult),
                                  Q_ARG(QVariant, QVariant::fromValue(properties)));
        emit tabletScriptingInterface->tabletNotification();
    }

    if (!invokeResult) {
        qWarning() << "Failed to create file open dialog";
        return QString();
    }

    QVariant result = FileDialogListener(qvariant_cast<QQuickItem*>(buildDialogResult)).waitForResult();
    if (!result.isValid()) {
        return QString();
    }
    qCDebug(uiLogging) << result.toString();
    return result.toString();
}

ModalDialogListener* OffscreenUi::fileDialogAsync(const QVariantMap& properties) {
    QVariant buildDialogResult;
    bool invokeResult = false;
    auto tabletScriptingInterface = DependencyManager::get<TabletScriptingInterface>();
    TabletProxy* tablet = dynamic_cast<TabletProxy*>(tabletScriptingInterface->getTablet("com.highfidelity.interface.tablet.system"));
    if (tablet->getToolbarMode() && _desktop) {
       invokeResult =  QMetaObject::invokeMethod(_desktop, "fileDialog",
                                  Q_RETURN_ARG(QVariant, buildDialogResult),
                                  Q_ARG(QVariant, QVariant::fromValue(properties)));
    } else if (QQuickItem* tabletRoot = tablet->getTabletRoot()) {
        invokeResult =  QMetaObject::invokeMethod(tabletRoot, "fileDialog",
                                  Q_RETURN_ARG(QVariant, buildDialogResult),
                                  Q_ARG(QVariant, QVariant::fromValue(properties)));
        emit tabletScriptingInterface->tabletNotification();
    }

    if (!invokeResult) {
        qWarning() << "Failed to create file open dialog";
        return nullptr;
    }

    FileDialogListener* fileDialogListener = new FileDialogListener(qvariant_cast<QQuickItem*>(buildDialogResult));
    QObject* fileModalDialog = qobject_cast<QObject*>(fileDialogListener);
    _modalDialogListeners.push_back(fileModalDialog);

    return fileDialogListener;
}

QString OffscreenUi::fileOpenDialog(const QString& caption, const QString& dir, const QString& filter, QString* selectedFilter, QFileDialog::Options options) {
    if (QThread::currentThread() != thread()) {
        QString result;
        BLOCKING_INVOKE_METHOD(this, "fileOpenDialog",
            Q_RETURN_ARG(QString, result),
            Q_ARG(QString, caption),
            Q_ARG(QString, dir),
            Q_ARG(QString, filter),
            Q_ARG(QString*, selectedFilter),
            Q_ARG(QFileDialog::Options, options));
        return result;
    }

    // FIXME support returning the selected filter... somehow?
    QVariantMap map;
    map.insert("caption", caption);
    map.insert("dir", QUrl::fromLocalFile(dir));
    map.insert("filter", filter);
    map.insert("options", static_cast<int>(options));
    return fileDialog(map);
}

ModalDialogListener* OffscreenUi::fileOpenDialogAsync(const QString& caption, const QString& dir, const QString& filter, QString* selectedFilter, QFileDialog::Options options) {
    if (QThread::currentThread() != thread()) {
        ModalDialogListener* ret;
        BLOCKING_INVOKE_METHOD(this, "fileOpenDialogAsync",
            Q_RETURN_ARG(ModalDialogListener*, ret),
            Q_ARG(QString, caption),
            Q_ARG(QString, dir),
            Q_ARG(QString, filter),
            Q_ARG(QString*, selectedFilter),
            Q_ARG(QFileDialog::Options, options));
        return ret;
    }

    // FIXME support returning the selected filter... somehow?
    QVariantMap map;
    map.insert("caption", caption);
    map.insert("dir", QUrl::fromLocalFile(dir));
    map.insert("filter", filter);
    map.insert("options", static_cast<int>(options));
    return fileDialogAsync(map);
}

QString OffscreenUi::fileSaveDialog(const QString& caption, const QString& dir, const QString& filter, QString* selectedFilter, QFileDialog::Options options) {
    if (QThread::currentThread() != thread()) {
        QString result;
        BLOCKING_INVOKE_METHOD(this, "fileSaveDialog",
            Q_RETURN_ARG(QString, result),
            Q_ARG(QString, caption),
            Q_ARG(QString, dir),
            Q_ARG(QString, filter),
            Q_ARG(QString*, selectedFilter),
            Q_ARG(QFileDialog::Options, options));
        return result;
    }

    // FIXME support returning the selected filter... somehow?
    QVariantMap map;
    map.insert("caption", caption);
    map.insert("dir", QUrl::fromLocalFile(dir));
    map.insert("filter", filter);
    map.insert("options", static_cast<int>(options));
    map.insert("saveDialog", true);

    return fileDialog(map);
}

ModalDialogListener* OffscreenUi::fileSaveDialogAsync(const QString& caption, const QString& dir, const QString& filter, QString* selectedFilter, QFileDialog::Options options) {
    if (QThread::currentThread() != thread()) {
        ModalDialogListener* ret;
        BLOCKING_INVOKE_METHOD(this, "fileSaveDialogAsync",
            Q_RETURN_ARG(ModalDialogListener*, ret),
            Q_ARG(QString, caption),
            Q_ARG(QString, dir),
            Q_ARG(QString, filter),
            Q_ARG(QString*, selectedFilter),
            Q_ARG(QFileDialog::Options, options));
        return ret;
    }

    // FIXME support returning the selected filter... somehow?
    QVariantMap map;
    map.insert("caption", caption);
    map.insert("dir", QUrl::fromLocalFile(dir));
    map.insert("filter", filter);
    map.insert("options", static_cast<int>(options));
    map.insert("saveDialog", true);

    return fileDialogAsync(map);
}

QString OffscreenUi::existingDirectoryDialog(const QString& caption, const QString& dir, const QString& filter, QString* selectedFilter, QFileDialog::Options options) {
    if (QThread::currentThread() != thread()) {
        QString result;
        BLOCKING_INVOKE_METHOD(this, "existingDirectoryDialog",
                                  Q_RETURN_ARG(QString, result),
                                  Q_ARG(QString, caption),
                                  Q_ARG(QString, dir),
                                  Q_ARG(QString, filter),
                                  Q_ARG(QString*, selectedFilter),
                                  Q_ARG(QFileDialog::Options, options));
        return result;
    }

    QVariantMap map;
    map.insert("caption", caption);
    map.insert("dir", QUrl::fromLocalFile(dir));
    map.insert("filter", filter);
    map.insert("options", static_cast<int>(options));
    map.insert("selectDirectory", true);
    return fileDialog(map);
}

ModalDialogListener* OffscreenUi::existingDirectoryDialogAsync(const QString& caption, const QString& dir, const QString& filter, QString* selectedFilter, QFileDialog::Options options) {
    if (QThread::currentThread() != thread()) {
        ModalDialogListener* ret;
        BLOCKING_INVOKE_METHOD(this, "existingDirectoryDialogAsync",
                               Q_RETURN_ARG(ModalDialogListener*, ret),
                               Q_ARG(QString, caption),
                               Q_ARG(QString, dir),
                               Q_ARG(QString, filter),
                               Q_ARG(QString*, selectedFilter),
                               Q_ARG(QFileDialog::Options, options));
        return ret;
    }

    QVariantMap map;
    map.insert("caption", caption);
    map.insert("dir", QUrl::fromLocalFile(dir));
    map.insert("filter", filter);
    map.insert("options", static_cast<int>(options));
    map.insert("selectDirectory", true);
    return fileDialogAsync(map);
}

QString OffscreenUi::getOpenFileName(void* ignored, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options options) {
    Q_UNUSED(ignored)
    return DependencyManager::get<OffscreenUi>()->fileOpenDialog(caption, dir, filter, selectedFilter, options);
}

ModalDialogListener* OffscreenUi::getOpenFileNameAsync(void* ignored, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options options) {
    Q_UNUSED(ignored)
    return DependencyManager::get<OffscreenUi>()->fileOpenDialogAsync(caption, dir, filter, selectedFilter, options);
}

QString OffscreenUi::getSaveFileName(void* ignored, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options options) {
    Q_UNUSED(ignored)
    return DependencyManager::get<OffscreenUi>()->fileSaveDialog(caption, dir, filter, selectedFilter, options);
}

ModalDialogListener* OffscreenUi::getSaveFileNameAsync(void* ignored, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options options) {
    Q_UNUSED(ignored)
    return DependencyManager::get<OffscreenUi>()->fileSaveDialogAsync(caption, dir, filter, selectedFilter, options);
}

QString OffscreenUi::getExistingDirectory(void* ignored, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options options) {
    Q_UNUSED(ignored)
    return DependencyManager::get<OffscreenUi>()->existingDirectoryDialog(caption, dir, filter, selectedFilter, options);
}

ModalDialogListener* OffscreenUi::getExistingDirectoryAsync(void* ignored, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options options) {
    Q_UNUSED(ignored)
    return DependencyManager::get<OffscreenUi>()->existingDirectoryDialogAsync(caption, dir, filter, selectedFilter, options);
}

class AssetDialogListener : public ModalDialogListener {
    // ATP equivalent of FileDialogListener.
    Q_OBJECT

    friend class OffscreenUi;
    AssetDialogListener(QQuickItem* messageBox) : ModalDialogListener(messageBox) {
        if (_finished) {
            return;
        }
        connect(_dialog, SIGNAL(selectedAsset(QVariant)), this, SLOT(onSelectedAsset(QVariant)));
        connect(_dialog, SIGNAL(canceled()), this, SLOT(onSelectedAsset()));
    }

    private slots:
    void onSelectedAsset(QVariant asset = "") {
        _result = asset;
        auto offscreenUi = DependencyManager::get<OffscreenUi>();
        emit response(_result);
        offscreenUi->removeModalDialog(qobject_cast<QObject*>(this));
        _finished = true;
        disconnect(_dialog);
    }
};


QString OffscreenUi::assetDialog(const QVariantMap& properties) {
    // ATP equivalent of fileDialog().
    QVariant buildDialogResult;
    bool invokeResult = false;
    auto tabletScriptingInterface = DependencyManager::get<TabletScriptingInterface>();
    TabletProxy* tablet = dynamic_cast<TabletProxy*>(tabletScriptingInterface->getTablet("com.highfidelity.interface.tablet.system"));
    if (tablet->getToolbarMode() && _desktop) {
        invokeResult = QMetaObject::invokeMethod(_desktop, "assetDialog",
            Q_RETURN_ARG(QVariant, buildDialogResult),
            Q_ARG(QVariant, QVariant::fromValue(properties)));
    } else if (QQuickItem* tabletRoot = tablet->getTabletRoot()) {
        invokeResult = QMetaObject::invokeMethod(tabletRoot, "assetDialog",
            Q_RETURN_ARG(QVariant, buildDialogResult),
            Q_ARG(QVariant, QVariant::fromValue(properties)));
        emit tabletScriptingInterface->tabletNotification();
    }

    if (!invokeResult) {
        qWarning() << "Failed to create asset open dialog";
        return QString();
    }

    QVariant result = AssetDialogListener(qvariant_cast<QQuickItem*>(buildDialogResult)).waitForResult();
    if (!result.isValid()) {
        return QString();
    }
    qCDebug(uiLogging) << result.toString();
    return result.toUrl().toString();
}

ModalDialogListener *OffscreenUi::assetDialogAsync(const QVariantMap& properties) {
    // ATP equivalent of fileDialog().
    QVariant buildDialogResult;
    bool invokeResult = false;
    auto tabletScriptingInterface = DependencyManager::get<TabletScriptingInterface>();
    TabletProxy* tablet = dynamic_cast<TabletProxy*>(tabletScriptingInterface->getTablet("com.highfidelity.interface.tablet.system"));
    if (tablet->getToolbarMode() && _desktop) {
        invokeResult = QMetaObject::invokeMethod(_desktop, "assetDialog",
            Q_RETURN_ARG(QVariant, buildDialogResult),
            Q_ARG(QVariant, QVariant::fromValue(properties)));
    } else if (QQuickItem* tabletRoot = tablet->getTabletRoot()) {
        invokeResult = QMetaObject::invokeMethod(tabletRoot, "assetDialog",
            Q_RETURN_ARG(QVariant, buildDialogResult),
            Q_ARG(QVariant, QVariant::fromValue(properties)));
        emit tabletScriptingInterface->tabletNotification();
    }

    if (!invokeResult) {
        qWarning() << "Failed to create asset open dialog";
        return nullptr;
    }

    AssetDialogListener* assetDialogListener = new AssetDialogListener(qvariant_cast<QQuickItem*>(buildDialogResult));
    QObject* assetModalDialog = qobject_cast<QObject*>(assetDialogListener);
    _modalDialogListeners.push_back(assetModalDialog);
    return assetDialogListener;
}

QList<QObject *> &OffscreenUi::getModalDialogListeners() {
    return _modalDialogListeners;
}

QString OffscreenUi::assetOpenDialog(const QString& caption, const QString& dir, const QString& filter, QString* selectedFilter, QFileDialog::Options options) {
    // ATP equivalent of fileOpenDialog().
    if (QThread::currentThread() != thread()) {
        QString result;
        BLOCKING_INVOKE_METHOD(this, "assetOpenDialog",
            Q_RETURN_ARG(QString, result),
            Q_ARG(QString, caption),
            Q_ARG(QString, dir),
            Q_ARG(QString, filter),
            Q_ARG(QString*, selectedFilter),
            Q_ARG(QFileDialog::Options, options));
        return result;
    }

    // FIXME support returning the selected filter... somehow?
    QVariantMap map;
    map.insert("caption", caption);
    map.insert("dir", dir);
    map.insert("filter", filter);
    map.insert("options", static_cast<int>(options));
    return assetDialog(map);
}

ModalDialogListener* OffscreenUi::assetOpenDialogAsync(const QString& caption, const QString& dir, const QString& filter, QString* selectedFilter, QFileDialog::Options options) {
    // ATP equivalent of fileOpenDialog().
    if (QThread::currentThread() != thread()) {
        ModalDialogListener* ret;
        BLOCKING_INVOKE_METHOD(this, "assetOpenDialogAsync",
            Q_RETURN_ARG(ModalDialogListener*, ret),
            Q_ARG(QString, caption),
            Q_ARG(QString, dir),
            Q_ARG(QString, filter),
            Q_ARG(QString*, selectedFilter),
            Q_ARG(QFileDialog::Options, options));
        return ret;
    }

    // FIXME support returning the selected filter... somehow?
    QVariantMap map;
    map.insert("caption", caption);
    map.insert("dir", dir);
    map.insert("filter", filter);
    map.insert("options", static_cast<int>(options));
    return assetDialogAsync(map);
}

QString OffscreenUi::getOpenAssetName(void* ignored, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options options) {
    // ATP equivalent of getOpenFileName().
    Q_UNUSED(ignored)
    return DependencyManager::get<OffscreenUi>()->assetOpenDialog(caption, dir, filter, selectedFilter, options);
}

ModalDialogListener* OffscreenUi::getOpenAssetNameAsync(void* ignored, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options options) {
    // ATP equivalent of getOpenFileName().
    Q_UNUSED(ignored)
    return DependencyManager::get<OffscreenUi>()->assetOpenDialogAsync(caption, dir, filter, selectedFilter, options);
}

bool OffscreenUi::eventFilter(QObject* originalDestination, QEvent* event) {
    if (!filterEnabled(originalDestination, event)) {
        return false;
    }

    // let the parent class do it's work
    bool result = OffscreenQmlSurface::eventFilter(originalDestination, event);

    switch (event->type()) {
        // Fall through
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseMove: {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QPointF transformedPos = mapToVirtualScreen(mouseEvent->localPos());
            // FIXME: touch events are always being accepted.  Use mouse events on the OffScreenUi for now, and investigate properly switching to touch events
            // (using handlePointerEvent) later
            QMouseEvent mappedEvent(mouseEvent->type(), transformedPos, mouseEvent->screenPos(), mouseEvent->button(), mouseEvent->buttons(), mouseEvent->modifiers());
            mappedEvent.ignore();
            if (QCoreApplication::sendEvent(getWindow(), &mappedEvent)) {
                return mappedEvent.isAccepted();
            }
            break;
        }
        default:
            break;
    }

    // Check if this is a key press/release event that might need special attention
    auto type = event->type();
    if (type != QEvent::KeyPress && type != QEvent::KeyRelease) {
        return result;
    }

    QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
    auto key = keyEvent->key();
    bool& pressed = _pressedKeys[key];

    // Keep track of which key press events the QML has accepted
    if (result && QEvent::KeyPress == type) {
        pressed = true;
    }

    // QML input elements absorb key press, but apparently not key release.
    // therefore we want to ensure that key release events for key presses that were
    // accepted by the QML layer are suppressed
    if (type == QEvent::KeyRelease && pressed) {
        pressed = false;
        return true;
    }

    return result;
}

ModalDialogListener::ModalDialogListener(QQuickItem *dialog) : _dialog(dialog) {
    if (!dialog) {
        _finished = true;
        return;
    }
    connect(_dialog, SIGNAL(destroyed()), this, SLOT(onDestroyed()));
}

ModalDialogListener::~ModalDialogListener() {
    if (_dialog) {
        disconnect(_dialog);
    }
}

QVariant ModalDialogListener::waitForResult() {
    while (!_finished) {
        QCoreApplication::processEvents();
    }
    return _result;
}

void ModalDialogListener::onDestroyed() {
    _finished = true;
    disconnect(_dialog);
    _dialog = nullptr;
}

#include "OffscreenUi.moc"
