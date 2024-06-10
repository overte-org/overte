//
//  ScriptUUID.cpp
//  libraries/script-engine/src/
//
//  Created by Andrew Meadows on 2014-04-07
//  Copyright (c) 2014 High Fidelity, Inc. All rights reserved.
//  Copyright 2023 Overte e.V.
//
//  Scriptable interface for a UUID helper class object. Used exclusively in the JavaScript API
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "ScriptUUID.h"

#include <QDebug>

#include "ScriptEngineLogging.h"
#include "ScriptEngine.h"
#include "ScriptManager.h"

QUuid ScriptUUID::fromString(const QString& s) {
    return QUuid(s);
}

QString ScriptUUID::toString(const QUuid& id) {
    return id.toString();
}

QUuid ScriptUUID::generate() {
    return QUuid::createUuid();    
}

bool ScriptUUID::isEqual(const QUuid& idA, const QUuid& idB) {
    return idA == idB;
}

bool ScriptUUID::isNull(const QUuid& id) {
    return id.isNull();
}

void ScriptUUID::print(const QString& label, const QUuid& id) {
    QString message = QString("%1 %2").arg(qPrintable(label));
    message = message.arg(id.toString());
    qCDebug(scriptengine) << message;
    Q_ASSERT(engine());
    if (ScriptManager* scriptManager = engine()->manager()) {
        scriptManager->print(message);
    }
}
