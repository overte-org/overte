//
//  KeyboardScriptingInterface.cpp
//  interface/src/scripting
//
//  Created by Dante Ruiz on 2018-08-27.
//  Copyright 2017 High Fidelity, Inc.
//  Copyright 2026 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ScriptPermissions.h"
#include "KeyboardScriptingInterface.h"
#include "ui/Keyboard.h"

bool KeyboardScriptingInterface::isRaised() const {
    return _keyboard->isRaised();
}

void KeyboardScriptingInterface::setRaised(bool raised) {
    _keyboard->setRaised(raised);
}

bool KeyboardScriptingInterface::isPassword() const {
    return _keyboard->isPassword();
}

void KeyboardScriptingInterface::setPassword(bool password) {
    _keyboard->setPassword(password);
}

void KeyboardScriptingInterface::loadKeyboardFile(const QString& keyboardFile) {
    _keyboard->loadKeyboardFile(keyboardFile);
}

bool KeyboardScriptingInterface::getUse3DKeyboard() const {
    return _keyboard->getUse3DKeyboard();
}

void KeyboardScriptingInterface::disableRightMallet() {
    _keyboard->disableRightMallet();
}

void KeyboardScriptingInterface::disableLeftMallet() {
    _keyboard->disableLeftMallet();
}

void KeyboardScriptingInterface::enableRightMallet() {
    _keyboard->enableRightMallet();
}

void KeyboardScriptingInterface::enableLeftMallet() {
    _keyboard->enableLeftMallet();
}

void KeyboardScriptingInterface::setLeftHandLaser(unsigned int leftHandLaser) {
    _keyboard->setLeftHandLaser(leftHandLaser);
}

void KeyboardScriptingInterface::setRightHandLaser(unsigned int rightHandLaser) {
    _keyboard->setRightHandLaser(rightHandLaser);
}

bool KeyboardScriptingInterface::getPreferMalletsOverLasers() const {
    return _keyboard->getPreferMalletsOverLasers();
}

void KeyboardScriptingInterface::setPreferMalletsOverLasers(bool mallets) {
    return DependencyManager::get<Keyboard>()->setPreferMalletsOverLasers(mallets);
}

bool KeyboardScriptingInterface::containsID(const QUuid& id) const {
    return _keyboard->containsID(id);
}

void KeyboardScriptingInterface::emitKeyEvent(const KeyEvent& event, bool pressed) const {
    if (!ScriptPermissions::hasScriptPermission(_manager, ScriptPermissions::Permission::SCRIPT_PERMISSION_KEYBOARD_EVENTS)) {
        return;
    }

    _keyboard->emitKeyEvent(event, pressed);
}
