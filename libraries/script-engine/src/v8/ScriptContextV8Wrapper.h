//
//  ScriptContextV8Wrapper.h
//  libraries/script-engine/src/v8
//
//  Created by Heather Anderson on 5/22/21.
//  Modified for V8 by dr Karol Suprynowicz on 2022/10/08
//  Copyright 2021 Vircadia contributors.
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
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
    ScriptContextV8Wrapper(ScriptEngineV8* engine, const v8::Local<v8::Context> context);
    ScriptContextV8Wrapper(ScriptEngineV8* engine, const v8::Local<v8::Context> context, std::shared_ptr<v8::FunctionCallbackInfo<v8::Value>> functionCallbackInfo);
    static ScriptContextV8Wrapper* unwrap(ScriptContext* val);
    v8::Local<v8::Context> toV8Value() const;

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

private: // storage
    v8::Persistent<v8::Context> _context;
    std::shared_ptr<v8::FunctionCallbackInfo<v8::Value>> _functionCallbackInfo;
    ScriptEngineV8* _engine;
};

class ScriptFunctionContextV8Wrapper final : public ScriptFunctionContext {
public:  // construction
    //V8TODO
    inline ScriptFunctionContextV8Wrapper(v8::Local<v8::Context> context) { }

public:  // ScriptFunctionContext implementation
    virtual QString fileName() const override;
    virtual QString functionName() const override;
    virtual FunctionType functionType() const override;
    virtual int lineNumber() const override;

//private: // storage
    //V8ScriptContextInfo _value;
};

#endif  // hifi_ScriptContextV8Wrapper_h

/// @}
