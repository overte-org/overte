//
//  ScriptEngineV8_cast.cpp
//  libraries/script-engine/src/v8
//
//  Created by Heather Anderson 12/9/2021
//  Modified for V8 by dr Karol Suprynowicz on 2022/10/08
//  Copyright 2021 Vircadia contributors.
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ScriptEngineV8.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include "libplatform/libplatform.h"
#include "v8.h"

#include "../ScriptEngineCast.h"
#include "../ScriptValueIterator.h"

#include "ScriptObjectV8Proxy.h"
#include "ScriptValueV8Wrapper.h"

void ScriptEngineV8::setDefaultPrototype(int metaTypeId, const ScriptValue& prototype) {
    ScriptValueV8Wrapper* unwrappedPrototype = ScriptValueV8Wrapper::unwrap(prototype);
    if (unwrappedPrototype) {
        const V8ScriptValue& scriptPrototype = unwrappedPrototype->toV8Value();
        _customTypeProtect.lockForWrite();
        _customPrototypes.insert(metaTypeId, scriptPrototype);
        _customTypeProtect.unlock();
    }
}

void ScriptEngineV8::registerCustomType(int type,
                                              ScriptEngine::MarshalFunction marshalFunc,
                                              ScriptEngine::DemarshalFunction demarshalFunc)
{
    _customTypeProtect.lockForWrite();

    // storing it in a map for our own benefit
    CustomMarshal& customType = _customTypes.insert(type, CustomMarshal()).value();
    customType.demarshalFunc = demarshalFunc;
    customType.marshalFunc = marshalFunc;
    _customTypeProtect.unlock();
}

Q_DECLARE_METATYPE(ScriptValue);

/*static V8ScriptValue ScriptValueToV8ScriptValue(ScriptEngineV8* engine, const ScriptValue& src) {
    return ScriptValueV8Wrapper::fullUnwrap(static_cast<ScriptEngineV8*>(engine), src);
}*/

/*static void ScriptValueFromV8ScriptValue(ScriptEngineV8* engine, const V8ScriptValue& src, ScriptValue& dest) {
    //ScriptEngineV8* engine = static_cast<ScriptEngineV8*>(src.engine());
    dest = ScriptValue(new ScriptValueV8Wrapper(engine, src));
}*/

static ScriptValue StringListToScriptValue(ScriptEngine* engine, const QStringList& src) {
    int len = src.length();
    ScriptValue dest = engine->newArray(len);
    for (int idx = 0; idx < len; ++idx) {
        dest.setProperty(idx, engine->newValue(src.at(idx)));
    }
    return dest;
}

static bool StringListFromScriptValue(const ScriptValue& src, QStringList& dest) {
    if(!src.isArray()) return false;
    int len = src.property("length").toInteger();
    dest.clear();
    for (int idx = 0; idx < len; ++idx) {
        dest.append(src.property(idx).toString());
    }
    return true;
}

static ScriptValue VariantListToScriptValue(ScriptEngine* engine, const QVariantList& src) {
    int len = src.length();
    ScriptValue dest = engine->newArray(len);
    for (int idx = 0; idx < len; ++idx) {
        dest.setProperty(idx, engine->newVariant(src.at(idx)));
    }
    return dest;
}

static bool VariantListFromScriptValue(const ScriptValue& src, QVariantList& dest) {
    if(!src.isArray()) return false;
    int len = src.property("length").toInteger();
    dest.clear();
    for (int idx = 0; idx < len; ++idx) {
        dest.append(src.property(idx).toVariant());
    }
    return true;
}

static ScriptValue VariantMapToScriptValue(ScriptEngine* engine, const QVariantMap& src) {
    ScriptValue dest = engine->newObject();
    for (QVariantMap::const_iterator iter = src.cbegin(); iter != src.cend(); ++iter) {
        dest.setProperty(iter.key(), engine->newVariant(iter.value()));
    }
    return dest;
}

static bool VariantMapFromScriptValue(const ScriptValue& src, QVariantMap& dest) {
    dest.clear();
    ScriptValueIteratorPointer iter = src.newIterator();
    while (iter->hasNext()) {
        iter->next();
        dest.insert(iter->name(), iter->value().toVariant());
    }
    return true;
}

static ScriptValue VariantHashToScriptValue(ScriptEngine* engine, const QVariantHash& src) {
    ScriptValue dest = engine->newObject();
    for (QVariantHash::const_iterator iter = src.cbegin(); iter != src.cend(); ++iter) {
        dest.setProperty(iter.key(), engine->newVariant(iter.value()));
    }
    return dest;
}

static bool VariantHashFromScriptValue(const ScriptValue& src, QVariantHash& dest) {
    dest.clear();
    ScriptValueIteratorPointer iter = src.newIterator();
    while (iter->hasNext()) {
        iter->next();
        dest.insert(iter->name(), iter->value().toVariant());
    }
    return true;
}

static ScriptValue JsonValueToScriptValue(ScriptEngine* engine, const QJsonValue& src) {
    return engine->newVariant(src.toVariant());
}

static bool JsonValueFromScriptValue(const ScriptValue& src, QJsonValue& dest) {
    dest = QJsonValue::fromVariant(src.toVariant());
    return true;
}

static ScriptValue JsonObjectToScriptValue(ScriptEngine* engine, const QJsonObject& src) {
    QVariantMap map = src.toVariantMap();
    ScriptValue dest = engine->newObject();
    for (QVariantMap::const_iterator iter = map.cbegin(); iter != map.cend(); ++iter) {
        dest.setProperty(iter.key(), engine->newVariant(iter.value()));
    }
    return dest;
}

static bool JsonObjectFromScriptValue(const ScriptValue& src, QJsonObject& dest) {
    QVariantMap map;
    ScriptValueIteratorPointer iter = src.newIterator();
    while (iter->hasNext()) {
        iter->next();
        map.insert(iter->name(), iter->value().toVariant());
    }
    dest = QJsonObject::fromVariantMap(map);
    return true;
}

static ScriptValue JsonArrayToScriptValue(ScriptEngine* engine, const QJsonArray& src) {
    QVariantList list = src.toVariantList();
    int len = list.length();
    ScriptValue dest = engine->newArray(len);
    for (int idx = 0; idx < len; ++idx) {
        dest.setProperty(idx, engine->newVariant(list.at(idx)));
    }
    return dest;
}

static bool JsonArrayFromScriptValue(const ScriptValue& src, QJsonArray& dest) {
    if(!src.isArray()) return false;
    QVariantList list;
    int len = src.property("length").toInteger();
    for (int idx = 0; idx < len; ++idx) {
        list.append(src.property(idx).toVariant());
    }
    dest = QJsonArray::fromVariantList(list);
    return true;
}

// QMetaType::QJsonArray

void ScriptEngineV8::registerSystemTypes() {
    //qScriptRegisterMetaType(this, ScriptValueToV8ScriptValue, ScriptValueFromV8ScriptValue);

    scriptRegisterMetaType<QStringList, StringListToScriptValue, StringListFromScriptValue>(static_cast<ScriptEngine*>(this));
    scriptRegisterMetaType<QVariantList, VariantListToScriptValue, VariantListFromScriptValue>(this);
    scriptRegisterMetaType<QVariantMap, VariantMapToScriptValue, VariantMapFromScriptValue>(this);
    scriptRegisterMetaType<QVariantHash, VariantHashToScriptValue, VariantHashFromScriptValue>(this);
    scriptRegisterMetaType<QJsonValue, JsonValueToScriptValue, JsonValueFromScriptValue>(this);
    scriptRegisterMetaType<QJsonObject, JsonObjectToScriptValue, JsonObjectFromScriptValue>(this);
    scriptRegisterMetaType<QJsonArray, JsonArrayToScriptValue, JsonArrayFromScriptValue>(this);
}

int ScriptEngineV8::computeCastPenalty(const V8ScriptValue& v8Val, int destTypeId) {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Local<v8::Context> context = getContext();
    v8::Context::Scope contextScope(context);

    const v8::Local<v8::Value> val = v8Val.constGet();
    if (val->IsNumber()) {
        switch (destTypeId){
            case QMetaType::Bool:
                // Conversion to bool is acceptable, but numbers are preferred
                return 5;
                break;
            case QMetaType::UInt:
            case QMetaType::ULong:
            case QMetaType::Int:
            case QMetaType::Long:
            case QMetaType::Short:
            case QMetaType::Double:
            case QMetaType::Float:
            case QMetaType::ULongLong:
            case QMetaType::LongLong:
            case QMetaType::UShort:
                // Perfect case. JS doesn't have separate integer and floating point type
                return 0;
                break;
            case QMetaType::QString:
            case QMetaType::QByteArray:
            case QMetaType::QDateTime:
            case QMetaType::QDate:
                // Conversion to string should be avoided, it's probably not what we want
                return 100;
                break;
            default:
                // Other, not predicted cases
                return 5;
        }
    } else if (val->IsString() || val->IsDate() || val->IsRegExp()) {
        switch (destTypeId){
            case QMetaType::Bool:
                // Conversion from to bool should be avoided if possible, it's probably not what we want
                return 100;
            case QMetaType::UInt:
            case QMetaType::ULong:
            case QMetaType::Int:
            case QMetaType::Long:
            case QMetaType::Short:
            case QMetaType::Double:
            case QMetaType::Float:
            case QMetaType::ULongLong:
            case QMetaType::LongLong:
            case QMetaType::UShort:
                // Conversion from to number should be avoided if possible, it's probably not what we want
                return 100;
            case QMetaType::QString:
                // Perfect case
                return 0;
            case QMetaType::QByteArray:
            case QMetaType::QDateTime:
            case QMetaType::QDate:
                // String to string should be slightly preferred
                return 5;
            default:
                return 5;
        }
    } else if (val->IsBoolean()) {
        switch (destTypeId){
            case QMetaType::Bool:
                // Perfect case
                return 0;
                break;
            case QMetaType::UInt:
            case QMetaType::ULong:
            case QMetaType::Int:
            case QMetaType::Long:
            case QMetaType::Short:
            case QMetaType::Double:
            case QMetaType::Float:
            case QMetaType::ULongLong:
            case QMetaType::LongLong:
            case QMetaType::UShort:
                // Using function with bool parameter should be preferred over converted bool to nimber
                return 5;
                break;
            case QMetaType::QString:
            case QMetaType::QByteArray:
            case QMetaType::QDateTime:
            case QMetaType::QDate:
                // Bool probably shouldn't be converted to string if there are better alternatives
                return 50;
                break;
            default:
                return 5;
        }
    }
    return 0;
}

bool ScriptEngineV8::castValueToVariant(const V8ScriptValue& v8Val, QVariant& dest, int destTypeId) {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Local<v8::Context> context = getContext();
    v8::Context::Scope contextScope(context);
    const v8::Local<v8::Value> val = v8Val.constGet();

    // Conversion debugging:
    /*if (destTypeId == QMetaType::QVariant && val->IsBoolean()) {
        //It's for placing breakpoint here
        qDebug() << "Conversion Debug: " << scriptValueDebugDetailsV8(v8Val);
    }*/

    // if we're not particularly interested in a specific type, try to detect if we're dealing with a registered type
    if (destTypeId == QMetaType::UnknownType) {
        QObject* obj = ScriptObjectV8Proxy::unwrap(v8Val);
        if (obj) {
            for (const QMetaObject* metaObject = obj->metaObject(); metaObject; metaObject = metaObject->superClass()) {
                QByteArray typeName = QByteArray(metaObject->className()) + "*";
                int typeId = QMetaType::type(typeName.constData());
                if (typeId != QMetaType::UnknownType) {
                    destTypeId = typeId;
                    break;
                }
            }
        }
    }

    if (destTypeId == qMetaTypeId<ScriptValue>()) {
        dest = QVariant::fromValue(ScriptValue(new ScriptValueV8Wrapper(this, v8Val)));
        return true;
    }

    QString errorMessage("");
    // do we have a registered handler for this type?
    ScriptEngine::DemarshalFunction demarshalFunc = nullptr;
    {
        _customTypeProtect.lockForRead();
        CustomMarshalMap::const_iterator lookup = _customTypes.find(destTypeId);
        if (lookup != _customTypes.cend()) {
            demarshalFunc = lookup.value().demarshalFunc;
        }
        _customTypeProtect.unlock();
    }
    if (demarshalFunc) {
        dest = QVariant(destTypeId, static_cast<void*>(NULL));
        ScriptValue wrappedVal(new ScriptValueV8Wrapper(this, v8Val));
        Q_ASSERT(dest.constData() != nullptr);
        //V8TODO: Data should never be written to dest.constData() according to Qt documentation.
        bool success = demarshalFunc(wrappedVal, const_cast<void*>(dest.constData()));
        if(!success) dest = QVariant();
        return success;
    } else {
        switch (destTypeId) {
            case QMetaType::UnknownType:
                if (val->IsUndefined()) {
                    dest = QVariant();
                    break;
                }
                if (val->IsNull()) {
                    dest = QVariant::fromValue(nullptr);
                    break;
                }
                if (val->IsBoolean()) {
                    //V8TODO is it right isolate? What if value from different script engine is used here
                    dest = QVariant::fromValue(val->ToBoolean(_v8Isolate)->Value());
                    break;
                }
                if (val->IsString()) {
                    //V8TODO is it right context? What if value from different script engine is used here
                    v8::String::Utf8Value string(_v8Isolate, val);
                    Q_ASSERT(*string != nullptr);
                    dest = QVariant::fromValue(QString(*string));
                    //dest = QVariant::fromValue(val->ToString(_v8Context.Get(_v8Isolate)).ToLocalChecked()->);
                    break;
                }
                if (val->IsNumber()) {
                    dest = QVariant::fromValue(val->ToNumber(context).ToLocalChecked()->Value());
                    break;
                }
                {
                    QObject* obj = ScriptObjectV8Proxy::unwrap(v8Val);
                    if (obj) {
                        dest = QVariant::fromValue(obj);
                        break;
                    }
                }
                {
                    QVariant var = ScriptVariantV8Proxy::unwrap(v8Val);
                    if (var.isValid()) {
                        dest = var;
                        break;
                    }
                }
                if (val->IsArray()) {
                    if (convertJSArrayToVariant(v8::Local<v8::Array>::Cast(val), dest)) {
                        return true;
                    }
                }
                // This is for generic JS objects
                if (val->IsObject()) {
                    if (convertJSObjectToVariant(v8::Local<v8::Object>::Cast(val), dest)) {
                        break;
                    }
                }
                // V8TODO
                errorMessage = QString() + "Conversion failure: " + QString(*v8::String::Utf8Value(_v8Isolate, val->ToDetailString(getConstContext()).ToLocalChecked()))
                               + "to variant. Destination type: " + QMetaType::typeName(destTypeId) +" details: "+ scriptValueDebugDetailsV8(v8Val);
                qDebug() << errorMessage;

                //Q_ASSERT(false);
                //dest = val->ToVariant();
                break;
            case QMetaType::Bool:
                dest = QVariant::fromValue(val->ToBoolean(_v8Isolate)->Value());
                break;
            case QMetaType::QDateTime:
            case QMetaType::QDate:
                if (val->IsDate()){
                    double timeMs = v8::Date::Cast(*val)->NumberValue(context).ToChecked();
                    dest = QVariant::fromValue(QDateTime::fromMSecsSinceEpoch(timeMs));
                } else if (val->IsNumber()) {
                    //V8TODO should we automatically cast numbers to datetime?
                    dest = QVariant::fromValue(QDateTime::fromMSecsSinceEpoch(val->ToNumber(context).ToLocalChecked()->Value()));
                } else {
                    return false;
                }
                break;
            case QMetaType::UInt:
            case QMetaType::ULong:
                if ( val->IsArray() || val->IsObject() ){
                    return false;
                }
                dest = QVariant::fromValue(val->ToUint32(context).ToLocalChecked()->Value());
                break;
            case QMetaType::Int:
            case QMetaType::Long:
            case QMetaType::Short:
                if ( val->IsArray() || val->IsObject() ){
                    return false;
                }
                dest = QVariant::fromValue(val->ToInt32(context).ToLocalChecked()->Value());
                break;
            case QMetaType::Double:
            case QMetaType::Float:
            case QMetaType::ULongLong:
            case QMetaType::LongLong:
                if ( val->IsArray() || val->IsObject() ){
                    return false;
                }
                dest = QVariant::fromValue(val->ToNumber(context).ToLocalChecked()->Value());
                break;
            case QMetaType::QString:
            case QMetaType::QByteArray:
                {
                    v8::String::Utf8Value string(_v8Isolate, val);
                    Q_ASSERT(*string != nullptr);
                    dest = QVariant::fromValue(QString(*string));
                }
                break;
            case QMetaType::UShort:
                if ( val->IsArray() || val->IsObject() ){
                    return false;
                }
                dest = QVariant::fromValue(static_cast<uint16_t>(val->ToUint32(context).ToLocalChecked()->Value()));
                break;
            case QMetaType::QObjectStar:
                dest = QVariant::fromValue(ScriptObjectV8Proxy::unwrap(v8Val));
                break;
            case QMetaType::QVariant:
                if (val->IsUndefined()) {
                    dest = QVariant();
                    return true;
                }
                if (val->IsNull()) {
                    dest = QVariant::fromValue(nullptr);
                    return true;
                }
                if (val->IsBoolean()) {
                    //V8TODO is it right isolate? What if value from different script engine is used here
                    dest = QVariant::fromValue(val->BooleanValue(_v8Isolate));
                    return true;
                }
                if (val->IsString()) {
                    //V8TODO is it right context? What if value from different script engine is used here
                    v8::String::Utf8Value string(_v8Isolate, val);
                    Q_ASSERT(*string != nullptr);
                    dest = QVariant::fromValue(QString(*string));
                    //dest = QVariant::fromValue(val->ToString(_v8Context.Get(_v8Isolate)).ToLocalChecked()->);
                    return true;
                }
                if (val->IsNumber()) {
                    dest = QVariant::fromValue(val->ToNumber(context).ToLocalChecked()->Value());
                    return true;
                }
                {
                    QObject* obj = ScriptObjectV8Proxy::unwrap(v8Val);
                    if (obj) {
                        dest = QVariant::fromValue(obj);
                        return true;
                    }
                }
                {
                    QVariant var = ScriptVariantV8Proxy::unwrap(v8Val);
                    if (var.isValid()) {
                        dest = var;
                        return true;
                    }
                }
                if (val->IsArray()) {
                    if (convertJSArrayToVariant(v8::Local<v8::Array>::Cast(val), dest)) {
                        return true;
                    }
                }
                if (val->IsObject()) {
                    if (convertJSObjectToVariant(v8::Local<v8::Object>::Cast(val), dest)) {
                        return true;
                    }
                }
                //V8TODO is v8::Array to QVariant conversion used anywhere?
                errorMessage = QString() + "Conversion to variant failed: " + QString(*v8::String::Utf8Value(_v8Isolate, val->ToDetailString(getConstContext()).ToLocalChecked()))
                                       + " Destination type: " + QMetaType::typeName(destTypeId) + " Value details: " + scriptValueDebugDetailsV8(v8Val);
                qDebug() << errorMessage;
                return false;
            default:
                // check to see if this is a pointer to a QObject-derived object
                if (QMetaType::typeFlags(destTypeId) & (QMetaType::PointerToQObject | QMetaType::TrackingPointerToQObject)) {
                    /* Do we really want to permit regular passing of nullptr to native functions?
                    if (!val.isValid() || val.isUndefined() || val.isNull()) {
                        dest = QVariant::fromValue(nullptr);
                        break;
                    }*/
                    QObject* obj = ScriptObjectV8Proxy::unwrap(v8Val);
                    if (!obj) return false;
                    const QMetaObject* destMeta = QMetaType::metaObjectForType(destTypeId);
                    Q_ASSERT(destMeta);
                    obj = destMeta->cast(obj);
                    if (!obj) return false;
                    dest = QVariant::fromValue(obj);
                    break;
                }
                // check to see if we have a registered prototype
                {
                    QVariant var = ScriptVariantV8Proxy::unwrap(v8Val);
                    if (var.isValid()) {
                        dest = var;
                        break;
                    }
                }
                // last chance, just convert it to a variant (impossible on V8)
                // V8TODO
                errorMessage = QString() + "Conversion failure: " + QString(*v8::String::Utf8Value(_v8Isolate, val->ToDetailString(getConstContext()).ToLocalChecked()))
                         + "to variant. Destination type: " + QMetaType::typeName(destTypeId);
                qDebug() << errorMessage;
                if(destTypeId == QMetaType::QVariant) {
                    Q_ASSERT(false);
                }
                //dest = val->ToVariant();
                break;
        }
    }

    return destTypeId == QMetaType::UnknownType || dest.userType() == destTypeId || dest.convert(destTypeId);
}

bool ScriptEngineV8::convertJSArrayToVariant(v8::Local<v8::Array> array, QVariant &dest) {
    v8::HandleScope handleScope(_v8Isolate);
    auto context = getContext();
    int length = array->Length();
    QList<QVariant> properties;
    for (int i = 0; i < length; i++) {
        v8::Local<v8::Value> v8Property;
        if (!array->Get(context, i).ToLocal(&v8Property)) {
            qDebug() << "ScriptEngineV8::convertJSArrayToVariant could not get property: " + QString(i);
            continue;
        }
        QVariant property;
        // Maybe QMetaType::QVariant?
        if (castValueToVariant(V8ScriptValue(this, v8Property), property, QMetaType::UnknownType)) {
            properties.append(property);
        } else {
            qDebug() << "ScriptEngineV8::convertJSArrayToVariant could cast property to variant: " + QString(i);
            ;
        }
    }
    dest = QVariant(properties);
    return true;
}

bool ScriptEngineV8::convertJSObjectToVariant(v8::Local<v8::Object> object, QVariant &dest) {
    v8::HandleScope handleScope(_v8Isolate);
    auto context = getContext();
    v8::Local<v8::Array> names;
    if(!object->GetPropertyNames(context).ToLocal(&names)) {
        qDebug() << "ScriptEngineV8::convertJSObjectToVariant could not get property names";
        return false;
    }
    int length = names->Length();
    QMap<QString, QVariant> properties;
    for (int i = 0; i < length; i++) {
        v8::Local<v8::Value> v8Property;
        QString name = *v8::String::Utf8Value(_v8Isolate, names->Get(context, i).ToLocalChecked());
        if (!object->Get(context, names->Get(context, i).ToLocalChecked()).ToLocal(&v8Property)) {
            qDebug() << "ScriptEngineV8::convertJSObjectToVariant could not get property: " + name;
            continue;
        }
        QVariant property;
        // Maybe QMetaType::QVariant?
        if (castValueToVariant(V8ScriptValue(this, v8Property), property, QMetaType::UnknownType)) {
            properties.insert( name, property);
        } else {
            qDebug() << "ScriptEngineV8::convertJSObjectToVariant could cast property to variant: " + name;
            ;
        }
    }
    dest = QVariant(properties);
    return true;
}

QString ScriptEngineV8::valueType(const V8ScriptValue& v8Val) {
    // V8TODO I'm not sure why is there a TODO here
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Local<v8::Context> context = getContext();
    v8::Context::Scope contextScope(context);

    //v8::HandleScope handleScope(const_cast<v8::Isolate*>(v8Val.constGetIsolate()));
    const v8::Local<v8::Value> val = v8Val.constGet();
    
    if (val->IsUndefined()) {
        return "undefined";
    }
    if (val->IsNull()) {
        return "null";
    }
    if (val->IsBoolean()) {
        return "boolean";
    }
    if (val->IsString()) {
        return "string";
    }
    if (val->IsNumber()) {
        return "number";
    }
    {
        QObject* obj = ScriptObjectV8Proxy::unwrap(v8Val);
        if (obj) {
            QString objectName = obj->objectName();
            if (!objectName.isEmpty()) return objectName;
            return obj->metaObject()->className();
        }
    }
    {
        QVariant var = ScriptVariantV8Proxy::unwrap(v8Val);
        if (var.isValid()) {
            return var.typeName();
        }
    }
    QVariant dest;
    if (castValueToVariant(v8Val, dest, QMetaType::QVariant)) {
        return dest.typeName();
    }
    qDebug() << "Cast to variant failed";
    // V8TODO: what to return here?
    return "undefined";
}

V8ScriptValue ScriptEngineV8::castVariantToValue(const QVariant& val) {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Local<v8::Context> context = getContext();
    v8::Context::Scope contextScope(context);

    int valTypeId = val.userType();

    if (valTypeId == qMetaTypeId<ScriptValue>()) {
        // this is a wrapped ScriptValue, so just unwrap it and call it good
        ScriptValue innerVal = val.value<ScriptValue>();
        return ScriptValueV8Wrapper::fullUnwrap(this, innerVal);
    }

    // do we have a registered handler for this type?
    ScriptEngine::MarshalFunction marshalFunc = nullptr;
    {
        _customTypeProtect.lockForRead();
        CustomMarshalMap::const_iterator lookup = _customTypes.find(valTypeId);
        if (lookup != _customTypes.cend()) {
            marshalFunc = lookup.value().marshalFunc;
        }
        _customTypeProtect.unlock();
    }
    if (marshalFunc) {
        Q_ASSERT(val.constData() != nullptr);
        ScriptValue wrappedVal = marshalFunc(this, val.constData());
        return ScriptValueV8Wrapper::fullUnwrap(this, wrappedVal);
    }

    switch (valTypeId) {
        case QMetaType::UnknownType:
        case QMetaType::Void:
            return V8ScriptValue(this, v8::Undefined(_v8Isolate));
        case QMetaType::Nullptr:
            return V8ScriptValue(this, v8::Null(_v8Isolate));
        case QMetaType::Bool:
            return V8ScriptValue(this, v8::Boolean::New(_v8Isolate, val.toBool()));
        case QMetaType::Int:
        case QMetaType::Long:
        case QMetaType::Short:
            return V8ScriptValue(this, v8::Integer::New(_v8Isolate, val.toInt()));
        case QMetaType::UInt:
        case QMetaType::UShort:
        case QMetaType::ULong:
            return V8ScriptValue(this, v8::Uint32::New(_v8Isolate, val.toUInt()));
        case QMetaType::ULongLong:
            return V8ScriptValue(this, v8::Number::New(_v8Isolate, val.toULongLong()));
        case QMetaType::LongLong:
            return V8ScriptValue(this, v8::Number::New(_v8Isolate, val.toLongLong()));
        case QMetaType::Float:
        case QMetaType::Double:
            return V8ScriptValue(this, v8::Number::New(_v8Isolate, val.toDouble()));
        case QMetaType::QString:
        case QMetaType::QByteArray:
            return V8ScriptValue(this, v8::String::NewFromUtf8(_v8Isolate, val.toString().toStdString().c_str()).ToLocalChecked());
        case QMetaType::QVariant:
            return castVariantToValue(val.value<QVariant>());
        case QMetaType::QObjectStar: {
            QObject* obj = val.value<QObject*>();
            if (obj == nullptr) return V8ScriptValue(this, v8::Null(_v8Isolate));
            return ScriptObjectV8Proxy::newQObject(this, obj);
        }
        case QMetaType::QDateTime:
            {
                double timeMs = val.value<QDateTime>().currentMSecsSinceEpoch();
                return V8ScriptValue(this, v8::Date::New(getContext(), timeMs).ToLocalChecked());
            }
        case QMetaType::QDate:
            {
                double timeMs = val.value<QDate>().startOfDay().currentMSecsSinceEpoch();
                return V8ScriptValue(this, v8::Date::New(getContext(), timeMs).ToLocalChecked());
            }
        default:
            // check to see if this is a pointer to a QObject-derived object
            if (QMetaType::typeFlags(valTypeId) & (QMetaType::PointerToQObject | QMetaType::TrackingPointerToQObject)) {
                QObject* obj = val.value<QObject*>();
                if (obj == nullptr) return V8ScriptValue(this, v8::Null(_v8Isolate));
                return ScriptObjectV8Proxy::newQObject(this, obj);
            }
            // have we set a prototype'd variant?
            {
                _customTypeProtect.lockForRead();
                CustomPrototypeMap::const_iterator lookup = _customPrototypes.find(valTypeId);
                if (lookup != _customPrototypes.cend()) {
                    return ScriptVariantV8Proxy::newVariant(this, val, lookup.value());
                }
                _customTypeProtect.unlock();
            }
            // just do a generic variant
            //V8TODO
            Q_ASSERT(false);
            return V8ScriptValue(this, v8::Undefined(_v8Isolate));
            //return QScriptEngine::newVariant(val);
    }
}
