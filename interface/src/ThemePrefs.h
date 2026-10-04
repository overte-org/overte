//
//  ThemePrefs.h
//  interface/src
//
//  Created by Ada <ada@thingvellir.net> on 2026-02-22
//  Copyright 2026 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef hifi_ThemePrefs_h
#define hifi_ThemePrefs_h

#include <QMutex>

#include <SettingHandle.h>

class ThemePrefs : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool darkMode READ getDarkMode WRITE setDarkMode NOTIFY themeChanged)
    Q_PROPERTY(bool highContrast READ getHighContrast WRITE setHighContrast NOTIFY themeChanged)
    Q_PROPERTY(bool reducedMotion READ getReducedMotion WRITE setReducedMotion NOTIFY themeChanged)

    Q_PROPERTY(bool useSystemColorScheme READ getUseSystemColorScheme WRITE setUseSystemColorScheme NOTIFY themeChanged)
    Q_PROPERTY(bool useSystemContrast READ getUseSystemContrast WRITE setUseSystemContrast NOTIFY themeChanged)

public:
    ThemePrefs(QObject* parent = nullptr) : QObject(parent) {}

    // returns the appropriate values depending on whether we're
    // using the system preferences or custom ones
    bool getDarkMode() const;
    bool getHighContrast() const;
    bool getReducedMotion() const;

    bool getUseSystemColorScheme() const;
    bool getUseSystemContrast() const;

    void setDarkMode(bool value);
    void setHighContrast(bool value);
    void setReducedMotion(bool value);

    void setUseSystemColorScheme(bool value);
    void setUseSystemContrast(bool value);

signals:
    void themeChanged();

private:
    mutable QRecursiveMutex _mutex;

    Setting::Handle<bool> _darkMode { "Theme/darkMode", true };
    Setting::Handle<bool> _highContrast { "Theme/highContrast", true };
    Setting::Handle<bool> _reducedMotion { "Theme/reducedMotion", true };
    Setting::Handle<bool> _useSystemColorScheme { "Theme/useSystemColorScheme", true };
    Setting::Handle<bool> _useSystemContrast { "Theme/useSystemContrast", true };
    // It looks like Qt doesn't have a system reduced motion setting?
    // KDE has one, but it modifies an internal KDE setting, not a Qt one.
    //Setting::Handle<bool> _useSystemReducedMotion { "Theme/useSystemReducedMotion", true };
};

#endif // hifi_ThemePrefs_h
