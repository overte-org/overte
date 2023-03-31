//
//  ScriptValueV8Wrapper.cpp
//  libraries/script-engine/src/v8
//
//  Created by Heather Anderson on 5/16/21.
//  Modified for V8 by dr Karol Suprynowicz on 2022/10/08
//  Copyright 2021 Vircadia contributors.
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "ScriptValueV8Wrapper.h"

#include "ScriptValueIteratorV8Wrapper.h"

#include "../ScriptEngineLogging.h"
#include "ScriptEngineLoggingV8.h"

void ScriptValueV8Wrapper::release() {
    // V8TODO: maybe add an assert to check if it happens on script engine thread?
    // With v8::Locker in V8ScriptValue such requirement shouldn't be necessary but deleting on different threadwww can cause deadlocks sometimes
    delete this;
}

ScriptValueProxy* ScriptValueV8Wrapper::copy() const {
    //V8TODO: check if the value needs to be copied or just wrapper
    v8::Isolate *isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    // V8TODO: I'm not sure if this part is right:
    v8::Context::Scope contextScope(_engine->getContext());
    ScriptValueV8Wrapper *copiedWrapper = new ScriptValueV8Wrapper(_engine, _value);
    return copiedWrapper;
}

ScriptValueV8Wrapper* ScriptValueV8Wrapper::unwrap(const ScriptValue& val) {
    return dynamic_cast<ScriptValueV8Wrapper*>(val.ptr());
}

V8ScriptValue ScriptValueV8Wrapper::fullUnwrap(const ScriptValue& value) const {
    ScriptValueV8Wrapper* unwrapped = unwrap(value);
    if (unwrapped) {
        if (unwrapped->engine().get() != _engine) {
            //return _engine->toScriptValue(unwrapped->toVariant());
            return _engine->castVariantToValue(unwrapped->toVariant());
        } else {
            return unwrapped->toV8Value();
        }
    }
    QVariant varValue = value.toVariant();
    return _engine->castVariantToValue(varValue);
}

V8ScriptValue ScriptValueV8Wrapper::fullUnwrap(ScriptEngineV8* engine, const ScriptValue& value) {
    ScriptValueV8Wrapper* unwrapped = unwrap(value);
    if (unwrapped) {
        if (unwrapped->engine().get() != engine) {
            //return static_cast<QScriptEngine*>(engine)->toScriptValue(unwrapped->toVariant());
            return engine->castVariantToValue(unwrapped->toVariant());
        } else {
            return unwrapped->toV8Value();
        }
    }
    QVariant varValue = value.toVariant();
    return engine->castVariantToValue(varValue);
}

ScriptValue ScriptValueV8Wrapper::call(const ScriptValue& thisObject, const ScriptValueList& args) {
    Q_ASSERT(_engine == _value.getEngine());
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    auto context = _engine->getContext();
    v8::Context::Scope contextScope(context);
    V8ScriptValue v8This = fullUnwrap(thisObject);
    //V8ScriptValueList qArgs;
    Q_ASSERT(args.length() <= Q_METAMETHOD_INVOKE_MAX_ARGS);
    //V8TODO I'm not sure how else to do this since v8::Local should probably be on stack, not heap
    v8::Local<v8::Value> v8Args[Q_METAMETHOD_INVOKE_MAX_ARGS];
    int argIndex = 0;
    for (ScriptValueList::const_iterator iter = args.begin(); iter != args.end(); ++iter) {
        v8Args[argIndex++] = fullUnwrap(*iter).get();
    }
    Q_ASSERT(_value.get()->IsFunction());
    v8::Local<v8::Function> v8Function = v8::Local<v8::Function>::Cast(_value.get());
    v8::TryCatch tryCatch(isolate);
    v8::Local<v8::Value> recv;
    if (v8This.get()->IsObject()) {
        recv = v8This.get();
        //qCDebug(scriptengine_v8) << "V8 This: " << _engine->scriptValueDebugDetailsV8(v8This);
    }else{
        recv = _engine->getContext()->Global();
        //recv = v8::Null(isolate);
        //qCDebug(scriptengine_v8) << "global";
    }
    //qCDebug(scriptengine_v8) << "V8 Call: " << *v8::String::Utf8Value(isolate, v8This.get()->TypeOf(isolate));
    auto maybeResult = v8Function->Call(_engine->getContext(), recv, args.length(), v8Args);
    // V8TODO: Exceptions don't seem to actually be caught here?
    if (tryCatch.HasCaught()) {
        qCDebug(scriptengine_v8) << "Function call failed: \"" << _engine->formatErrorMessageFromTryCatch(tryCatch);
    }
    v8::Local<v8::Value> result;
    Q_ASSERT(_engine == _value.getEngine());
    if (maybeResult.ToLocal(&result)) {
        return ScriptValue(new ScriptValueV8Wrapper(_engine, V8ScriptValue(_engine, result)));
    } else {
        //V8TODO Add more details
        qCWarning(scriptengine_v8) << "JS function call failed";
        return _engine->undefinedValue();
    }
}

ScriptValue ScriptValueV8Wrapper::call(const ScriptValue& thisObject, const ScriptValue& arguments) {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    V8ScriptValue v8This = fullUnwrap(thisObject);
    V8ScriptValue v8Args = fullUnwrap(arguments);
    // V8TODO should there be a v8 try-catch here?
    // IsFunction check should be here probably
    // V8TODO I'm not sure in what format arguments are yet, backtrace will show how it is used
    Q_ASSERT(false);
    return _engine->undefinedValue();
    /*v8::Local<v8::Function> v8Function = v8::Local<v8::Function>::Cast(_value.get());
    auto maybeResult = v8Function->Call(_engine->getContext(), v8This, v8Args);
    v8::Local<v8::Value> result;
    if (maybeResult.ToLocal(&result)) {
        return ScriptValue(new ScriptValueV8Wrapper(_engine, V8ScriptValue(_engine->getContext(), result)));
    } else {
        //V8TODO Add more details
        qCWarning(scriptengine_v8) << "JS function call failed";
        return _engine->undefinedValue();
    }*/
}

ScriptValue ScriptValueV8Wrapper::construct(const ScriptValueList& args) {
    //V8TODO: there is CallAsContructor in V8
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    Q_ASSERT(args.length() <= Q_METAMETHOD_INVOKE_MAX_ARGS);
    //V8TODO I'm not sure how else to do this since v8::Local should probably be on stack, not heap
    v8::Local<v8::Value> v8Args[Q_METAMETHOD_INVOKE_MAX_ARGS];
    int argIndex = 0;
    for (ScriptValueList::const_iterator iter = args.begin(); iter != args.end(); ++iter) {
        v8Args[argIndex++] = fullUnwrap(*iter).get();
    }
    //V8TODO should there be a v8 try-catch here?
    Q_ASSERT(_value.get()->IsFunction());
    v8::Local<v8::Function> v8Function = v8::Local<v8::Function>::Cast(_value.get());
    auto maybeResult = v8Function->NewInstance(_engine->getContext(), args.length(), v8Args);
    v8::Local<v8::Object> result;
    if (maybeResult.ToLocal(&result)) {
        return ScriptValue(new ScriptValueV8Wrapper(_engine, V8ScriptValue(_engine, result)));
    } else {
        //V8TODO Add more details
        qCWarning(scriptengine_v8) << "JS function call failed";
        return _engine->undefinedValue();
    }
}

ScriptValue ScriptValueV8Wrapper::construct(const ScriptValue& arguments) {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    // V8TODO I'm not sure in what format arguments are yet, backtrace will show how it is used
    Q_ASSERT(false);
    return _engine->undefinedValue();
    //V8ScriptValue unwrapped = fullUnwrap(arguments);
    //V8ScriptValue result = _value.construct(unwrapped);
    //return ScriptValue(new ScriptValueV8Wrapper(_engine, std::move(result)));
}

// V8TODO: check how data() is used and if it needs fixing
ScriptValue ScriptValueV8Wrapper::data() const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    // Private properties are an experimental feature for now on V8, so we are using regular value for now
    if (_value.constGet()->IsObject()) {
        auto v8Object = v8::Local<v8::Object>::Cast(_value.constGet());
         v8::Local<v8::Value> data;
         //bool createData = false;
         if (!v8Object->Get(_engine->getContext(), v8::String::NewFromUtf8(isolate, "__data").ToLocalChecked()).ToLocal(&data)) {
             data = v8::Undefined(isolate);
             Q_ASSERT(false);
             //createData = true;
         }
         /*else {
             if (data->IsUndefined()) {
                 createData = true;
             }
         }
         if (createData) {
             qCDebug(scriptengine_v8) << "ScriptValueV8Wrapper::data(): Data object doesn't exist, creating new one";
             // Create data object if it's non-existent or invalid
             data = v8::Object::New(isolate);
             if( !v8Object->Set(_engine->getContext(), v8::String::NewFromUtf8(isolate, "__data").ToLocalChecked(), data).FromMaybe(false)) {
                 qCDebug(scriptengine_v8) << "ScriptValueV8Wrapper::data(): Data object couldn't be created";
                 Q_ASSERT(false);
             }
         }*/
         V8ScriptValue result(_engine, data);
         return ScriptValue(new ScriptValueV8Wrapper(_engine, std::move(result)));
    } else {
        qCDebug(scriptengine_v8) << "ScriptValueV8Wrapper::data() was called on a value that is not an object";
        Q_ASSERT(false);
    }
    //V8TODO I'm not sure how this would work in V8
    //V8ScriptValue result = _value.data();
    //return ScriptValue(new ScriptValueV8Wrapper(_engine, std::move(result)));
    return _engine->nullValue();
}

ScriptEnginePointer ScriptValueV8Wrapper::engine() const {
    if (!_engine) {
        return ScriptEnginePointer();
    }
    return _engine->shared_from_this();
}

ScriptValueIteratorPointer ScriptValueV8Wrapper::newIterator() const {
    v8::Locker locker(_engine->getIsolate());
    v8::Isolate::Scope isolateScope(_engine->getIsolate());
    v8::HandleScope handleScope(_engine->getIsolate());
    v8::Context::Scope contextScope(_engine->getContext());
    ScriptValueIteratorPointer iterator = std::make_shared<ScriptValueIteratorV8Wrapper>(_engine, _value);
    return iterator;
}

bool ScriptValueV8Wrapper::hasProperty(const QString& name) const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    //V8TODO: does function return true on IsObject too?
    if (_value.constGet()->IsObject()) {
    //V8TODO: what about flags?
        v8::Local<v8::Value> resultLocal;
        v8::Local<v8::String> key = v8::String::NewFromUtf8(isolate, name.toStdString().c_str(),v8::NewStringType::kNormal).ToLocalChecked();
        const v8::Local<v8::Object> object = v8::Local<v8::Object>::Cast(_value.constGet());
        //V8TODO: Which context?
        if (object->Get(_engine->getContext(), key).ToLocal(&resultLocal)) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}



ScriptValue ScriptValueV8Wrapper::property(const QString& name, const ScriptValue::ResolveFlags &mode) const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(_engine->getIsolate());
    v8::Isolate::Scope isolateScope(_engine->getIsolate());
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    if (_value.constGet()->IsNullOrUndefined()) {
        return _engine->undefinedValue();
    }
    if (_value.constGet()->IsObject()) {
    //V8TODO: what about flags?
        v8::Local<v8::Value> resultLocal;
        v8::Local<v8::String> key = v8::String::NewFromUtf8(_engine->getIsolate(), name.toStdString().c_str(),v8::NewStringType::kNormal).ToLocalChecked();
        const v8::Local<v8::Object> object = v8::Local<v8::Object>::Cast(_value.constGet());
        //V8TODO: Which context?
        if (object->Get(_engine->getContext(), key).ToLocal(&resultLocal)) {
        //if (object->Get(_value.constGetContext(), key).ToLocal(&resultLocal)) {
            V8ScriptValue result(_engine, resultLocal);
            return ScriptValue(new ScriptValueV8Wrapper(_engine, std::move(result)));
        } else {
            QString parentValueQString("");
            v8::Local<v8::String> parentValueString;
            if (_value.constGet()->ToDetailString(_engine->getContext()).ToLocal(&parentValueString)) {
                QString(*v8::String::Utf8Value(isolate, parentValueString));
            }
            qCDebug(scriptengine_v8) << "Failed to get property, parent of value: " << name << ", parent type: " << QString(*v8::String::Utf8Value(isolate, _value.constGet()->TypeOf(isolate))) << " parent value: " << parentValueQString;
        }
    }
    if (name == QString("x")) {
        printf("x");
    }
    //This displays too many messages during correct operation, but is useful for debugging
    //qCDebug(scriptengine_v8) << "Failed to get property, parent of value: " << name << " is not a V8 object, reported type: " << QString(*v8::String::Utf8Value(isolate, _value.constGet()->TypeOf(isolate)));
    //qCDebug(scriptengine_v8) << "Backtrace: " << _engine->currentContext()->backtrace();
    return _engine->undefinedValue();
    /*v8::Local<v8::Value> nullValue = v8::Null(_engine->getIsolate());
    V8ScriptValue nullScriptValue(_engine->getIsolate(), std::move(nullValue));
    return ScriptValue(new ScriptValueV8Wrapper(_engine, nullScriptValue));*/
    //V8ScriptValue result = _value.property(name, (V8ScriptValue::ResolveFlags)(int)mode);
    //return ScriptValue(new ScriptValueV8Wrapper(_engine, std::move(result)));
}

ScriptValue ScriptValueV8Wrapper::property(quint32 arrayIndex, const ScriptValue::ResolveFlags& mode) const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    if (_value.constGet()->IsNullOrUndefined()) {
        qCDebug(scriptengine_v8) << "Failed to get property, parent of value: " << arrayIndex << " is not a V8 object, reported type: " << QString(*v8::String::Utf8Value(isolate, _value.constGet()->TypeOf(isolate)));
        return _engine->undefinedValue();
    }
    if (_value.constGet()->IsObject()) {
    //V8TODO: what about flags?
        v8::Local<v8::Value> resultLocal;
        const v8::Local<v8::Object> object = v8::Local<v8::Object>::Cast(_value.constGet());
        if (object->Get(_value.constGetContext(), arrayIndex).ToLocal(&resultLocal)) {
            V8ScriptValue result(_engine, resultLocal);
            return ScriptValue(new ScriptValueV8Wrapper(_engine, std::move(result)));
        }
    }
    qCDebug(scriptengine_v8) << "Failed to get property, parent of value: " << arrayIndex << " is not a V8 object, reported type: " << QString(*v8::String::Utf8Value(isolate, _value.constGet()->TypeOf(isolate)));
    return _engine->undefinedValue();
}

void ScriptValueV8Wrapper::setData(const ScriptValue& value) {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    V8ScriptValue unwrapped = fullUnwrap(value);
    // V8TODO Check if it uses same isolate. Would pointer check be enough?
    // Private properties are an experimental feature for now on V8, so we are using regular value for now
    if (_value.constGet()->IsNullOrUndefined()) {
        qCDebug(scriptengine_v8) << "ScriptValueV8Wrapper::setData() was called on a value that is null or undefined";
        return;
    }
    if (_value.constGet()->IsObject()) {
        auto v8Object = v8::Local<v8::Object>::Cast(_value.constGet());
        if( !v8Object->Set(_engine->getContext(), v8::String::NewFromUtf8(isolate, "__data").ToLocalChecked(), unwrapped.constGet()).FromMaybe(false)) {
            qCDebug(scriptengine_v8) << "ScriptValueV8Wrapper::data(): Data object couldn't be created";
            Q_ASSERT(false);
        }
    } else {
        qCDebug(scriptengine_v8) << "ScriptValueV8Wrapper::data() was called on a value that is not an object";
        Q_ASSERT(false);
    }
}

void ScriptValueV8Wrapper::setProperty(const QString& name, const ScriptValue& value, const ScriptValue::PropertyFlags& flags) {
    Q_ASSERT(flags != ScriptValue::PropertyGetter || flags != ScriptValue::PropertySetter);
    auto isolate = _engine->getIsolate();
    v8::Locker locker(_engine->getIsolate());
    v8::Isolate::Scope isolateScope(_engine->getIsolate());
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    V8ScriptValue unwrapped = fullUnwrap(value);
    if (_value.constGet()->IsNullOrUndefined()) {
        qCDebug(scriptengine_v8) << "ScriptValueV8Wrapper::setProperty() was called on a value that is null or undefined";
        return;
    }
    if(_value.constGet()->IsObject()) {
        v8::Local<v8::String> key = v8::String::NewFromUtf8(isolate, name.toStdString().c_str(),v8::NewStringType::kNormal).ToLocalChecked();
        Q_ASSERT(_value.get()->IsObject());
        auto object = v8::Local<v8::Object>::Cast(_value.get());
        // V8TODO: What if value context and current context is different?
        //v8::Maybe<bool> retVal = object->Set(_value.getContext(), key, unwrapped.constGet());
        //v8::Maybe<bool> retVal = object->Set(_engine->getContext(), key, unwrapped.constGet());
        v8::Maybe<bool> retVal = object->Set(isolate->GetCurrentContext(), key, unwrapped.constGet());
        if (retVal.IsJust() ? !retVal.FromJust() : true){
            qCDebug(scriptengine_v8) << "Failed to set property";
        }
    } else {
        v8::Local<v8::String> details;
        QString detailsString("");
        if(_value.get()->ToDetailString(_engine->getContext()).ToLocal(&details)) {
            v8::String::Utf8Value utf8Value(isolate,details);
            detailsString = *utf8Value;
        }
        qCDebug(scriptengine_v8) << "Failed to set property:" + name + " - parent is not an object. Parent details: " + " Type: " + QString(*v8::String::Utf8Value(isolate, _value.constGet()->TypeOf(isolate)));
        qCDebug(scriptengine_v8) << _engine->currentContext()->backtrace();
    }
    //V8TODO: what about flags?
    //_value.setProperty(name, unwrapped, (V8ScriptValue::PropertyFlags)(int)flags);
}

void ScriptValueV8Wrapper::setProperty(quint32 arrayIndex, const ScriptValue& value, const ScriptValue::PropertyFlags& flags) {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    V8ScriptValue unwrapped = fullUnwrap(value);
    if (_value.constGet()->IsNullOrUndefined()) {
        qCDebug(scriptengine_v8) << "ScriptValueV8Wrapper::setProperty() was called on a value that is null or undefined";
        return;
    }
    if(_value.constGet()->IsObject()) {
        auto object = v8::Local<v8::Object>::Cast(_value.get());
        //V8TODO: I don't know which context to use here
        v8::Maybe<bool> retVal(object->Set(_engine->getContext(), arrayIndex, unwrapped.constGet()));
        //v8::Maybe<bool> retVal(object->Set(_value.getContext(), arrayIndex, unwrapped.constGet()));
        if (retVal.IsJust() ? !retVal.FromJust() : true){
            qCDebug(scriptengine_v8) << "Failed to set property";
        }
    } else {
        qCDebug(scriptengine_v8) << "Failed to set property: " + QString(arrayIndex) + " - parent is not an object";
    }
    //V8TODO: what about flags?
    //_value.setProperty(arrayIndex, unwrapped, (V8ScriptValue::PropertyFlags)(int)flags);
}

void ScriptValueV8Wrapper::setPrototype(const ScriptValue& prototype) {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    auto context = _engine->getContext();
    v8::Context::Scope contextScope(context);
    ScriptValueV8Wrapper* unwrappedPrototype = unwrap(prototype);
    if (unwrappedPrototype) {
        if(unwrappedPrototype->toV8Value().constGet()->IsNullOrUndefined() && _value.constGet()->IsNullOrUndefined()) {
            qCDebug(scriptengine_v8) << "Failed to assign prototype - one of values is null or undefined";
        }
        if(unwrappedPrototype->toV8Value().constGet()->IsObject() && _value.constGet()->IsObject()) {
            auto object = v8::Local<v8::Object>::Cast(_value.get());
            //V8TODO: I don't know which context to use here
            v8::Maybe<bool> retVal = object->SetPrototype(context, unwrappedPrototype->toV8Value().constGet());
            //v8::Maybe<bool> retVal = object->SetPrototype(_value.getContext(), unwrappedPrototype->toV8Value().constGet());
            if (retVal.IsJust() ? !retVal.FromJust() : true){
                qCDebug(scriptengine_v8) << "Failed to assign prototype";
            }
        } else {
            qCDebug(scriptengine_v8) << "Failed to assign prototype - one of values is not an object";
        }
    }
}

bool ScriptValueV8Wrapper::strictlyEquals(const ScriptValue& other) const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    ScriptValueV8Wrapper* unwrappedOther = unwrap(other);
    return unwrappedOther ? _value.constGet()->StrictEquals(unwrappedOther->toV8Value().constGet()) : false;
}

inline QList<QString> ScriptValueV8Wrapper::getPropertyNames() const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    auto context = _engine->getContext();
    v8::Context::Scope contextScope(context);
    v8::Local<v8::Value> value = _value.constGet();
    if (value->IsNullOrUndefined()) {
        return QList<QString>();
    }
    if (!value->IsObject()) {
        return QList<QString>();
    }
    v8::Local<v8::Object> object = v8::Local<v8::Object>::Cast(value);
    v8::Local<v8::Array> array;
    if (!object->GetPropertyNames(context).ToLocal(&array)) {
        return QList<QString>();
    }
    QList<QString> names;
    for (uint32_t n = 0; n < array->Length(); n++) {
        v8::Local<v8::String> name = array->Get(context, n).ToLocalChecked()->ToString(context).ToLocalChecked();
        names.append(*v8::String::Utf8Value(isolate, name));
    }
    return names;
}

bool ScriptValueV8Wrapper::toBool() const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    return _value.constGet()->ToBoolean(_engine->getIsolate())->Value();
}

qint32 ScriptValueV8Wrapper::toInt32() const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    v8::Local<v8::Integer> integer;
    if (!_value.constGet()->ToInteger(_engine->getContext()).ToLocal(&integer)) {
        Q_ASSERT(false);
    }
    return static_cast<int32_t>((integer)->Value());
}

double ScriptValueV8Wrapper::toInteger() const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    v8::Local<v8::Integer> integer;
    if (!_value.constGet()->ToInteger(_engine->getContext()).ToLocal(&integer)) {
        Q_ASSERT(false);
    }
    return (integer)->Value();
}

double ScriptValueV8Wrapper::toNumber() const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    v8::Local<v8::Number> number;
    if (!_value.constGet()->ToNumber(_engine->getContext()).ToLocal(&number)) {
        Q_ASSERT(false);
    }
    return number->Value();
}

QString ScriptValueV8Wrapper::toString() const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    v8::String::Utf8Value string(_engine->getIsolate(), _value.constGet());
    Q_ASSERT(*string != nullptr);
    return QString(*string);
}

quint16 ScriptValueV8Wrapper::toUInt16() const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    v8::Local<v8::Uint32> integer;
    if (!_value.constGet()->ToUint32(_engine->getContext()).ToLocal(&integer)) {
        Q_ASSERT(false);
    }
    return static_cast<uint16_t>(integer->Value());
}

quint32 ScriptValueV8Wrapper::toUInt32() const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    v8::Local<v8::Uint32> integer;
    if (!_value.constGet()->ToUint32(_engine->getContext()).ToLocal(&integer)) {
        Q_ASSERT(false);
    }
    return integer->Value();
}

QVariant ScriptValueV8Wrapper::toVariant() const {
    QVariant dest;
    if (_engine->castValueToVariant(_value, dest, QMetaType::UnknownType)) {
        return dest;
    } else {
        Q_ASSERT(false);
        return QVariant();
    }
}

QObject* ScriptValueV8Wrapper::toQObject() const {
    QVariant dest;
    if (_engine->castValueToVariant(_value, dest, QMetaType::QObjectStar)) {
        if (dest.canConvert<QObject*>()) {
            return dest.value<QObject*>();
        } else {
            //Q_ASSERT(false);
            return nullptr;
        }
    } else {
        //Q_ASSERT(false);
        return nullptr;
    }
}

bool ScriptValueV8Wrapper::equals(const ScriptValue& other) const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    ScriptValueV8Wrapper* unwrappedOther = unwrap(other);
    //V8TODO: is this used with different isolates?
    // in such case conversion will probably be necessary
    Q_ASSERT(_engine->getIsolate() == unwrappedOther->_engine->getIsolate());
    if (!unwrappedOther) {
        return false;
    }else{
        // V8TODO: which context needs to be used here?
        if (_value.constGet()->Equals(_engine->getContext(), unwrappedOther->toV8Value().constGet()).IsNothing()) {
            return false;
        } else {
            return _value.constGet()->Equals(_engine->getContext(), unwrappedOther->toV8Value().constGet()).FromJust();
        }
        /*if (_value.constGet()->Equals(unwrappedOther->toV8Value().constGetContext(), unwrappedOther->toV8Value().constGet()).IsNothing()) {
            return false;
        } else {
            return _value.constGet()->Equals(unwrappedOther->toV8Value().constGetContext(), unwrappedOther->toV8Value().constGet()).FromJust();
        }*/
    }
}

bool ScriptValueV8Wrapper::isArray() const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    return _value.constGet()->IsArray();
}

bool ScriptValueV8Wrapper::isBool() const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    return _value.constGet()->IsBoolean();
}

bool ScriptValueV8Wrapper::isError() const {
    //auto isolate = _engine->getIsolate();
    // Q_ASSERT(isolate->IsCurrent());
    // v8::HandleScope handleScope(isolate);
    // v8::Context::Scope contextScope(_engine->getContext());
    //V8TODO
    return false;
    //return _value.constGet()->IsError();
}

bool ScriptValueV8Wrapper::isFunction() const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    return _value.constGet()->IsFunction();
}

bool ScriptValueV8Wrapper::isNumber() const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    return _value.constGet()->IsNumber();
}

bool ScriptValueV8Wrapper::isNull() const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    return _value.constGet()->IsNull();
}

bool ScriptValueV8Wrapper::isObject() const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    return _value.constGet()->IsObject();
}

bool ScriptValueV8Wrapper::isString() const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    return _value.constGet()->IsString();
}

bool ScriptValueV8Wrapper::isUndefined() const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    return _value.constGet()->IsUndefined();
}

bool ScriptValueV8Wrapper::isValid() const {
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    if (_value.constGet()->IsNullOrUndefined()) {
        return false;
    }
    return true;
}

bool ScriptValueV8Wrapper::isVariant() const {
    //V8TODO
    auto isolate = _engine->getIsolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_engine->getContext());
    Q_ASSERT(false);
    return false;
    //return _value.isVariant();
}
