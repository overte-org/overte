//
//  TypedArrays.h
//
//
//  Created by Clement on 7/9/14.
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

/// @addtogroup ScriptEngine
/// @{

#ifndef hifi_TypedArrays_h
#define hifi_TypedArrays_h

// V8TODO Do not remove yet, this will be useful in later PRs
/*#include "ArrayBufferViewClass.h"

static const QString BYTES_PER_ELEMENT_PROPERTY_NAME = "BYTES_PER_ELEMENT";
static const QString LENGTH_PROPERTY_NAME = "length";

static const QString INT_8_ARRAY_CLASS_NAME = "Int8Array";
static const QString UINT_8_ARRAY_CLASS_NAME = "Uint8Array";
static const QString UINT_8_CLAMPED_ARRAY_CLASS_NAME = "Uint8ClampedArray";
static const QString INT_16_ARRAY_CLASS_NAME = "Int16Array";
static const QString UINT_16_ARRAY_CLASS_NAME = "Uint16Array";
static const QString INT_32_ARRAY_CLASS_NAME = "Int32Array";
static const QString UINT_32_ARRAY_CLASS_NAME = "Uint32Array";
static const QString FLOAT_32_ARRAY_CLASS_NAME = "Float32Array";
static const QString FLOAT_64_ARRAY_CLASS_NAME = "Float64Array";

/// [QtScript] Implements the <code><a href="https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/TypedArray">TypedArray</a></code> scripting class
class TypedArray : public ArrayBufferViewClass {
    Q_OBJECT
public:
    TypedArray(ScriptEngineV8* scriptEngine, QString name);
    virtual V8ScriptValue newInstance(quint32 length);
    virtual V8ScriptValue newInstance(V8ScriptValue array);
    virtual V8ScriptValue newInstance(V8ScriptValue buffer, quint32 byteOffset, quint32 length);

    virtual QueryFlags queryProperty(const V8ScriptValue& object,
                                     const V8ScriptString& name,
                                     QueryFlags flags, uint* id) override;
    virtual V8ScriptValue property(const V8ScriptValue& object,
                                  const V8ScriptString& name, uint id) override;
    virtual void setProperty(V8ScriptValue& object, const V8ScriptString& name, uint id, const V8ScriptValue& value) override = 0;
    virtual V8ScriptValue::PropertyFlags propertyFlags(const V8ScriptValue& object,
                                                      const V8ScriptString& name, uint id) override;

    QString name() const override;
    V8ScriptValue prototype() const override;

protected:
    static V8ScriptValue construct(V8ScriptContext* context, QScriptEngine* engine);

    void setBytesPerElement(quint32 bytesPerElement);

    V8ScriptValue _proto;
    V8ScriptValue _ctor;

    V8ScriptString _name;
    V8ScriptString _bytesPerElementName;
    V8ScriptString _lengthName;

    quint32 _bytesPerElement;

    friend class TypedArrayPrototype;
};

class Int8ArrayClass : public TypedArray {
    Q_OBJECT
public:
    Int8ArrayClass(ScriptEngineV8* scriptEngine);

    V8ScriptValue property(const V8ScriptValue& object, const V8ScriptString& name, uint id) override;
    void setProperty(V8ScriptValue& object, const V8ScriptString& name, uint id, const V8ScriptValue& value) override;
};

class Uint8ArrayClass : public TypedArray {
    Q_OBJECT
public:
    Uint8ArrayClass(ScriptEngineV8* scriptEngine);

    V8ScriptValue property(const V8ScriptValue& object, const V8ScriptString& name, uint id) override;
    void setProperty(V8ScriptValue& object, const V8ScriptString& name, uint id, const V8ScriptValue& value) override;
};

class Uint8ClampedArrayClass : public TypedArray {
    Q_OBJECT
public:
    Uint8ClampedArrayClass(ScriptEngineV8* scriptEngine);

    V8ScriptValue property(const V8ScriptValue& object, const V8ScriptString& name, uint id) override;
    void setProperty(V8ScriptValue& object, const V8ScriptString& name, uint id, const V8ScriptValue& value) override;
};

class Int16ArrayClass : public TypedArray {
    Q_OBJECT
public:
    Int16ArrayClass(ScriptEngineV8* scriptEngine);

    V8ScriptValue property(const V8ScriptValue& object, const V8ScriptString& name, uint id) override;
    void setProperty(V8ScriptValue& object, const V8ScriptString& name, uint id, const V8ScriptValue& value) override;
};

class Uint16ArrayClass : public TypedArray {
    Q_OBJECT
public:
    Uint16ArrayClass(ScriptEngineV8* scriptEngine);

    V8ScriptValue property(const V8ScriptValue& object, const V8ScriptString& name, uint id) override;
    void setProperty(V8ScriptValue& object, const V8ScriptString& name, uint id, const V8ScriptValue& value) override;
};

class Int32ArrayClass : public TypedArray {
    Q_OBJECT
public:
    Int32ArrayClass(ScriptEngineV8* scriptEngine);

    V8ScriptValue property(const V8ScriptValue& object, const V8ScriptString& name, uint id) override;
    void setProperty(V8ScriptValue& object, const V8ScriptString& name, uint id, const V8ScriptValue& value) override;
};

class Uint32ArrayClass : public TypedArray {
    Q_OBJECT
public:
    Uint32ArrayClass(ScriptEngineV8* scriptEngine);

    V8ScriptValue property(const V8ScriptValue& object, const V8ScriptString& name, uint id) override;
    void setProperty(V8ScriptValue& object, const V8ScriptString& name, uint id, const V8ScriptValue& value) override;
};

class Float32ArrayClass : public TypedArray {
    Q_OBJECT
public:
    Float32ArrayClass(ScriptEngineV8* scriptEngine);

    V8ScriptValue property(const V8ScriptValue& object, const V8ScriptString& name, uint id) override;
    void setProperty(V8ScriptValue& object, const V8ScriptString& name, uint id, const V8ScriptValue& value) override;
};

class Float64ArrayClass : public TypedArray {
    Q_OBJECT
public:
    Float64ArrayClass(ScriptEngineV8* scriptEngine);

    V8ScriptValue property(const V8ScriptValue& object, const V8ScriptString& name, uint id) override;
    void setProperty(V8ScriptValue& object, const V8ScriptString& name, uint id, const V8ScriptValue& value) override;
};
*/
#endif // hifi_TypedArrays_h

/// @}
