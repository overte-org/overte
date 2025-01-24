//
//  ScriptObjectV8Proxy.cpp
//  libraries/script-engine/src/v8
//
//  Created by Heather Anderson on 12/5/21.
//  Modified for V8 by dr Karol Suprynowicz on 2022/10/08
//  Copyright 2021 Vircadia contributors.
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "ScriptObjectV8Proxy.h"

#include <QElapsedTimer>
#include <QtCore/QList>
#include <QtCore/QSharedPointer>

#include "../ScriptEngineLogging.h"

#include "ScriptContextV8Wrapper.h"
#include "ScriptValueV8Wrapper.h"
#include "ScriptEngineLoggingV8.h"

Q_DECLARE_METATYPE(ScriptValue)

Q_DECLARE_METATYPE(QSharedPointer<ScriptObjectV8Proxy>)
Q_DECLARE_METATYPE(QSharedPointer<ScriptVariantV8Proxy>)


// These values are put into internal fields of V8 objects, to signalize what kind of data is the pointer in another
// internal field pointing to. Proxy unwrapping functions recognize proxies by checking for these values in internal field 0
// of V8 object.

// Value of internal field with index 0 when object contains ScriptObjectV8Proxy pointer in internal field 1
static const void *internalPointsToQObjectProxy = (void *)0x13370000;
// Internal field value of object pointing to ScriptObjectV8Proxy is changed to this value upon proxy's deletion
static const void *internalPointsToDeletedQObjectProxy = (void *)0x13370010;
static const void *internalPointsToQVariantProxy = (void *)0x13371000;
//static const void *internalPointsToSignalProxy = (void *)0x13372000;
static const void *internalPointsToMethodProxy = (void *)0x13373000;
// This is used to pass object in ScriptVariantV8Proxy to methods of prototype object, for example passing AnimationPointer to AnimationObject
// Object is then converted using scriptvalue_cast for use inside the prototype
static const void *internalPointsToQVariantInProxy = (void *)0x13374000;

// Used strictly to replace the "this" object value for property access.  May expand to a full context element
// if we find it necessary to, but hopefully not needed
class ScriptPropertyContextV8Wrapper final : public ScriptContext {
public:  // construction
    inline ScriptPropertyContextV8Wrapper(const ScriptValue& object, ScriptContext* parentContext) :
        _parent(parentContext), _object(object) {}

public:  // ScriptContext implementation
    virtual int argumentCount() const override { return _parent->argumentCount(); }
    virtual ScriptValue argument(int index) const override { return _parent->argument(index); }
    virtual QStringList backtrace() const override { return _parent->backtrace(); }

    // Name of the file in which message was generated. Empty string when no file name is available.
    virtual int currentLineNumber() const override { return _parent->currentLineNumber(); }

    // Number of the line on which message was generated. -1 if there line number is not available.
    virtual QString currentFileName() const override { return _parent->currentFileName(); }

    virtual ScriptValue callee() const override { return _parent->callee(); }
    virtual ScriptEnginePointer engine() const override { return _parent->engine(); }
    virtual ScriptFunctionContextPointer functionContext() const override { return _parent->functionContext(); }
    virtual ScriptContextPointer parentContext() const override { return _parent->parentContext(); }
    virtual ScriptValue thisObject() const override { return _object; }
    virtual ScriptValue throwError(const QString& text) override { return _parent->throwError(text); }
    virtual ScriptValue throwValue(const ScriptValue& value) override { return _parent->throwValue(value); }

private:  // storage
    ScriptContext* _parent;
    const ScriptValue& _object;
};

ScriptObjectV8Proxy::ScriptObjectV8Proxy(ScriptEngineV8* engine, QObject* object, bool ownsObject, const ScriptEngine::QObjectWrapOptions& options) :
    _engine(engine), _wrapOptions(options), _ownsObject(ownsObject), _object(object) {
    Q_ASSERT(_engine != nullptr);
    investigate();
}

V8ScriptValue ScriptObjectV8Proxy::newQObject(ScriptEngineV8* engine, QObject* object,
                                             ScriptEngine::ValueOwnership ownership,
                                             const ScriptEngine::QObjectWrapOptions& options) {
    // do we already have a valid wrapper for this QObject?
    {
        QMutexLocker guard(&engine->_qobjectWrapperMapProtect);
        ScriptEngineV8::ObjectWrapperMap::const_iterator lookup = engine->_qobjectWrapperMap.find(object);
        if (lookup != engine->_qobjectWrapperMap.end()) {
            QSharedPointer<ScriptObjectV8Proxy> proxy = lookup.value().lock();
            if (proxy) {
                return V8ScriptValue(engine, proxy.get()->toV8Value());
            }
        }
    }

    bool ownsObject;
    switch (ownership) {
        case ScriptEngine::QtOwnership:
            ownsObject = false;
            break;
        case ScriptEngine::ScriptOwnership:
            ownsObject = true;
            break;
        case ScriptEngine::AutoOwnership:
            ownsObject = !object->parent();
            break;
        default:
            ownsObject = false;
            qCCritical(scriptengine_v8) << "Wrong ScriptEngine::ValueOwnership value: " << ownership;
            break;
    }

    // create the wrapper
    auto proxy = QSharedPointer<ScriptObjectV8Proxy>::create(engine, object, ownsObject, options);

    {
        QMutexLocker guard(&engine->_qobjectWrapperMapProtect);

        // check again to see if someone else created the wrapper while we were busy
        ScriptEngineV8::ObjectWrapperMap::const_iterator lookup = engine->_qobjectWrapperMap.find(object);
        if (lookup != engine->_qobjectWrapperMap.end()) {
            QSharedPointer<ScriptObjectV8Proxy> proxy = lookup.value().lock();
            if (proxy) {
                return V8ScriptValue(engine, proxy.get()->toV8Value());
            }
        }
        // register the wrapper with the engine and make sure it cleans itself up
        engine->_qobjectWrapperMap.insert(object, proxy);
        engine->_qobjectWrapperMapV8.insert(object, proxy);
        QPointer<ScriptEngineV8> enginePtr = engine;
        object->connect(object, &QObject::destroyed, engine, [enginePtr, object]() {
            if (!enginePtr) return;
            // Lookup needs to be done twice, because _qobjectWrapperMapProtect must not be locked when disconnecting signals
            QSharedPointer<ScriptObjectV8Proxy> proxy;
            {
                QMutexLocker guard(&enginePtr->_qobjectWrapperMapProtect);
                auto lookupV8 = enginePtr->_qobjectWrapperMapV8.find(object);
                if (lookupV8 != enginePtr->_qobjectWrapperMapV8.end()) {
                    proxy = lookupV8.value();
                }
            }
            if (proxy) {
                for (auto signal : proxy->_signalInstances) {
                    if (signal) {
                        signal->disconnectAllScriptSignalProxies();
                    }
                }
            }

            QMutexLocker guard(&enginePtr->_qobjectWrapperMapProtect);
            ScriptEngineV8::ObjectWrapperMap::iterator lookup = enginePtr->_qobjectWrapperMap.find(object);
            if (lookup != enginePtr->_qobjectWrapperMap.end()) {
                enginePtr->_qobjectWrapperMap.erase(lookup);
            }
            auto lookupV8 = enginePtr->_qobjectWrapperMapV8.find(object);
            if (lookupV8 != enginePtr->_qobjectWrapperMapV8.end()) {
                enginePtr->_qobjectWrapperMapV8.erase(lookupV8);
            }
        });
    }

    return V8ScriptValue(engine, proxy.get()->toV8Value());
}

ScriptObjectV8Proxy* ScriptObjectV8Proxy::unwrapProxy(const V8ScriptValue& val) {
    auto isolate = val.getEngine()->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Local<v8::Context> context = val.constGetContext();
    v8::Context::Scope contextScope(context);
    auto v8Value = val.constGet();
    Q_ASSERT(!v8Value.IsEmpty());

    if (v8Value->IsNullOrUndefined()) {
        return nullptr;
    }
    if (!v8Value->IsObject()) {
        return nullptr;
    }
    v8::Local<v8::Object> v8Object = v8::Local<v8::Object>::Cast(v8Value);
    if (v8Object->InternalFieldCount() != 3) {
        return nullptr;
    }
    if (v8Object->GetAlignedPointerFromInternalField(0) != internalPointsToQObjectProxy) {
        qCDebug(scriptengine_v8) << "Cannot unwrap proxy - internal fields don't point to object proxy";
        return nullptr;
    }
    return reinterpret_cast<ScriptObjectV8Proxy*>(v8Object->GetAlignedPointerFromInternalField(1));
}

ScriptObjectV8Proxy* ScriptObjectV8Proxy::unwrapProxy(v8::Isolate* isolate, v8::Local<v8::Value> &value) {
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);

    if (value->IsNullOrUndefined()) {
        return nullptr;
    }
    if (!value->IsObject()) {
        return nullptr;
    }
    v8::Local<v8::Object> v8Object = v8::Local<v8::Object>::Cast(value);
    if (v8Object->InternalFieldCount() != 3) {
        return nullptr;
    }
    if (v8Object->GetAlignedPointerFromInternalField(0) != internalPointsToQObjectProxy) {
        qCDebug(scriptengine_v8) << "Cannot unwrap proxy - internal fields don't point to object proxy";
        return nullptr;
    }

    ScriptObjectV8Proxy* proxy = reinterpret_cast<ScriptObjectV8Proxy*>(v8Object->GetAlignedPointerFromInternalField(1));
    if (proxy) {
        Q_ASSERT(!proxy->_wasDestroyed);
    }

    return proxy;
}

QObject* ScriptObjectV8Proxy::unwrap(const V8ScriptValue& val) {
    ScriptObjectV8Proxy* proxy = unwrapProxy(val);
    return proxy ? proxy->toQObject() : nullptr;
}

ScriptObjectV8Proxy::~ScriptObjectV8Proxy() {
    for (auto signal : _signalInstances) {
        if (signal) {
            signal->disconnectAllScriptSignalProxies();
        }
    }
    _wasDestroyed = true;
    if (_ownsObject) {
        auto isolate = _engine->getIsolate();
        v8::Locker locker(isolate);
        v8::Isolate::Scope isolateScope(isolate);
        v8::HandleScope handleScope(isolate);
        _v8Object.Reset();
        QObject* qobject = _object;
        if(qobject) qobject->deleteLater();
    } else {
        auto isolate = _engine->getIsolate();
        v8::Locker locker(isolate);
        v8::Isolate::Scope isolateScope(isolate);
        v8::HandleScope handleScope(isolate);
        if (_object)
            qCDebug(scriptengine_v8) << "Deleting object proxy: " << name();
        Q_ASSERT(!_v8Object.Get(isolate)->IsNullOrUndefined());
        // This prevents unwrap function from unwrapping proxy that was deleted
        _v8Object.Get(isolate)->SetAlignedPointerInInternalField(0, const_cast<void*>(internalPointsToDeletedQObjectProxy));
        _v8Object.Reset();
    }
}

void ScriptObjectV8Proxy::investigate() {
    QObject* qobject = _object;
    if (!qobject) {
        QStringList backtrace = _engine->currentContext()->backtrace();
        qCDebug(scriptengine_v8) << "ScriptObjectV8Proxy::investigate: Object pointer is NULL, " << backtrace;
    }
    if (!qobject) return;

    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(_engine->getIsolate());
    auto context = _engine->getContext();
    v8::Context::Scope contextScope(context);

    const QMetaObject* metaObject = qobject->metaObject();

    auto objectTemplate = _engine->getObjectProxyTemplate();

    if (QString("ConsoleScriptingInterface") == metaObject->className()) {
        printf("ConsoleScriptingInterface");
    }
    // discover properties
    int startIdx = _wrapOptions & ScriptEngine::ExcludeSuperClassProperties ? metaObject->propertyOffset() : 0;
    int num = metaObject->propertyCount();
    for (int idx = startIdx; idx < num; ++idx) {
        QMetaProperty prop = metaObject->property(idx);
        if (!prop.isScriptable()) continue;

        // always exclude child objects (at least until we decide otherwise)
        int metaTypeId = prop.userType();
        if (metaTypeId != QMetaType::UnknownType) {
            QMetaType metaType(metaTypeId);
            if (metaType.flags() & QMetaType::PointerToQObject) {
                continue;
            }
        }

        PropertyDef& propDef = _props.insert(idx, PropertyDef(prop.name(), idx)).value();
        _propNameMap.insert(prop.name(), &propDef);
        propDef.flags = ScriptValue::Undeletable | ScriptValue::PropertyGetter | ScriptValue::PropertySetter |
                        ScriptValue::QObjectMember;
        if (prop.isConstant()) propDef.flags |= ScriptValue::ReadOnly;
    }

    // discover methods
    startIdx = (_wrapOptions & ScriptEngine::ExcludeSuperClassMethods) ? metaObject->methodOffset() : 0;
    num = metaObject->methodCount();
    QHash<QString, int> methodNames;
    for (int idx = startIdx; idx < num; ++idx) {
        QMetaMethod method = metaObject->method(idx);

        // perhaps keep this comment?  Calls (like AudioScriptingInterface::playSound) seem to expect non-public methods to be script-accessible
        if (method.access() == QMetaMethod::Private) continue;

        bool isSignal = false;
        QByteArray szName = method.name();

        switch (method.methodType()) {
            case QMetaMethod::Constructor:
                continue;
            case QMetaMethod::Signal:
                if (szName == "destroyed") {
                    continue;
                }
                if (szName == "objectNameChanged") {
                    continue;
                }
                isSignal = true;
                break;
            case QMetaMethod::Slot:
                if (_wrapOptions & ScriptEngine::ExcludeSlots) {
                    continue;
                }
                if (szName == "deleteLater") {
                    continue;
                }
                break;
            default:
                break;
        }

        auto nameString = v8::String::NewFromUtf8(_engine->getIsolate(), szName.data(), v8::NewStringType::kNormal, szName.length()).ToLocalChecked();
        V8ScriptString name(_engine, nameString);
        auto nameLookup = methodNames.find(szName);
        if (isSignal) {
            if (nameLookup == methodNames.end()) {
                SignalDef& signalDef = _signals.insert(idx, SignalDef(szName, idx)).value();
                signalDef.name = szName;
                signalDef.signal = method;
                _signalNameMap.insert(szName, &signalDef);
                methodNames.insert(szName, idx);
            } else {
                int originalMethodId = nameLookup.value();
                SignalDefMap::iterator signalLookup = _signals.find(originalMethodId);
                Q_ASSERT(signalLookup != _signals.end());
                SignalDef& signalDef = signalLookup.value();
                Q_ASSERT(signalDef.signal.parameterCount() != method.parameterCount());
                if (signalDef.signal.parameterCount() < method.parameterCount()) {
                    signalDef.signal = method;
                }
            }
        } else {
            int parameterCount = method.parameterCount();
            if(method.returnType() == QMetaType::UnknownType) {
                qCCritical(scriptengine_v8) << "Method " << metaObject->className() << "::" << name.toQString() << " has QMetaType::UnknownType return value";
            }
            for (int i = 0; i < method.parameterCount(); i++) {
                if (method.parameterType(i) == QMetaType::UnknownType) {
                    qCCritical(scriptengine_v8) << "Parameter " << i << "in method " << metaObject->className() << "::" << name.toQString() << " is of type QMetaType::UnknownType";
                }
            }
            if (nameLookup == methodNames.end()) {
                MethodDef& methodDef = _methods.insert(idx, MethodDef(szName, idx)).value();
                methodDef.name = szName;
                methodDef.numMaxParams = parameterCount;
                methodDef.methods.append(method);
                _methodNameMap.insert(szName, &methodDef);
                methodNames.insert(szName, idx);
            } else {
                int originalMethodId = nameLookup.value();
                MethodDefMap::iterator methodLookup = _methods.find(originalMethodId);
                Q_ASSERT(methodLookup != _methods.end());
                MethodDef& methodDef = methodLookup.value();
                if(methodDef.numMaxParams < parameterCount) methodDef.numMaxParams = parameterCount;
                methodDef.methods.append(method);
            }
        }
    }

    v8::Local<v8::Object> v8Object = objectTemplate->NewInstance(context).ToLocalChecked();

    v8Object->SetAlignedPointerInInternalField(0, const_cast<void*>(internalPointsToQObjectProxy));
    v8Object->SetAlignedPointerInInternalField(1, reinterpret_cast<void*>(this));

    _v8Object.Reset(_engine->getIsolate(), v8Object);
    if (_ownsObject) {
        _v8Object.SetWeak(this, weakHandleCallback, v8::WeakCallbackType::kParameter);
    }

    // Properties added later will be stored in this object
    v8::Local<v8::Object> propertiesObject = v8::Object::New(_engine->getIsolate());
    v8Object->SetInternalField(2, propertiesObject);

    // Add all the methods objects as properties - this allows adding properties to a given method later. Is used by Script.request.
    for (auto i = _methods.begin(); i != _methods.end(); i++) {
        V8ScriptValue method = ScriptMethodV8Proxy::newMethod(_engine, qobject, V8ScriptValue(_engine, v8Object),
                                                              i.value().methods, i.value().numMaxParams);
        if(!propertiesObject->Set(context, v8::String::NewFromUtf8(isolate, i.value().name.toStdString().c_str()).ToLocalChecked(), method.get()).FromMaybe(false)) {
            Q_ASSERT(false);
        }
    }
}

void ScriptObjectV8Proxy::weakHandleCallback(const v8::WeakCallbackInfo<ScriptObjectV8Proxy>& info) {
    auto proxy = info.GetParameter();
    proxy->_v8Object.Reset();
    info.GetParameter()->_object->deleteLater();
}

QString ScriptObjectV8Proxy::name() const {
    Q_ASSERT(_object);
    if (!_object) return "";
    return _object ? _object->objectName() : "";
    QString objectName = _object->objectName();
    if (!objectName.isEmpty()) return objectName;
    return _object->metaObject()->className();
}

ScriptObjectV8Proxy::QueryFlags ScriptObjectV8Proxy::queryProperty(const V8ScriptValue& object, const V8ScriptString& name, QueryFlags flags, uint* id) {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Local<v8::Context> context = _engine->getContext();
    v8::Context::Scope contextScope(context);
    QString nameStr(*v8::String::Utf8Value(isolate, name.constGet()));
    // V8TODO: registering methods on V8 side would make API calls a lot faster

    // check for methods
    MethodNameMap::const_iterator method = _methodNameMap.find(nameStr);
    if (method != _methodNameMap.cend()) {
        const MethodDef* methodDef = method.value();
        *id = methodDef->_id | METHOD_TYPE;
        return flags & (HandlesReadAccess | HandlesWriteAccess);
    }

    // check for properties
    PropertyNameMap::const_iterator prop = _propNameMap.find(nameStr);
    if (prop != _propNameMap.cend()) {
        const PropertyDef* propDef = prop.value();
        *id = propDef->_id | PROPERTY_TYPE;
        return flags & (HandlesReadAccess | HandlesWriteAccess);
    }

    // check for signals
    // V8TODO: this should use _signalNameMap QHash for faster search
    for (SignalDefMap::const_iterator trans = _signals.cbegin(); trans != _signals.cend(); ++trans) {
        if (!(trans.value().name == nameStr)) continue;
        *id = trans.key() | SIGNAL_TYPE;
        return flags & (HandlesReadAccess | HandlesWriteAccess);
    }

    return QueryFlags();
}

ScriptValue::PropertyFlags ScriptObjectV8Proxy::propertyFlags(const V8ScriptValue& object, const V8ScriptString& name, uint id) {
    QObject* qobject = _object;
    if (!qobject) {
        return ScriptValue::PropertyFlags();
    }

    switch (id & TYPE_MASK) {
        case PROPERTY_TYPE: {
            PropertyDefMap::const_iterator lookup = _props.find(id & ~TYPE_MASK);
            if (lookup == _props.cend()) return ScriptValue::PropertyFlags();
            const PropertyDef& propDef = lookup.value();
            return propDef.flags;
        }
        case METHOD_TYPE: {
            MethodDefMap::const_iterator lookup = _methods.find(id & ~TYPE_MASK);
            if (lookup == _methods.cend()) return ScriptValue::PropertyFlags();
            return ScriptValue::ReadOnly | ScriptValue::Undeletable | ScriptValue::QObjectMember;
        }
        case SIGNAL_TYPE: {
            SignalDefMap::const_iterator lookup = _signals.find(id & ~TYPE_MASK);
            if (lookup == _signals.cend()) return ScriptValue::PropertyFlags();
            return ScriptValue::ReadOnly | ScriptValue::Undeletable | ScriptValue::QObjectMember;
        }
    }
    return ScriptValue::PropertyFlags();
}

void ScriptObjectV8Proxy::v8Get(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
    v8::HandleScope handleScope(info.GetIsolate());
    v8::String::Utf8Value utf8Value(info.GetIsolate(), name);
    v8::Local<v8::Value> objectV8 = info.This();
    ScriptObjectV8Proxy *proxy = ScriptObjectV8Proxy::unwrapProxy(info.GetIsolate(), objectV8);
    if (!proxy) {
        qCDebug(scriptengine_v8) << "Proxy object not found when getting: " << *utf8Value;
        return;
    }
    V8ScriptValue object(proxy->_engine, objectV8);
    if (!name->IsString() && !name->IsSymbol()) {
        QString notStringMessage("ScriptObjectV8Proxy::v8Get: " + proxy->_engine->scriptValueDebugDetailsV8(V8ScriptValue(proxy->_engine, name)));
        qCDebug(scriptengine_v8) << notStringMessage;
        Q_ASSERT(false);
    }
    v8::Local<v8::String> v8NameString;

    ContextScopeV8 contextScopeV8(proxy->_engine);

    if (name->IsString()) {
        V8ScriptString nameString(proxy->_engine, v8::Local<v8::String>::Cast(name));
        uint id;
        QueryFlags flags = proxy->queryProperty(object, nameString, HandlesReadAccess, &id);
        if (flags) {
            V8ScriptValue value = proxy->property(object, nameString, id);
            info.GetReturnValue().Set(value.get());
            return;
        }
    }

    v8::Local<v8::Value> property;
    if(info.This()->GetInternalField(2).As<v8::Object>()->Get(proxy->_engine->getContext(), name).ToLocal(&property)) {
        info.GetReturnValue().Set(property);
    } else {
        qCDebug(scriptengine_v8) << "Value not found: " << *utf8Value;
    }
}

void ScriptObjectV8Proxy::v8Set(v8::Local<v8::Name> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<v8::Value>& info) {
    v8::HandleScope handleScope(info.GetIsolate());
    v8::String::Utf8Value utf8Value(info.GetIsolate(), name);
    v8::Local<v8::Value> objectV8 = info.This();
    ScriptObjectV8Proxy *proxy = ScriptObjectV8Proxy::unwrapProxy(info.GetIsolate(), objectV8);
    if (!proxy) {
        qCDebug(scriptengine_v8) << "Proxy object not found when setting: " << *utf8Value;
        return;
    }
    V8ScriptValue object(proxy->_engine, objectV8);
    if (!name->IsString() && !name->IsSymbol()) {
        QString notStringMessage("ScriptObjectV8Proxy::v8Set: " + proxy->_engine->scriptValueDebugDetailsV8(V8ScriptValue(proxy->_engine, name)));
        qCDebug(scriptengine_v8) << notStringMessage;
        Q_ASSERT(false);
    }

    ContextScopeV8 contextScopeV8(proxy->_engine);

    if (name->IsString()) {
        V8ScriptString nameString(proxy->_engine, v8::Local<v8::String>::Cast(name));
        uint id;
        QueryFlags flags = proxy->queryProperty(object, nameString, HandlesWriteAccess, &id);
        if (flags) {
            proxy->setProperty(object, nameString, id, V8ScriptValue(proxy->_engine, value));
            info.GetReturnValue().Set(value);
            return;
        }
    }

    if (info.This()->GetInternalField(2).As<v8::Object>()->Set(proxy->_engine->getContext(), name, value).FromMaybe(false)) {
        info.GetReturnValue().Set(value);
    } else {
        qCDebug(scriptengine_v8) << "Set failed: " << *utf8Value;
    }
}

void ScriptObjectV8Proxy::v8GetPropertyNames(const v8::PropertyCallbackInfo<v8::Array>& info) {
    v8::HandleScope handleScope(info.GetIsolate());
    auto context = info.GetIsolate()->GetCurrentContext();
    v8::Context::Scope contextScope(context);
    v8::Local<v8::Value> objectV8 = info.This();
    ScriptObjectV8Proxy *proxy = ScriptObjectV8Proxy::unwrapProxy(info.GetIsolate(), objectV8);
    if (!proxy) {
        qCDebug(scriptengine_v8) << "ScriptObjectV8Proxy::v8GetPropertyNames: Proxy object not found when listing";
        return;
    }
    V8ScriptValue object(proxy->_engine, objectV8);
    v8::Local<v8::Array> properties = proxy->getPropertyNames();
    v8::Local<v8::Array> objectProperties;
    uint32_t propertiesLength = properties->Length();
    if (info.This()->GetInternalField(2).As<v8::Object>()->GetPropertyNames(context).ToLocal(&objectProperties)) {
        for (uint32_t n = 0; n < objectProperties->Length(); n++) {
            if(!properties->Set(context, propertiesLength+n, objectProperties->Get(context, n).ToLocalChecked()).FromMaybe(false)) {
                qCDebug(scriptengine_v8) << "ScriptObjectV8Proxy::v8GetPropertyNames: Cannot add member name";
            }
        }
    }
    info.GetReturnValue().Set(properties);
}

v8::Local<v8::Array> ScriptObjectV8Proxy::getPropertyNames() {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::EscapableHandleScope handleScope(_engine->getIsolate());
    auto context = _engine->getContext();
    v8::Context::Scope contextScope(context);

    //V8TODO: this is really slow. It could be cached if this is called often.
    v8::Local<v8::Array> properties = v8::Array::New(isolate, _props.size() + _methods.size() + _signals.size());
    uint32_t position = 0;
    for (PropertyDefMap::const_iterator i = _props.begin(); i != _props.end(); i++){
        v8::Local<v8::String> name = v8::String::NewFromUtf8(isolate, i.value().name.toStdString().c_str()).ToLocalChecked();
        if(!properties->Set(context, position++, name).FromMaybe(false)) {
            qCDebug(scriptengine_v8) << "ScriptObjectV8Proxy::getPropertyNames: Cannot add property member name";
        }
    }
    for (MethodDefMap::const_iterator i = _methods.begin(); i != _methods.end(); i++){
        v8::Local<v8::String> name = v8::String::NewFromUtf8(isolate, i.value().name.toStdString().c_str()).ToLocalChecked();
        if(!properties->Set(context, position++, name).FromMaybe(false)) {
            qCDebug(scriptengine_v8) << "ScriptObjectV8Proxy::getPropertyNames: Cannot add property member name";
        }
    }
    for (SignalDefMap::const_iterator i = _signals.begin(); i != _signals.end(); i++){
        v8::Local<v8::String> name = v8::String::NewFromUtf8(isolate, i.value().name.toStdString().c_str()).ToLocalChecked();
        if(!properties->Set(context, position++, name).FromMaybe(false)) {
            qCDebug(scriptengine_v8) << "ScriptObjectV8Proxy::getPropertyNames: Cannot add property member name";
        }
    }
    return handleScope.Escape(properties);
}


V8ScriptValue ScriptObjectV8Proxy::property(const V8ScriptValue& object, const V8ScriptString& name, uint id) {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    auto context = _engine->getContext();
    v8::Context::Scope contextScope(context);
    QObject* qobject = _object;
    if (!qobject) {
        _engine->getIsolate()->ThrowError("Referencing deleted native object");
        return V8ScriptValue(_engine, v8::Null(isolate));
    }

    const QMetaObject* metaObject = qobject->metaObject();

    switch (id & TYPE_MASK) {
        case PROPERTY_TYPE: {
            int propId = id & ~TYPE_MASK;
            PropertyDefMap::const_iterator lookup = _props.find(propId);
            if (lookup == _props.cend()) return V8ScriptValue(_engine, v8::Null(isolate));

            QMetaProperty prop = metaObject->property(propId);
            ScriptValue scriptThis = ScriptValue(new ScriptValueV8Wrapper(_engine, object));
            ScriptPropertyContextV8Wrapper ourContext(scriptThis, _engine->currentContext());
            ScriptContextGuard guard(&ourContext);

            QVariant varValue = prop.read(qobject);
            return _engine->castVariantToValue(varValue);
        }
        case METHOD_TYPE: {
            int methodId = id & ~TYPE_MASK;
            MethodDefMap::const_iterator lookup = _methods.find(methodId);
            if (lookup == _methods.cend()) return V8ScriptValue(_engine, v8::Null(isolate));
            const MethodDef& methodDef = lookup.value();
            for (auto iter = methodDef.methods.begin(); iter != methodDef.methods.end(); iter++ ) {
                if((*iter).returnType() == QMetaType::UnknownType) {
                    qCDebug(scriptengine_v8) << "Method with QMetaType::UnknownType " << metaObject->className() << " " << (*iter).name();
                }
            } //V8TODO: is new method created during every call? It needs to be cached instead
            v8::Local<v8::Value> property;
            if(_v8Object.Get(isolate)->GetInternalField(2).As<v8::Object>()->Get(context, name.constGet()).ToLocal(&property)) {
                if (!property->IsUndefined()) {
                    return V8ScriptValue(_engine, property);
                }
            }
            Q_ASSERT(false);
            qCDebug(scriptengine_v8) << "(This should not happen) Creating new method object for " << metaObject->className() << " " << name.toQString();
            return ScriptMethodV8Proxy::newMethod(_engine, qobject, object, methodDef.methods, methodDef.numMaxParams);
        }
        case SIGNAL_TYPE: {
            int signalId = id & ~TYPE_MASK;
            SignalDefMap::const_iterator defLookup = _signals.find(signalId);
            if (defLookup == _signals.cend()) return V8ScriptValue(_engine, v8::Null(isolate));

            InstanceMap::const_iterator instLookup = _signalInstances.find(signalId);
            if (instLookup == _signalInstances.cend() || instLookup.value().isNull()) {
                instLookup = _signalInstances.insert(signalId,
                    new ScriptSignalV8Proxy(_engine, qobject, object, defLookup.value().signal));
                Q_ASSERT(instLookup != _signalInstances.cend());
            }
            ScriptSignalV8Proxy* proxy = instLookup.value();

            ScriptEngine::QObjectWrapOptions options = ScriptEngine::ExcludeSuperClassContents |
                                                        //V8TODO ScriptEngine::ExcludeDeleteLater |
                                                        ScriptEngine::PreferExistingWrapperObject;
            // It's not necessarily new, newQObject looks for it first in object wrapper map
            // V8TODO: won't ScriptEngine::ScriptOwnership cause trouble here?
            return ScriptObjectV8Proxy::newQObject(_engine, proxy, ScriptEngine::ScriptOwnership, options);
        }
    }
    return V8ScriptValue(_engine, v8::Null(isolate));
}

void ScriptObjectV8Proxy::setProperty(V8ScriptValue& object, const V8ScriptString& name, uint id, const V8ScriptValue& value) {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    if (!(id & PROPERTY_TYPE)) return;
    QObject* qobject = _object;
    if (!qobject) {
        _engine->getIsolate()->ThrowError("Referencing deleted native object");
        return;
    }

    int propId = id & ~TYPE_MASK;
    PropertyDefMap::const_iterator lookup = _props.find(propId);
    if (lookup == _props.cend()) return;
    const PropertyDef& propDef = lookup.value();
    if (propDef.flags & ScriptValue::ReadOnly) return;

    const QMetaObject* metaObject = qobject->metaObject();
    QMetaProperty prop = metaObject->property(propId);

    ScriptValue scriptThis = ScriptValue(new ScriptValueV8Wrapper(_engine, object));
    ScriptPropertyContextV8Wrapper ourContext(scriptThis, _engine->currentContext());
    ScriptContextGuard guard(&ourContext);

    int propTypeId = prop.userType();
    Q_ASSERT(propTypeId != QMetaType::UnknownType);
    QVariant varValue;
    if(!_engine->castValueToVariant(value, varValue, propTypeId)) {
        QByteArray propTypeName = QMetaType(propTypeId).name();
        QByteArray valTypeName = _engine->valueType(value).toLatin1();
        isolate->ThrowError(v8::String::NewFromUtf8(isolate, QString("Cannot convert %1 to %2").arg(valTypeName, propTypeName).toStdString().c_str()).ToLocalChecked());
        return;
    }
    prop.write(qobject, varValue);
}

ScriptVariantV8Proxy::ScriptVariantV8Proxy(ScriptEngineV8* engine, const QVariant& variant, V8ScriptValue scriptProto, ScriptObjectV8Proxy* proto) :
    _engine(engine), _variant(variant), _scriptProto(scriptProto), _proto(proto) {
    auto isolate = engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    auto context = engine->getContext();
    v8::Context::Scope contextScope(context);
    auto variantDataTemplate = _engine->getVariantDataTemplate();
    auto variantData = variantDataTemplate->NewInstance(context).ToLocalChecked();
    variantData->SetAlignedPointerInInternalField(0, const_cast<void*>(internalPointsToQVariantInProxy));
    // Internal field doesn't point directly to QVariant, because then alignment would need to be guaranteed in all compilers
    variantData->SetAlignedPointerInInternalField(1, reinterpret_cast<void*>(this));
    _v8Object.Reset(isolate, v8::Local<v8::Object>::Cast(variantData));
    _name = QString::fromLatin1(variant.typeName());
}

ScriptVariantV8Proxy::~ScriptVariantV8Proxy() {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    // V8TODO: Add similar deletion handling as for object proxy
    _v8Object.Reset();
}

V8ScriptValue ScriptVariantV8Proxy::newVariant(ScriptEngineV8* engine, const QVariant& variant, V8ScriptValue proto) {
    qDebug() << "ScriptVariantV8Proxy::newVariant";
    auto isolate = engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    auto context = engine->getContext();
    v8::Context::Scope contextScope(context);
    ScriptObjectV8Proxy* protoProxy = ScriptObjectV8Proxy::unwrapProxy(proto);
    if (!protoProxy) {
        Q_ASSERT(protoProxy);
        //return engine->newVariant(variant);
        return V8ScriptValue(engine, v8::Undefined(isolate));
    }
    // V8TODO probably needs connection to be deleted
    auto proxy = new ScriptVariantV8Proxy(engine, variant, proto, protoProxy);

    auto variantProxyTemplate = engine->getVariantProxyTemplate();
    auto variantProxy = variantProxyTemplate->NewInstance(context).ToLocalChecked();
    variantProxy->SetAlignedPointerInInternalField(0, const_cast<void*>(internalPointsToQVariantProxy));
    variantProxy->SetAlignedPointerInInternalField(1, reinterpret_cast<void*>(proxy));
    return V8ScriptValue(engine, variantProxy);
}

ScriptVariantV8Proxy* ScriptVariantV8Proxy::unwrapProxy(const V8ScriptValue& val) {
    auto isolate = val.getEngine()->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(val.getEngine()->getContext());

    auto v8Value = val.constGet();
    if (!v8Value->IsObject()) {
        return nullptr;
    }
    v8::Local<v8::Object> v8Object = v8::Local<v8::Object>::Cast(v8Value);
    if (v8Object->InternalFieldCount() != 2) {
        return nullptr;
    }
    if (v8Object->GetAlignedPointerFromInternalField(0) != internalPointsToQVariantProxy) {
        return nullptr;
    }
    return reinterpret_cast<ScriptVariantV8Proxy*>(v8Object->GetAlignedPointerFromInternalField(1));
}

ScriptVariantV8Proxy* ScriptVariantV8Proxy::unwrapProxy(v8::Isolate* isolate, v8::Local<v8::Value> &value) {
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);

    if (!value->IsObject()) {
        return nullptr;
    }
    v8::Local<v8::Object> v8Object = v8::Local<v8::Object>::Cast(value);
    if (v8Object->InternalFieldCount() != 2) {
        return nullptr;
    }
    if (v8Object->GetAlignedPointerFromInternalField(0) != internalPointsToQVariantProxy) {
        return nullptr;
    }
    return reinterpret_cast<ScriptVariantV8Proxy*>(v8Object->GetAlignedPointerFromInternalField(1));
}

QVariant* ScriptVariantV8Proxy::unwrapQVariantPointer(v8::Isolate* isolate, const v8::Local<v8::Value> &value) {
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);

    if (!value->IsObject()) {
        return nullptr;
    }
    v8::Local<v8::Object> v8Object = v8::Local<v8::Object>::Cast(value);
    if (v8Object->InternalFieldCount() != 2) {
        return nullptr;
    }
    if (v8Object->GetAlignedPointerFromInternalField(0) != internalPointsToQVariantInProxy) {
        return nullptr;
    }
    auto proxy = reinterpret_cast<ScriptVariantV8Proxy*>(v8Object->GetAlignedPointerFromInternalField(1));
    return &(proxy->_variant);
}


void ScriptVariantV8Proxy::v8Get(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
    v8::HandleScope handleScope(info.GetIsolate());
    v8::String::Utf8Value utf8Name(info.GetIsolate(), name);
    v8::Local<v8::Value> objectV8 = info.This();
    ScriptVariantV8Proxy *proxy = ScriptVariantV8Proxy::unwrapProxy(info.GetIsolate(), objectV8);
    if (!proxy) {
        qCDebug(scriptengine_v8) << "Proxy object not found when getting: " << *utf8Name;
        return;
    }
    V8ScriptValue object(proxy->_engine, proxy->_v8Object.Get(info.GetIsolate()));

    ContextScopeV8 contextScopeV8(proxy->_engine);

    if (name->IsString()) {
        V8ScriptString nameString(proxy->_engine, v8::Local<v8::String>::Cast(name));
        uint id;
        ScriptObjectV8Proxy::QueryFlags flags = proxy->_proto->queryProperty(object, nameString, ScriptObjectV8Proxy::HandlesReadAccess, &id);
        if (flags) {
            V8ScriptValue value = proxy->property(object, nameString, id);
            info.GetReturnValue().Set(value.get());
            return;
        }
    }

    qCDebug(scriptengine_v8) << "Value not found: " << *utf8Name;
}

void ScriptVariantV8Proxy::v8Set(v8::Local<v8::Name> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<v8::Value>& info) {
    v8::HandleScope handleScope(info.GetIsolate());
    v8::String::Utf8Value utf8Name(info.GetIsolate(), name);
    v8::Local<v8::Value> objectV8 = info.This();
    ScriptVariantV8Proxy *proxy = ScriptVariantV8Proxy::unwrapProxy(info.GetIsolate(), objectV8);
    if (!proxy) {
        qCDebug(scriptengine_v8) << "Proxy object not found when getting: " << *utf8Name;
        return;
    }

    V8ScriptValue object(proxy->_engine, objectV8);
    if (!name->IsString() && !name->IsSymbol()) {
        QString notStringMessage("ScriptObjectV8Proxy::v8Set: " + proxy->_engine->scriptValueDebugDetailsV8(V8ScriptValue(proxy->_engine, name)));
        qCDebug(scriptengine_v8) << notStringMessage;
        Q_ASSERT(false);
    }

    ContextScopeV8 contextScopeV8(proxy->_engine);
    if (name->IsString()) {
        V8ScriptString nameString(proxy->_engine, v8::Local<v8::String>::Cast(name));
        uint id;
        ScriptObjectV8Proxy::QueryFlags flags = proxy->_proto->queryProperty(object, nameString, ScriptObjectV8Proxy::HandlesWriteAccess, &id);
        if (flags) {
            proxy->setProperty(object, nameString, id, V8ScriptValue(proxy->_engine, value));
            info.GetReturnValue().Set(value);
            return;
        }
    }
    qCDebug(scriptengine_v8) << "Set failed: " << *utf8Name;
}

void ScriptVariantV8Proxy::v8GetPropertyNames(const v8::PropertyCallbackInfo<v8::Array>& info) {
    //V8TODO: Only methods from the prototype should be listed.
    v8::HandleScope handleScope(info.GetIsolate());
    auto context = info.GetIsolate()->GetCurrentContext();
    v8::Context::Scope contextScope(context);
    v8::Local<v8::Value> objectV8 = info.This();
    ScriptVariantV8Proxy *proxy = ScriptVariantV8Proxy::unwrapProxy(info.GetIsolate(), objectV8);
    if (!proxy) {
        qCDebug(scriptengine_v8) << "ScriptObjectV8Proxy::v8GetPropertyNames: Proxy object not found when listing";
        return;
    }
    V8ScriptValue object(proxy->_engine, objectV8);
    v8::Local<v8::Array> properties = proxy->_proto->getPropertyNames();
    v8::Local<v8::Array> objectProperties;
    info.GetReturnValue().Set(properties);
}

QVariant ScriptVariantV8Proxy::unwrap(const V8ScriptValue& val) {
    ScriptVariantV8Proxy* proxy = unwrapProxy(val);
    return proxy ? proxy->toQVariant() : QVariant();
    // V8TODO
}

ScriptMethodV8Proxy::ScriptMethodV8Proxy(ScriptEngineV8* engine, QObject* object, V8ScriptValue lifetime,
                               const QList<QMetaMethod>& metas, int numMaxParams) :
    _numMaxParams(numMaxParams), _engine(engine), _object(object), /*_objectLifetime(lifetime),*/ _metas(metas) {
    auto isolate = engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(engine->getContext());
    _objectLifetime.Reset(isolate, lifetime.get());
    _objectLifetime.SetWeak(this, weakHandleCallback, v8::WeakCallbackType::kParameter);
}

ScriptMethodV8Proxy::~ScriptMethodV8Proxy() {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    _objectLifetime.Reset();
}

void ScriptMethodV8Proxy::weakHandleCallback(const v8::WeakCallbackInfo<ScriptMethodV8Proxy>& info) {
    auto proxy = info.GetParameter();
    proxy->_objectLifetime.Reset();
    info.GetParameter()->deleteLater();
}

V8ScriptValue ScriptMethodV8Proxy::newMethod(ScriptEngineV8* engine, QObject* object, V8ScriptValue lifetime,
                               const QList<QMetaMethod>& metas, int numMaxParams) {
    auto isolate = engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    auto context = engine->getContext();
    v8::Context::Scope contextScope(context);
    auto methodDataTemplate = engine->getMethodDataTemplate();
    auto methodData = methodDataTemplate->NewInstance(context).ToLocalChecked();
    methodData->SetAlignedPointerInInternalField(0, const_cast<void*>(internalPointsToMethodProxy));
    methodData->SetAlignedPointerInInternalField(1, reinterpret_cast<void*>(new ScriptMethodV8Proxy(engine, object, lifetime, metas, numMaxParams)));
    auto v8Function = v8::Function::New(context, callback, methodData, numMaxParams).ToLocalChecked();
    return V8ScriptValue(engine, v8Function);
}

QString ScriptMethodV8Proxy::fullName() const {
    Q_ASSERT(_object);
    if (!_object) return "";
    Q_ASSERT(!_metas.isEmpty());
    const QMetaMethod& firstMethod = _metas.front();
    QString objectName = _object->objectName();
    if (!objectName.isEmpty()) {
        return QString("%1.%2").arg(objectName, firstMethod.name());
    }
    return QString("%1::%2").arg(_object->metaObject()->className(), firstMethod.name());
}

void ScriptMethodV8Proxy::callback(const v8::FunctionCallbackInfo<v8::Value>& arguments) {
    v8::Locker locker(arguments.GetIsolate());
    v8::Isolate::Scope isolateScope(arguments.GetIsolate());
    v8::HandleScope handleScope(arguments.GetIsolate());
    v8::Context::Scope contextScope(arguments.GetIsolate()->GetCurrentContext());
    if (!arguments.Data()->IsObject()) {
        arguments.GetIsolate()->ThrowError("Method value is not an object");
        return;
    }
    v8::Local<v8::Object> data = v8::Local<v8::Object>::Cast(arguments.Data());
    if (data->InternalFieldCount() != 2) {
        arguments.GetIsolate()->ThrowError("Incorrect number of internal fields during method call");
        return;
    }
    if (data->GetAlignedPointerFromInternalField(0) != internalPointsToMethodProxy) {
        arguments.GetIsolate()->ThrowError("Internal field 0 of ScriptMethodV8Proxy V8 object has wrong value");
        return;
    }
    ScriptMethodV8Proxy *proxy = reinterpret_cast<ScriptMethodV8Proxy*>(data->GetAlignedPointerFromInternalField(1));

    ContextScopeV8 contextScopeV8(proxy->_engine);
    proxy->call(arguments);
}

void ScriptMethodV8Proxy::call(const v8::FunctionCallbackInfo<v8::Value>& arguments) {
    v8::Isolate *isolate = arguments.GetIsolate();
    Q_ASSERT(isolate == _engine->getIsolate());
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    ContextScopeV8 contextScopeV8(_engine);
    auto context = _engine->getContext();
    v8::Context::Scope contextScope(context);
    QObject* qobject = _object;
    if (!qobject) {
        isolate->ThrowError("Referencing deleted native object");
        return;
    }

    int scriptNumArgs = arguments.Length();
    int numArgs = std::min(scriptNumArgs, _numMaxParams);

    const int scriptValueTypeId = qMetaTypeId<ScriptValue>();

    int parameterConversionFailureId = 0;
    int parameterConversionFailureCount = 0;

    int num_metas = _metas.size();
    QVector< QList<ScriptValue> > qScriptArgLists;
    QVector< QVector <QGenericArgument> > qGenArgsVectors;
    QVector< QList<QVariant> > qVarArgLists;
    qScriptArgLists.resize(num_metas);
    qGenArgsVectors.resize(num_metas);
    qVarArgLists.resize(num_metas);
    bool isValidMetaSelected = false;
    int bestMeta = 0;
    int bestConversionPenaltyScore = 0;

    for (int i = 0; i < num_metas; i++) {
        const QMetaMethod& meta = _metas[i];
        int methodNumArgs = meta.parameterCount();
        if (methodNumArgs != numArgs) {
            continue;
        }

        qGenArgsVectors[i].resize(10);
        int conversionPenaltyScore = 0;
        int conversionFailures = 0;

        for (int arg = 0; arg < numArgs; ++arg) {
            int methodArgTypeId = meta.parameterType(arg);
            if (methodArgTypeId == QMetaType::UnknownType) {
                QString methodName = fullName();
                qCDebug(scriptengine_v8) << "One of the arguments is QMetaType::UnknownType for method " << methodName;
                Q_ASSERT(false);
            }
            v8::Local<v8::Value> argVal = arguments[arg];
            if (methodArgTypeId == scriptValueTypeId) {
                qScriptArgLists[i].append(ScriptValue(new ScriptValueV8Wrapper(_engine, V8ScriptValue(_engine, argVal))));
                qGenArgsVectors[i][arg] = Q_ARG(ScriptValue, qScriptArgLists[i].back());
            } else if (methodArgTypeId == QMetaType::QVariant) {
                QVariant varArgVal;
                if (!_engine->castValueToVariant(V8ScriptValue(_engine, argVal), varArgVal, methodArgTypeId)) {
                    conversionFailures++;
                } else {
                    qVarArgLists[i].append(varArgVal);
                    qGenArgsVectors[i][arg] = Q_ARG(QVariant, qVarArgLists[i].back());
                }
            } else {
                QVariant varArgVal;
                if (!_engine->castValueToVariant(V8ScriptValue(_engine, argVal), varArgVal, methodArgTypeId)) {
                    conversionFailures++;
                } else {
                    qVarArgLists[i].append(varArgVal);
                    const QVariant& converted = qVarArgLists[i].back();
                    conversionPenaltyScore += _engine->computeCastPenalty(V8ScriptValue(_engine, argVal), methodArgTypeId);

                    // a lot of type conversion assistance thanks to https://stackoverflow.com/questions/28457819/qt-invoke-method-with-qvariant
                    // A const_cast is needed because calling data() would detach the QVariant.
                    qGenArgsVectors[i][arg] =
                        QGenericArgument(QMetaType::typeName(converted.userType()), const_cast<void*>(converted.constData()));
                }
            }
        }
        if (conversionFailures) {
            if (conversionFailures < parameterConversionFailureCount || !parameterConversionFailureCount) {
                parameterConversionFailureCount = conversionFailures;
                parameterConversionFailureId = meta.methodIndex();
            }
            continue;
        }

        if (!isValidMetaSelected) {
            isValidMetaSelected = true;
            bestMeta = i;
            bestConversionPenaltyScore = conversionPenaltyScore;
        }
        if (isValidMetaSelected && bestConversionPenaltyScore > conversionPenaltyScore) {
            bestMeta = i;
            bestConversionPenaltyScore = conversionPenaltyScore;
        }
    }

    if (isValidMetaSelected) {
        // V8TODO: is this the correct wrapper?
        ScriptContextV8Wrapper ourContext(_engine, &arguments, context,
                                          _engine->currentContext()->parentContext());
        ScriptContextGuard guard(&ourContext);
        const QMetaMethod& meta = _metas[bestMeta];
        int returnTypeId = meta.returnType();
        QVector <QGenericArgument> &qGenArgs = qGenArgsVectors[bestMeta];

        // The Qt MOC engine will automatically call qRegisterMetaType on invokable parameters and properties, but there's
        // nothing in there for return values so these need to be explicitly runtime-registered!
        if (returnTypeId == QMetaType::UnknownType) {
            QString methodName = fullName();
            qCDebug(scriptengine_v8) << "returnTypeId == QMetaType::UnknownType for method " << methodName;
            _engine->logBacktrace("");
            //Q_ASSERT(false);
        }
        if (returnTypeId == QMetaType::UnknownType) {
            isolate->ThrowError(v8::String::NewFromUtf8(isolate, QString("Cannot call native function %1, its return value has not been registered with Qt").arg(fullName()).toStdString().c_str()).ToLocalChecked());
            return;
        } else if (returnTypeId == QMetaType::Void) {
            bool success = meta.invoke(qobject, Qt::DirectConnection, qGenArgs[0], qGenArgs[1], qGenArgs[2], qGenArgs[3],
                                        qGenArgs[4], qGenArgs[5], qGenArgs[6], qGenArgs[7], qGenArgs[8], qGenArgs[9]);
            if (!success) {
                isolate->ThrowError(v8::String::NewFromUtf8(isolate, QString("Unexpected: Native call of %1 failed").arg(fullName()).toStdString().c_str()).ToLocalChecked());
            }
            return;
        } else if (returnTypeId == scriptValueTypeId) {
            ScriptValue result;
            bool success = meta.invoke(qobject, Qt::DirectConnection, Q_RETURN_ARG(ScriptValue, result), qGenArgs[0],
                                       qGenArgs[1], qGenArgs[2], qGenArgs[3], qGenArgs[4], qGenArgs[5], qGenArgs[6],
                                       qGenArgs[7], qGenArgs[8], qGenArgs[9]);
            if (!success) {
                isolate->ThrowError(v8::String::NewFromUtf8(isolate, QString("Unexpected: Native call of %1 failed").arg(fullName()).toStdString().c_str()).ToLocalChecked());
                return;
            }
            V8ScriptValue v8Result = ScriptValueV8Wrapper::fullUnwrap(_engine, result);
            arguments.GetReturnValue().Set(v8Result.get());
            return;
        } else {
            // a lot of type conversion assistance thanks to https://stackoverflow.com/questions/28457819/qt-invoke-method-with-qvariant
            const char* typeName = meta.typeName();
            QVariant qRetVal(returnTypeId, static_cast<void*>(NULL));
            QGenericReturnArgument sRetVal(typeName, const_cast<void*>(qRetVal.constData()));

            bool success =
                meta.invoke(qobject, Qt::DirectConnection, sRetVal, qGenArgs[0], qGenArgs[1], qGenArgs[2], qGenArgs[3],
                            qGenArgs[4], qGenArgs[5], qGenArgs[6], qGenArgs[7], qGenArgs[8], qGenArgs[9]);
            if (!success) {
                isolate->ThrowError(v8::String::NewFromUtf8(isolate, QString("Unexpected: Native call of %1 failed").arg(fullName()).toStdString().c_str()).ToLocalChecked());
                return;
            }
            V8ScriptValue v8Result = _engine->castVariantToValue(qRetVal);
            arguments.GetReturnValue().Set(v8Result.get());
            return;
        }
    }

    // we failed to convert the call to C++, try to create a somewhat sane error message
    if (parameterConversionFailureCount == 0) {
        isolate->ThrowError(v8::String::NewFromUtf8(isolate, QString("Native call of %1 failed: unexpected parameter count").arg(fullName()).toStdString().c_str()).ToLocalChecked());
        return;
    }

    const QMetaMethod& meta = _object->metaObject()->method(parameterConversionFailureId);
    int methodNumArgs = meta.parameterCount();
    Q_ASSERT(methodNumArgs == numArgs);

    for (int arg = 0; arg < numArgs; ++arg) {
        int methodArgTypeId = meta.parameterType(arg);
        Q_ASSERT(methodArgTypeId != QMetaType::UnknownType);
        v8::Local<v8::Value> argVal = arguments[arg];
        if (methodArgTypeId != scriptValueTypeId) {
            QVariant varArgVal;
            if (!_engine->castValueToVariant(V8ScriptValue(_engine, argVal), varArgVal, methodArgTypeId)) {
                QByteArray methodTypeName = QMetaType(methodArgTypeId).name();
                QByteArray argTypeName = _engine->valueType(V8ScriptValue(_engine, argVal)).toLatin1();
                QString errorMessage = QString("Native call of %1 failed: Cannot convert parameter %2 from %3 to %4")
                    .arg(fullName()).arg(arg+1).arg(argTypeName, methodTypeName);
                qCDebug(scriptengine_v8) << errorMessage << "\n Backtrace:" << _engine->currentContext()->backtrace();
                isolate->ThrowError(v8::String::NewFromUtf8(isolate, errorMessage.toStdString().c_str()).ToLocalChecked());
                return;
            }
        }
    }
    QString errorMessage = QString("Native call of %1 failed: could not locate an overload with the requested arguments").arg(fullName());
    qCDebug(scriptengine_v8) << errorMessage;
    isolate->ThrowError(v8::String::NewFromUtf8(isolate, errorMessage.toStdString().c_str()).ToLocalChecked());
    Q_ASSERT(false); // really shouldn't have gotten here -- it didn't work before and it's working now?
    return;
}

ScriptSignalV8Proxy::ScriptSignalV8Proxy(ScriptEngineV8* engine, QObject* object, V8ScriptValue lifetime, const QMetaMethod& meta) :
    _engine(engine), _object(object), _meta(meta), _metaCallId(discoverMetaCallIdx()) {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    auto context = _engine->getContext();
    v8::Context::Scope contextScope(context);
    _objectLifetime.Reset(isolate, lifetime.get());
    _objectLifetime.SetWeak(this, weakHandleCallback, v8::WeakCallbackType::kParameter);
    _v8Context.Reset(isolate, context);
    _engine->_signalProxySetLock.lockForWrite();
    _engine->_signalProxySet.insert(this);
    _engine->_signalProxySetLock.unlock();
}

ScriptSignalV8Proxy::~ScriptSignalV8Proxy() {
    if (!_cleanup) {
        disconnectAllScriptSignalProxies();
    }

    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    _objectLifetime.Reset();
    _v8Context.Reset();
#ifdef OVERTE_SCRIPT_USE_AFTER_DELETE_GUARD
    Q_ASSERT(!_wasDeleted);
    Q_ASSERT(!_engine->_wasDestroyed);
    _wasDeleted = true;
#endif
    _engine->_signalProxySetLock.lockForWrite();
    _engine->_signalProxySet.remove(this);
    _engine->_signalProxySetLock.unlock();
}

void ScriptSignalV8Proxy::weakHandleCallback(const v8::WeakCallbackInfo<ScriptSignalV8Proxy>& info) {
    auto proxy = info.GetParameter();
    proxy->_objectLifetime.Reset();
    proxy->deleteLater();
}

QString ScriptSignalV8Proxy::fullName() const {
    Q_ASSERT(_object);
    if (!_object) return "";
    QString objectName = _object->objectName();
    if (!objectName.isEmpty()) {
        return QString("%1.%2").arg(objectName, _meta.name());
    }
    return QString("%1::%2").arg(_object->metaObject()->className(), _meta.name());
}

// Adapted from https://doc.qt.io/archives/qq/qq16-dynamicqobject.html, for connecting to a signal without a compile-time definition for it
int ScriptSignalV8Proxy::qt_metacall(QMetaObject::Call call, int id, void** arguments) {
    id = ScriptSignalV8ProxyBase::qt_metacall(call, id, arguments);
    if (id != 0 || call != QMetaObject::InvokeMetaMethod) {
        return id;
    }

#ifdef SCRIPT_EVENT_PERFORMANCE_STATISTICS
    _callCounter++;
    if (_callCounter % 1000 == 0) {
        qCDebug(scriptengine_v8) << "Script engine: " << _engine->manager()->getFilename() << " Signal proxy " << fullName()
                 << " call count: " << _callCounter << " total time: " << _totalCallTime_s;
    }
    QElapsedTimer callTimer;
    callTimer.start();
#endif

    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);

    QList<Connection> connections;
    withReadLock([&]{
        connections = _connections;
    });

    // V8TODO: this may cause deadlocks on connect/disconnect, so the connect/disconnect procedure needs to be reworked.
    // It should probably add events to a separate list that would be processed before and after all the events for the signal.
    {
        v8::HandleScope handleScope(isolate);
        // V8TODO: should we use function creation context, or context in which connect happened?
        auto context = _engine->getContext();
        v8::Context::Scope contextScope(context);
        v8::Local<v8::Value> args[Q_METAMETHOD_INVOKE_MAX_ARGS];
        int numArgs = _meta.parameterCount();
        for (int arg = 0; arg < numArgs; ++arg) {
            int methodArgTypeId = _meta.parameterType(arg);
            Q_ASSERT(methodArgTypeId != QMetaType::UnknownType);
            QVariant argValue(methodArgTypeId, arguments[arg + 1]);
            args[arg] = _engine->castVariantToValue(argValue).get();
        }
        for (ConnectionList::iterator iter = connections.begin(); iter != connections.end(); ++iter) {
            Connection& conn = *iter;
            {
                auto functionContext = context;

                Q_ASSERT(!conn.callback.get().IsEmpty());
                Q_ASSERT(!conn.callback.get()->IsUndefined());
                if (conn.callback.get()->IsNull()) {
                    qCDebug(scriptengine_v8) << "ScriptSignalV8Proxy::qt_metacall: Connection callback is Null";
                    _engine->popContext();
                    continue;
                }
                if (!conn.callback.get()->IsFunction()) {
                    auto stringV8 = conn.callback.get()->ToDetailString(functionContext).ToLocalChecked();
                    QString error = *v8::String::Utf8Value(_engine->getIsolate(), stringV8);
                    qCDebug(scriptengine_v8) << error;
                    Q_ASSERT(false);
                }
                v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(conn.callback.get());

                v8::Local<v8::Value> v8This;
                if (conn.thisValue.get()->IsObject()) {
                    v8This = conn.thisValue.get();
                } else {
                    v8This = functionContext->Global();
                }

                v8::TryCatch tryCatch(isolate);
                auto maybeResult = callback->Call(functionContext, v8This, numArgs, args);
                Q_UNUSED(maybeResult); // Signals don't have return values
                if (tryCatch.HasCaught()) {
                    QString errorMessage(QString("Signal proxy ") + fullName() + " connection call failed: \""
                                          + _engine->formatErrorMessageFromTryCatch(tryCatch)
                                          + "\nThis provided: " + QString::number(conn.thisValue.get()->IsObject()));
                    v8::Local<v8::Message> exceptionMessage = tryCatch.Message();
                    int errorLineNumber = -1;
                    if (!exceptionMessage.IsEmpty()) {
                        errorLineNumber = exceptionMessage->GetLineNumber(context).FromJust();
                    }
                    if (_engine->_manager) {
                        _engine->_manager->scriptErrorMessage(errorMessage, getFileNameFromTryCatch(tryCatch, isolate, context),
                                                              errorLineNumber);
                    } else {
                        qDebug(scriptengine_v8) << errorMessage;
                    }

                    _engine->setUncaughtException(tryCatch, "Error in signal proxy");
                }
            }
        }
    }

#ifdef SCRIPT_EVENT_PERFORMANCE_STATISTICS
    _totalCallTime_s += callTimer.elapsed() / 1000.0f;
#endif
    return -1;
}

int ScriptSignalV8Proxy::discoverMetaCallIdx() {
    const QMetaObject* ourMeta = metaObject();
    return ourMeta->methodCount();
}

ScriptSignalV8Proxy::ConnectionList::iterator ScriptSignalV8Proxy::findConnection(V8ScriptValue thisObject, V8ScriptValue callback) {
    auto iterOut = resultWithReadLock<ScriptSignalV8Proxy::ConnectionList::iterator>([&]{
        v8::Locker locker(_engine->getIsolate());
        v8::Isolate::Scope isolateScope(_engine->getIsolate());
        v8::HandleScope handleScope(_engine->getIsolate());
        v8::Context::Scope contextScope(_engine->getContext());
        ConnectionList::iterator iter;
        for (iter = _connections.begin(); iter != _connections.end(); ++iter) {
            Connection& conn = *iter;
            if (conn.callback.constGet()->StrictEquals(callback.constGet()) && conn.thisValue.constGet()->StrictEquals(thisObject.constGet())) {
                break;
            }
        }
        return iter;
    });
    return iterOut;
}

void ScriptSignalV8Proxy::connect(ScriptValue arg0, ScriptValue arg1) {
    v8::Isolate *isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    auto context = _engine->getContext();
    v8::Context::Scope contextScope(context);
    QObject* qobject = _object;
    if (!qobject) {
        isolate->ThrowError("Referencing deleted native object");
        return;
    }

    // untangle the arguments
    V8ScriptValue callback(_engine, v8::Null(isolate));
    V8ScriptValue callbackThis(_engine, v8::Null(isolate));
    if (arg1.isFunction()) {
        auto unwrappedArg0 = ScriptValueV8Wrapper::unwrap(arg0);
        auto unwrappedArg1 = ScriptValueV8Wrapper::unwrap(arg1);
        if (!unwrappedArg0 || !unwrappedArg1) {
            Q_ASSERT(false);
            return;
        }
        callbackThis = unwrappedArg0->toV8Value();
        callback = unwrappedArg1->toV8Value();
    } else {
        auto unwrappedArg0 = ScriptValueV8Wrapper::unwrap(arg0);
        if (!unwrappedArg0) {
            Q_ASSERT(false);
            return;
        }
        callback = unwrappedArg0->toV8Value();
    }
    if (!callback.get()->IsFunction()) {
        isolate->ThrowError("Function expected as argument to 'connect'");
        return;
    }

    // are we already connected?
    {
        ConnectionList::iterator lookup = findConnection(callbackThis, callback);
        if (lookup != _connections.end()) {
            return; // already exists
        }
    }

    // add a reference to ourselves to the destination callback
    Q_ASSERT(!callback.get().IsEmpty());
    Q_ASSERT(!callback.get()->IsUndefined());
    Q_ASSERT(callback.get()->IsFunction());
    v8::Local<v8::Function> destFunction = v8::Local<v8::Function>::Cast(callback.get());
    v8::Local<v8::String> destDataName = v8::String::NewFromUtf8(isolate, "__data__").ToLocalChecked();
    v8::Local<v8::Value> destData;
    // V8TODO: I'm not sure which context to use here
    //auto destFunctionContext = destFunction->CreationContext();
    auto destFunctionContext = context;
    Q_ASSERT(thisObject().isObject());
    V8ScriptValue v8ThisObject = ScriptValueV8Wrapper::fullUnwrap(_engine, thisObject());
    Q_ASSERT(ScriptObjectV8Proxy::unwrapProxy(v8ThisObject));
    ScriptSignalV8Proxy* thisProxy = dynamic_cast<ScriptSignalV8Proxy*>(ScriptObjectV8Proxy::unwrapProxy(v8ThisObject)->toQObject());
    Q_ASSERT(thisProxy);
    qCDebug(scriptengine_v8) << "ScriptSignalV8Proxy::connect: " << thisProxy->fullName() << " fullName: " << fullName();
    if (!destFunction->Get(destFunctionContext, destDataName).ToLocal(&destData)) {
        Q_ASSERT(false);
    }
    if (destData->IsArray()) {
        v8::Local<v8::Array> destArray = v8::Local<v8::Array>::Cast(destData);
        int length = destArray->Length();
        // V8TODO: Maybe copying array is unnecessary?
        v8::Local<v8::Array> newArray = v8::Array::New(isolate, length + 1);
        bool foundIt = false;
        for (int idx = 0; idx < length && !foundIt; ++idx) {
            v8::Local<v8::Value> entry = destArray->Get(destFunctionContext, idx).ToLocalChecked();
            {
                qCDebug(scriptengine_v8) << "ScriptSignalV8Proxy::connect: entry details: " << _engine->scriptValueDebugDetailsV8(V8ScriptValue(_engine, entry));
                Q_ASSERT(entry->IsObject());
                V8ScriptValue v8EntryObject(_engine, entry);
                Q_ASSERT(ScriptObjectV8Proxy::unwrapProxy(v8EntryObject));
                // For debugging
                ScriptSignalV8Proxy* entryProxy = dynamic_cast<ScriptSignalV8Proxy*>(ScriptObjectV8Proxy::unwrapProxy(v8EntryObject)->toQObject());
                Q_ASSERT(entryProxy);
                qCDebug(scriptengine_v8) << "ScriptSignalV8Proxy::connect: entry proxy: " << entryProxy->fullName();
            }
            if (!newArray->Set(destFunctionContext, idx, entry).FromMaybe(false)) {
                Q_ASSERT(false);
            }
            if (entry->StrictEquals(v8ThisObject.get())) {
                foundIt = true;
            }
        }
        if (!foundIt) {
            if (!newArray->Set(destFunctionContext, length, v8ThisObject.get()).FromMaybe(false)) {
                Q_ASSERT(false);
            }
            if (!destFunction->Set(destFunctionContext, destDataName, newArray).FromMaybe(false)) {
                Q_ASSERT(false);
            }
        }
    } else {
        v8::Local<v8::Array> newArray = v8::Array::New(isolate, 1);
        if (!newArray->Set(destFunctionContext, 0, v8ThisObject.get()).FromMaybe(false)) {
            Q_ASSERT(false);
        }
        if (!destFunction->Set(destFunctionContext, destDataName, newArray).FromMaybe(false)) {
            Q_ASSERT(false);
        }
    }

    // add this to our internal list of connections
    Connection newConnection(callbackThis, callback);

    withWriteLock([&]{
        _connections.append(newConnection);
    });

    // inform Qt that we're connecting to this signal
    if (!_isConnected) {
        auto result = QMetaObject::connect(qobject, _meta.methodIndex(), this, _metaCallId);
        Q_ASSERT(result);
        _isConnected = true;
    }
}

void ScriptSignalV8Proxy::disconnect(ScriptValue arg0, ScriptValue arg1) {
    QObject* qobject = _object;
    v8::Isolate *isolate = _engine->getIsolate();
    if (!_cleanup) {
        if (!qobject) {
            isolate->ThrowError("Referencing deleted native object");
            return;
        }
    }
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    auto context = _engine->getContext();
    v8::Context::Scope contextScope(context);

    // untangle the arguments
    V8ScriptValue callback(_engine, v8::Null(isolate));
    V8ScriptValue callbackThis(_engine, v8::Null(isolate));
    if (arg1.isFunction()) {
        auto unwrappedArg0 = ScriptValueV8Wrapper::unwrap(arg0);
        auto unwrappedArg1 = ScriptValueV8Wrapper::unwrap(arg1);
        if (!unwrappedArg0 || !unwrappedArg1) {
            Q_ASSERT(false);
            return;
        }
        callbackThis = unwrappedArg0->toV8Value();
        callback = unwrappedArg1->toV8Value();
    } else {
        auto unwrappedArg0 = ScriptValueV8Wrapper::unwrap(arg0);
        if (!unwrappedArg0) {
            Q_ASSERT(false);
            return;
        }
        callback = unwrappedArg0->toV8Value();
    }
    if (!callback.get()->IsFunction()) {
        isolate->ThrowError("Function expected as argument to 'disconnect'");
        return;
    }

    // locate this connection in our list of connections
    {
        ConnectionList::iterator lookup = findConnection(callbackThis, callback);
        if (lookup == _connections.end()) {
            return;  // not here
        }

        // remove it from our internal list of connections
        withWriteLock([&]{
            _connections.erase(lookup);
        });
    }

    // remove a reference to ourselves from the destination callback
    v8::Local<v8::Function> destFunction = v8::Local<v8::Function>::Cast(callback.get());
    v8::Local<v8::String> destDataName = v8::String::NewFromUtf8(isolate, "__data__").ToLocalChecked();
    v8::Local<v8::Value> destData;

    auto destFunctionContext = context;
    ScriptEngine::QObjectWrapOptions options = ScriptEngine::ExcludeSuperClassContents |
                                               ScriptEngine::PreferExistingWrapperObject;
    // It's not necessarily new, newQObject looks for it first in object wrapper map, and for already existing signal it should be found there
    V8ScriptValue v8ThisObject = ScriptObjectV8Proxy::newQObject(_engine, this, ScriptEngine::ScriptOwnership, options);
    if (!destFunction->Get(destFunctionContext, destDataName).ToLocal(&destData)) {
        Q_ASSERT(false);
    }
    if (destData->IsArray()) {
        v8::Local<v8::Array> destArray = v8::Local<v8::Array>::Cast(destData);
        int length = destArray->Length();
        v8::Local<v8::Array> newArray = v8::Array::New(isolate, length - 1);
        int findCounter = 0;
        int newIndex = 0;
        for (int idx = 0; idx < length; ++idx) {
            v8::Local<v8::Value> entry = destArray->Get(destFunctionContext, idx).ToLocalChecked();
            // For debugging:
            {
                Q_ASSERT(entry->IsObject());
                V8ScriptValue v8EntryObject(_engine, entry);
                Q_ASSERT(ScriptObjectV8Proxy::unwrapProxy(v8EntryObject));
            }
            if (entry->StrictEquals(v8ThisObject.get())) {
                findCounter++;
            } else {
                if (!newArray->Set(destFunctionContext, newIndex, entry).FromMaybe(false)) {
                    Q_ASSERT(false);
                }
                newIndex++;
            }
        }
        Q_ASSERT(findCounter == 1);
        if (!destFunction->Set(destFunctionContext, destDataName, newArray).FromMaybe(false)) {
            Q_ASSERT(false);
        }
    } else {
        Q_ASSERT(false);
    }

    // inform Qt that we're no longer connected to this signal
    if (_connections.empty()) {
        Q_ASSERT(_isConnected);
        // During cleanup qobject might be null since it might have been already deleted
        if (!_cleanup || (_cleanup && qobject)) {
            Q_ASSERT(qobject);
            bool result = QMetaObject::disconnect(qobject, _meta.methodIndex(), this, _metaCallId);
            Q_ASSERT(result);
        }
        _isConnected = false;
    }
}

void ScriptSignalV8Proxy::disconnectAllScriptSignalProxies() {
    _cleanup = true;
    QList<Connection> connections;
    withReadLock([&]{
        connections = _connections;
    });

    for (auto &connection : connections) {
        ScriptValue thisValue(new ScriptValueV8Wrapper(_engine, connection.thisValue));
        ScriptValue callback(new ScriptValueV8Wrapper(_engine, connection.callback));
        disconnect(thisValue, callback);
    }
}

void ScriptSignalV8Proxy::disconnectAll() {
    QObject::disconnect(this, nullptr, nullptr, nullptr);
}
