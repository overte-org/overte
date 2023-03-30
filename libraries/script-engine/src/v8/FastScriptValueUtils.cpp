//
//  FastScriptValueUtils.cpp
//  libraries/script-engine/src/v8/FastScriptValueUtils.cpp
//
//  Created by dr Karol Suprynowicz on 2023/03/30.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "FastScriptValueUtils.h"

#include <qcolor.h>

#include "../ScriptEngine.h"
#include "V8Types.h"
#include "ScriptValueV8Wrapper.h"

ScriptValue vec3ToScriptValueFast(ScriptEngine* engine, const glm::vec3& vec3) {
    auto prototype = engine->globalObject().property("__hifi_vec3__");
    if (!prototype.hasProperty("defined") || !prototype.property("defined").toBool()) {
        prototype = engine->evaluate(
            "__hifi_vec3__ = Object.defineProperties({}, { "
            "defined: { value: true },"
            "0: { set: function(nv) { return this.x = nv; }, get: function() { return this.x; } },"
            "1: { set: function(nv) { return this.y = nv; }, get: function() { return this.y; } },"
            "2: { set: function(nv) { return this.z = nv; }, get: function() { return this.z; } },"
            "r: { set: function(nv) { return this.x = nv; }, get: function() { return this.x; } },"
            "g: { set: function(nv) { return this.y = nv; }, get: function() { return this.y; } },"
            "b: { set: function(nv) { return this.z = nv; }, get: function() { return this.z; } },"
            "red: { set: function(nv) { return this.x = nv; }, get: function() { return this.x; } },"
            "green: { set: function(nv) { return this.y = nv; }, get: function() { return this.y; } },"
            "blue: { set: function(nv) { return this.z = nv; }, get: function() { return this.z; } }"
            "})");
    }
    ScriptValue value = engine->newObject();

    ScriptValueV8Wrapper *proxy = ScriptValueV8Wrapper::unwrap(value);

    auto engineV8 = proxy->getV8Engine();

    auto isolate = engineV8->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    auto context = engineV8->getContext();
    v8::Context::Scope contextScope(engineV8->getContext());
    V8ScriptValue v8ScriptValue = proxy->toV8Value();
    v8::Local<v8::Object> v8Object = v8::Local<v8::Object>::Cast(v8ScriptValue.get());

    if (!v8Object->Set(context, v8::String::NewFromUtf8(isolate, "x").ToLocalChecked(), v8::Number::New(isolate, vec3.x)).FromMaybe(false)) {
        Q_ASSERT(false);
    }
    if (!v8Object->Set(context, v8::String::NewFromUtf8(isolate, "y").ToLocalChecked(), v8::Number::New(isolate, vec3.y)).FromMaybe(false)) {
        Q_ASSERT(false);
    }
    if (!v8Object->Set(context, v8::String::NewFromUtf8(isolate, "z").ToLocalChecked(), v8::Number::New(isolate, vec3.z)).FromMaybe(false)) {
        Q_ASSERT(false);
    }

    auto v8Prototype = ScriptValueV8Wrapper::fullUnwrap(engineV8, prototype);
    if (!v8Object->SetPrototype(context, v8Prototype.get()).FromMaybe(false)) {
        Q_ASSERT(false);
    }
    value.setPrototype(prototype);
    return value;
}

bool vec3FromScriptValueFast(const ScriptValue& object, glm::vec3& vec3) {
    ScriptValueV8Wrapper *proxy = ScriptValueV8Wrapper::unwrap(object);

    auto engineV8 = proxy->getV8Engine();

    auto isolate = engineV8->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    auto context = engineV8->getContext();
    v8::Context::Scope contextScope(engineV8->getContext());
    V8ScriptValue v8ScriptValue = proxy->toV8Value();

    v8::Local<v8::Value> v8Value = v8ScriptValue.get();


    if (v8Value->IsNumber()) {
        vec3 = glm::vec3(v8Value->NumberValue(context).ToChecked());
    } else if (v8Value->IsString()) {
        QColor qColor(QString(*v8::String::Utf8Value(isolate, v8::Local<v8::String>::Cast(v8Value))));
        if (qColor.isValid()) {
            vec3.x = qColor.red();
            vec3.y = qColor.green();
            vec3.z = qColor.blue();
        } else {
            return false;
        }
    } else if (v8Value->IsArray()) {
        auto array = v8::Local<v8::Array>::Cast(v8Value);
        //QVariantList list = object.toVariant().toList();
        if (array->Length() == 3) {
            v8::Local<v8::Value> xValue,yValue,zValue;
            if (!array->Get(context, 0).ToLocal(&xValue)) {
                return false;
            }
            if (!array->Get(context, 1).ToLocal(&yValue)) {
                return false;
            }
            if (!array->Get(context, 2).ToLocal(&zValue)) {
                return false;
            }
            //V8TODO:
            if (xValue->IsNullOrUndefined() || yValue->IsNullOrUndefined() || zValue->IsNullOrUndefined()) {
                return false;
            }
            double x,y,z;
            if (!xValue->NumberValue(context).To(&x)
                || !yValue->NumberValue(context).To(&y)
                || !zValue->NumberValue(context).To(&z)) {
                return false;
            }

            vec3.x = xValue->NumberValue(context).FromMaybe(0.0);
            vec3.y = yValue->NumberValue(context).FromMaybe(0.0);
            vec3.z = zValue->NumberValue(context).FromMaybe(0.0);
        } else {
            return false;
        }
    } else if (v8Value->IsObject()) {
        v8::Local<v8::Object> v8Object = v8::Local<v8::Object>::Cast(v8Value);
        v8::Local<v8::Value> xValue;
        if (!v8Object->Get(context, v8::String::NewFromUtf8(isolate, "x").ToLocalChecked()).ToLocal(&xValue)) {
            Q_ASSERT(false);
        }
        if (xValue->IsNullOrUndefined()) {
            if (!v8Object->Get(context, v8::String::NewFromUtf8(isolate, "r").ToLocalChecked()).ToLocal(&xValue)) {
                Q_ASSERT(false);
            }
        }
        if (xValue->IsNullOrUndefined()) {
            if (!v8Object->Get(context, v8::String::NewFromUtf8(isolate, "red").ToLocalChecked()).ToLocal(&xValue)) {
                Q_ASSERT(false);
            }
        }

        v8::Local<v8::Value> yValue;
        if (!v8Object->Get(context, v8::String::NewFromUtf8(isolate, "y").ToLocalChecked()).ToLocal(&yValue)) {
            Q_ASSERT(false);
        }
        if (yValue->IsNullOrUndefined()) {
            if (!v8Object->Get(context, v8::String::NewFromUtf8(isolate, "g").ToLocalChecked()).ToLocal(&yValue)) {
                Q_ASSERT(false);
            }
        }
        if (yValue->IsNullOrUndefined()) {
            if (!v8Object->Get(context, v8::String::NewFromUtf8(isolate, "green").ToLocalChecked()).ToLocal(&yValue)) {
                Q_ASSERT(false);
            }
        }

        v8::Local<v8::Value> zValue;
        if (!v8Object->Get(context, v8::String::NewFromUtf8(isolate, "z").ToLocalChecked()).ToLocal(&zValue)) {
            Q_ASSERT(false);
        }
        if (zValue->IsNullOrUndefined()) {
            if (!v8Object->Get(context, v8::String::NewFromUtf8(isolate, "b").ToLocalChecked()).ToLocal(&zValue)) {
                Q_ASSERT(false);
            }
        }
        if (zValue->IsNullOrUndefined()) {
            if (!v8Object->Get(context, v8::String::NewFromUtf8(isolate, "blue").ToLocalChecked()).ToLocal(&zValue)) {
                Q_ASSERT(false);
            }
        }

        //f (!x.isValid() || !y.isValid() || !z.isValid()) {
            // V8TODO: This breaks the sit script for some reason
            //return false;
        //}

        vec3.x = xValue->NumberValue(context).FromMaybe(0.0);
        vec3.y = yValue->NumberValue(context).FromMaybe(0.0);
        vec3.z = zValue->NumberValue(context).FromMaybe(0.0);
    } else {
        return false;
    }
    return true;
}
