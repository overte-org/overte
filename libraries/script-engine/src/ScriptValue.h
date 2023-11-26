//
//  ScriptValue.h
//  libraries/script-engine/src
//
//  Created by Heather Anderson on 4/25/21.
//  Copyright 2021 Vircadia contributors.
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

/// @addtogroup ScriptEngine
/// @{

#ifndef hifi_ScriptValue_h
#define hifi_ScriptValue_h

#include <memory>

#include <QtCore/QObject>
#include <QtCore/QFlags>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QDebug>

#define SCRIPT_VALUE_MAIN_THREAD_DEBUG

#ifdef SCRIPT_VALUE_MAIN_THREAD_DEBUG
#include <QThread>
#include <QCoreApplication>
#define SCRIPT_VALUE_MAIN_THREAD_CHECK() scriptValueOnMainThreadCheck()
#else
#define SCRIPT_VALUE_MAIN_THREAD_CHECK()
#endif

#include "ScriptEngineLogging.h"

class ScriptEngine;
class ScriptValue;
class ScriptValueIterator;
class ScriptValueProxy;
using ScriptEnginePointer = std::shared_ptr<ScriptEngine>;
using ScriptValueList = QList<ScriptValue>;
using ScriptValueIteratorPointer = std::shared_ptr<ScriptValueIterator>;

/// [ScriptInterface] Provides an engine-independent interface for QScriptValue
class ScriptValue {
public:
    enum ResolveFlag
    {
        ResolveLocal = 0,
        ResolvePrototype = 1,
    };
    using ResolveFlags = QFlags<ResolveFlag>;

    enum PropertyFlag
    {
        ReadOnly = 0x00000001,
        Undeletable = 0x00000002,
        SkipInEnumeration = 0x00000004,
        PropertyGetter = 0x00000008,
        PropertySetter = 0x00000010,
        QObjectMember = 0x00000020,
        KeepExistingFlags = 0x00000800,
    };
    Q_DECLARE_FLAGS(PropertyFlags, PropertyFlag);


public:
    ScriptValue();
    inline ScriptValue(const ScriptValue& src);
    inline ~ScriptValue();
    inline ScriptValue(ScriptValueProxy* proxy) : _proxy(proxy) {}
    inline ScriptValueProxy* ptr() const { return _proxy; }
    inline ScriptValue& operator=(const ScriptValue& other);

public:
    inline ScriptValue call(const ScriptValue& thisObject = ScriptValue(),
                                    const ScriptValueList& args = ScriptValueList()) const;
    inline ScriptValue call(const ScriptValue& thisObject, const ScriptValue& arguments) const;
    inline ScriptValue construct(const ScriptValueList& args = ScriptValueList()) const;
    inline ScriptValue construct(const ScriptValue& arguments) const;
    inline ScriptValue data() const;
    inline ScriptEnginePointer engine() const;
    inline bool equals(const ScriptValue& other) const;
    inline bool isArray() const;
    inline bool isBool() const;
    inline bool isError() const;
    inline bool isFunction() const;
    inline bool isNumber() const;
    inline bool isNull() const;
    inline bool isObject() const;
    inline bool isString() const;
    inline bool isUndefined() const;
    inline bool isValid() const;
    inline bool isVariant() const;
    inline ScriptValueIteratorPointer newIterator() const;
    inline ScriptValue property(const QString& name, const ResolveFlags& mode = ResolvePrototype) const;
    inline ScriptValue property(quint32 arrayIndex, const ResolveFlags& mode = ResolvePrototype) const;
    inline ScriptValue prototype() const;
    inline void setData(const ScriptValue& val);
    inline bool hasProperty(const QString &name) const;
    inline void setProperty(const QString& name,
                             const ScriptValue& value,
                             const PropertyFlags& flags = KeepExistingFlags);
    inline void setProperty(quint32 arrayIndex,
                             const ScriptValue& value,
                             const PropertyFlags& flags = KeepExistingFlags);
    template <typename TYP, class ScriptEnginePointer = ScriptEnginePointer>
    inline void setProperty(const QString& name, const TYP& value,
                             const PropertyFlags& flags = KeepExistingFlags);
    template <typename TYP, class ScriptEnginePointer = ScriptEnginePointer>
    inline void setProperty(quint32 arrayIndex, const TYP& value,
                             const PropertyFlags& flags = KeepExistingFlags);
    inline void setPrototype(const ScriptValue& prototype);
    inline bool strictlyEquals(const ScriptValue& other) const;
    inline QList<QString> getPropertyNames() const;

    inline bool toBool() const;
    inline qint32 toInt32() const;
    inline double toInteger() const;
    inline double toNumber() const;
    inline QString toString() const;
    inline quint16 toUInt16() const;
    inline quint32 toUInt32() const;
    inline QVariant toVariant() const;
    inline QObject* toQObject() const;

protected:
    ScriptValueProxy* _proxy;
private:
    void scriptValueOnMainThreadCheck() const;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(ScriptValue::PropertyFlags);

/// [ScriptInterface] Provides an engine-independent interface for QScriptValue
class ScriptValueProxy {
public:
    virtual void release() = 0;
    virtual ScriptValueProxy* copy() const = 0;

public:
    virtual ScriptValue call(const ScriptValue& thisObject = ScriptValue(),
                             const ScriptValueList& args = ScriptValueList()) = 0;
    virtual ScriptValue call(const ScriptValue& thisObject, const ScriptValue& arguments) = 0;
    virtual ScriptValue construct(const ScriptValueList& args = ScriptValueList()) = 0;
    virtual ScriptValue construct(const ScriptValue& arguments) = 0;
    virtual ScriptValue data() const = 0;
    virtual ScriptEnginePointer engine() const = 0;
    virtual bool equals(const ScriptValue& other) const = 0;
    virtual bool isArray() const = 0;
    virtual bool isBool() const = 0;
    virtual bool isError() const = 0;
    virtual bool isFunction() const = 0;
    virtual bool isNumber() const = 0;
    virtual bool isNull() const = 0;
    virtual bool isObject() const = 0;
    virtual bool isString() const = 0;
    virtual bool isUndefined() const = 0;
    virtual bool isValid() const = 0;
    virtual bool isVariant() const = 0;
    virtual ScriptValueIteratorPointer newIterator() const = 0;
    virtual ScriptValue property(const QString& name,
                                 const ScriptValue::ResolveFlags& mode = ScriptValue::ResolvePrototype) const = 0;
    virtual ScriptValue property(quint32 arrayIndex,
                                 const ScriptValue::ResolveFlags& mode = ScriptValue::ResolvePrototype) const = 0;
    virtual ScriptValue prototype() const = 0;
    virtual void setData(const ScriptValue& val) = 0;
    virtual bool hasProperty(const QString &name) const = 0;
    virtual void setProperty(const QString& name,
                             const ScriptValue& value,
                             const ScriptValue::PropertyFlags& flags = ScriptValue::KeepExistingFlags) = 0;
    virtual void setProperty(quint32 arrayIndex,
                             const ScriptValue& value,
                             const ScriptValue::PropertyFlags& flags = ScriptValue::KeepExistingFlags) = 0;
    virtual void setPrototype(const ScriptValue& prototype) = 0;
    virtual bool strictlyEquals(const ScriptValue& other) const = 0;
    virtual QList<QString> getPropertyNames() const = 0;

    virtual bool toBool() const = 0;
    virtual qint32 toInt32() const = 0;
    virtual double toInteger() const = 0;
    virtual double toNumber() const = 0;
    virtual QString toString() const = 0;
    virtual quint16 toUInt16() const = 0;
    virtual quint32 toUInt32() const = 0;
    virtual QVariant toVariant() const = 0;
    virtual QObject* toQObject() const = 0;

protected:
    virtual ~ScriptValueProxy() {}  // prevent explicit deletion of base class
};

// the second template parameter is used to defer evaluation of calls to the engine until ScriptEngine isn't forward-declared
template <typename TYP, class ScriptEnginePointer>
void ScriptValue::setProperty(const QString& name, const TYP& value, const PropertyFlags& flags) {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    setProperty(name, static_cast<const ScriptEnginePointer&>(engine())->newValue(value), flags);
}

// the second template parameter is used to defer evaluation of calls to the engine until ScriptEngine isn't forward-declared
template <typename TYP, class ScriptEnginePointer>
void ScriptValue::setProperty(quint32 arrayIndex, const TYP& value, const PropertyFlags& flags) {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    setProperty(arrayIndex, static_cast<const ScriptEnginePointer&>(engine())->newValue(value), flags);
}

ScriptValue::ScriptValue(const ScriptValue& src) : _proxy(src.ptr()->copy()) {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
}

ScriptValue::~ScriptValue() {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    _proxy->release();
}

ScriptValue& ScriptValue::operator=(const ScriptValue& other) {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    _proxy->release();
    _proxy = other.ptr()->copy();
    return *this;
}

ScriptValue ScriptValue::call(const ScriptValue& thisObject, const ScriptValueList& args) const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    ScriptEnginePointer scriptEngine = _proxy->engine();
    if (scriptEngine == nullptr) {
        qCDebug(scriptengine) << "Call to deleted or non-existing script engine";
        return ScriptValue();
    }
    return _proxy->call(thisObject, args);
}

ScriptValue ScriptValue::call(const ScriptValue& thisObject, const ScriptValue& arguments) const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->call(thisObject, arguments);
}

ScriptValue ScriptValue::construct(const ScriptValueList& args) const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->construct(args);
}

ScriptValue ScriptValue::construct(const ScriptValue& arguments) const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->construct(arguments);
}

ScriptValue ScriptValue::data() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->data();
}

ScriptEnginePointer ScriptValue::engine() const {
    Q_ASSERT(_proxy != nullptr);
    return _proxy->engine();
}

bool ScriptValue::equals(const ScriptValue& other) const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->equals(other);
}

bool ScriptValue::isArray() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->isArray();
}

bool ScriptValue::isBool() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->isBool();
}

bool ScriptValue::isError() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->isError();
}

bool ScriptValue::isFunction() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->isFunction();
}

bool ScriptValue::isNumber() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->isNumber();
}

bool ScriptValue::isNull() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->isNull();
}

bool ScriptValue::isObject() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->isObject();
}

bool ScriptValue::isString() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->isString();
}

bool ScriptValue::isUndefined() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->isUndefined();
}

bool ScriptValue::isValid() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->isValid();
}

bool ScriptValue::isVariant() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->isVariant();
}

ScriptValueIteratorPointer ScriptValue::newIterator() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->newIterator();
}

ScriptValue ScriptValue::property(const QString& name, const ResolveFlags& mode) const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->property(name, mode);
}

ScriptValue ScriptValue::property(quint32 arrayIndex, const ResolveFlags& mode) const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->property(arrayIndex, mode);
}

ScriptValue ScriptValue::prototype() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->prototype();
}

void ScriptValue::setData(const ScriptValue& val) {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->setData(val);
}


bool ScriptValue::hasProperty(const QString& name) const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->hasProperty(name);
}

void ScriptValue::setProperty(const QString& name, const ScriptValue& value, const PropertyFlags& flags) {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->setProperty(name, value, flags);
}

void ScriptValue::setProperty(quint32 arrayIndex, const ScriptValue& value, const PropertyFlags& flags) {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->setProperty(arrayIndex, value, flags);
}

void ScriptValue::setPrototype(const ScriptValue& prototype) {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->setPrototype(prototype);
}

bool ScriptValue::strictlyEquals(const ScriptValue& other) const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->strictlyEquals(other);
}

inline QList<QString> ScriptValue::getPropertyNames() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->getPropertyNames();
};

bool ScriptValue::toBool() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->toBool();
}

qint32 ScriptValue::toInt32() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->toInt32();
}

double ScriptValue::toInteger() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->toInteger();
}

double ScriptValue::toNumber() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->toNumber();
}

QString ScriptValue::toString() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->toString();
}

quint16 ScriptValue::toUInt16() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->toUInt16();
}

quint32 ScriptValue::toUInt32() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->toUInt32();
}

QVariant ScriptValue::toVariant() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->toVariant();
}

QObject* ScriptValue::toQObject() const {
    SCRIPT_VALUE_MAIN_THREAD_CHECK();
    Q_ASSERT(_proxy != nullptr);
    return _proxy->toQObject();
}

#endif // hifi_ScriptValue_h

/// @}
