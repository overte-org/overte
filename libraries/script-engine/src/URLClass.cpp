//
//  URLClass.cpp
//  libraries/script-engine/src/
//
//  Created by Ada <ada@thingvellir.net> on 2026-04-09
//  Copyright 2026 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "ScriptContext.h"
#include "URLClass.h"

URLClass::URLClass(ScriptEngine* engine, QUrl url) : _engine(engine), _url(url) {}

ScriptValue URLClass::constructor(ScriptContext *context, ScriptEngine *engine) {
    if (context->argumentCount() == 0) {
        engine->raiseException("URL constructor requires at least one argument");
        return engine->nullValue();
    }

    QString argUrl = context->argument(0).toString();
    QString argBase;

    if (context->argumentCount() > 1) {
        argBase = context->argument(1).toString();
    }

    auto value = URLClass::parse(engine, argUrl, argBase);

    if (value.isNull()) {
        engine->raiseException("URL could not be parsed");
        return engine->nullValue();
    }

    return value;
}

bool URLClass::canParse(const QString& argUrl, const QString& argBase) {
    auto url = QUrl(argUrl);
    auto base = QUrl(argBase);

    if (!url.isValid()) { return false; }

    if (url.isRelative() && !base.isValid()) { return false; }

    auto full = base.resolved(url);

    return full.isValid();
}

ScriptValue URLClass::canParse(ScriptContext* context, ScriptEngine* engine) {
    if (context->argumentCount() == 0) {
        engine->raiseException("URL.canParse requires at least one argument");
        return engine->nullValue();
    }

    QString argUrl = context->argument(0).toString();
    QString argBase;

    if (context->argumentCount() > 1) {
        argBase = context->argument(1).toString();
    }

    return engine->newValue(URLClass::canParse(argUrl, argBase));
}

ScriptValue URLClass::parse(ScriptContext* context, ScriptEngine* engine) {
    if (context->argumentCount() == 0) {
        engine->raiseException("URL.parse requires at least one argument");
        return engine->nullValue();
    }

    QString argUrl = context->argument(0).toString();
    QString argBase;

    if (context->argumentCount() > 1) {
        argBase = context->argument(1).toString();
    }

    return URLClass::parse(engine, argUrl, argBase);
}

ScriptValue URLClass::parse(ScriptEngine* engine, const QString& argUrl, const QString& argBase) {
    auto url = QUrl(argUrl);
    auto base = QUrl(argBase);

    if (!url.isValid()) { return engine->nullValue(); }

    if (url.isRelative() && !base.isValid()) {
        return engine->nullValue();
    }

    if (base.isValid()) {
        url = base.resolved(url);
    }

    return engine->newQObject(new URLClass(engine, url), ScriptEngine::ScriptOwnership);
}

QString URLClass::getHash() const {
    if (_url.hasFragment()) {
        return QString("#%1").arg(_url.fragment(QUrl::FullyEncoded));
    } else {
        return "";
    }
}

QString URLClass::getHost() const {
    if (_url.port() == -1) {
        return _url.host();
    } else {
        return QString("%1:%2").arg(_url.host()).arg(_url.port());
    }
}

QString URLClass::getHostname() const {
    return _url.host();
}

QString URLClass::getHref() const {
    return _url.toString(QUrl::FullyEncoded);
}

QString URLClass::getOrigin() const {
    return QString("%1://%2").arg(_url.scheme()).arg(getHost());
}

QString URLClass::getPassword() const {
    return _url.password();
}

QString URLClass::getPathname() const {
    auto path = _url.path();

    if (path.isEmpty()) {
        if (
            auto scheme = _url.scheme();
            scheme == "http" ||
            scheme == "https" ||
            scheme == "file"
        ) {
            return "/";
        }
    }

    return path;
}

QString URLClass::getPort() const {
    if (auto port = _url.port(); port != -1) {
        return QString::number(port);
    } else {
        return "";
    }
}

QString URLClass::getProtocol() const {
    return QString("%1:").arg(_url.scheme());
}

QString URLClass::getSearch() const {
    if (auto query = _url.query(QUrl::FullyEncoded); !query.isEmpty()) {
        return QString("?%1").arg(query);
    } else {
        return "";
    }
}

QString URLClass::getUsername() const {
    return _url.userName();
}

void URLClass::setHash(const QString& value) {
    if (value.startsWith('#')) {
        // QT6TODO: use QString::slice instead
        _url.setFragment(QString(value.constData() + 1, value.size() - 1));
    } else {
        _url.setFragment(value);
    }
}

void URLClass::setHost(const QString& value) {
    if (value.contains(':')) {
        auto parts = value.split(':');

        _url.setHost(parts[0]);

        if (parts.size() > 1) {
            bool ok = false;
            auto port = parts[1].toInt(&ok);

            if (ok && port > 0 && port < 65536) {
                _url.setPort(port);
            } else {
                _url.setPort(-1);
            }
        } else {
            _url.setPort(-1);
        }
    } else {
        _url.setHost(value);
        _url.setPort(-1);
    }
}

void URLClass::setHostname(const QString& value) {
    _url.setHost(value);
}

void URLClass::setHref(const QString& value) {
    auto tmp = QUrl(value);

    if (!tmp.isValid()) {
        _engine->raiseException("Invalid URL");
        return;
    }

    _url = tmp;
}

void URLClass::setPassword(const QString& value) {
    _url.setPassword(value);
}

void URLClass::setPathname(const QString& value) {
    _url.setPath(value);
}

void URLClass::setPort(const QString& value) {
    if (value.isEmpty()) {
        _url.setPort(-1);
        return;
    }

    bool ok = false;
    int port = value.toInt(&ok);

    if (!ok) {
        _engine->raiseException("Invalid port");
        return;
    }

    _url.setPort(port);
}

void URLClass::setProtocol(const QString& value) {
    if (value.isEmpty()) {
        _url.setScheme("");
    } else if (value.endsWith(':')) {
        _url.setScheme(QString(value.constData(), value.size() - 1));
    } else {
        _url.setScheme(value);
    }
}

void URLClass::setSearch(const QString& value) {
    if (value.startsWith('?')) {
        // QT6TODO: use QString::slice instead
        _url.setQuery(QString(value.constData() + 1, value.size() - 1));
    } else {
        _url.setQuery(value);
    }
}

void URLClass::setUsername(const QString& value) {
    _url.setUserName(value);
}
