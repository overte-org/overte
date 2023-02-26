//
//  DataViewClass.h
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

/// @addtogroup ScriptEngine
/// @{

#ifndef hifi_DataViewClass_h
#define hifi_DataViewClass_h

// V8TODO
/*
#include "ArrayBufferViewClass.h"

/// [V8] Implements the <code><a href="https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/DataView">DataView</a></code> scripting class
class DataViewClass : public ArrayBufferViewClass {
    Q_OBJECT
public:
    DataViewClass(ScriptEngineV8* scriptEngine);
    V8ScriptValue newInstance(V8ScriptValue buffer, quint32 byteOffset, quint32 byteLength);

    QString name() const override;
    V8ScriptValue prototype() const override;

private:
    static V8ScriptValue construct(V8ScriptContext* context, QScriptEngine* engine);

    V8ScriptValue _proto;
    V8ScriptValue _ctor;

    V8ScriptString _name;
};
*/

#endif // hifi_DataViewClass_h

/// @}
