//
//  DataViewClass.cpp
//
//
//  Created by Clement on 7/8/14.
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "DataViewClass.h"

// V8TODO
/*#include "DataViewPrototype.h"

Q_DECLARE_METATYPE(QByteArray*)

static const QString DATA_VIEW_NAME = "DataView";

DataViewClass::DataViewClass(ScriptEngineV8* scriptEngine) : ArrayBufferViewClass(scriptEngine) {
    V8ScriptValue global = engine()->globalObject();
    
    // Save string handles for quick lookup
    _name = engine()->toStringHandle(DATA_VIEW_NAME.toLatin1());
    
    // build prototype
    _proto = engine()->newQObject(new DataViewPrototype(this),
                                QScriptEngine::QtOwnership,
                                QScriptEngine::SkipMethodsInEnumeration |
                                QScriptEngine::ExcludeSuperClassMethods |
                                QScriptEngine::ExcludeSuperClassProperties);
    _proto.setPrototype(global.property("Object").property("prototype"));
    
    // Register constructor
    _ctor = engine()->newFunction(construct, _proto);
    _ctor.setData(engine()->toScriptValue(this));
    engine()->globalObject().setProperty(name(), _ctor);
}

V8ScriptValue DataViewClass::newInstance(V8ScriptValue buffer, quint32 byteOffset, quint32 byteLentgh) {
    V8ScriptValue data = engine()->newObject();
    data.setProperty(_bufferName, buffer);
    data.setProperty(_byteOffsetName, byteOffset);
    data.setProperty(_byteLengthName, byteLentgh);
    
    return engine()->newObject(this, data);
}

V8ScriptValue DataViewClass::construct(V8ScriptContext* context, QScriptEngine* engine) {
    DataViewClass* cls = qscriptvalue_cast<DataViewClass*>(context->callee().data());
    if (!cls || context->argumentCount() < 1) {
        return V8ScriptValue();
    }
    
    V8ScriptValue bufferArg = context->argument(0);
    V8ScriptValue byteOffsetArg = (context->argumentCount() >= 2) ? context->argument(1) : V8ScriptValue();
    V8ScriptValue byteLengthArg = (context->argumentCount() >= 3) ? context->argument(2) : V8ScriptValue();
    
    QByteArray* arrayBuffer = (bufferArg.isValid()) ? qscriptvalue_cast<QByteArray*>(bufferArg.data()) :NULL;
    if (!arrayBuffer) {
        engine->evaluate("throw \"TypeError: 1st argument not a ArrayBuffer\"");
        return V8ScriptValue();
    }
    if (byteOffsetArg.isNumber() &&
        (byteOffsetArg.toInt32() < 0 ||
         byteOffsetArg.toInt32() > arrayBuffer->size())) {
            engine->evaluate("throw \"RangeError: byteOffset out of range\"");
            return V8ScriptValue();
        }
    if (byteLengthArg.isNumber() &&
        (byteLengthArg.toInt32() < 0 ||
         byteOffsetArg.toInt32() + byteLengthArg.toInt32() > arrayBuffer->size())) {
            engine->evaluate("throw \"RangeError: byteLength out of range\"");
            return V8ScriptValue();
        }
    quint32 byteOffset = (byteOffsetArg.isNumber()) ? byteOffsetArg.toInt32() : 0;
    quint32 byteLength = (byteLengthArg.isNumber()) ? byteLengthArg.toInt32() : arrayBuffer->size() - byteOffset;
    V8ScriptValue newObject = cls->newInstance(bufferArg, byteOffset, byteLength);
    
    if (context->isCalledAsConstructor()) {
        context->setThisObject(newObject);
        return engine->undefinedValue();
    }
    
    return newObject;
}

QString DataViewClass::name() const {
    return _name.toString();
}

V8ScriptValue DataViewClass::prototype() const {
    return _proto;
}
*/
