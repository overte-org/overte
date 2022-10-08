//
//  ArrayBufferViewClass.h
//
//
//  Created by Clement on 7/8/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

/// @addtogroup ScriptEngine
/// @{

#ifndef hifi_ArrayBufferViewClass_h
#define hifi_ArrayBufferViewClass_h

#include <QtCore/QObject>
#include "V8Types.h"

// V8TODO
/*class ScriptEngineV8;

static const QString BUFFER_PROPERTY_NAME = "buffer";
static const QString BYTE_OFFSET_PROPERTY_NAME = "byteOffset";
static const QString BYTE_LENGTH_PROPERTY_NAME = "byteLength";

/// [V8] The base class containing common code for ArrayBuffer views
class ArrayBufferViewClass : public QObject, public QScriptClass {
    Q_OBJECT
public:
    ArrayBufferViewClass(ScriptEngineV8* scriptEngine);

    ScriptEngineV8* getScriptEngine() { return _scriptEngine; }

    virtual QueryFlags queryProperty(const V8ScriptValue& object,
                                     const V8ScriptString& name,
                                     QueryFlags flags, uint* id) override;
    virtual V8ScriptValue property(const V8ScriptValue& object,
                                  const V8ScriptString& name, uint id) override;
    virtual V8ScriptValue::PropertyFlags propertyFlags(const V8ScriptValue& object,
                                                      const V8ScriptString& name, uint id) override;
protected:
    // JS Object attributes
    V8ScriptString _bufferName;
    V8ScriptString _byteOffsetName;
    V8ScriptString _byteLengthName;

    ScriptEngineV8* _scriptEngine;
};
*/
#endif // hifi_ArrayBufferViewClass_h

/// @}
