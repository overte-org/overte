//
//  ClipboardScriptingInterface.cpp
//  interface/src/scripting
//
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ClipboardScriptingInterface.h"

#include <shared/QtHelpers.h>

#include "Application.h"

ClipboardScriptingInterface::ClipboardScriptingInterface() {
}

glm::vec3 ClipboardScriptingInterface::getContentsDimensions() {
    return qApp->getEntityClipboard()->getContentsDimensions();
}

float ClipboardScriptingInterface::getClipboardContentsLargestDimension() {
    return qApp->getEntityClipboard()->getContentsLargestDimension();
}

bool ClipboardScriptingInterface::exportEntities(const QString& filename, const QVector<QUuid>& entityIDs) {
    bool retVal;
    BLOCKING_INVOKE_METHOD(qApp, "exportEntities",
                              Q_GENERIC_RETURN_ARG(bool, retVal),
                              Q_GENERIC_ARG(const QString&, filename),
                              Q_GENERIC_ARG(const QVector<QUuid>&, entityIDs));
    return retVal;
}

bool ClipboardScriptingInterface::exportEntities(const QString& filename, float x, float y, float z, float scale) {
    bool retVal;
    BLOCKING_INVOKE_METHOD(qApp, "exportEntities",
                              Q_GENERIC_RETURN_ARG(bool, retVal),
                              Q_GENERIC_ARG(const QString&, filename),
                              Q_GENERIC_ARG(float, x),
                              Q_GENERIC_ARG(float, y),
                              Q_GENERIC_ARG(float, z),
                              Q_GENERIC_ARG(float, scale));
    return retVal;
}

bool ClipboardScriptingInterface::importEntities(
    const QString& filename,
    const bool isObservable,
    const qint64 callerId
) {
    bool retVal;
    BLOCKING_INVOKE_METHOD(qApp, "importEntities",
                              Q_GENERIC_RETURN_ARG(bool, retVal),
                              Q_GENERIC_ARG(const QString&, filename),
                              Q_GENERIC_ARG(const bool, isObservable),
                              Q_GENERIC_ARG(const qint64, callerId));
    return retVal;
}

QVector<EntityItemID> ClipboardScriptingInterface::pasteEntities(glm::vec3 position, const QString& entityHostType) {
    QVector<EntityItemID> retVal;
    BLOCKING_INVOKE_METHOD(qApp, "pasteEntities",
                              Q_GENERIC_RETURN_ARG(QVector<EntityItemID>, retVal),
                              Q_GENERIC_ARG(const QString&, entityHostType),
                              Q_GENERIC_ARG(float, position.x),
                              Q_GENERIC_ARG(float, position.y),
                              Q_GENERIC_ARG(float, position.z));
    return retVal;
}
