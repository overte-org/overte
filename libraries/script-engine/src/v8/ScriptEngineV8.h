//
//  ScriptEngineV8.h
//  libraries/script-engine/src/qtscript
//
//  Created by Brad Hefta-Gaub on 12/14/13.
//  Modified for V8 by dr Karol Suprynowicz on 2022/10/08
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors.
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

/// @addtogroup ScriptEngine
/// @{

#ifndef hifi_ScriptEngineV8_h
#define hifi_ScriptEngineV8_h

#include <memory>

#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QMetaEnum>
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QSharedPointer>
#include <QtCore/QString>

#include "libplatform/libplatform.h"
#include "v8.h"

#include "../ScriptEngine.h"
#include "../ScriptManager.h"
#include "V8Types.h"

#include "ArrayBufferClass.h"

class ScriptContextV8Wrapper;
class ScriptEngineV8;
class ScriptManager;
class ScriptObjectV8Proxy;
class ScriptMethodV8Proxy;
using ScriptContextQtPointer = std::shared_ptr<ScriptContextV8Wrapper>;

const double GARBAGE_COLLECTION_TIME_LIMIT_S = 1.0;

Q_DECLARE_METATYPE(ScriptEngine::FunctionSignature)

/// [V8] Implements ScriptEngine for V8 and translates calls for QScriptEngine
class ScriptEngineV8 final : public ScriptEngine,
                                   public std::enable_shared_from_this<ScriptEngineV8> {
    Q_OBJECT

public:  // construction
    ScriptEngineV8(ScriptManager* scriptManager = nullptr);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NOTE - these are NOT intended to be public interfaces available to scripts, the are only Q_INVOKABLE so we can
    //        properly ensure they are only called on the correct thread

public:  // ScriptEngine implementation
    virtual void abortEvaluation() override;
    virtual void clearExceptions() override;
    virtual ScriptValue cloneUncaughtException(const QString& detail = QString()) override;
    virtual ScriptContext* currentContext() const override;
    Q_INVOKABLE virtual ScriptValue evaluate(const QString& program, const QString& fileName = QString()) override;
    Q_INVOKABLE virtual ScriptValue evaluate(const ScriptProgramPointer& program) override;
    Q_INVOKABLE virtual ScriptValue evaluateInClosure(const ScriptValue& locals, const ScriptProgramPointer& program) override;
    virtual ScriptValue globalObject() const override;
    virtual bool hasUncaughtException() const override;
    virtual bool isEvaluating() const override;
    //virtual ScriptValue lintScript(const QString& sourceCode, const QString& fileName, const int lineNumber = 1) override;
    virtual ScriptValue cheskScriptSyntax(ScriptProgramPointer program) override;
    virtual ScriptValue makeError(const ScriptValue& other, const QString& type = "Error") override;
    virtual ScriptManager* manager() const override;

    // if there is a pending exception and we are at the top level (non-recursive) stack frame, this emits and resets it
    virtual bool maybeEmitUncaughtException(const QString& debugHint = QString()) override;

    virtual ScriptValue newArray(uint length = 0) override;
    virtual ScriptValue newArrayBuffer(const QByteArray& message) override;
    virtual ScriptValue newFunction(ScriptEngine::FunctionSignature fun, int length = 0) override;
    virtual ScriptValue newObject() override;
    //virtual ScriptValue newObject( v8::Local<v8::ObjectTemplate> );
    virtual ScriptValue newMethod(QObject* object, V8ScriptValue lifetime,
                               const QList<QMetaMethod>& metas, int numMaxParams);
    virtual ScriptProgramPointer newProgram(const QString& sourceCode, const QString& fileName) override;
    virtual ScriptValue newQObject(QObject *object, ScriptEngine::ValueOwnership ownership = ScriptEngine::QtOwnership,
        const ScriptEngine::QObjectWrapOptions& options = ScriptEngine::QObjectWrapOptions()) override;
    virtual ScriptValue newValue(bool value) override;
    virtual ScriptValue newValue(int value) override;
    virtual ScriptValue newValue(uint value) override;
    virtual ScriptValue newValue(double value) override;
    virtual ScriptValue newValue(const QString& value) override;
    virtual ScriptValue newValue(const QLatin1String& value) override;
    virtual ScriptValue newValue(const char* value) override;
    virtual ScriptValue newVariant(const QVariant& value) override;
    virtual ScriptValue nullValue() override;
    virtual bool raiseException(const ScriptValue& exception) override;
    Q_INVOKABLE virtual void registerEnum(const QString& enumName, QMetaEnum newEnum) override;
    Q_INVOKABLE virtual void registerFunction(const QString& name,
                                              ScriptEngine::FunctionSignature fun,
                                              int numArguments = -1) override;
    Q_INVOKABLE virtual void registerFunction(const QString& parent,
                                              const QString& name,
                                              ScriptEngine::FunctionSignature fun,
                                              int numArguments = -1) override;
    Q_INVOKABLE virtual void registerGetterSetter(const QString& name,
                                                  ScriptEngine::FunctionSignature getter,
                                                  ScriptEngine::FunctionSignature setter,
                                                  const QString& parent = QString("")) override;
    Q_INVOKABLE virtual void registerGlobalObject(const QString& name, QObject* object) override;
    virtual void setDefaultPrototype(int metaTypeId, const ScriptValue& prototype) override;
    // Already implemented by QObject
    //virtual void setObjectName(const QString& name) override;
    virtual bool setProperty(const char* name, const QVariant& value) override;
    virtual void setProcessEventsInterval(int interval) override;
    virtual QThread* thread() const override;
    virtual void setThread(QThread* thread) override;
    virtual ScriptValue undefinedValue() override;
    virtual ScriptValue uncaughtException() const override;
    virtual QStringList uncaughtExceptionBacktrace() const override;
    virtual int uncaughtExceptionLineNumber() const override;
    virtual void updateMemoryCost(const qint64& deltaSize) override;
    virtual void requestCollectGarbage() override { while(!_v8Isolate->IdleNotificationDeadline(getV8Platform()->MonotonicallyIncreasingTime() + GARBAGE_COLLECTION_TIME_LIMIT_S)) {}; }

    // helper to detect and log warnings when other code invokes QScriptEngine/BaseScriptEngine in thread-unsafe ways
    inline bool IS_THREADSAFE_INVOCATION(const QString& method) { return ScriptEngine::IS_THREADSAFE_INVOCATION(method); }

protected: // brought over from BaseScriptEngine
    V8ScriptValue makeError(const V8ScriptValue& other, const QString& type = "Error");

    // if the currentContext() is valid then throw the passed exception; otherwise, immediately emit it.
    // note: this is used in cases where C++ code might call into JS API methods directly
    bool raiseException(const V8ScriptValue& exception);

    // helper to detect and log warnings when other code invokes QScriptEngine/BaseScriptEngine in thread-unsafe ways
    static bool IS_THREADSAFE_INVOCATION(const QThread* thread, const QString& method);

public: // public non-interface methods for other QtScript-specific classes to use
    /*typedef V8ScriptValue (*FunctionSignature)(v8::Local<v8::Context>, ScriptEngineV8 *);
    typedef V8ScriptValue (*FunctionWithArgSignature)(v8::Local<v8::Context>, ScriptEngineV8 *, void *);
    /// registers a global getter/setter
    Q_INVOKABLE void registerGetterSetter(const QString& name, FunctionSignature getter,
                                          FunctionSignature setter, const QString& parent = QString(""));

    /// register a global function
    Q_INVOKABLE void registerFunction(const QString& name, FunctionSignature fun, int numArguments = -1);

    /// register a function as a method on a previously registered global object
    Q_INVOKABLE void registerFunction(const QString& parent, const QString& name, FunctionSignature fun,
                                      int numArguments = -1);*/

    /// registers a global object by name
    Q_INVOKABLE void registerValue(const QString& valueName, V8ScriptValue value);

    // NOTE - this is used by the TypedArray implementation. we need to review this for thread safety
    // V8TODO
    //inline ArrayBufferClass* getArrayBufferClass() { return _arrayBufferClass; }

public: // not for public use, but I don't like how Qt strings this along with private friend functions
    virtual ScriptValue create(int type, const void* ptr) override;
    virtual QVariant convert(const ScriptValue& value, int typeId) override;
    virtual void registerCustomType(int type, ScriptEngine::MarshalFunction marshalFunc,
                                    ScriptEngine::DemarshalFunction demarshalFunc) override;
    int computeCastPenalty(const V8ScriptValue& val, int destTypeId);
    bool castValueToVariant(const V8ScriptValue& val, QVariant& dest, int destTypeId);
    V8ScriptValue castVariantToValue(const QVariant& val);
    static QString valueType(const V8ScriptValue& val);
    v8::Isolate* getIsolate() {return _v8Isolate;}
    v8::Local<v8::Context> getContext() {
        v8::EscapableHandleScope handleScope(_v8Isolate);
        return handleScope.Escape(_v8Context.Get(_v8Isolate));
    }
    const v8::Local<v8::Context> getConstContext() const {return _v8Context.Get(_v8Isolate);}

    using ObjectWrapperMap = QMap<QObject*, QWeakPointer<ScriptObjectV8Proxy>>;
    mutable QMutex _qobjectWrapperMapProtect;
    ObjectWrapperMap _qobjectWrapperMap;
    

protected:
    // like `newFunction`, but allows mapping inline C++ lambdas with captures as callable V8ScriptValues
    // even though the context/engine parameters are redundant in most cases, the function signature matches `newFunction`
    // anyway so that newLambdaFunction can be used to rapidly prototype / test utility APIs and then if becoming
    // permanent more easily promoted into regular static newFunction scenarios.
    ScriptValue newLambdaFunction(std::function<V8ScriptValue(V8ScriptContext* context, ScriptEngineV8* engine)> operation,
                                   const V8ScriptValue& data,
                                   const ValueOwnership& ownership = AutoOwnership);

    void registerSystemTypes();

protected:
    static QMutex _v8InitMutex;
    static std::once_flag _v8InitOnceFlag;
    static v8::Platform* getV8Platform();
    
    v8::Isolate* _v8Isolate;
    v8::UniquePersistent<v8::Context> _v8Context;
    
    struct CustomMarshal {
        ScriptEngine::MarshalFunction marshalFunc;
        ScriptEngine::DemarshalFunction demarshalFunc;
    };
    using CustomMarshalMap = QHash<int, CustomMarshal>;
    using CustomPrototypeMap = QHash<int, V8ScriptValue>;

    QPointer<ScriptManager> _scriptManager;

    mutable QReadWriteLock _customTypeProtect { QReadWriteLock::Recursive };
    CustomMarshalMap _customTypes;
    CustomPrototypeMap _customPrototypes;
    ScriptValue _nullValue;
    ScriptValue _undefinedValue;
    mutable ScriptContextQtPointer _currContext;
    QThread *_currentThread;

    //V8TODO
    //ArrayBufferClass* _arrayBufferClass;
};

// Lambda helps create callable V8ScriptValues out of std::functions:
// (just meant for use from within the script engine itself)
class Lambda : public QObject {
    Q_OBJECT
public:
    Lambda(ScriptEngineV8* engine,
           std::function<V8ScriptValue(V8ScriptContext* context, ScriptEngineV8* engine)> operation,
           V8ScriptValue data);
    ~Lambda();
public slots:
    V8ScriptValue call();
    QString toString() const;

private:
    ScriptEngineV8* _engine;
    std::function<V8ScriptValue(V8ScriptContext* context, ScriptEngineV8* engine)> _operation;
    V8ScriptValue _data;
};

#endif  // hifi_ScriptEngineV8_h

/// @}
