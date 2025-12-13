//
//  VKCanvas.h
//  interface/src
//
//  Copied from GLCanvas.h on 2025/01/27.
//  originally created by Stephen Birarda on 8/14/13.
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once

// TODO: find a way of doing this from CMake. I'm not familiar enough with build system to figure this out.
#ifndef USE_GL

#include "vk/VKWidget.h"

/// customized canvas that simply forwards requests/events to the singleton application
class VKCanvas : public VKWidget {
    Q_OBJECT
protected:
    virtual bool event(QEvent* event) override;
};

#endif
