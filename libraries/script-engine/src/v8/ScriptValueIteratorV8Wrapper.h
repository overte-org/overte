//
//  ScriptValueIteratorV8Wrapper.h
//  libraries/script-engine/src/v8
//
//  Created by Heather Anderson on 8/29/21.
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

#ifndef hifi_ScriptValueIteratorV8Wrapper_h
#define hifi_ScriptValueIteratorV8Wrapper_h

#include <QtCore/QPointer>

#include "../ScriptValueIterator.h"
#include "ScriptEngineV8.h"
#include "ScriptValueV8Wrapper.h"

class V8ScriptValueIterator {
public:
    V8ScriptValueIterator(ScriptEngineV8* engine, v8::Local<v8::Value> object);
    ~V8ScriptValueIterator();
    bool hasNext() const;
    QString name() const;
    void next();
    V8ScriptValue value();
private:
    v8::UniquePersistent<v8::Array> _propertyNames;
    v8::UniquePersistent<v8::Object> _object;
    v8::UniquePersistent<v8::Context> _context;
    int _length;
    int _currentIndex;
    ScriptEngineV8 *_engine;
    Q_DISABLE_COPY(V8ScriptValueIterator)
};

/// [V8] Implements ScriptValueIterator for V8 and translates calls for V8ScriptValueIterator
class ScriptValueIteratorV8Wrapper final : public ScriptValueIterator {
public: // construction
    inline ScriptValueIteratorV8Wrapper(ScriptEngineV8* engine, const ScriptValue& object) :
        _engine(engine), _value(new V8ScriptValueIterator(engine, ScriptValueV8Wrapper::fullUnwrap(engine, object).get())) {}
    inline ScriptValueIteratorV8Wrapper(ScriptEngineV8* engine, const V8ScriptValue& object) :
        _engine(engine), _value(new V8ScriptValueIterator(engine, object.constGet())) {}

public:  // ScriptValueIterator implementation
    virtual ScriptValue::PropertyFlags flags() const override;
    virtual bool hasNext() const override;
    virtual QString name() const override;
    virtual void next() override;
    virtual ScriptValue value() const override;

private: // storage
    ScriptEngineV8 *_engine;
    std::shared_ptr<V8ScriptValueIterator> _value;
};

#endif  // hifi_ScriptValueIteratorV8Wrapper_h

/// @}
