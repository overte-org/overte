//
//  V8Types.h
//  libraries/script-engine/src/v8
//
//  Created by dr Karol Suprynowicz on 2022/10/08
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef hifi_V8Types_h
#define hifi_V8Types_h

#include <memory>

#include <libplatform/libplatform.h>
#include <v8.h>

#include "ScriptEngineV8.h"

template <typename T>
class V8ScriptValueTemplate {
public:
    V8ScriptValueTemplate() = delete;

    V8ScriptValueTemplate(ScriptEngineV8 *engine, const v8::Local<T> value/*, V8ScriptValueTemplate::Ownership ownership*/) : _engine(engine) {
        v8::Locker locker(_engine->getIsolate());
        v8::Isolate::Scope isolateScope(_engine->getIsolate());
        v8::HandleScope handleScope(_engine->getIsolate());
        v8::Context::Scope(_engine->getContext());
#ifdef OVERTE_V8_MEMORY_DEBUG
        _engine->incrementScriptValueCounter();
#endif
        _value.reset(new v8::UniquePersistent<T>(_engine->getIsolate(), value));
    };

    V8ScriptValueTemplate& operator= (const V8ScriptValueTemplate &source) {
        v8::Locker locker(_engine->getIsolate());
        v8::Isolate::Scope isolateScope(_engine->getIsolate());
        v8::HandleScope handleScope(_engine->getIsolate());
        v8::Context::Scope(_engine->getContext());
        _engine = source.getEngine();
        _value.reset(new v8::UniquePersistent<T>(_engine->getIsolate(), source.constGet()));
        return *this;
    };

    V8ScriptValueTemplate(ScriptEngineV8 *engine) : _engine(engine) {
        v8::Locker locker(_engine->getIsolate());
        v8::Isolate::Scope isolateScope(_engine->getIsolate());
        v8::HandleScope handleScope(_engine->getIsolate());
        v8::Context::Scope(_engine->getContext());
#ifdef OVERTE_V8_MEMORY_DEBUG
        _engine->incrementScriptValueCounter();
#endif
        _value.reset(new v8::UniquePersistent<T>(_engine->getIsolate(), v8::Local<T>()));
    };

    V8ScriptValueTemplate(const V8ScriptValueTemplate &copied) : _engine(copied.getEngine()) {
        v8::Locker locker(_engine->getIsolate());
        v8::Isolate::Scope isolateScope(_engine->getIsolate());
        v8::HandleScope handleScope(_engine->getIsolate());
        v8::Context::Scope(_engine->getContext());
#ifdef OVERTE_V8_MEMORY_DEBUG
        _engine->incrementScriptValueCounter();
#endif
        _value.reset(new v8::UniquePersistent<T>(_engine->getIsolate(), copied.constGet()));
    }

    v8::Local<T> get() {
        Q_ASSERT(_engine->getIsolate()->IsCurrent());
        v8::EscapableHandleScope handleScope(_engine->getIsolate());
        return handleScope.Escape(_value.get()->Get(_engine->getIsolate()));
    };

    const v8::Local<T> constGet() const {
        Q_ASSERT(_engine->getIsolate()->IsCurrent());
        v8::EscapableHandleScope handleScope(_engine->getIsolate());
        return handleScope.Escape(_value.get()->Get(_engine->getIsolate()));
    };

    const v8::Local<v8::Context> constGetContext() const {
        Q_ASSERT(_engine->getIsolate()->IsCurrent());
        v8::EscapableHandleScope handleScope(_engine->getIsolate());
        return handleScope.Escape(_engine->getIsolate()->GetCurrentContext());
    };

    const v8::Isolate* constGetIsolate() const { return _engine->getIsolate(); };
    v8::Isolate* getIsolate() { return _engine->getIsolate();};

    ScriptEngineV8* getEngine() const { return _engine; };

    v8::Local<v8::Context> getContext() {
        Q_ASSERT(_engine->getIsolate()->IsCurrent());
        v8::EscapableHandleScope handleScope(_engine->getIsolate());
        return handleScope.Escape(_engine->getIsolate()->GetCurrentContext());
    };

    void reset(v8::Isolate *isolate, v8::Local<T>) {
        Q_ASSERT(false);
    };
    // V8TODO: add thread safe destructors to all objects that have persistent handles
    ~V8ScriptValueTemplate() {
        v8::Locker locker(_engine->getIsolate());
        v8::Isolate::Scope isolateScope(_engine->getIsolate());
        v8::HandleScope handleScope(_engine->getIsolate());
        //v8::Context::Scope(_engine->getContext());
#ifdef OVERTE_V8_MEMORY_DEBUG
        _engine->decrementScriptValueCounter();
#endif
        _value->Reset();
    }

private:
    std::shared_ptr<v8::UniquePersistent<T>> _value;
    ScriptEngineV8 *_engine;
};

class V8ScriptString : public V8ScriptValueTemplate<v8::String> {
public:
    V8ScriptString() = delete;
    V8ScriptString(ScriptEngineV8 *engine, const v8::Local<v8::String> value) : V8ScriptValueTemplate<v8::String>(engine, value) {};
    const QString toQString() const {
        v8::Locker locker(getEngine()->getIsolate());
        v8::Isolate::Scope isolateScope(getEngine()->getIsolate());
        v8::HandleScope handleScope(getEngine()->getIsolate());
        v8::Context::Scope contextScope(getEngine()->getContext());
        Q_ASSERT(constGet()->IsString());
        return QString(*v8::String::Utf8Value(const_cast<v8::Isolate*>(constGetIsolate()), constGet()));
    };
    bool operator==(const V8ScriptString& string) const {
        v8::Locker locker(getEngine()->getIsolate());
        v8::Isolate::Scope isolateScope(getEngine()->getIsolate());
        v8::HandleScope handleScope(getEngine()->getIsolate());
        v8::Context::Scope contextScope(getEngine()->getContext());
        Q_ASSERT(constGet()->IsString());
        return constGet()->StringEquals(string.constGet());
    }
};

inline uint qHash(const V8ScriptString &key, uint seed = 0) {
    return qHash(key.toQString(), seed);
};

#endif
