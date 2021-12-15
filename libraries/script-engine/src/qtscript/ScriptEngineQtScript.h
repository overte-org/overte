//
//  ScriptEngineQtScript.h
//  libraries/script-engine/src/qtscript
//
//  Created by Brad Hefta-Gaub on 12/14/13.
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

/// @addtogroup ScriptEngine
/// @{

#ifndef hifi_ScriptEngineQtScript_h
#define hifi_ScriptEngineQtScript_h

#include <memory>

#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QMetaEnum>
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QSharedPointer>
#include <QtCore/QString>

#include <QtScript/QScriptEngine>

#include "../ScriptEngine.h"
#include "../ScriptManager.h"

#include "ArrayBufferClass.h"

class ScriptContextQtWrapper;
class ScriptEngineQtScript;
class ScriptManager;
class ScriptObjectQtProxy;
using ScriptContextQtPointer = std::shared_ptr<ScriptContextQtWrapper>;

Q_DECLARE_METATYPE(ScriptEngine::FunctionSignature)

/// [QtScript] Implements ScriptEngine for QtScript and translates calls for QScriptEngine
class ScriptEngineQtScript final : public QScriptEngine,
                                   public ScriptEngine,
                                   public std::enable_shared_from_this<ScriptEngineQtScript> {
    Q_OBJECT

public:  // construction
    ScriptEngineQtScript(ScriptManager* scriptManager = nullptr);

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
    virtual ScriptValue lintScript(const QString& sourceCode, const QString& fileName, const int lineNumber = 1) override;
    virtual ScriptValue makeError(const ScriptValue& other, const QString& type = "Error") override;
    virtual ScriptManager* manager() const override;

    // if there is a pending exception and we are at the top level (non-recursive) stack frame, this emits and resets it
    virtual bool maybeEmitUncaughtException(const QString& debugHint = QString()) override;

    virtual ScriptValue newArray(uint length = 0) override;
    virtual ScriptValue newArrayBuffer(const QByteArray& message) override;
    virtual ScriptValue newFunction(ScriptEngine::FunctionSignature fun, int length = 0) override;
    virtual ScriptValue newObject() override;
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
    virtual void setObjectName(const QString& name) override;
    virtual bool setProperty(const char* name, const QVariant& value) override;
    virtual void setProcessEventsInterval(int interval) override;
    virtual QThread* thread() const override;
    virtual void setThread(QThread* thread) override;
    virtual ScriptValue undefinedValue() override;
    virtual ScriptValue uncaughtException() const override;
    virtual QStringList uncaughtExceptionBacktrace() const override;
    virtual int uncaughtExceptionLineNumber() const override;
    virtual void updateMemoryCost(const qint64& deltaSize) override;
    virtual void requestCollectGarbage() override { collectGarbage(); }

    // helper to detect and log warnings when other code invokes QScriptEngine/BaseScriptEngine in thread-unsafe ways
    inline bool IS_THREADSAFE_INVOCATION(const QString& method) { return ScriptEngine::IS_THREADSAFE_INVOCATION(method); }

protected: // brought over from BaseScriptEngine
    QScriptValue makeError(const QScriptValue& other = QScriptValue(), const QString& type = "Error");

    // if the currentContext() is valid then throw the passed exception; otherwise, immediately emit it.
    // note: this is used in cases where C++ code might call into JS API methods directly
    bool raiseException(const QScriptValue& exception);

    // helper to detect and log warnings when other code invokes QScriptEngine/BaseScriptEngine in thread-unsafe ways
    static bool IS_THREADSAFE_INVOCATION(const QThread* thread, const QString& method);

public: // public non-interface methods for other QtScript-specific classes to use
    /// registers a global getter/setter
    Q_INVOKABLE void registerGetterSetter(const QString& name, QScriptEngine::FunctionSignature getter,
                                          QScriptEngine::FunctionSignature setter, const QString& parent = QString(""));

    /// register a global function
    Q_INVOKABLE void registerFunction(const QString& name, QScriptEngine::FunctionSignature fun, int numArguments = -1);

    /// register a function as a method on a previously registered global object
    Q_INVOKABLE void registerFunction(const QString& parent, const QString& name, QScriptEngine::FunctionSignature fun,
                                      int numArguments = -1);

    /// registers a global object by name
    Q_INVOKABLE void registerValue(const QString& valueName, QScriptValue value);

    // NOTE - this is used by the TypedArray implementation. we need to review this for thread safety
    inline ArrayBufferClass* getArrayBufferClass() { return _arrayBufferClass; }

public: // not for public use, but I don't like how Qt strings this along with private friend functions
    virtual ScriptValue create(int type, const void* ptr) override;
    virtual QVariant convert(const ScriptValue& value, int typeId) override;
    virtual void registerCustomType(int type, ScriptEngine::MarshalFunction marshalFunc,
                                    ScriptEngine::DemarshalFunction demarshalFunc) override;
    bool castValueToVariant(const QScriptValue& val, QVariant& dest, int destTypeId);
    QScriptValue castVariantToValue(const QVariant& val);
    static QString valueType(const QScriptValue& val);

    using ObjectWrapperMap = QMap<QObject*, QWeakPointer<ScriptObjectQtProxy>>;
    mutable QMutex _qobjectWrapperMapProtect;
    ObjectWrapperMap _qobjectWrapperMap;

protected:
    // like `newFunction`, but allows mapping inline C++ lambdas with captures as callable QScriptValues
    // even though the context/engine parameters are redundant in most cases, the function signature matches `newFunction`
    // anyway so that newLambdaFunction can be used to rapidly prototype / test utility APIs and then if becoming
    // permanent more easily promoted into regular static newFunction scenarios.
    QScriptValue newLambdaFunction(std::function<QScriptValue(QScriptContext* context, ScriptEngineQtScript* engine)> operation,
                                   const QScriptValue& data = QScriptValue(),
                                   const QScriptEngine::ValueOwnership& ownership = QScriptEngine::AutoOwnership);

    void registerSystemTypes();

protected:
    struct CustomMarshal {
        ScriptEngine::MarshalFunction marshalFunc;
        ScriptEngine::DemarshalFunction demarshalFunc;
    };
    using CustomMarshalMap = QHash<int, CustomMarshal>;
    using CustomPrototypeMap = QHash<int, QScriptValue>;

    QPointer<ScriptManager> _scriptManager;

    mutable QMutex _customTypeProtect;
    CustomMarshalMap _customTypes;
    CustomPrototypeMap _customPrototypes;
    ScriptValue _nullValue;
    ScriptValue _undefinedValue;
    mutable ScriptContextQtPointer _currContext;

    ArrayBufferClass* _arrayBufferClass;
};

// Lambda helps create callable QScriptValues out of std::functions:
// (just meant for use from within the script engine itself)
class Lambda : public QObject {
    Q_OBJECT
public:
    Lambda(ScriptEngineQtScript* engine,
           std::function<QScriptValue(QScriptContext* context, ScriptEngineQtScript* engine)> operation,
           QScriptValue data);
    ~Lambda();
public slots:
    QScriptValue call();
    QString toString() const;

private:
    ScriptEngineQtScript* engine;
    std::function<QScriptValue(QScriptContext* context, ScriptEngineQtScript* engine)> operation;
    QScriptValue data;
};

#endif  // hifi_ScriptEngineQtScript_h

/// @}