//
//  Created by Sam Gondelman on 3/7/19.
//  Copyright 2019 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_RenderForward_h
#define hifi_RenderForward_h

#include <QString>
#include <QProcess>

static const QString RENDER_FORWARD_STRING { "HIFI_RENDER_FORWARD" };
static bool RENDER_FORWARD = QProcessEnvironment::systemEnvironment().contains(RENDER_FORWARD_STRING);

#endif // hifi_RenderForward_h