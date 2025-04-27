//
//  ScriptValueV8Wrapper.h
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

/// @addtogroup ScriptEngine
/// @{

#ifndef hifi_ScriptValueV8Wrapper_h
#define hifi_ScriptValueV8Wrapper_h

#include <QtCore/QPointer>

#include <utility>

#include "../ScriptValue.h"
#include "ScriptEngineV8.h"
#include "V8Types.h"

//#define OVERTE_V8_SCRIPT_VALUE_WRAPPER_DELETE_GUARD

class ScriptBufferViewV8Wrapper final : public ScriptBufferView {
public:
    bool hasBuffer() const override {
        auto view = v8::ArrayBufferView::Cast(*_view.Get(_isolate));
        return view->HasBuffer();
    }

    void* buffer() const override {
        auto view = v8::ArrayBufferView::Cast(*_view.Get(_isolate));

        // FIXME: calling IsExternal makes some buffers live longer??
        view->Buffer()->IsExternal();

        if (view->HasBuffer()) {
            return view->Buffer()->Data();
        } else {
            return nullptr;
        }
    }

    size_t byteOffset() const override {
        auto view = v8::ArrayBufferView::Cast(*_view.Get(_isolate));
        return view->ByteOffset();
    }

    size_t byteLength() const override {
        auto view = v8::ArrayBufferView::Cast(*_view.Get(_isolate));
        return view->ByteLength();
    }

    ScriptBufferViewV8Wrapper(v8::Isolate *isolate, v8::Persistent<v8::Value, v8::CopyablePersistentTraits<v8::Value>> view) : _isolate(isolate), _view(view) {}

    ~ScriptBufferViewV8Wrapper() {
        _view.Reset();
    }

private:
    v8::Isolate *_isolate;
    v8::Persistent<v8::Value, v8::CopyablePersistentTraits<v8::Value>> _view;
};

/// [V8] Implements ScriptValue for V8 and translates calls for V8ScriptValue
class ScriptValueV8Wrapper final : public ScriptValueProxy {
public: // construction
    ScriptValueV8Wrapper() = delete;
    //ScriptValueV8Wrapper(ScriptValueV8Wrapper &) = delete;
    inline ScriptValueV8Wrapper(ScriptEngineV8* engine, const V8ScriptValue& value) :
        _engine(engine), _value(value) {
#ifdef OVERTE_V8_MEMORY_DEBUG
        engine->incrementScriptValueProxyCounter();
#endif
    }
    inline ScriptValueV8Wrapper(ScriptEngineV8* engine, V8ScriptValue&& value) :
        _engine(engine), _value(std::move(value)) {
#ifdef OVERTE_V8_MEMORY_DEBUG
        engine->incrementScriptValueProxyCounter();
#endif
    }
    static ScriptValueV8Wrapper* unwrap(const ScriptValue& val);
    inline const V8ScriptValue& toV8Value() const { return _value; }
    static V8ScriptValue fullUnwrap(ScriptEngineV8* engine, const ScriptValue& value);
    ScriptEngineV8* getV8Engine() {return _engine;}

public:
    virtual void release() override;
    virtual ScriptValueProxy* copy() const override;

public:  // ScriptValue implementation
    virtual ScriptValue call(const ScriptValue& thisObject = ScriptValue(),
                             const ScriptValueList& args = ScriptValueList()) override;
    virtual ScriptValue call(const ScriptValue& thisObject, const ScriptValue& arguments) override;
    virtual ScriptValue construct(const ScriptValueList& args = ScriptValueList()) override;
    virtual ScriptValue construct(const ScriptValue& arguments) override;
    virtual ScriptValue data() const override;
    virtual ScriptEnginePointer engine() const override;
    virtual ScriptValueIteratorPointer newIterator() const override;
    virtual ScriptValue property(const QString& name,
                                 const ScriptValue::ResolveFlags& mode = ScriptValue::ResolvePrototype) const override;
    virtual ScriptValue property(quint32 arrayIndex,
                                 const ScriptValue::ResolveFlags& mode = ScriptValue::ResolvePrototype) const override;
    virtual ScriptValue prototype() const override;
    virtual void setData(const ScriptValue& val) override;

    virtual bool hasProperty(const QString &name) const override;

    virtual void setProperty(const QString& name,
                             const ScriptValue& value,
                             const ScriptValue::PropertyFlags& flags = ScriptValue::KeepExistingFlags) override;
    virtual void setProperty(quint32 arrayIndex,
                             const ScriptValue& value,
                             const ScriptValue::PropertyFlags& flags = ScriptValue::KeepExistingFlags) override;
    virtual void setPrototype(const ScriptValue& prototype) override;
    virtual bool strictlyEquals(const ScriptValue& other) const override;
    virtual QList<QString> getPropertyNames() const override;

    virtual bool equals(const ScriptValue& other) const override;
    virtual bool isArray() const override;
    virtual bool isBool() const override;
    virtual bool isError() const override;
    virtual bool isFunction() const override;
    virtual bool isNumber() const override;
    virtual bool isNull() const override;
    virtual bool isObject() const override;
    virtual bool isString() const override;
    virtual bool isUndefined() const override;
    virtual bool isValid() const override;
    virtual bool isVariant() const override;
    virtual bool isArrayBufferView() const override;
    virtual bool toBool() const override;
    virtual qint32 toInt32() const override;
    virtual double toInteger() const override;
    virtual double toNumber() const override;
    virtual QString toString() const override;
    virtual quint16 toUInt16() const override;
    virtual quint32 toUInt32() const override;
    virtual QVariant toVariant() const override;
    virtual QObject* toQObject() const override;
    virtual std::shared_ptr<ScriptBufferView> toArrayBufferView() const override;

#ifdef OVERTE_V8_SCRIPT_VALUE_WRAPPER_DELETE_GUARD
    // These can be used for debugging crashes caused access after delete
    // If delete guard is enabled, deleting wrapper will cause a crash and thus trigger debugger and reveal location where object was deleted.
    void enableDeleteGuard() { deleteGuard = true;}
    void disableDeleteGuard() { deleteGuard = false;}
#endif

protected:
    virtual ~ScriptValueV8Wrapper() {
#ifdef OVERTE_V8_MEMORY_DEBUG
        _engine->decrementScriptValueProxyCounter();
#endif
#ifdef OVERTE_V8_SCRIPT_VALUE_WRAPPER_DELETE_GUARD
        if (deleteGuard) {
            uint32_t* crashTrigger = nullptr;
            *crashTrigger = 0x12345678;
        }
#endif
    };

private: // helper functions
    V8ScriptValue fullUnwrap(const ScriptValue& value) const;

private: // storage
    ScriptEngineV8 *_engine;
    V8ScriptValue _value;

#ifdef OVERTE_V8_SCRIPT_VALUE_WRAPPER_DELETE_GUARD
    bool deleteGuard{false};
#endif
    // This is to prevent proxy being deleted when it is in use for example during callbacks from inside it
    mutable QReadWriteLock lock;

    Q_DISABLE_COPY(ScriptValueV8Wrapper)
};

#endif  // hifi_ScriptValueV8Wrapper_h

/// @}
