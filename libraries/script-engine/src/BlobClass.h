//
//  BlobClass.h
//  libraries/script-engine/src/
//
//  Created by Ada <ada@thingvellir.net> on 2026-05-09
//  Copyright 2026 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef hifi_BlobClass_h
#define hifi_BlobClass_h

#include <QObject>
#include <QHash>
#include <QUuid>
#include <QUrl>
#include <DependencyManager.h>

#include "ScriptEngine.h"

class BlobClass : public QObject {
    Q_OBJECT
    Q_PROPERTY(int size READ getSize)
    Q_PROPERTY(QString type READ getType)
    Q_PROPERTY(QUrl url READ getURL)

public:
    BlobClass(ScriptEngine* engine, QUrl url, QByteArray data = {}, QString mimeType = {});
    ~BlobClass();

    static ScriptValue constructor(ScriptContext* context, ScriptEngine* engine);

    Q_INVOKABLE ScriptValue slice(int start, int end = -1, ScriptValue typeOverride = {}, ScriptValue url = {});

    Q_INVOKABLE QString text() const;
    Q_INVOKABLE QByteArray arrayBuffer() const;

    QUrl getURL() const { return _url; }

private:
    int getSize() const { return _data.size(); }
    QString getType() const { return _mimeType; }

    ScriptEngine* _engine;
    QByteArray _data;
    QString _mimeType;
    QUrl _url;
};

class BlobRegistry : public QObject, public Dependency {
    Q_OBJECT
public:
    void add(BlobClass* blob);
    void remove(BlobClass* blob);

    BlobClass* value(const QUrl& url);
    bool contains(const QUrl& url);

public slots:
    void blobAdded(const QUrl& url);
    void blobRemoved(const QUrl& url);

private:
    QMutex _mapMutex;
    QHash<QUrl, BlobClass*> _map;
};

#endif
