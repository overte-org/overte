//
//  V8Types.h
//  libraries/script-engine/src/v8
//
//  Created by dr Karol Suprynowicz on 2022/10/08
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
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
    //V8ScriptValueTemplate(v8::Isolate *isolate, v8::Local<T> value) : _isolate(isolate) {
        //_value.reset(v8::UniquePersistent<T>::New(_isolate, value));
    //};
    V8ScriptValueTemplate(ScriptEngineV8 *engine, const v8::Local<T> value) : _engine(engine) {
        v8::Locker locker(_engine->getIsolate());
        v8::Isolate::Scope isolateScope(_engine->getIsolate());
        v8::HandleScope handleScope(_engine->getIsolate());
        v8::Context::Scope(_engine->getContext());
        _value.reset(new v8::UniquePersistent<T>(_engine->getIsolate(), std::move(value)));
    };

    V8ScriptValueTemplate(const V8ScriptValueTemplate &copied) : _engine(copied.getEngine()) {
        v8::Locker locker(_engine->getIsolate());
        v8::Isolate::Scope isolateScope(_engine->getIsolate());
        v8::HandleScope handleScope(_engine->getIsolate());
        v8::Context::Scope(_engine->getContext());
        _value.reset(new v8::UniquePersistent<T>(_engine->getIsolate(), std::move(copied.constGet())));
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

    /*V8ScriptValueTemplate<T>&& copy() const {
        Q_ASSERT(_engine->getIsolate()->IsCurrent());
        v8::HandleScope handleScope(_engine->getIsolate());
        return new V8ScriptValueTemplate(_engine->getIsolate(), v8::Local<T>::New(_engine->getIsolate(), constGet()));
    };*/

    const v8::Local<v8::Context> constGetContext() const {
        Q_ASSERT(_engine->getIsolate()->IsCurrent());
        v8::EscapableHandleScope handleScope(_engine->getIsolate());
        return handleScope.Escape(_engine->getIsolate()->GetCurrentContext());
        //return handleScope.Escape(_context.Get(_isolate));
    };

    const v8::Isolate* constGetIsolate() const { return _engine->getIsolate(); };
    v8::Isolate* getIsolate() { return _engine->getIsolate();};

    //v8::Persistent<v8::Context, v8::CopyablePersistentTraits<v8::Context>>& getContext() { return _context;};

    ScriptEngineV8* getEngine() const { return _engine; };

    v8::Local<v8::Context> getContext() {
        Q_ASSERT(_engine->getIsolate()->IsCurrent());
        v8::EscapableHandleScope handleScope(_engine->getIsolate());
        return handleScope.Escape(_engine->getIsolate()->GetCurrentContext());
        //return handleScope.Escape(_context.Get(_isolate));
    };

    void reset(v8::Isolate *isolate, v8::Local<T>) {
        Q_ASSERT(false);
    };
private:
    std::shared_ptr<v8::UniquePersistent<T>> _value;
    // V8TODO: maybe make it weak
    // does it need , CopyablePersistentTraits<Value>?
    // V8TODO: is context needed at all?
    //v8::Isolate *_isolate;
    ScriptEngineV8 *_engine;
    //v8::Persistent<v8::Context, v8::CopyablePersistentTraits<v8::Context>> _context;
};

//typedef V8ScriptValueTemplate<v8::Value> V8ScriptValue;
//typedef V8ScriptValueTemplate<v8::Script> V8ScriptProgram;
// V8TODO: Maybe weak?
typedef v8::Persistent<v8::Context> V8ScriptContext;

class V8ScriptString : public V8ScriptValueTemplate<v8::String> {
public:
    V8ScriptString() = delete;
    V8ScriptString(ScriptEngineV8 *engine, const v8::Local<v8::String> value) : V8ScriptValueTemplate<v8::String>(engine, value) {};
    const QString toQString() const {
        Q_ASSERT(constGetIsolate()->IsCurrent());
        Q_ASSERT(constGet()->IsString());
        return QString(*v8::String::Utf8Value(const_cast<v8::Isolate*>(constGetIsolate()), constGet()));
    };
    bool operator==(const V8ScriptString& string) const {
        Q_ASSERT(constGetIsolate()->IsCurrent());
        Q_ASSERT(constGet()->IsString());
        return constGet()->StringEquals(string.constGet());
    }
};

inline uint qHash(const V8ScriptString &key, uint seed = 0) {
    return qHash(key.toQString(), seed);
};

#endif