//
//  ScriptObjectV8Proxy.h
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

/// @addtogroup ScriptEngine
/// @{

#ifndef hifi_ScriptObjectV8Proxy_h
#define hifi_ScriptObjectV8Proxy_h

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QPointer>
#include <QtCore/QString>

#include "../ScriptEngine.h"
#include "../Scriptable.h"
#include "ScriptEngineV8.h"
#include "V8Types.h"

#include <shared/ReadWriteLockable.h>

class ScriptEngineV8;
class ScriptSignalV8Proxy;

/// [V8] (re-)implements the translation layer between ScriptValue and QObject.  This object
/// will focus exclusively on property get/set until function calls appear to be a problem
class ScriptObjectV8Proxy final {
private:  // implementation
    class PropertyDef {
    public:
        PropertyDef(ScriptEngineV8 *engine, v8::Local<v8::String> string, uint id) : name(engine, string), _id(id) {};
        V8ScriptString name;
        ScriptValue::PropertyFlags flags;
        uint _id;
    };
    class MethodDef {
    public:
        MethodDef(ScriptEngineV8 *engine, v8::Local<v8::String> string, uint id) : name(engine, string), _id(id) {};
        V8ScriptString name;
        int numMaxParms;
        QList<QMetaMethod> methods;
        uint _id;
    };
    class SignalDef {
    public:
        SignalDef(ScriptEngineV8 *engine, v8::Local<v8::String> string, uint id) : name(engine, string), _id(id) {};
        V8ScriptString name;
        QMetaMethod signal;
        uint _id;
    };
    using PropertyDefMap = QHash<uint, PropertyDef>;
    using MethodDefMap = QHash<uint, MethodDef>;
    using SignalDefMap = QHash<uint, SignalDef>;
    using InstanceMap = QHash<uint, QPointer<ScriptSignalV8Proxy> >;
    using PropertyNameMap = QHash<V8ScriptString, PropertyDef*>;
    using MethodNameMap = QHash<V8ScriptString, MethodDef*>;
    using SignalNameMap = QHash<V8ScriptString, SignalDef*>;

    static constexpr uint PROPERTY_TYPE = 0x1000;
    static constexpr uint METHOD_TYPE = 0x2000;
    static constexpr uint SIGNAL_TYPE = 0x3000;
    static constexpr uint TYPE_MASK = 0xF000;

public:  // construction
    ScriptObjectV8Proxy(ScriptEngineV8* engine, QObject* object, bool ownsObject, const ScriptEngine::QObjectWrapOptions& options);
    virtual ~ScriptObjectV8Proxy();

    static V8ScriptValue newQObject(ScriptEngineV8* engine,
                                   QObject* object,
                                   ScriptEngine::ValueOwnership ownership = ScriptEngine::QtOwnership,
                                   const ScriptEngine::QObjectWrapOptions& options = ScriptEngine::QObjectWrapOptions());
    static ScriptObjectV8Proxy* unwrapProxy(const V8ScriptValue& val);
    static ScriptObjectV8Proxy* unwrapProxy(v8::Isolate* isolate, v8::Local<v8::Value>& value);
    static QObject* unwrap(const V8ScriptValue& val);
    inline QObject* toQObject() const { return _object; }
    inline v8::Local<v8::Object> toV8Value() const {
        v8::EscapableHandleScope handleScope(_engine->getIsolate());
        return handleScope.Escape(_v8Object.Get(_engine->getIsolate()));
    }

public:
    enum QueryFlag
    {
        HandlesReadAccess = 0x00000001,
        HandlesWriteAccess = 0x00000002,
    };
    Q_DECLARE_FLAGS(QueryFlags, QueryFlag);

    virtual QString name() const;

    virtual V8ScriptValue property(const V8ScriptValue& object, const V8ScriptString& name, uint id);
    virtual ScriptValue::PropertyFlags propertyFlags(const V8ScriptValue& object, const V8ScriptString& name, uint id);
    //V8TODO
    virtual QueryFlags queryProperty(const V8ScriptValue& object, const V8ScriptString& name, QueryFlags flags, uint* id);
    virtual void setProperty(V8ScriptValue& object, const V8ScriptString& name, uint id, const V8ScriptValue& value);
    v8::Local<v8::Array> getPropertyNames();
    static void v8Get(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value>& info);
    static void v8Set(v8::Local<v8::Name> name, v8::Local<v8::Value> value_obj, const v8::PropertyCallbackInfo<v8::Value>& info);
    static void v8GetPropertyNames(const v8::PropertyCallbackInfo<v8::Array>& info);

private:  // implementation
    void investigate();

private:  // storage
    ScriptEngineV8* _engine;
    const ScriptEngine::QObjectWrapOptions _wrapOptions;
    PropertyDefMap _props;
    MethodDefMap _methods;
    SignalDefMap _signals;
    // These are used for property lookups from V8 callbacks
    PropertyNameMap _propNameMap;
    MethodNameMap _methodNameMap;
    SignalNameMap _signalNameMap;
    InstanceMap _signalInstances;
    const bool _ownsObject;
    QPointer<QObject> _object;
    // Handle for its own object
    // V8TODO Maybe depending on object ownership it should be different type of handles? For example weak persistent would allow
    // script engine-owned objects to be garbage collected. This will also need adding a garbage collector callback from V8
    // to let proxy know that it is not valid anymore
    v8::UniquePersistent<v8::Object> _v8Object;
    int pointerCorruptionTest = 12345678;

    Q_DISABLE_COPY(ScriptObjectV8Proxy)
};

/**
 * @brief [V8] (re-)implements the translation layer between ScriptValue and QVariant where a prototype is set.
 *
 * This object depends on a ScriptObjectV8Proxy to provide the prototype's behavior.
 * ScriptVariantV8Proxy uses prototype class which provides methods which operate on QVariant.
 * Typically it's used for class with larger number of methods which has a simplified JS API.
 * For example it's used to provide JS scripting interface to AnimationPointer by using methods of AnimationObject.
 * To use this functionality, given type has to be registered with script engine together with its prototype:
 *
 * engine->setDefaultPrototype(qMetaTypeId<AnimationPointer>(), engine->newQObject(
        new AnimationObject(), ScriptEngine::ScriptOwnership));
 *
 */
class ScriptVariantV8Proxy final {
public:  // construction
    ScriptVariantV8Proxy(ScriptEngineV8* engine, const QVariant& variant, V8ScriptValue scriptProto, ScriptObjectV8Proxy* proto);
    ~ScriptVariantV8Proxy();

    static V8ScriptValue newVariant(ScriptEngineV8* engine, const QVariant& variant, V8ScriptValue proto);
    static ScriptVariantV8Proxy* unwrapProxy(const V8ScriptValue& val);
    static ScriptVariantV8Proxy* unwrapProxy(v8::Isolate* isolate, v8::Local<v8::Value> &value);
    /**
     * @brief Used to retrieve QVariant pointer contained inside script value. This is indirectly used by ScriptVariantV8Proxy
     * getters and setters through scriptvalue_cast and ScriptEngineV8::castValueToVariant.
     */
    static QVariant* unwrapQVariantPointer(v8::Isolate* isolate, const v8::Local<v8::Value> &value);
    static QVariant unwrap(const V8ScriptValue& val);
    inline QVariant toQVariant() const { return _variant; }
    //inline QVariant toV8Value() const { return _variant; }
    inline v8::Local<v8::Object> toV8Value() const {
        v8::EscapableHandleScope handleScope(_engine->getIsolate());
        return handleScope.Escape(_v8Object.Get(_engine->getIsolate()));
    }

public:  // QScriptClass implementation
    virtual QString name() const { return _name; }

    virtual V8ScriptValue prototype() const { return _scriptProto; }

    virtual V8ScriptValue property(const V8ScriptValue& object, const V8ScriptString& name, uint id) {
        return _proto->property(object, name, id);
    }
    virtual ScriptValue::PropertyFlags propertyFlags(const V8ScriptValue& object, const V8ScriptString& name, uint id) {
        return _proto->propertyFlags(object, name, id);
    }
    /*virtual QueryFlags queryProperty(const V8ScriptValue& object, const V8ScriptString& name, QueryFlags flags, uint* id) {
        return _proto->queryProperty(object, name, flags, id);
    }*/
    virtual void setProperty(V8ScriptValue& object, const V8ScriptString& name, uint id, const V8ScriptValue& value) {
        return _proto->setProperty(object, name, id, value);
    }
    static void v8Get(v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value>& info);
    static void v8Set(v8::Local<v8::Name> name, v8::Local<v8::Value> value_obj, const v8::PropertyCallbackInfo<v8::Value>& info);
    static void v8GetPropertyNames(const v8::PropertyCallbackInfo<v8::Array>& info);

private:
    ScriptEngineV8* _engine;
    QVariant _variant;
    V8ScriptValue _scriptProto;
    ScriptObjectV8Proxy* _proto;
    QString _name;
    //v8::UniquePersistent<v8::ObjectTemplate> _v8ObjectTemplate;
    v8::UniquePersistent<v8::Object> _v8Object;

    Q_DISABLE_COPY(ScriptVariantV8Proxy)
};

class ScriptMethodV8Proxy final {
public:  // construction
    ScriptMethodV8Proxy(ScriptEngineV8* engine, QObject* object, V8ScriptValue lifetime,
                               const QList<QMetaMethod>& metas, int numMaxParams);
    virtual ~ScriptMethodV8Proxy();

public:  // QScriptClass implementation
    virtual QString name() const { return fullName(); }
    //virtual bool supportsExtension(Extension extension) const;
    static void callback(const v8::FunctionCallbackInfo<v8::Value>& arguments);
    void call(const v8::FunctionCallbackInfo<v8::Value>& arguments);
    //virtual QVariant extension(Extension extension, const QVariant& argument = QVariant());
    static V8ScriptValue newMethod(ScriptEngineV8* engine, QObject* object, V8ScriptValue lifetime,
                               const QList<QMetaMethod>& metas, int numMaxParams);

private:
    QString fullName() const;

private:  // storage
    const int _numMaxParams;
    ScriptEngineV8* _engine;
    QPointer<QObject> _object;
    V8ScriptValue _objectLifetime;
    const QList<QMetaMethod> _metas;

    Q_DISABLE_COPY(ScriptMethodV8Proxy)
};

// This abstract base class serves solely to declare the Q_INVOKABLE methods for ScriptSignalV8Proxy
// as we're overriding qt_metacall later for the signal callback yet still want to support
// metacalls for the connect/disconnect API
class ScriptSignalV8ProxyBase : public QObject, protected Scriptable {
    Q_OBJECT
public:  // API
    // arg1 was had Null default value, but that needs isolate pointer in V8
    Q_INVOKABLE virtual void connect(ScriptValue arg0, ScriptValue arg1 = ScriptValue()) = 0;
    Q_INVOKABLE virtual void disconnect(ScriptValue arg0, ScriptValue arg1 = ScriptValue()) = 0;
    //Q_INVOKABLE virtual void connect(ScriptValue arg0) = 0;
    //Q_INVOKABLE virtual void disconnect(ScriptValue arg0) = 0;
};

class ScriptSignalV8Proxy final : public ScriptSignalV8ProxyBase, public ReadWriteLockable {
private:  // storage
    class Connection {
    public:
        V8ScriptValue thisValue;
        V8ScriptValue callback;
        Connection(const V8ScriptValue &v8ThisValue, const V8ScriptValue &v8Callback) : 
            thisValue(v8ThisValue), callback(v8Callback) {};
    };
    using ConnectionList = QList<Connection>;

public:  // construction
    inline ScriptSignalV8Proxy(ScriptEngineV8* engine, QObject* object, V8ScriptValue lifetime, const QMetaMethod& meta) :
        _engine(engine), _object(object), _objectLifetime(lifetime), _meta(meta), _metaCallId(discoverMetaCallIdx()) {
        v8::Locker locker(_engine->getIsolate());
        v8::Isolate::Scope isolateScope(_engine->getIsolate());
        v8::HandleScope handleScope(_engine->getIsolate());
        v8::Context::Scope contextScope(_engine->getContext());
        _v8Context.Reset(_engine->getIsolate(), _engine->getContext());
    }

    ~ScriptSignalV8Proxy();

private:  // implementation
    virtual int qt_metacall(QMetaObject::Call call, int id, void** arguments) override;
    int discoverMetaCallIdx();
    ConnectionList::iterator findConnection(V8ScriptValue thisObject, V8ScriptValue callback);
    //QString fullName() const;

public:  // API
    // arg1 was had Null default value, but that needs isolate pointer to create Null in V8
    virtual void connect(ScriptValue arg0, ScriptValue arg1 = ScriptValue()) override;
    virtual void disconnect(ScriptValue arg0, ScriptValue arg1 = ScriptValue()) override;
    //Moved to public temporarily for debugging:
    QString fullName() const;

    //virtual void connect(V8ScriptValue arg0) override;
    //virtual void disconnect(V8ScriptValue arg0) override;

private:  // storage
    ScriptEngineV8* _engine;
    QPointer<QObject> _object;
    V8ScriptValue _objectLifetime;
    const QMetaMethod _meta;
    const int _metaCallId;
    ConnectionList _connections;
    bool _isConnected{ false };
    // Context in which it was created
    v8::UniquePersistent<v8::Context> _v8Context;
    // Call counter for debugging purposes. It can be used to determine which signals are overwhelming script engine.
    int _callCounter{0};
    float _totalCallTime_s{ 0.0 };

    Q_DISABLE_COPY(ScriptSignalV8Proxy)
};

#endif  // hifi_ScriptObjectV8Proxy_h

/// @}
