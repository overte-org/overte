//
//  ScriptObjectV8Proxy.cpp
//  libraries/script-engine/src/v8
//
//  Created by Heather Anderson on 12/5/21.
//  Modified for V8 by dr Karol Suprynowicz on 2022/10/08
//  Copyright 2021 Vircadia contributors.
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ScriptObjectV8Proxy.h"

#include <QtCore/QList>
#include <QtCore/QSharedPointer>

#include "../ScriptEngineLogging.h"

#include "ScriptContextV8Wrapper.h"
#include "ScriptValueV8Wrapper.h"

Q_DECLARE_METATYPE(V8ScriptContext*)
Q_DECLARE_METATYPE(ScriptValue)
//V8TODO?
//Q_DECLARE_METATYPE(V8ScriptValue)

Q_DECLARE_METATYPE(QSharedPointer<ScriptObjectV8Proxy>)
Q_DECLARE_METATYPE(QSharedPointer<ScriptVariantV8Proxy>)

// Value of internal field with index 0 when object contains ScriptObjectV8Proxy pointer in internal field 1
static const void *internalPointsToQObjectProxy = (void *)0x13370000;
static const void *internalPointsToQVariantProxy = (void *)0x13371000;
static const void *internalPointsToSignalProxy = (void *)0x13372000;
static const void *internalPointsToMethodProxy = (void *)0x13373000;

// Used strictly to replace the "this" object value for property access.  May expand to a full context element
// if we find it necessary to, but hopefully not needed
class ScriptPropertyContextQtWrapper final : public ScriptContext {
public:  // construction
    inline ScriptPropertyContextQtWrapper(const ScriptValue& object, ScriptContext* parentContext) :
        _parent(parentContext), _object(object) {}

public:  // ScriptContext implementation
    virtual int argumentCount() const override { return _parent->argumentCount(); }
    virtual ScriptValue argument(int index) const override { return _parent->argument(index); }
    virtual QStringList backtrace() const override { return _parent->backtrace(); }
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
    _engine(engine), _wrapOptions(options), _ownsObject(ownsObject), _object(object),
    _v8ObjectTemplate(engine->getIsolate(), v8::ObjectTemplate::New(engine->getIsolate())) {
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
            // V8TODO: is conversion to QVariant and back needed?
            if (proxy) {
                return V8ScriptValue(engine->getIsolate(), proxy.get()->toV8Value());
            }
            //if (proxy) return engine->newObject(proxy.get(), engine->newVariant(QVariant::fromValue(proxy)));;
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
            qCritical() << "Wrong ScriptEngine::ValueOwnership value: " << ownership;
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
            //if (proxy) return qengine->newObject(proxy.get(), qengine->newVariant(QVariant::fromValue(proxy)));;
            if (proxy) {
                return V8ScriptValue(engine->getIsolate(), proxy.get()->toV8Value());
            }
        }
        // V8TODO add a V8 callback that removes pointer from the map so that it gets deleted
        // register the wrapper with the engine and make sure it cleans itself up
        engine->_qobjectWrapperMap.insert(object, proxy);
        engine->_qobjectWrapperMapV8.insert(object, proxy);
        QPointer<ScriptEngineV8> enginePtr = engine;
        object->connect(object, &QObject::destroyed, engine, [enginePtr, object]() {
            if (!enginePtr) return;
            QMutexLocker guard(&enginePtr->_qobjectWrapperMapProtect);
            ScriptEngineV8::ObjectWrapperMap::iterator lookup = enginePtr->_qobjectWrapperMap.find(object);
            if (lookup != enginePtr->_qobjectWrapperMap.end()) {
                enginePtr->_qobjectWrapperMap.erase(lookup);
            }
        });
    }

    return V8ScriptValue(engine->getIsolate(), proxy.get()->toV8Value());
    //return qengine->newObject(proxy.get(), qengine->newVariant(QVariant::fromValue(proxy)));
}

ScriptObjectV8Proxy* ScriptObjectV8Proxy::unwrapProxy(const V8ScriptValue& val) {
    //V8TODO This shouldn't cause problems but I'm not sure if it's ok
    v8::HandleScope handleScope(const_cast<v8::Isolate*>(val.constGetIsolate()));
    auto v8Value = val.constGet();
    if (!v8Value->IsObject()) {
        qDebug(scriptengine) << "Cannot unwrap proxy - value is not an object";
        return nullptr;
    }
    v8::Local<v8::Object> v8Object = v8::Local<v8::Object>::Cast(v8Value);
    if (v8Object->InternalFieldCount() != 2) {
        qDebug(scriptengine) << "Cannot unwrap proxy - wrong number of internal fields";
        return nullptr;
    }
    if (v8Object->GetAlignedPointerFromInternalField(0) != internalPointsToQObjectProxy) {
        qDebug(scriptengine) << "Cannot unwrap proxy - internal fields don't point to object proxy";
        return nullptr;
    }
    return reinterpret_cast<ScriptObjectV8Proxy*>(v8Object->GetAlignedPointerFromInternalField(1));
}

QObject* ScriptObjectV8Proxy::unwrap(const V8ScriptValue& val) {
    ScriptObjectV8Proxy* proxy = unwrapProxy(val);
    return proxy ? proxy->toQObject() : nullptr;
}

ScriptObjectV8Proxy::~ScriptObjectV8Proxy() {
    qDebug(scriptengine) << "Deleting object proxy: " << name();
    if (_ownsObject) {
        QObject* qobject = _object;
        if(qobject) qobject->deleteLater();
    }
}

void ScriptObjectV8Proxy::investigate() {
    QObject* qobject = _object;
    Q_ASSERT(qobject);
    if (!qobject) return;
    
    v8::HandleScope handleScope(_engine->getIsolate());
    v8::Context::Scope contextScope(_engine->getContext());
    
    auto objectTemplate = _v8ObjectTemplate.Get(_engine->getIsolate());
    objectTemplate->SetInternalFieldCount(2);
    objectTemplate->SetHandler(v8::NamedPropertyHandlerConfiguration(v8Get, v8Set));
    
    const QMetaObject* metaObject = qobject->metaObject();

    qDebug(scriptengine) << "Investigate: " << metaObject->className();
    if (QString("Vec3") == metaObject->className()) {
        printf("Vec3");
    }
    if (QString("ConsoleScriptingInterface") == metaObject->className()) {
        printf("ConsoleScriptingInterface");
    }
    // discover properties
    int startIdx = _wrapOptions & ScriptEngine::ExcludeSuperClassProperties ? metaObject->propertyOffset() : 0;
    int num = metaObject->propertyCount();
    for (int idx = startIdx; idx < num; ++idx) {
        QMetaProperty prop = metaObject->property(idx);
        if (!prop.isScriptable()) continue;

        qDebug(scriptengine) << "Investigate: " << metaObject->className() << " Property: " << prop.name();
        // always exclude child objects (at least until we decide otherwise)
        int metaTypeId = prop.userType();
        if (metaTypeId != QMetaType::UnknownType) {
            QMetaType metaType(metaTypeId);
            if (metaType.flags() & QMetaType::PointerToQObject) {
                continue;
            }
        }

        auto v8Name = v8::String::NewFromUtf8(_engine->getIsolate(), prop.name()).ToLocalChecked();
        PropertyDef& propDef = _props.insert(idx, PropertyDef(_engine->getIsolate(), v8Name)).value();
        propDef.flags = ScriptValue::Undeletable | ScriptValue::PropertyGetter | ScriptValue::PropertySetter |
                        ScriptValue::QObjectMember;
        if (prop.isConstant()) propDef.flags |= ScriptValue::ReadOnly;
    }

    // discover methods
    startIdx = (_wrapOptions & ScriptEngine::ExcludeSuperClassMethods) ? metaObject->methodOffset() : 0;
    num = metaObject->methodCount();
    QHash<V8ScriptString, int> methodNames;
    for (int idx = startIdx; idx < num; ++idx) {
        QMetaMethod method = metaObject->method(idx);
        qDebug(scriptengine) << "Investigate: " << metaObject->className() << " Method: " << method.name();
        
        // perhaps keep this comment?  Calls (like AudioScriptingInterface::playSound) seem to expect non-public methods to be script-accessible
        /* if (method.access() != QMetaMethod::Public) continue;*/

        bool isSignal = false;
        QByteArray szName = method.name();

        switch (method.methodType()) {
            case QMetaMethod::Constructor:
                continue;
            case QMetaMethod::Signal:
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
        V8ScriptString name(_engine->getIsolate(), nameString);
        auto nameLookup = methodNames.find(name);
        if (isSignal) {
            if (nameLookup == methodNames.end()) {
                SignalDef& signalDef = _signals.insert(idx, SignalDef(_engine->getIsolate(), name.get())).value();
                signalDef.name = name;
                signalDef.signal = method;
                //qDebug(scriptengine) << "Utf8Value 1: " << QString(*v8::String::Utf8Value(const_cast<v8::Isolate*>(_engine->getIsolate()), nameString));
                //qDebug(scriptengine) << "Utf8Value 2: " << QString(*v8::String::Utf8Value(const_cast<v8::Isolate*>(_engine->getIsolate()), name.constGet()));
                //qDebug(scriptengine) << "toQString: " << name.toQString();
                methodNames.insert(name, idx);
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
                qCritical(scriptengine) << "Method " << metaObject->className() << "::" << name.toQString() << " has QMetaType::UnknownType return value";
            }
            for (int i = 0; i < method.parameterCount(); i++) {
                if (method.parameterType(i) == QMetaType::UnknownType) {
                    qCritical(scriptengine) << "Parameter " << i << "in method " << metaObject->className() << "::" << name.toQString() << " is of type QMetaType::UnknownType";
                }
            }
            if (nameLookup == methodNames.end()) {
                MethodDef& methodDef = _methods.insert(idx, MethodDef(_engine->getIsolate(), name.get())).value();
                methodDef.name = name;
                methodDef.numMaxParms = parameterCount;
                methodDef.methods.append(method);
                methodNames.insert(name, idx);
            } else {
                int originalMethodId = nameLookup.value();
                MethodDefMap::iterator methodLookup = _methods.find(originalMethodId);
                Q_ASSERT(methodLookup != _methods.end());
                MethodDef& methodDef = methodLookup.value();
                if(methodDef.numMaxParms < parameterCount) methodDef.numMaxParms = parameterCount;
                methodDef.methods.append(method);
            }
        }
    }

    v8::Local<v8::Object> v8Object = objectTemplate->NewInstance(_engine->getContext()).ToLocalChecked();
    v8Object->SetAlignedPointerInInternalField(0, const_cast<void*>(internalPointsToQObjectProxy));
    v8Object->SetAlignedPointerInInternalField(1, reinterpret_cast<void*>(this));

    _v8Object.Reset(_engine->getIsolate(), v8Object);
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
    v8::HandleScope handleScope(_engine->getIsolate());
    // V8TODO: this might be inefficient when there's large number of properties
    v8::Local<v8::Context> context = _engine->getContext();
    v8::String::Utf8Value nameStr(_engine->getIsolate(), name.constGet());

    // check for methods
    for (MethodDefMap::const_iterator trans = _methods.cbegin(); trans != _methods.cend(); ++trans) {
        v8::String::Utf8Value methodNameStr(_engine->getIsolate(), trans.value().name.constGet());
        qDebug(scriptengine) << "queryProperty : " << *nameStr << " method: " << *methodNameStr;
        if (!(trans.value().name == name)) continue;
        *id = trans.key() | METHOD_TYPE;
        return flags & (HandlesReadAccess | HandlesWriteAccess);
    }

    // check for properties
    for (PropertyDefMap::const_iterator trans = _props.cbegin(); trans != _props.cend(); ++trans) {
        const PropertyDef& propDef = trans.value();
        if (!(propDef.name == name)) continue;
        *id = trans.key() | PROPERTY_TYPE;
        return flags & (HandlesReadAccess | HandlesWriteAccess);
    }

    // check for signals
    for (SignalDefMap::const_iterator trans = _signals.cbegin(); trans != _signals.cend(); ++trans) {
        if (!(trans.value().name == name)) continue;
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
    qDebug(scriptengine) << "Get: " << *utf8Value;
    V8ScriptValue object(info.GetIsolate(), info.This());
    ScriptObjectV8Proxy *proxy = ScriptObjectV8Proxy::unwrapProxy(object);
    if (!proxy) {
        qDebug(scriptengine) << "Proxy object not found when getting: " << *utf8Value;
        return;
    }
    V8ScriptString nameString(info.GetIsolate(), v8::Local<v8::String>::Cast(name));
    uint id;
    QueryFlags flags = proxy->queryProperty(object, nameString, HandlesReadAccess, &id);
    if (flags) {
        V8ScriptValue value = proxy->property(object, nameString, id);
        info.GetReturnValue().Set(value.get());
    } else {
        qDebug(scriptengine) << "Value not found: " << *utf8Value;
    }
}

void ScriptObjectV8Proxy::v8Set(v8::Local<v8::Name> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<v8::Value>& info) {
    v8::HandleScope handleScope(info.GetIsolate());
    v8::String::Utf8Value utf8Value(info.GetIsolate(), name);
    qDebug(scriptengine) << "Set: " << *utf8Value;
    V8ScriptValue object(info.GetIsolate(), info.This());
    ScriptObjectV8Proxy *proxy = ScriptObjectV8Proxy::unwrapProxy(object);
    if (!proxy) {
        qDebug(scriptengine) << "Proxy object not found when setting: " << *utf8Value;
        return;
    }
    V8ScriptString nameString(info.GetIsolate(), v8::Local<v8::String>::Cast(name));
    //V8ScriptString nameString(info.GetIsolate(), name->ToString(proxy->_engine->getContext()).ToLocalChecked());
    uint id;
    QueryFlags flags = proxy->queryProperty(object, nameString, HandlesWriteAccess, &id);
    if (flags) {
        proxy->setProperty(object, nameString, id, V8ScriptValue(info.GetIsolate(), value));
        info.GetReturnValue().Set(value);
    } else {
        qDebug(scriptengine) << "Value not found: " << *utf8Value;
    }
}


V8ScriptValue ScriptObjectV8Proxy::property(const V8ScriptValue& object, const V8ScriptString& name, uint id) {
    v8::HandleScope handleScope(_engine->getIsolate());
    QObject* qobject = _object;
    if (!qobject) {
        _engine->getIsolate()->ThrowError("Referencing deleted native object");
        return V8ScriptValue(_engine->getIsolate(), v8::Null(_engine->getIsolate()));
    }

    const QMetaObject* metaObject = qobject->metaObject();

    switch (id & TYPE_MASK) {
        case PROPERTY_TYPE: {
            int propId = id & ~TYPE_MASK;
            PropertyDefMap::const_iterator lookup = _props.find(propId);
            if (lookup == _props.cend()) return V8ScriptValue(_engine->getIsolate(), v8::Null(_engine->getIsolate()));

            QMetaProperty prop = metaObject->property(propId);
            ScriptValue scriptThis = ScriptValue(new ScriptValueV8Wrapper(_engine, object));
            ScriptPropertyContextQtWrapper ourContext(scriptThis, _engine->currentContext());
            ScriptContextGuard guard(&ourContext);

            QVariant varValue = prop.read(qobject);
            return _engine->castVariantToValue(varValue);
        }
        case METHOD_TYPE: {
            int methodId = id & ~TYPE_MASK;
            MethodDefMap::const_iterator lookup = _methods.find(methodId);
            if (lookup == _methods.cend()) return V8ScriptValue(_engine->getIsolate(), v8::Null(_engine->getIsolate()));
            const MethodDef& methodDef = lookup.value();
            for (auto iter = methodDef.methods.begin(); iter != methodDef.methods.end(); iter++ ) {
                if((*iter).returnType() == QMetaType::UnknownType) {
                    qDebug(scriptengine) << "Method with QMetaType::UnknownType " << metaObject->className() << " " << (*iter).name();
                }
            } //V8TODO: is new method created during every call? It needs to be cached instead
            return ScriptMethodV8Proxy::newMethod(_engine, qobject, object, methodDef.methods, methodDef.numMaxParms);
        }
        case SIGNAL_TYPE: {
            int signalId = id & ~TYPE_MASK;
            SignalDefMap::const_iterator defLookup = _signals.find(signalId);
            if (defLookup == _signals.cend()) return V8ScriptValue(_engine->getIsolate(), v8::Null(_engine->getIsolate()));

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
            return ScriptObjectV8Proxy::newQObject(_engine, proxy, ScriptEngine::ScriptOwnership, options);
            //return _engine->newQObject(proxy, ScriptEngine::ScriptOwnership, options);
        }
    }
    return V8ScriptValue(_engine->getIsolate(), v8::Null(_engine->getIsolate()));
}

void ScriptObjectV8Proxy::setProperty(V8ScriptValue& object, const V8ScriptString& name, uint id, const V8ScriptValue& value) {
    v8::HandleScope handleScope(_engine->getIsolate());
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
    ScriptPropertyContextQtWrapper ourContext(scriptThis, _engine->currentContext());
    ScriptContextGuard guard(&ourContext);

    int propTypeId = prop.userType();
    Q_ASSERT(propTypeId != QMetaType::UnknownType);
    QVariant varValue;
    if(!_engine->castValueToVariant(value, varValue, propTypeId)) {
        QByteArray propTypeName = QMetaType(propTypeId).name();
        QByteArray valTypeName = _engine->valueType(value).toLatin1();
        _engine->getIsolate()->ThrowError(v8::String::NewFromUtf8(_engine->getIsolate(), QString("Cannot convert %1 to %2").arg(valTypeName, propTypeName).toStdString().c_str()).ToLocalChecked());
        return;
    }
    prop.write(qobject, varValue);
}

ScriptVariantV8Proxy::ScriptVariantV8Proxy(ScriptEngineV8* engine, const QVariant& variant, V8ScriptValue scriptProto, ScriptObjectV8Proxy* proto) :
    _engine(engine), _variant(variant), _scriptProto(scriptProto), _proto(proto) {
    _name = QString::fromLatin1(variant.typeName());
}

V8ScriptValue ScriptVariantV8Proxy::newVariant(ScriptEngineV8* engine, const QVariant& variant, V8ScriptValue proto) {
    ScriptObjectV8Proxy* protoProxy = ScriptObjectV8Proxy::unwrapProxy(proto);
    if (!protoProxy) {
        Q_ASSERT(protoProxy);
        //return engine->newVariant(variant);
        return V8ScriptValue(engine->getIsolate(), v8::Undefined(engine->getIsolate()));
    }
    // V8TODO probably needs connection to be deleted
    // V8TODO what to do with proto variable?
    auto proxy = new ScriptVariantV8Proxy(engine, variant, proto, protoProxy);
    auto variantDataTemplate = v8::ObjectTemplate::New(engine->getIsolate());
    variantDataTemplate->SetInternalFieldCount(2);
    auto variantData = variantDataTemplate->NewInstance(engine->getContext()).ToLocalChecked();
    variantData->SetAlignedPointerInInternalField(0, const_cast<void*>(internalPointsToQVariantProxy));
    variantData->SetAlignedPointerInInternalField(1, reinterpret_cast<void*>(proxy));
    return V8ScriptValue(engine->getIsolate(), variantData);
}

ScriptVariantV8Proxy* ScriptVariantV8Proxy::unwrapProxy(const V8ScriptValue& val) {
    // V8TODO
    v8::HandleScope handleScope(const_cast<v8::Isolate*>(val.constGetIsolate()));
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

QVariant ScriptVariantV8Proxy::unwrap(const V8ScriptValue& val) {
    ScriptVariantV8Proxy* proxy = unwrapProxy(val);
    return proxy ? proxy->toQVariant() : QVariant();
}

ScriptMethodV8Proxy::ScriptMethodV8Proxy(ScriptEngineV8* engine, QObject* object, V8ScriptValue lifetime,
                               const QList<QMetaMethod>& metas, int numMaxParams) :
    _numMaxParams(numMaxParams), _engine(engine), _object(object), _objectLifetime(lifetime), _metas(metas) {
}

ScriptMethodV8Proxy::~ScriptMethodV8Proxy() {
    qDebug(scriptengine) << "ScriptMethodV8Proxy destroyed";
    printf("ScriptMethodV8Proxy destroyed");
}

V8ScriptValue ScriptMethodV8Proxy::newMethod(ScriptEngineV8* engine, QObject* object, V8ScriptValue lifetime,
                               const QList<QMetaMethod>& metas, int numMaxParams) {
    auto methodDataTemplate = v8::ObjectTemplate::New(engine->getIsolate());
    methodDataTemplate->SetInternalFieldCount(2);
    auto methodData = methodDataTemplate->NewInstance(engine->getContext()).ToLocalChecked();
    methodData->SetAlignedPointerInInternalField(0, const_cast<void*>(internalPointsToMethodProxy));
    // V8TODO it needs to be deleted somehow on object destruction
    methodData->SetAlignedPointerInInternalField(1, reinterpret_cast<void*>(new ScriptMethodV8Proxy(engine, object, lifetime, metas, numMaxParams)));
    auto v8Function = v8::Function::New(engine->getContext(), callback, methodData, numMaxParams).ToLocalChecked();
    return V8ScriptValue(engine->getIsolate(), v8Function);
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

// V8TODO
/*bool ScriptMethodV8Proxy::supportsExtension(Extension extension) const {
    switch (extension) {
        case Callable:
            return true;
        default:
            return false;
    }
}*/

void ScriptMethodV8Proxy::callback(const v8::FunctionCallbackInfo<v8::Value>& arguments) {
    v8::HandleScope handleScope(arguments.GetIsolate());
    if (!arguments.Data()->IsObject()) {
        arguments.GetIsolate()->ThrowError("Method value is not an object");
        return;
    }
    v8::Local<v8::Object> data = v8::Local<v8::Object>::Cast(arguments.Data());
    /*if (!arguments.Data()->IsCallable()) {
        arguments.GetIsolate()->ThrowError("Method value is not callable");
        return;
    }*/
    if (data->InternalFieldCount() != 2) {
        arguments.GetIsolate()->ThrowError("Incorrect number of internal fields during method call");
        return;
    }
    if (data->GetAlignedPointerFromInternalField(0) != internalPointsToMethodProxy) {
        arguments.GetIsolate()->ThrowError("Internal field 0 of ScriptMethodV8Proxy V8 object has wrong value");
        return;
    }
    ScriptMethodV8Proxy *proxy = reinterpret_cast<ScriptMethodV8Proxy*>(data->GetAlignedPointerFromInternalField(1));
    proxy->call(arguments);
}

void ScriptMethodV8Proxy::call(const v8::FunctionCallbackInfo<v8::Value>& arguments) {
    QObject* qobject = _object;
    v8::Isolate *isolate = arguments.GetIsolate();
    if (!qobject) {
        isolate->ThrowError("Referencing deleted native object");
        return;
    }
    
    v8::HandleScope handleScope(_engine->getIsolate());

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
            Q_ASSERT(methodArgTypeId != QMetaType::UnknownType);
            v8::Local<v8::Value> argVal = arguments[arg];
            if (methodArgTypeId == scriptValueTypeId) {
                qScriptArgLists[i].append(ScriptValue(new ScriptValueV8Wrapper(_engine, V8ScriptValue(isolate, argVal))));
                qGenArgsVectors[i][arg] = Q_ARG(ScriptValue, qScriptArgLists[i].back());
            } else if (methodArgTypeId == QMetaType::QVariant) {
                QVariant varArgVal;
                if (!_engine->castValueToVariant(V8ScriptValue(isolate, argVal), varArgVal, methodArgTypeId)) {
                    conversionFailures++;
                } else {
                    qVarArgLists[i].append(varArgVal);
                    qGenArgsVectors[i][arg] = Q_ARG(QVariant, qVarArgLists[i].back());
                }
            } else {
                QVariant varArgVal;
                if (!_engine->castValueToVariant(V8ScriptValue(isolate, argVal), varArgVal, methodArgTypeId)) {
                    conversionFailures++;
                } else {
                    qVarArgLists[i].append(varArgVal);
                    const QVariant& converted = qVarArgLists[i].back();
                    conversionPenaltyScore = _engine->computeCastPenalty(V8ScriptValue(isolate, argVal), methodArgTypeId);

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
        //ScriptContextV8Wrapper ourContext(_engine, context);
        //ScriptContextGuard guard(&ourContext);
        const QMetaMethod& meta = _metas[bestMeta];
        int returnTypeId = meta.returnType();
        QVector <QGenericArgument> &qGenArgs = qGenArgsVectors[bestMeta];

        // The Qt MOC engine will automatically call qRegisterMetaType on invokable parameters and properties, but there's
        // nothing in there for return values so these need to be explicitly runtime-registered!
        Q_ASSERT(returnTypeId != QMetaType::UnknownType);
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
        if (methodArgTypeId != scriptValueTypeId && methodArgTypeId != QMetaType::QVariant) {
            QVariant varArgVal;
            if (!_engine->castValueToVariant(V8ScriptValue(isolate, argVal), varArgVal, methodArgTypeId)) {
                QByteArray methodTypeName = QMetaType(methodArgTypeId).name();
                QByteArray argTypeName = _engine->valueType(V8ScriptValue(isolate, argVal)).toLatin1();
                isolate->ThrowError(v8::String::NewFromUtf8(isolate, QString("Native call of %1 failed: Cannot convert parameter %2 from %3 to %4")
                                            .arg(fullName()).arg(arg+1).arg(argTypeName, methodTypeName).toStdString().c_str()).ToLocalChecked());
                //context->throwError(V8ScriptContext::TypeError, QString("Native call of %1 failed: Cannot convert parameter %2 from %3 to %4")
                //                                                   .arg(fullName()).arg(arg+1).arg(argTypeName, methodTypeName));
                return;
            }
        }
    }

    isolate->ThrowError(v8::String::NewFromUtf8(isolate, QString("Native call of %1 failed: could not locate an overload with the requested arguments").arg(fullName()).toStdString().c_str()).ToLocalChecked());
    Q_ASSERT(false); // really shouldn't have gotten here -- it didn't work before and it's working now?
    return;
}

/*QVariant ScriptMethodV8Proxy::extension(Extension extension, const QVariant& argument) {
    if (extension != Callable) return QVariant();
    V8ScriptContext* context = qvariant_cast<V8ScriptContext*>(argument);

    QObject* qobject = _object;
    if (!qobject) {
        context->throwError(V8ScriptContext::ReferenceError, "Referencing deleted native object");
        return QVariant();
    }

    int scriptNumArgs = context->argumentCount();
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
            Q_ASSERT(methodArgTypeId != QMetaType::UnknownType);
            V8ScriptValue argVal = context->argument(arg);
            if (methodArgTypeId == scriptValueTypeId) {
                qScriptArgLists[i].append(ScriptValue(new ScriptValueV8Wrapper(_engine, argVal)));
                qGenArgsVectors[i][arg] = Q_ARG(ScriptValue, qScriptArgLists[i].back());
            } else if (methodArgTypeId == QMetaType::QVariant) {
                qVarArgLists[i].append(argVal.toVariant());
                qGenArgsVectors[i][arg] = Q_ARG(QVariant, qVarArgLists[i].back());
            } else {
                QVariant varArgVal;
                if (!_engine->castValueToVariant(argVal, varArgVal, methodArgTypeId)) {
                    conversionFailures++;
                } else {
                    qVarArgLists[i].append(varArgVal);
                    const QVariant& converted = qVarArgLists[i].back();
                    conversionPenaltyScore = _engine->computeCastPenalty(argVal, methodArgTypeId);

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
        ScriptContextV8Wrapper ourContext(_engine, context);
        ScriptContextGuard guard(&ourContext);
        const QMetaMethod& meta = _metas[bestMeta];
        int returnTypeId = meta.returnType();
        QVector <QGenericArgument> &qGenArgs = qGenArgsVectors[bestMeta];

        // The Qt MOC engine will automatically call qRegisterMetaType on invokable parameters and properties, but there's
        // nothing in there for return values so these need to be explicitly runtime-registered!
        Q_ASSERT(returnTypeId != QMetaType::UnknownType);
        if (returnTypeId == QMetaType::UnknownType) {
            _engine->getIsolate()->ThrowError(v8::String::NewFromUtf8(_engine->getIsolate(), QString("Cannot call native function %1, its return value has not been registered with Qt").arg(fullName()).toStdString().c_str()).ToLocalChecked());
            return QVariant();
        } else if (returnTypeId == QMetaType::Void) {
            bool success = meta.invoke(qobject, Qt::DirectConnection, qGenArgs[0], qGenArgs[1], qGenArgs[2], qGenArgs[3],
                                        qGenArgs[4], qGenArgs[5], qGenArgs[6], qGenArgs[7], qGenArgs[8], qGenArgs[9]);
            if (!success) {
                _engine->getIsolate()->ThrowError(v8::String::NewFromUtf8(_engine->getIsolate(), QString("Unexpected: Native call of %1 failed").arg(fullName()).toStdString().c_str()).ToLocalChecked());
            }
            return QVariant();
        } else if (returnTypeId == scriptValueTypeId) {
            ScriptValue result;
            bool success = meta.invoke(qobject, Qt::DirectConnection, Q_RETURN_ARG(ScriptValue, result), qGenArgs[0],
                                        qGenArgs[1], qGenArgs[2], qGenArgs[3], qGenArgs[4], qGenArgs[5], qGenArgs[6],
                                        qGenArgs[7], qGenArgs[8], qGenArgs[9]);
            if (!success) {
                _engine->getIsolate()->ThrowError(v8::String::NewFromUtf8(_engine->getIsolate(), QString("Unexpected: Native call of %1 failed").arg(fullName()).toStdString().c_str()).ToLocalChecked());
                return QVariant();
            }
            V8ScriptValue qResult = ScriptValueV8Wrapper::fullUnwrap(_engine, result);
            return QVariant::fromValue(qResult);
        } else {
            // a lot of type conversion assistance thanks to https://stackoverflow.com/questions/28457819/qt-invoke-method-with-qvariant
            const char* typeName = meta.typeName();
            QVariant qRetVal(returnTypeId, static_cast<void*>(NULL));
            QGenericReturnArgument sRetVal(typeName, const_cast<void*>(qRetVal.constData()));

            bool success =
                meta.invoke(qobject, Qt::DirectConnection, sRetVal, qGenArgs[0], qGenArgs[1], qGenArgs[2], qGenArgs[3],
                            qGenArgs[4], qGenArgs[5], qGenArgs[6], qGenArgs[7], qGenArgs[8], qGenArgs[9]);
            if (!success) {
                _engine->getIsolate()->ThrowError(v8::String::NewFromUtf8(_engine->getIsolate(), QString("Unexpected: Native call of %1 failed").arg(fullName()).toStdString().c_str()).ToLocalChecked());
                return QVariant();
            }
            V8ScriptValue qResult = _engine->castVariantToValue(qRetVal);
            return QVariant::fromValue(qResult);
        }
    }

    // we failed to convert the call to C++, try to create a somewhat sane error message
    if (parameterConversionFailureCount == 0) {
        _engine->getIsolate()->ThrowError(v8::String::NewFromUtf8(_engine->getIsolate(), QString("Native call of %1 failed: unexpected parameter count").arg(fullName()).toStdString().c_str()).ToLocalChecked());
        return QVariant();
    }

    const QMetaMethod& meta = _object->metaObject()->method(parameterConversionFailureId);
    int methodNumArgs = meta.parameterCount();
    Q_ASSERT(methodNumArgs == numArgs);

    for (int arg = 0; arg < numArgs; ++arg) {
        int methodArgTypeId = meta.parameterType(arg);
        Q_ASSERT(methodArgTypeId != QMetaType::UnknownType);
        V8ScriptValue argVal = context->argument(arg);
        if (methodArgTypeId != scriptValueTypeId && methodArgTypeId != QMetaType::QVariant) {
            QVariant varArgVal;
            if (!_engine->castValueToVariant(argVal, varArgVal, methodArgTypeId)) {
                QByteArray methodTypeName = QMetaType(methodArgTypeId).name();
                QByteArray argTypeName = _engine->valueType(argVal).toLatin1();
                _engine->getIsolate()->ThrowError(v8::String::NewFromUtf8(_engine->getIsolate(), QString("Native call of %1 failed: Cannot convert parameter %2 from %3 to %4")
                                                                   .arg(fullName()).arg(arg+1).arg(argTypeName, methodTypeName).toStdString().c_str()).ToLocalChecked());
                context->throwError(V8ScriptContext::TypeError, QString("Native call of %1 failed: Cannot convert parameter %2 from %3 to %4")
                                                                   .arg(fullName()).arg(arg+1).arg(argTypeName, methodTypeName));
                return QVariant();
            }
        }
    }

    context->throwError(QString("Native call of %1 failed: could not locate an overload with the requested arguments").arg(fullName()));
    Q_ASSERT(false); // really shouldn't have gotten here -- it didn't work before and it's working now?
    return QVariant();
}*/

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
    
    v8::HandleScope handleScope(_engine->getIsolate());

    //V8ScriptValueList args(isolate, v8::Null(isolate));
    v8::Local<v8::Value> args[Q_METAMETHOD_INVOKE_MAX_ARGS];
    int numArgs = _meta.parameterCount();
    for (int arg = 0; arg < numArgs; ++arg) {
        int methodArgTypeId = _meta.parameterType(arg);
        Q_ASSERT(methodArgTypeId != QMetaType::UnknownType);
        QVariant argValue(methodArgTypeId, arguments[arg+1]);
        args[arg] = _engine->castVariantToValue(argValue).get();
    }

    QList<Connection> connections;
    withReadLock([&]{
        connections = _connections;
    });

    for (ConnectionList::iterator iter = connections.begin(); iter != connections.end(); ++iter) {
        Connection& conn = *iter;
        v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(conn.callback.get());
        v8::Local<v8::Value> v8This;
        if (conn.thisValue.get()->IsNull()) {
            v8This = _engine->getContext()->Global();
        } else {
            v8This = conn.thisValue.get();
        }
        //V8TODO: should there be a trycatch here?
        callback->Call(_engine->getContext(), v8This, numArgs, args);
    }

    return -1;
}

int ScriptSignalV8Proxy::discoverMetaCallIdx() {
    const QMetaObject* ourMeta = metaObject();
    return ourMeta->methodCount();
}

ScriptSignalV8Proxy::ConnectionList::iterator ScriptSignalV8Proxy::findConnection(V8ScriptValue thisObject, V8ScriptValue callback) {
    auto iterOut = resultWithReadLock<ScriptSignalV8Proxy::ConnectionList::iterator>([&]{
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


void ScriptSignalV8Proxy::connect(V8ScriptValue arg0, V8ScriptValue arg1) {
    QObject* qobject = _object;
    v8::Isolate *isolate = _engine->getIsolate();
    if (!qobject) {
        isolate->ThrowError("Referencing deleted native object");
        return;
    }
    
    v8::HandleScope handleScope(isolate);

    // untangle the arguments
    V8ScriptValue callback(isolate, v8::Null(isolate));
    V8ScriptValue callbackThis(isolate, v8::Null(isolate));
    if (arg1.get()->IsFunction()) {
        callbackThis = arg0;
        callback = arg1;
    } else {
        callback = arg0;
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
    v8::Local<v8::Function> destFunction = v8::Local<v8::Function>::Cast(callback.get());
    v8::Local<v8::String> destDataName = v8::String::NewFromUtf8(isolate, "__data__").ToLocalChecked();
    v8::Local<v8::Value> destData;
    auto destFunctionContext = destFunction->CreationContext();
    V8ScriptValue v8ThisObject = ScriptValueV8Wrapper::fullUnwrap(_engine, thisObject());
    //Q_ASSERT(destFunction->InternalFieldCount() == 4);
    //Q_ASSERT(destData.get()->IsArray());
    //v8::Local<v8::Value> destData = destFunction->GetInternalField(3);
    if (destFunction->Get(destFunctionContext, destDataName).ToLocal(&destData) && destData->IsArray()) {
        v8::Local<v8::Array> destArray = v8::Local<v8::Array>::Cast(destData);
        int length = destArray->Length();//destData.property("length").toInteger();
        v8::Local<v8::Array> newArray = v8::Array::New(isolate, length + 1);
        bool foundIt = false;
        for (int idx = 0; idx < length && !foundIt; ++idx) {
            v8::Local<v8::Value> entry = destArray->Get(destFunctionContext, idx).ToLocalChecked();
            newArray->Set(destFunctionContext, idx, entry);
        }
        newArray->Set(destFunctionContext, length, v8ThisObject.get());
        destFunction->Set(destFunctionContext, destDataName, newArray);
    } else {
        v8::Local<v8::Array> newArray = v8::Array::New(isolate, 1);
        newArray->Set(destFunctionContext, 0, v8ThisObject.get());
        destFunction->Set(destFunctionContext, destDataName, newArray);
    }
    /*{
        V8ScriptValueList args;
        args << thisObject();
        destData.property("push").call(destData, args);
    }*/

    // add this to our internal list of connections
    Connection newConnection(callbackThis, callback);
    //newConn.callback = callback;
    //newConn.thisValue = callbackThis;
    
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

void ScriptSignalV8Proxy::disconnect(V8ScriptValue arg0, V8ScriptValue arg1) {
    QObject* qobject = _object;
    v8::Isolate *isolate = _engine->getIsolate();
    if (!qobject) {
        isolate->ThrowError("Referencing deleted native object");
        return;
    }
    v8::HandleScope handleScope(isolate);

    // untangle the arguments
    V8ScriptValue callback(isolate, v8::Null(isolate));
    V8ScriptValue callbackThis(isolate, v8::Null(isolate));
    if (arg1.get()->IsFunction()) {
        callbackThis = arg0;
        callback = arg1;
    } else {
        callback = arg0;
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
    auto destFunctionContext = destFunction->CreationContext();
    V8ScriptValue v8ThisObject = ScriptValueV8Wrapper::fullUnwrap(_engine, thisObject());
    //V8ScriptValue destData = callback.data();
    //Q_ASSERT(destData->IsArray());
    if (destFunction->Get(destFunctionContext, destDataName).ToLocal(&destData) && destData->IsArray()) {
        v8::Local<v8::Array> destArray = v8::Local<v8::Array>::Cast(destData);
        int length = destArray->Length();//destData.property("length").toInteger();
        v8::Local<v8::Array> newArray = v8::Array::New(isolate, length - 1);
        bool foundIt = false;
        int newIndex = 0;
        for (int idx = 0; idx < length && !foundIt; ++idx) {
            v8::Local<v8::Value> entry = destArray->Get(destFunctionContext, idx).ToLocalChecked();
            if (entry->StrictEquals(v8ThisObject.get())) {
                foundIt = true;
                //V8ScriptValueList args;
                //args << idx << 1;
                //destData.property("splice").call(destData, args);
            } else {
                newArray->Set(destFunctionContext, newIndex, entry);
                newIndex++;
            }
        }
        Q_ASSERT(foundIt);
        destFunction->Set(destFunctionContext, destDataName, newArray);
    } else {
        Q_ASSERT(false);
    }

    // inform Qt that we're no longer connected to this signal
    if (_connections.empty()) {
        Q_ASSERT(_isConnected);
        bool result = QMetaObject::disconnect(qobject, _meta.methodIndex(), this, _metaCallId);
        Q_ASSERT(result);
        _isConnected = false;
    }
}
