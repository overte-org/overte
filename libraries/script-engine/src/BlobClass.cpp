//
//  BlobClass.cpp
//  libraries/script-engine/src/
//
//  Created by Ada <ada@thingvellir.net> on 2026-05-09
//  Copyright 2026 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "ScriptContext.h"
#include "ScriptValue.h"
#include "ScriptValueIterator.h"
#include "BlobClass.h"

BlobClass::BlobClass(ScriptEngine* engine, QUrl url, QByteArray data, QString mimeType) :
    _engine(engine),
    _data(data),
    _mimeType(mimeType),
    _url(url) {

    DependencyManager::get<BlobRegistry>()->add(this);
}

BlobClass::~BlobClass() {
    DependencyManager::get<BlobRegistry>()->remove(this);
}

ScriptValue BlobClass::constructor(ScriptContext *context, ScriptEngine *engine) {
    if (context->argumentCount() == 0) {
        engine->raiseException("Blob requires at least one argument, blobParts");
        return engine->undefinedValue();
    }

    auto data = QByteArray();
    auto mimeType = QString();
    auto url = std::optional<QUrl>{};

    if (context->argumentCount() > 1) {
        auto options = context->argument(1);

        if (options.hasProperty("type")) {
            mimeType = options.property("type").toString();
        }

        if (options.hasProperty("url")) {
            auto urlString = options.property("url").toString();
            auto urlTmp = QUrl(urlString);

            if (urlTmp.isValid() && urlTmp.scheme() == "blob") {
                url = urlTmp;
            }
        }
    }

    if (!url) {
        url = QUrl(
            QString("blob:%1/%2")
            .arg(engine->currentContext()->currentFileName())
            .arg(QUuid::createUuid().toString(QUuid::WithoutBraces)));
    }

    if (DependencyManager::get<BlobRegistry>()->contains(*url)) {
        engine->raiseException(QString("A Blob with the URL \"%1\" already exists").arg(url->toString()));
        return engine->undefinedValue();
    }

    auto dataIter = context->argument(0).newIterator();

    int i = 0;
    do {
        auto value = dataIter->value();

        if (value.isString()) {
            data.append(value.toString().toUtf8());
        } else if (auto* dataBlob = qobject_cast<BlobClass*>(value.toQObject())) {
            data.append(dataBlob->_data);
        } else if (
            auto valueVariant = value.toVariant();
            valueVariant.type() == QVariant::Type::ByteArray
        ) {
            data.append(qvariant_cast<QByteArray>(valueVariant));
        } else {
            engine->raiseException(QString("Unexpected value in blob part %1, expected string, ArrayBuffer, or Blob (Constructing from TypedArray is not supported yet)").arg(i));
            return engine->undefinedValue();
        }

        dataIter->next();
        i++;
    } while (dataIter->hasNext());

    return engine->newQObject(
        new BlobClass(engine, *url, data, mimeType), ScriptEngine::ScriptOwnership);
}

ScriptValue BlobClass::slice(int start, int end, ScriptValue typeOverrideValue, ScriptValue urlValue) {
    auto type = _mimeType;
    auto url = QUrl(
            QString("blob:%1/%2")
            .arg(_engine->currentContext()->currentFileName())
            .arg(QUuid::createUuid().toString(QUuid::WithoutBraces)));

    if (typeOverrideValue.isString()) {
        type = typeOverrideValue.toString();
    }

    if (urlValue.isString()) {
        if (QUrl tmp(urlValue.toString()); tmp.isValid() && tmp.scheme() == "blob") {
            url = tmp;
        }
    }

    // QT6TODO: use QByteArray::sliced instead of this manual pointer math
    if (end == -1) {
        end = _data.size();
    } else {
        end = std::min(end, _data.size());
    }

    start = std::clamp(start, 0, end);

    int size = end - start;

    auto data = QByteArray(_data.constData() + start, size);

    return _engine->newQObject(
        new BlobClass(_engine, url, data, type), ScriptEngine::ScriptOwnership);
}

QString BlobClass::text() const {
    return QString::fromUtf8(_data.constData(), _data.size());
}

QByteArray BlobClass::arrayBuffer() const {
    return _data;
}


void BlobRegistry::add(BlobClass* blob) {
    QMutexLocker lock(&_mapMutex);
    _map.insert(blob->getURL(), blob);
    emit blobAdded(blob->getURL());
}

void BlobRegistry::remove(BlobClass* blob) {
    QMutexLocker lock(&_mapMutex);
    _map.remove(blob->getURL());
    emit blobRemoved(blob->getURL());
}

BlobClass* BlobRegistry::value(const QUrl& url) {
    QMutexLocker lock(&_mapMutex);

    if (_map.contains(url)) {
        return _map.value(url);
    } else {
        return nullptr;
    }
}

bool BlobRegistry::contains(const QUrl& url) {
    QMutexLocker lock(&_mapMutex);
    return _map.contains(url);
}
