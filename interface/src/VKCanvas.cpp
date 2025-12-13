//
//  VKCanvas.cpp
//  interface/src
//
//  Copied from GLCanvas.cpp on 2025/01/27.
//  Created by Stephen Birarda on 2013/08/14.
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

// TODO: find a way of doing this from CMake. I'm not familiar enough with build system to figure this out.
#ifndef USE_GL
#include "VKCanvas.h"

#include "Application.h"

bool VKCanvas::event(QEvent* event) {
    if (QEvent::Paint == event->type() && qApp->isAboutToQuit()) {
        return true;
    }
    return VKWidget::event(event);
}
#endif