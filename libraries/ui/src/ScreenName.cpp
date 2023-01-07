//
//  Created by Dale Glass on 7/01/2023
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ScreenName.h"

QString ScreenName::getNameForScreen(QScreen *screen) {
    // The data provided by QScreen isn't as convenient as it could be.
    // So far testing shows:
    //
    // Windows:
    //      model() returns an empty string
    //      name() returns something like \\.\DISPLAY1
    //
    // Linux:
    //      model() returns a name, like "LG Ultra HD/525000"
    //      name() returns the output's name, like "HDMI1"

    // So we try to assemble something unique and readable from all the possibilities.

    QString ret;
    bool addParens = false;

    ret.append(screen->manufacturer());

    if (!ret.isEmpty()) {
        ret.append(" - ");
    }
    ret.append(screen->model());

    addParens = !ret.isEmpty();

    if(addParens) {
        ret.append(" (");
    }

    ret.append(screen->name().replace(QString("\\\\.\\"), QString("")));


    if(addParens) {
        ret.append(")");
    }

    return ret;
}
