//
//  ScriptEngineCast.h
//  libraries/script-engine/src
//
//  Created by Heather Anderson on 5/9/2021.
//  Copyright 2021 Vircadia contributors.
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

/// @addtogroup ScriptEngine
/// @{

#ifndef hifi_ScriptEngineCast_h
#define hifi_ScriptEngineCast_h

// Object conversion helpers (copied from QScriptEngine)

#include <QtCore/QMetaType>
#include <QtCore/QMetaEnum>
#include <QtCore/QDebug>

#include "ScriptEngine.h"
#include "ScriptValue.h"

template <typename T>
inline ScriptValue scriptValueFromValue(ScriptEngine* engine, const T& t) {
    if (!engine) {
        return ScriptValue();
    }

    return engine->create(qMetaTypeId<T>(), &t);
}

template <>
inline ScriptValue scriptValueFromValue<QVariant>(ScriptEngine* engine, const QVariant& v) {
    if (!engine) {
        return ScriptValue();
    }

    return engine->create(v.userType(), v.data());
}

template <typename T>
inline T scriptvalue_cast(const ScriptValue& value) {
    const int id = qMetaTypeId<T>();

    auto engine = value.engine();
    if (engine) {
        QVariant varValue = engine->convert(value, id);
        if (varValue.isValid()) {
            return varValue.value<T>();
        }
    }
    if (value.isVariant()) {
        return qvariant_cast<T>(value.toVariant());
    }

    return T();
}

template <>
inline QVariant scriptvalue_cast<QVariant>(const ScriptValue& value) {
    return value.toVariant();
}

//#define MARSHAL_MACRO(FUNCTION, TYPE) +[FUNCTION](ScriptEngine* engine, const void* p) -> ScriptValue { FUNCTION(engine, *(static_cast<const TYPE*>(p)) ); } 

template <typename T, ScriptValue (*f)(ScriptEngine*, const T&)>
ScriptValue toScriptValueWrapper(ScriptEngine* engine, const void *p) {
    Q_ASSERT(p != NULL);
    auto &src = *(reinterpret_cast<const T*>(p));
    return f(engine, src);
}

template <typename T, bool (*f)(const ScriptValue&, T&)>
bool fromScriptValueWrapper(const ScriptValue& val, void* p) {
    Q_ASSERT(p != NULL);
    auto &dest = *(reinterpret_cast<T*>(p));
    return f(val, dest);
}

template <typename T, ScriptValue (*toScriptValue)(ScriptEngine*, const T&), bool (*fromScriptValue)(const ScriptValue&, T&)>
int scriptRegisterMetaType(ScriptEngine* eng, const char* name = "",
                           T* = 0)
{
    int id;
    if (strlen(name) > 0) { // make sure it's registered
        id = qRegisterMetaType<T>(name);
    } else {
        id = qRegisterMetaType<T>();
    }
    eng->registerCustomType(id, toScriptValueWrapper<T, toScriptValue>,
                            fromScriptValueWrapper<T, fromScriptValue>);
    return id;
}

template <typename T>
int scriptRegisterMetaTypeWithLambdas(ScriptEngine* eng,
                           ScriptValue (*toScriptValue)(ScriptEngine*, const T& t),
                           bool (*fromScriptValue)(const ScriptValue&, T& t), const char* name = "",
                           T* = 0)
{
    int id;
    if (strlen(name) > 0) { // make sure it's registered
        id = qRegisterMetaType<T>(name);
    } else {
        id = qRegisterMetaType<T>();
    }
    eng->registerCustomType(id, reinterpret_cast<ScriptEngine::MarshalFunction>(toScriptValue),
                            reinterpret_cast<ScriptEngine::DemarshalFunction>(fromScriptValue));
    return id;
}

template <class Container>
ScriptValue scriptValueFromSequence(ScriptEngine* eng, const Container& cont) {
    ScriptValue a = eng->newArray();
    typename Container::const_iterator begin = cont.begin();
    typename Container::const_iterator end = cont.end();
    typename Container::const_iterator it;
    quint32 i;
    for (it = begin, i = 0; it != end; ++it, ++i) {
        a.setProperty(i, eng->toScriptValue(*it));
    }
    return a;
}

template <class Container>
bool scriptValueToSequence(const ScriptValue& value, Container& cont) {
    quint32 len = value.property(QLatin1String("length")).toUInt32();
    for (quint32 i = 0; i < len; ++i) {
        ScriptValue item = value.property(i);
        cont.push_back(scriptvalue_cast<typename Container::value_type>(item));
    }
    return true;
}

template <class T>
ScriptValue scriptValueFromEnumClass(ScriptEngine* eng, const T& enumValue) {
    ScriptValue a = eng->newValue(static_cast<int>(enumValue));
    return a;
}

template <class T>
bool scriptValueToEnumClass(const ScriptValue& value, T& enumValue) {
    if(!value.isNumber()){
        qDebug() << "ScriptValue \"" << value.toQObject()->metaObject()->className() << "\" is not a number";
        return false;
    }
    QMetaEnum metaEnum = QMetaEnum::fromType<T>();
    if (!metaEnum.isValid()) {
        qDebug() << "Invalid QMetaEnum";
        return false;
    }
    bool isValid = false;
    int enumInteger = static_cast<int>(value.toInteger());
    for(int i = 0; i < metaEnum.keyCount(); i++){
        if (metaEnum.value(i) == enumInteger) {
            isValid = true;
            break;
        }
    }
    if (isValid) {
        enumValue = static_cast<T>(enumInteger);
        return true;
    } else {
        qDebug() << "ScriptValue has invalid value " << value.toInteger() << " for enum" << metaEnum.name();
        return false;
    }
}

template <typename T>
int scriptRegisterSequenceMetaType(ScriptEngine* engine,
                                   T* = 0) {
    return scriptRegisterMetaType<T, scriptValueFromSequence, scriptValueToSequence>(engine);
}

#endif // hifi_ScriptEngineCast_h

/// @}
