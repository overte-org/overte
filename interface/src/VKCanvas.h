//
// Created by ksuprynowicz on 1/27/25.
//

//
//  GLCanvas.h
//  interface/src
//
//  Created by Stephen Birarda on 8/14/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once

//#include "src/vk/VKWidget.h"

#include "vk/VKWidget.h"

/// customized canvas that simply forwards requests/events to the singleton application
class VKCanvas : public VKWidget {
    Q_OBJECT
protected:
    virtual bool event(QEvent* event) override;
};
