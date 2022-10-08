//
//  ArrayBufferViewClass.cpp
//
//
//  Created by Clement on 7/8/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ArrayBufferViewClass.h"
#include "ScriptEngineV8.h"

Q_DECLARE_METATYPE(QByteArray*)

// V8TODO
/*ArrayBufferViewClass::ArrayBufferViewClass(ScriptEngineV8* scriptEngine) :
    QObject(scriptEngine),
    QScriptClass(scriptEngine),
    _scriptEngine(scriptEngine)
{
    // Save string handles for quick lookup
    _bufferName = engine()->toStringHandle(BUFFER_PROPERTY_NAME.toLatin1());
    _byteOffsetName = engine()->toStringHandle(BYTE_OFFSET_PROPERTY_NAME.toLatin1());
    _byteLengthName = engine()->toStringHandle(BYTE_LENGTH_PROPERTY_NAME.toLatin1());
}

QScriptClass::QueryFlags ArrayBufferViewClass::queryProperty(const V8ScriptValue& object,
                                                             const V8ScriptString& name,
                                                             QueryFlags flags, uint* id) {
    if (name == _bufferName || name == _byteOffsetName || name == _byteLengthName) {
        return flags &= HandlesReadAccess; // Only keep read access flags
    }
    return QScriptClass::QueryFlags(); // No access
}

V8ScriptValue ArrayBufferViewClass::property(const V8ScriptValue& object,
                                            const V8ScriptString& name, uint id) {
    if (name == _bufferName) {
        return object.data().property(_bufferName);
    }
    if (name == _byteOffsetName) {
        return object.data().property(_byteOffsetName);
    }
    if (name == _byteLengthName) {
        return object.data().property(_byteLengthName);
    }
    return V8ScriptValue();
}

V8ScriptValue::PropertyFlags ArrayBufferViewClass::propertyFlags(const V8ScriptValue& object,
                                                                const V8ScriptString& name, uint id) {
    return V8ScriptValue::Undeletable;
}*/
