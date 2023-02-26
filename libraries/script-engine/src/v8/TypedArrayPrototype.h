//
//  TypedArrayPrototype.h
//
//
//  Created by Clement on 7/14/14.
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

/// @addtogroup ScriptEngine
/// @{

#ifndef hifi_TypedArrayPrototype_h
#define hifi_TypedArrayPrototype_h

#include <QtCore/QObject>

#include "V8Types.h"
#include "../Scriptable.h"

// V8TODO
/// [V8] The javascript functions associated with a <code><a href="https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/TypedArray">TypedArray</a></code> instance prototype
/*class TypedArrayPrototype : public QObject, public Scriptable {
    Q_OBJECT
public:
    TypedArrayPrototype(QObject* parent = NULL);
    
public slots:
    void set(V8ScriptValue array, qint32 offset = 0);
    V8ScriptValue subarray(qint32 begin);
    V8ScriptValue subarray(qint32 begin, qint32 end);
    
    V8ScriptValue get(quint32 index);
    void set(quint32 index, V8ScriptValue& value);
private:
    QByteArray* thisArrayBuffer() const;
};
*/
#endif // hifi_TypedArrayPrototype_h

/// @}
