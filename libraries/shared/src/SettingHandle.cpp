//
//  SettingHandle.h
//
//
//  Created by AndrewMeadows 2015.10.05
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "SettingHandle.h"
#include "SettingManager.h"

#include <math.h>


Q_LOGGING_CATEGORY(settings_handle, "settings.handle")

const QString Settings::firstRun { "firstRun" };


Settings::Settings() : _manager(DependencyManager::get<Setting::Manager>())
{
}

QString Settings::fileName() const {
    return _manager->fileName();
}

void Settings::remove(const QString& key) {
    // NOTE: you can't check _manager->contains(key) here because it will return 'false'
    // when the key is a valid child group with child keys.
    // However, calling remove() without checking will do the right thing.
    _manager->remove(key);
}

// QStringList Settings::childGroups() const {
// }

// QStringList Settings::childKeys() const {
// }

QStringList Settings::allKeys() const {
    return _manager->allKeys();
}

bool Settings::contains(const QString& key) const {
    // NOTE: this will return 'false' if key is a valid child group with child keys.
    return _manager->contains(key);
}

int Settings::beginReadArray(const QString& prefix) {
    _groups.push(Group(prefix));
    _groupPrefix = getGroupPrefix();
    int size = _manager->value(_groupPrefix + "/size", -1).toInt();
    _groups.top().setSize(size);
    return size;
}

void Settings::beginWriteArray(const QString& prefix, int size) {
    _groups.push(Group(prefix));
    _groupPrefix = getGroupPrefix();
    _manager->setValue(_groupPrefix + "/size", size);

    _groups.top().setSize(size);
    _groups.top().setIndex(0);

    _groupPrefix = getGroupPrefix();
}

void Settings::endArray() {
    if (!_groups.empty()) {
        _groups.pop();
        _groupPrefix = getGroupPrefix();
    }
}

void Settings::setArrayIndex(int i) {
    if (!_groups.empty()) {
        _groups.top().setIndex(i);
        _groupPrefix = getGroupPrefix();
    }
}

void Settings::beginGroup(const QString& prefix) {
    _groups.push(Group(prefix));
    _groupPrefix = getGroupPrefix();
}

void Settings::endGroup() {
    _groups.pop();
    _groupPrefix = getGroupPrefix();
}

void Settings::setValue(const QString& name, const QVariant& value) {
    QString fullPath = getPath(name);
    _manager->setValue(fullPath, value);
}

QVariant Settings::value(const QString& name, const QVariant& defaultValue) const {
    QString fullPath = getPath(name);
    return _manager->value(fullPath, defaultValue);
}


void Settings::getFloatValueIfValid(const QString& name, float& floatValue) {
    const QVariant badDefaultValue = NAN;
    bool ok = true;
    float tempFloat = value(name, badDefaultValue).toFloat(&ok);
    if (ok && !glm::isnan(tempFloat)) {
        floatValue = tempFloat;
    }
}

void Settings::getBoolValue(const QString& name, bool& boolValue) {
    const QVariant defaultValue = false;
    boolValue = value(name, defaultValue).toBool();
}


void Settings::setVec3Value(const QString& name, const glm::vec3& vecValue) {
    beginGroup(name);
    {
        setValue(QString("x"), vecValue.x);
        setValue(QString("y"), vecValue.y);
        setValue(QString("z"), vecValue.z);
    }
    endGroup();
}

void Settings::getVec3ValueIfValid(const QString& name, glm::vec3& vecValue) {
    beginGroup(name);
    {
        bool ok = true;
        const QVariant badDefaultValue = NAN;
        float x = value(QString("x"), badDefaultValue).toFloat(&ok);
        float y = value(QString("y"), badDefaultValue).toFloat(&ok);
        float z = value(QString("z"), badDefaultValue).toFloat(&ok);
        if (ok && (!glm::isnan(x) && !glm::isnan(y) && !glm::isnan(z))) {
            vecValue = glm::vec3(x, y, z);
        }
    }
    endGroup();
}

void Settings::setQuatValue(const QString& name, const glm::quat& quatValue) {
    beginGroup(name);
    {
        setValue(QString("x"), quatValue.x);
        setValue(QString("y"), quatValue.y);
        setValue(QString("z"), quatValue.z);
        setValue(QString("w"), quatValue.w);
    }
    endGroup();
}

void Settings::getQuatValueIfValid(const QString& name, glm::quat& quatValue) {
    beginGroup(name);
    {
        bool ok = true;
        const QVariant badDefaultValue = NAN;
        float x = value(QString("x"), badDefaultValue).toFloat(&ok);
        float y = value(QString("y"), badDefaultValue).toFloat(&ok);
        float z = value(QString("z"), badDefaultValue).toFloat(&ok);
        float w = value(QString("w"), badDefaultValue).toFloat(&ok);
        if (ok && (!glm::isnan(x) && !glm::isnan(y) && !glm::isnan(z) && !glm::isnan(w))) {
            quatValue = glm::quat(w, x, y, z);
        }
    }
    endGroup();
}

QString Settings::getGroupPrefix() const {
    QString ret;

    for(Group g : _groups) {
        if (!ret.isEmpty()) {
            ret.append("/");
        }

        ret.append(g.name());

        if ( g.isArray() ) {
            // QSettings indexes arrays from 1
            ret.append(QString("/%1").arg(g.index() + 1));
        }
    }

    return ret;
}

QString Settings::getPath(const QString &key) const {
    QString ret = _groupPrefix;
    if (!ret.isEmpty() ) {
        ret.append("/");
    }

    ret.append(key);
    return ret;
}