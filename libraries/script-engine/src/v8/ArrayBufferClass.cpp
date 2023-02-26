//
//  ArrayBufferClass.cpp
//
//
//  Created by Clement on 7/3/14.
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "ArrayBufferClass.h"

#include <QDebug>

#include "ArrayBufferPrototype.h"
#include "DataViewClass.h"
#include "ScriptEngineV8.h"
#include "TypedArrays.h"


// V8TODO
/*static const QString CLASS_NAME = "ArrayBuffer";

// FIXME: Q_DECLARE_METATYPE is global and really belongs in a shared header file, not per .cpp like this
// (see DataViewClass.cpp, etc. which would also have to be updated to resolve)
Q_DECLARE_METATYPE(QByteArray*)

ArrayBufferClass::ArrayBufferClass(ScriptEngineV8* scriptEngine) :
QObject(scriptEngine),
QScriptClass(scriptEngine) {
    qScriptRegisterMetaType<QByteArray>(engine(), toScriptValue, fromScriptValue);
    V8ScriptValue global = engine()->globalObject();

    // Save string handles for quick lookup
    _name = engine()->toStringHandle(CLASS_NAME.toLatin1());
    _byteLength = engine()->toStringHandle(BYTE_LENGTH_PROPERTY_NAME.toLatin1());

    // build prototype
    _proto = engine()->newQObject(new ArrayBufferPrototype(this),
                                QScriptEngine::QtOwnership,
                                QScriptEngine::SkipMethodsInEnumeration |
                                QScriptEngine::ExcludeSuperClassMethods |
                                QScriptEngine::ExcludeSuperClassProperties);
    _proto.setPrototype(global.property("Object").property("prototype"));

    // Register constructor
    _ctor = engine()->newFunction(construct, _proto);
    _ctor.setData(engine()->toScriptValue(this));

    engine()->globalObject().setProperty(name(), _ctor);

    // Registering other array types
    // The script engine is there parent so it'll delete them with itself
    new DataViewClass(scriptEngine);
    new Int8ArrayClass(scriptEngine);
    new Uint8ArrayClass(scriptEngine);
    new Uint8ClampedArrayClass(scriptEngine);
    new Int16ArrayClass(scriptEngine);
    new Uint16ArrayClass(scriptEngine);
    new Int32ArrayClass(scriptEngine);
    new Uint32ArrayClass(scriptEngine);
    new Float32ArrayClass(scriptEngine);
    new Float64ArrayClass(scriptEngine);
}

V8ScriptValue ArrayBufferClass::newInstance(qint32 size) {
    const qint32 MAX_LENGTH = 100000000;
    if (size < 0) {
        engine()->evaluate("throw \"ArgumentError: negative length\"");
        return V8ScriptValue();
    }
    if (size > MAX_LENGTH) {
        engine()->evaluate("throw \"ArgumentError: absurd length\"");
        return V8ScriptValue();
    }
    // We've patched qt to fix https://highfidelity.atlassian.net/browse/BUGZ-46 on mac and windows only.
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    engine()->reportAdditionalMemoryCost(size);
#endif
    QScriptEngine* eng = engine();
    QVariant variant = QVariant::fromValue(QByteArray(size, 0));
    V8ScriptValue data =  eng->newVariant(variant);
    return engine()->newObject(this, data);
}

V8ScriptValue ArrayBufferClass::newInstance(const QByteArray& ba) {
    QScriptEngine* eng = engine();
    V8ScriptValue data = eng->newVariant(QVariant::fromValue(ba));
    return eng->newObject(this, data);
}

V8ScriptValue ArrayBufferClass::construct(V8ScriptContext* context, QScriptEngine* engine) {
    ArrayBufferClass* cls = qscriptvalue_cast<ArrayBufferClass*>(context->callee().data());
    if (!cls) {
        // return if callee (function called) is not of type ArrayBuffer
        return V8ScriptValue();
    }
    V8ScriptValue arg = context->argument(0);
    if (!arg.isValid() || !arg.isNumber()) {
        return V8ScriptValue();
    }

    quint32 size = arg.toInt32();
    V8ScriptValue newObject = cls->newInstance(size);

    if (context->isCalledAsConstructor()) {
        // if called with keyword new, replace this object.
        context->setThisObject(newObject);
        return engine->undefinedValue();
    }

    return newObject;
}

ScriptObjectV8Proxy::QueryFlags ArrayBufferClass::queryProperty(const V8ScriptValue& object,
                                                    const V8ScriptString& name,
                                                    QueryFlags flags, uint* id) {
    QByteArray* ba = qscriptvalue_cast<QByteArray*>(object.data());
    if (ba && name == _byteLength) {
        // if the property queried is byteLength, only handle read access
        return flags &= HandlesReadAccess;
    }
    return ScriptObjectV8Proxy::QueryFlags(); // No access
}

V8ScriptValue ArrayBufferClass::property(const V8ScriptValue& object,
                                   const V8ScriptString& name, uint id) {
    QByteArray* ba = qscriptvalue_cast<QByteArray*>(object.data());
    if (ba && name == _byteLength) {
        return ba->length();
    }
    return V8ScriptValue();
}

V8ScriptValue::PropertyFlags ArrayBufferClass::propertyFlags(const V8ScriptValue& object,
                                                       const V8ScriptString& name, uint id) {
    return V8ScriptValue::Undeletable;
}

QString ArrayBufferClass::name() const {
    return _name.toString();
}

V8ScriptValue ArrayBufferClass::prototype() const {
    return _proto;
}

V8ScriptValue ArrayBufferClass::toScriptValue(QScriptEngine* engine, const QByteArray& ba) {
    V8ScriptValue ctor = engine->globalObject().property(CLASS_NAME);
    ArrayBufferClass* cls = qscriptvalue_cast<ArrayBufferClass*>(ctor.data());
    if (!cls) {
        if (engine->currentContext()) {
            engine->currentContext()->throwError("arrayBufferClass::toScriptValue -- could not get " + CLASS_NAME + " class constructor");
        }
        return V8ScriptValue::NullValue;
    }
    return cls->newInstance(ba);
}

void ArrayBufferClass::fromScriptValue(const V8ScriptValue& object, QByteArray& byteArray) {
    if (object.isString()) {
        // UTF-8 encoded String
        byteArray = object.toString().toUtf8();
    } else if (object.isArray()) {
        // Array of uint8s eg: [ 128, 3, 25, 234 ]
        auto Uint8Array = object.engine()->globalObject().property("Uint8Array");
        auto typedArray = Uint8Array.construct(V8ScriptValueList{object});
        if (QByteArray* buffer = qscriptvalue_cast<QByteArray*>(typedArray.property("buffer"))) {
            byteArray = *buffer;
        }
    } else if (object.isObject()) {
        // ArrayBuffer instance (or any JS class that supports coercion into QByteArray*)
        if (QByteArray* buffer = qscriptvalue_cast<QByteArray*>(object.data())) {
            byteArray = *buffer;
        }
    }
}
*/
