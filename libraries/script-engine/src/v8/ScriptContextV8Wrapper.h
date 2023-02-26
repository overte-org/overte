//
//  ScriptContextV8Wrapper.h
//  libraries/script-engine/src/v8
//
//  Created by Heather Anderson on 5/22/21.
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

#ifndef hifi_ScriptContextV8Wrapper_h
#define hifi_ScriptContextV8Wrapper_h

#include <QtCore/QString>

#include "../ScriptContext.h"
#include "../ScriptValue.h"

#include <libplatform/libplatform.h>
#include <v8.h>

//class V8ScriptContext;
class ScriptEngineV8;

/// [V8] Implements ScriptContext for V8 and translates calls for V8ScriptContextInfo
class ScriptContextV8Wrapper final : public ScriptContext {
public: // construction
    ScriptContextV8Wrapper(ScriptEngineV8* engine, const v8::Local<v8::Context> context, ScriptContextPointer parent);
    ScriptContextV8Wrapper(ScriptEngineV8* engine, const v8::FunctionCallbackInfo<v8::Value> *functionCallbackInfo,
                           const v8::Local<v8::Context> context, ScriptContextPointer parent);
    ScriptContextV8Wrapper(ScriptEngineV8* engine, const v8::PropertyCallbackInfo<v8::Value> *propertyCallbackInfo,
                           const v8::Local<v8::Context> context, ScriptContextPointer parent);
    static ScriptContextV8Wrapper* unwrap(ScriptContext* val);

public: // ScriptContext implementation
    virtual int argumentCount() const override;
    virtual ScriptValue argument(int index) const override;
    virtual QStringList backtrace() const override;
    virtual ScriptValue callee() const override;
    virtual ScriptEnginePointer engine() const override;
    virtual ScriptFunctionContextPointer functionContext() const override;
    virtual ScriptContextPointer parentContext() const override;
    virtual ScriptValue thisObject() const override;
    virtual ScriptValue throwError(const QString& text) override;
    virtual ScriptValue throwValue(const ScriptValue& value) override;

public: // For use by V8-related functions
    v8::Local<v8::Context> toV8Value() const;

private: // storage
    const v8::FunctionCallbackInfo<v8::Value> *_functionCallbackInfo;
    const v8::PropertyCallbackInfo<v8::Value> *_propertyCallbackInfo;
    ScriptEngineV8* _engine;
    // V8TODO: Is custom copy constructor needed for thread safety?
    v8::Persistent<v8::Context> _context;
    ScriptContextPointer _parentContext;
    Q_DISABLE_COPY(ScriptContextV8Wrapper)
};

class ScriptFunctionContextV8Wrapper final : public ScriptFunctionContext {
public:  // construction
    //V8TODO
    ScriptFunctionContextV8Wrapper(ScriptEngineV8* engine, const v8::Local<v8::Context> context);

public:  // ScriptFunctionContext implementation
    virtual QString fileName() const override;
    virtual QString functionName() const override;
    virtual FunctionType functionType() const override;
    virtual int lineNumber() const override;

private: // storage
    ScriptEngineV8* _engine;
    // V8TODO: Is custom copy constructor needed for thread safety?
    v8::Persistent<v8::Context> _context;
    //V8ScriptContextInfo _value;
    Q_DISABLE_COPY(ScriptFunctionContextV8Wrapper)
};

#endif  // hifi_ScriptContextV8Wrapper_h

/// @}
