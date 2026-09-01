//
//  ThemePrefs.cpp
//  interface/src
//
//  Created by Ada <ada@thingvellir.net> on 2026-02-22
//  Copyright 2026 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include <QStyleHints>
#include <QAccessibilityHints>

#include "Application.h"
#include "ThemePrefs.h"

bool ThemePrefs::getDarkMode() const {
    QMutexLocker lock(&_mutex);

    if (_useSystemColorScheme.get()) {
        const auto* styleHints = qApp->styleHints();
        // default to dark theme if qt doesn't know what to use
        return styleHints->colorScheme() != Qt::ColorScheme::Light;
    } else {
        return _darkMode.get();
    }
}

bool ThemePrefs::getHighContrast() const {
    QMutexLocker lock(&_mutex);

    if (_useSystemContrast.get()) {
        const auto* a11yHints = qApp->styleHints()->accessibility();
        return a11yHints->contrastPreference() == Qt::ContrastPreference::HighContrast;
    } else {
        return _highContrast.get();
    }
}

bool ThemePrefs::getReducedMotion() const {
    QMutexLocker lock(&_mutex);
    return _reducedMotion.get();
}

bool ThemePrefs::getUseSystemColorScheme() const {
    QMutexLocker lock(&_mutex);
    return _useSystemColorScheme.get();
}

bool ThemePrefs::getUseSystemContrast() const {
    QMutexLocker lock(&_mutex);
    return _useSystemContrast.get();
}

void ThemePrefs::setDarkMode(bool value) {
    QMutexLocker lock(&_mutex);

    auto old = _darkMode.get();

    if (value != old) {
        _darkMode.set(value);
        emit themeChanged();
    }
}

void ThemePrefs::setHighContrast(bool value) {
    QMutexLocker lock(&_mutex);

    auto old = _highContrast.get();

    if (old != value) {
        _highContrast.set(value);
        emit themeChanged();
    }
}

void ThemePrefs::setReducedMotion(bool value) {
    QMutexLocker lock(&_mutex);

    auto old = _reducedMotion.get();

    if (old != value) {
        _reducedMotion.set(value);
        emit themeChanged();
    }
}

void ThemePrefs::setUseSystemColorScheme(bool value) {
    QMutexLocker lock(&_mutex);

    auto old = _useSystemColorScheme.get();

    if (old != value) {
        _useSystemColorScheme.set(value);
        emit themeChanged();
    }
}

void ThemePrefs::setUseSystemContrast(bool value) {
    QMutexLocker lock(&_mutex);

    auto old = _useSystemContrast.get();

    if (old != value) {
        _useSystemContrast.set(value);
        emit themeChanged();
    }
}
