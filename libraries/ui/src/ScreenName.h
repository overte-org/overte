//
//  Created by Dale Glass on 7/01/2023
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <QScreen>
#include <QString>
#pragma once

/**
 * @brief Screen naming
 *
 * This class exists because display-plugins and interface need to share the same,
 * fairly involved rule for converting QScreen data to user-facing text.
 */
class ScreenName {
    public:
    /**
     * @brief Get a descriptive name for a screen
     *
     * This is used in the graphics settings, to name monitors. This function tries to generate
     * human friendly and unique, even if two identical monitors are present.
     *
     * @param screen Screen to provide a name for
     * @return QString Descriptive name for the screen
     */
    static QString getNameForScreen(QScreen *screen);

};
