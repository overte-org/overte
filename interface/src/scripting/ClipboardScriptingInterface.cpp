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

bool ClipboardScriptingInterface::exportEntities(const QString& filename, const QVector<QUuid>& entityIDs, const QVariantMap& options) {
    bool retVal;
    BLOCKING_INVOKE_METHOD(qApp, "exportEntities",
                              Q_RETURN_ARG(bool, retVal),
                              Q_ARG(const QString&, filename),
                              Q_ARG(const QVector<QUuid>&, entityIDs),
                              Q_ARG(const glm::vec3*, nullptr),
                              Q_ARG(const QVariantMap&, options));
    return retVal;
}

bool ClipboardScriptingInterface::exportEntities(const QString& filename, float x, float y, float z, float scale, const QVariantMap& options) {
    bool retVal;
    BLOCKING_INVOKE_METHOD(qApp, "exportEntities",
                              Q_RETURN_ARG(bool, retVal),
                              Q_ARG(const QString&, filename),
                              Q_ARG(float, x),
                              Q_ARG(float, y),
                              Q_ARG(float, z),
                              Q_ARG(float, scale),
                              Q_ARG(const QVariantMap&, options));
    return retVal;
}

bool ClipboardScriptingInterface::importEntities(
    const QString& filename,
    const bool isObservable,
    const qint64 callerId
) {
    bool retVal;
    BLOCKING_INVOKE_METHOD(qApp, "importEntities",
                              Q_RETURN_ARG(bool, retVal),
                              Q_ARG(const QString&, filename),
                              Q_ARG(const bool, isObservable),
                              Q_ARG(const qint64, callerId));
    return retVal;
}

QVector<EntityItemID> ClipboardScriptingInterface::pasteEntities(glm::vec3 position, const QString& entityHostType) {
    QVector<EntityItemID> retVal;
    BLOCKING_INVOKE_METHOD(qApp, "pasteEntities",
                              Q_RETURN_ARG(QVector<EntityItemID>, retVal),
                              Q_ARG(const QString&, entityHostType),
                              Q_ARG(float, position.x),
                              Q_ARG(float, position.y),
                              Q_ARG(float, position.z));
    return retVal;
}
