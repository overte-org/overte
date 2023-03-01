//
//  ScriptValueIteratorV8Wrapper.cpp
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

#include "ScriptValueIteratorV8Wrapper.h"

V8ScriptValueIterator::V8ScriptValueIterator(ScriptEngineV8* engine, v8::Local<v8::Value> object) : _engine(engine)  {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    // V8TODO Is this necessary?
    _context.Reset(isolate, _engine->getContext());
    auto context = _context.Get(isolate);
    v8::Context::Scope contextScope(context);
    Q_ASSERT(object->IsObject());
    v8::Local<v8::Object> v8Object = v8::Local<v8::Object>::Cast(object);
    _object.Reset(isolate, v8Object);
    _propertyNames.Reset(isolate, v8Object->GetOwnPropertyNames(context).ToLocalChecked());
    _length = _propertyNames.Get(isolate)->Length();
    _currentIndex = -1;
}

V8ScriptValueIterator::~V8ScriptValueIterator() {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    _propertyNames.Reset();
    _object.Reset();
    _context.Reset();
}

bool V8ScriptValueIterator::hasNext() const {
    return _currentIndex < _length - 1;
}

QString V8ScriptValueIterator::name() const {
    Q_ASSERT(_currentIndex >= 0);
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    auto context = _context.Get(isolate);
    v8::Context::Scope contextScope(context);
    v8::Local<v8::Value> propertyName;
    if (!_propertyNames.Get(isolate)->Get(context, _currentIndex).ToLocal(&propertyName)) {
        Q_ASSERT(false);
    }
    return QString(*v8::String::Utf8Value(isolate, propertyName));
}

void V8ScriptValueIterator::next() {
    if (_currentIndex < _length - 1) {
        _currentIndex++;
    }
}

V8ScriptValue V8ScriptValueIterator::value() {
    Q_ASSERT(_currentIndex >= 0);
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    auto context = _context.Get(isolate);
    v8::Context::Scope contextScope(context);
    v8::Local<v8::Value> v8Value;
    v8::Local<v8::Value> propertyName;
    if (!_propertyNames.Get(isolate)->Get(context, _currentIndex).ToLocal(&propertyName)) {
        Q_ASSERT(false);
    }
    if (!_object.Get(isolate)->Get(context, propertyName->ToString(context).ToLocalChecked()).ToLocal(&v8Value)) {
        Q_ASSERT(false);
    }
    return V8ScriptValue(_engine, v8Value);
}

ScriptValue::PropertyFlags ScriptValueIteratorV8Wrapper::flags() const {
    //V8TODO
    //return (ScriptValue::PropertyFlags)(int)_value.flags();
    return ScriptValue::PropertyFlags();
}

bool ScriptValueIteratorV8Wrapper::hasNext() const {
    return _value->hasNext();
}

QString ScriptValueIteratorV8Wrapper::name() const {
    return _value->name();
}

void ScriptValueIteratorV8Wrapper::next() {
    _value->next();
}

ScriptValue ScriptValueIteratorV8Wrapper::value() const {
    V8ScriptValue result = _value->value();
    return ScriptValue(new ScriptValueV8Wrapper(_engine, std::move(result)));
}
