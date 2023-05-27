//
//  ArrayBufferClass.h
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

/// @addtogroup ScriptEngine
/// @{

#ifndef hifi_ArrayBufferClass_h
#define hifi_ArrayBufferClass_h

#include <QtCore/QObject>
#include "libplatform/libplatform.h"
#include "v8.h"
#include <QtCore/QDateTime>

// V8TODO Do not remove yet, this will be useful in later PRs
//#include "V8Types.h"
/*
class ScriptEngineV8;

/// [V8] Implements the <code><a href="https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/ArrayBuffer">ArrayBuffer</a></code> scripting class
class ArrayBufferClass : public QObject, public ScriptClass {
    Q_OBJECT
public:
    ArrayBufferClass(ScriptEngineV8* scriptEngine);
    V8ScriptValue newInstance(qint32 size);
    V8ScriptValue newInstance(const QByteArray& ba);

    QueryFlags queryProperty(const V8ScriptValue& object,
                             const V8ScriptString& name,
                             QueryFlags flags, uint* id) override;
    V8ScriptValue property(const V8ScriptValue& object,
                          const V8ScriptString& name, uint id) override;
    V8ScriptValue::PropertyFlags propertyFlags(const V8ScriptValue& object,
                                              const V8ScriptString& name, uint id) override;

    QString name() const override;
    V8ScriptValue prototype() const override;


private:
    static V8ScriptValue construct(V8ScriptContext* context, QScriptEngine* engine);

    static V8ScriptValue toScriptValue(QScriptEngine* eng, const QByteArray& ba);
    static void fromScriptValue(const V8ScriptValue& obj, QByteArray& ba);

    V8ScriptValue _proto;
    V8ScriptValue _ctor;

    // JS Object attributes
    V8ScriptString _name;
    V8ScriptString _byteLength;

};

*/
#endif // hifi_ArrayBufferClass_h

/// @}
