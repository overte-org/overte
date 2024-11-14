//
//  ScriptEngineV8.h
//  libraries/script-engine/src/qtscript
//
//  Created by Brad Hefta-Gaub on 12/14/13.
//  Modified for V8 by dr Karol Suprynowicz on 2022/10/08
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors.
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
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
#include <QQueue>
#include <v8-profiler.h>

#include "libplatform/libplatform.h"
#include "v8.h"

#include "ScriptEngineDebugFlags.h"
#include "../ScriptEngine.h"
#include "../ScriptManager.h"
#include "../ScriptException.h"
//#include "V8Types.h"

#include "ArrayBufferClass.h"

class ScriptContextV8Wrapper;
class ScriptEngineV8;
class ScriptManager;
class ScriptObjectV8Proxy;
class ScriptMethodV8Proxy;
class ScriptValueV8Wrapper;
class ScriptSignalV8Proxy;

template <typename T> class V8ScriptValueTemplate;
typedef V8ScriptValueTemplate<v8::Value> V8ScriptValue;
typedef V8ScriptValueTemplate<v8::Script> V8ScriptProgram;

using ScriptContextV8Pointer = std::shared_ptr<ScriptContextV8Wrapper>;

const double GARBAGE_COLLECTION_TIME_LIMIT_S = 1.0;

Q_DECLARE_METATYPE(ScriptEngine::FunctionSignature)

/// [V8] Implements ScriptEngine for V8 and translates calls for QScriptEngine
class ScriptEngineV8 final : public ScriptEngine,
                                   public std::enable_shared_from_this<ScriptEngineV8> {
    Q_OBJECT

public:  // construction
    ScriptEngineV8(ScriptManager *manager = nullptr);
    virtual ~ScriptEngineV8();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NOTE - these are NOT intended to be public interfaces available to scripts, the are only Q_INVOKABLE so we can
    //        properly ensure they are only called on the correct thread

public:  // ScriptEngine implementation
    virtual void abortEvaluation() override;
    virtual void clearExceptions() override;
    virtual ScriptContext* currentContext() const override;
    Q_INVOKABLE virtual ScriptValue evaluate(const QString& program, const QString& fileName = QString()) override;
    Q_INVOKABLE virtual ScriptValue evaluate(const ScriptProgramPointer& program) override;
    Q_INVOKABLE virtual ScriptValue evaluateInClosure(const ScriptValue& locals, const ScriptProgramPointer& program) override;
    virtual ScriptValue globalObject() override;
    virtual bool hasUncaughtException() const override;
    virtual bool isEvaluating() const override;
    virtual ScriptValue checkScriptSyntax(ScriptProgramPointer program) override;

    virtual ScriptValue newArray(uint length = 0) override;
    virtual ScriptValue newArrayBuffer(const QByteArray& message) override;
    virtual ScriptValue newFunction(ScriptEngine::FunctionSignature fun, int length = 0) override;
    virtual ScriptValue newObject() override;
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

    virtual ScriptValue makeError(const ScriptValue& other, const QString& type = "Error") override;

    virtual bool raiseException(const QString& exception, const QString &reason = QString()) override;
    virtual bool raiseException(const ScriptValue& exception, const QString &reason = QString()) override;
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
    Q_INVOKABLE virtual void registerGlobalObject(const QString& name, QObject* object, ScriptEngine::ValueOwnership = ScriptEngine::QtOwnership) override;
    virtual void setDefaultPrototype(int metaTypeId, const ScriptValue& prototype) override;
    virtual void setObjectName(const QString& name) override;
    virtual bool setProperty(const char* name, const QVariant& value) override;
    virtual void setProcessEventsInterval(int interval) override;
    virtual QThread* thread() const override;
    virtual void setThread(QThread* thread) override;
    virtual ScriptValue undefinedValue() override;
    virtual std::shared_ptr<ScriptException> uncaughtException() const override;
    virtual void updateMemoryCost(const qint64& deltaSize) override;
    virtual void requestCollectGarbage() override { while(!_v8Isolate->IdleNotificationDeadline(getV8Platform()->MonotonicallyIncreasingTime() + GARBAGE_COLLECTION_TIME_LIMIT_S)) {}; }
    virtual void compileTest() override;
    virtual QString scriptValueDebugDetails(const ScriptValue &value) override;
    QString scriptValueDebugDetailsV8(const V8ScriptValue &value);
    virtual QString scriptValueDebugListMembers(const ScriptValue &value) override;
    QString scriptValueDebugListMembersV8(const V8ScriptValue &v8Value);
    virtual void logBacktrace(const QString &title = QString("")) override;
    virtual ScriptEngineMemoryStatistics getMemoryUsageStatistics() override;
    virtual void startCollectingObjectStatistics() override;
    virtual void dumpHeapObjectStatistics() override;
    virtual void startProfiling() override;
    virtual void stopProfilingAndSave() override;
    void scheduleValueWrapperForDeletion(ScriptValueV8Wrapper* wrapper) {_scriptValueWrappersToDelete.enqueue(wrapper);}
    void deleteUnusedValueWrappers();
    virtual void perManagerLoopIterationCleanup() override;
    virtual void disconnectSignalProxies() override;

    // helper to detect and log warnings when other code invokes QScriptEngine/BaseScriptEngine in thread-unsafe ways
    inline bool IS_THREADSAFE_INVOCATION(const QString& method) { return ScriptEngine::IS_THREADSAFE_INVOCATION(method); }

protected: // brought over from BaseScriptEngine


    // if the currentContext() is valid then throw the passed exception; otherwise, immediately emit it.
    // note: this is used in cases where C++ code might call into JS API methods directly
    bool raiseException(const V8ScriptValue& exception);

    // helper to detect and log warnings when other code invokes QScriptEngine/BaseScriptEngine in thread-unsafe ways
    static bool IS_THREADSAFE_INVOCATION(const QThread* thread, const QString& method);

public: // public non-interface methods for other QtScript-specific classes to use

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

    // Converts JS objects created in V8 to variants. Iterates over all properties and converts them to variants.
    bool convertJSArrayToVariant(v8::Local<v8::Array> array, QVariant &dest);
    bool convertJSObjectToVariant(v8::Local<v8::Object> object, QVariant &dest);
    V8ScriptValue castVariantToValue(const QVariant& val);
    QString valueType(const V8ScriptValue& val);
    v8::Isolate* getIsolate() {
        Q_ASSERT(_v8Isolate != nullptr);
        return _v8Isolate;}
    v8::Local<v8::Context> getContext();
    const v8::Local<v8::Context> getConstContext() const;
    QString formatErrorMessageFromTryCatch(v8::TryCatch &tryCatch);
    // Useful for debugging
    virtual QStringList getCurrentScriptURLs() const override;

    using ObjectWrapperMap = QMap<QObject*, QWeakPointer<ScriptObjectV8Proxy>>;
    mutable QMutex _qobjectWrapperMapProtect;
    ObjectWrapperMap _qobjectWrapperMap;
    // Second map, from which wrappers are removed by script engine upon deletion
    QMap<QObject*, QSharedPointer<ScriptObjectV8Proxy>> _qobjectWrapperMapV8;
    // V8TODO: maybe just a single map can be used instead to increase performance?

    // Sometimes ScriptValueV8Wrapper::release() is called from inside ScriptValueV8Wrapper.
    // Then wrapper needs to be deleted in the event loop
    QQueue<ScriptValueV8Wrapper*> _scriptValueWrappersToDelete;

    // Used by ScriptObjectV8Proxy to create JS objects referencing C++ ones
    v8::Local<v8::ObjectTemplate> getObjectProxyTemplate();
    v8::Local<v8::ObjectTemplate> getMethodDataTemplate();
    v8::Local<v8::ObjectTemplate> getFunctionDataTemplate();
    v8::Local<v8::ObjectTemplate> getVariantDataTemplate();
    v8::Local<v8::ObjectTemplate> getVariantProxyTemplate();

    ScriptContextV8Pointer pushContext(v8::Local<v8::Context> context);
    void popContext();
    void storeGlobalObjectContents();
#ifdef OVERTE_V8_MEMORY_DEBUG
    void incrementScriptValueCounter() { scriptValueCount++; };
    void decrementScriptValueCounter() { scriptValueCount--; };
    void incrementScriptValueProxyCounter() { scriptValueProxyCount++; };
    void decrementScriptValueProxyCounter() { scriptValueProxyCount--; };
#endif

protected:

    void registerSystemTypes();

protected:
    static QMutex _v8InitMutex;
    static std::once_flag _v8InitOnceFlag;
    static v8::Platform* getV8Platform();

    void setUncaughtEngineException(const QString &message, const QString& info = QString());
    void setUncaughtException(const v8::TryCatch &tryCatch, const QString& info = QString());
    void setUncaughtException(std::shared_ptr<ScriptException> exception);

    friend class ScriptSignalV8Proxy;

    std::shared_ptr<ScriptException> _uncaughtException;


    // V8TODO: clean up isolate when script engine is destroyed?
    v8::Isolate* _v8Isolate;

    struct CustomMarshal {
        ScriptEngine::MarshalFunction marshalFunc;
        ScriptEngine::DemarshalFunction demarshalFunc;
    };
    using CustomMarshalMap = QHash<int, CustomMarshal>;
    using CustomPrototypeMap = QHash<int, V8ScriptValue>;

    mutable QReadWriteLock _customTypeProtect { QReadWriteLock::Recursive };
    CustomMarshalMap _customTypes;
    CustomPrototypeMap _customPrototypes;
    ScriptValue _nullValue;
    ScriptValue _undefinedValue;
    // Current context stack. Main context is first on the list and current one is last.
    QList<ScriptContextV8Pointer> _contexts;
    // V8TODO: release in destructor
    v8::Persistent<v8::Object> _globalObjectContents;
    bool areGlobalObjectContentsStored {false};

    // Used by ScriptObjectV8Proxy to create JS objects referencing C++ ones
    // V8TODO: release in destructor
    v8::Persistent<v8::ObjectTemplate> _objectProxyTemplate;
    v8::Persistent<v8::ObjectTemplate> _methodDataTemplate;
    v8::Persistent<v8::ObjectTemplate> _functionDataTemplate;
    v8::Persistent<v8::ObjectTemplate> _variantDataTemplate;
    v8::Persistent<v8::ObjectTemplate> _variantProxyTemplate;

public:
    volatile int _memoryCorruptionIndicator = 12345678;
private:
    //V8TODO
    //ArrayBufferClass* _arrayBufferClass;
    // Counts how many nested evaluate calls are there at a given point
    int _evaluatingCounter;
#ifdef OVERTE_V8_MEMORY_DEBUG
    std::atomic<size_t> scriptValueCount{0};
    std::atomic<size_t> scriptValueProxyCount{0};
#endif

#ifdef OVERTE_SCRIPT_USE_AFTER_DELETE_GUARD
    bool _wasDestroyed{false};
#endif
    // Pointers to profiling classes. These are valid only when profiler is running, otherwise null
    // Smart pointer cannot be used here since profiler has private destructor
    v8::CpuProfiler *_profiler{nullptr};
    v8::ProfilerId _profilerId{0};

    // Set of script signal proxy pointers. Used for disconnecting signals on cleanup.
    // V8TODO: later it would be also worth to make sure that script proxies themselves get deleted together with script engine
    QReadWriteLock _signalProxySetLock;
    QSet<ScriptSignalV8Proxy*> _signalProxySet;

    friend ScriptValueV8Wrapper;
    friend ScriptSignalV8Proxy;
};

// This class is used to automatically add context to script engine's context list that is used by C++ calls
// An instance of it needs to be created in every V8 callback

class ContextScopeV8 {
public:
    ContextScopeV8(ScriptEngineV8 *engine);
    ~ContextScopeV8();
private:
    bool _isContextChangeNeeded;
    ScriptEngineV8* _engine;
};

QString getFileNameFromTryCatch(v8::TryCatch &tryCatch, v8::Isolate *isolate, v8::Local<v8::Context> &context );

#include "V8Types.h"

#endif  // hifi_ScriptEngineV8_h

/// @}
