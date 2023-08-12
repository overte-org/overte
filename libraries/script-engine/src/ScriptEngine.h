//
//  ScriptEngine.h
//  libraries/script-engine/src
//
//  Created by Brad Hefta-Gaub on 12/14/13.
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

/// @addtogroup ScriptEngine
/// @{

#ifndef hifi_ScriptEngine_h
#define hifi_ScriptEngine_h

#include <memory>

#include <QtCore/QFlags>
#include <QtCore/QObject>

#include "ScriptValue.h"
#include "ScriptException.h"

// These are used for debugging memory leaks caused by persistent handles
//#define OVERTE_V8_MEMORY_DEBUG

class QByteArray;
class QLatin1String;
class QString;
class QThread;
class QVariant;
class ScriptContext;
class ScriptEngine;
class ScriptManager;
class ScriptProgram;
using ScriptEnginePointer = std::shared_ptr<ScriptEngine>;
using ScriptProgramPointer = std::shared_ptr<ScriptProgram>;

Q_DECLARE_METATYPE(ScriptEnginePointer);

template <typename T>
inline ScriptValue

scriptValueFromValue(ScriptEngine* engine, const T& t);

template <typename T>
inline T scriptvalue_cast(const ScriptValue& value);

class ScriptEngineMemoryStatistics {
public:
    size_t totalHeapSize;
    size_t usedHeapSize;
    size_t totalAvailableSize;
    size_t totalGlobalHandlesSize;
    size_t usedGlobalHandlesSize;
#ifdef OVERTE_V8_MEMORY_DEBUG
    size_t scriptValueCount;
    size_t scriptValueProxyCount;
    size_t qObjectCount;
    //size_t qVariantProxyCount;
#endif
};

/**
 * @brief Provides an engine-independent interface for a scripting engine
 *
 * Each script engine is strictly single threaded.
 *
 * This class only provides an interface to the underlying scripting engine, and doesn't
 * provide the full environment needed to execute scripts.
 *
 * To execute scripts that have access to the API, use ScriptManager.
 *
 * Exception handling
 * ==================
 *
 * Exceptions are handled in two directions: exceptions thrown by the code executing in the scripting
 * engine, but not captured by the running code are caught by this object and can be inspected.
 *
 * If an exception in the running code occurs, then the exception() signal is emitted. Also,
 * hasUncaughtException() returns true, and uncaughException() returns the ScriptException with the
 * details. Both the signal and uncaughtException() return the same information, and either can
 * be used depending on what best fits the program.
 *
 * To inject an exception into the running script, use raiseException(). This may result in the script
 * not capturing it and an uncaughtException happening as a result.
 */
class ScriptEngine : public QObject {
    Q_OBJECT
public:

    ScriptEngine(ScriptManager *manager = nullptr) : _manager(manager) {

    }

    typedef ScriptValue (*FunctionSignature)(ScriptContext*, ScriptEngine*);
    typedef ScriptValue (*MarshalFunction)(ScriptEngine*, const void*);
    typedef bool (*DemarshalFunction)(const ScriptValue&, QVariant &dest);

    /**
     * @brief Who owns a given object
     *
     */
    enum ValueOwnership {
        /**
         * @brief Object is managed by Qt
         *
         */
        QtOwnership = 0,

        /**
         * @brief Object is managed by the script
         *
         */
        ScriptOwnership = 1,

        /**
         * @brief Ownership is determined automatically.
         * If the object has a parent, it's deemed QtOwnership.
         * If the object has no parent, it's deemed ScriptOwnership.
         *
         */
        AutoOwnership = 2,
    };

    /**
     * @brief Which part of an object is exposed to the script
     *
     */
    enum QObjectWrapOption {

        /**
         * @brief The script object will not expose child objects as properties.
         *
         */
        //ExcludeChildObjects	= 0x0001,

        /**
         * @brief The script object will not expose signals and slots inherited from the superclass.
         *
         */
        ExcludeSuperClassMethods = 0x0002,

        /**
         * @brief The script object will not expose properties inherited from the superclass.
         *
         */
        ExcludeSuperClassProperties	= 0x0004,

        /**
         * @brief The script object will not expose the QObject::deleteLater() slot.
         *
         */
        ExcludeSuperClassContents = ExcludeSuperClassMethods | ExcludeSuperClassProperties,

        //ExcludeDeleteLater = 0x0010,

        /**
         * @brief The script object will not expose the QObject's slots.
         *
         */
        ExcludeSlots = 0x0020,

        /**
         * @brief Properties that don't already exist in the QObject will be created as dynamic properties of that object, rather than as properties of the script object.
         *
         */
        AutoCreateDynamicProperties = 0x0100,

        /**
         * @brief If a wrapper object with the requested configuration already exists, return that object.
         *
         */
        PreferExistingWrapperObject = 0x0200,

        /**
         * @brief Don't include methods (signals and slots) when enumerating the object's properties.
         *
         */
        SkipMethodsInEnumeration = 0x0008,
    };
    Q_DECLARE_FLAGS(QObjectWrapOptions, QObjectWrapOption);

public:
    /**
     * @brief Stops the currently running script
     *
     */
    virtual void abortEvaluation() = 0;

    /**
     * @brief Clears uncaughtException and related
     *
     */
    virtual void clearExceptions() = 0;

    /**
     * @brief Context of the currently running script
     *
     * This allows getting a backtrace, the local variables of the currently running function, etc.
     *
     * @return ScriptContext*
     */
    virtual ScriptContext* currentContext() const = 0;

    /**
     * @brief Runs a script
     *
     * This may be called several times during the lifetime of a scripting engine, with the
     * side effects accumulating.
     *
     * @param program Code to run
     * @param fileName Name of the script, for informational purposes
     * @return ScriptValue Return value of the script when it finishes running.
     */
    virtual ScriptValue evaluate(const QString& program, const QString& fileName = QString()) = 0;

    /**
     * @brief Evaluates a pre-compiled program
     *
     * @param program Program to evaluaate
     * @return ScriptValue
     */
    virtual ScriptValue evaluate(const ScriptProgramPointer &program) = 0;


    /**
     * @brief Evaluate a script in a separate environment
     *
     * Used for evaluating included scripts
     *
     * @param locals Local variables available to the script
     * @param program Code to run
     * @return ScriptValue
     */
    virtual ScriptValue evaluateInClosure(const ScriptValue& locals, const ScriptProgramPointer& program) = 0;

    /**
     * @brief Global object which holds all the functions and variables available everywhere
     *
     * This is a JavaScript concept, https://javascript.info/global-object
     *
     * @note This may not belong in the base class.
     * @return ScriptValue Global Object
     */
    virtual ScriptValue globalObject() {
        Q_ASSERT(false);
        return ScriptValue();
    }

    /**
     * @brief Whether the script has an uncaught exception
     *
     * @return true There is an uncaught exception
     * @return false There's no exception
     */
    virtual bool hasUncaughtException() const = 0;

    /**
     * @brief Whether a script is currently being evaluated
     *
     * @return true A script is currently being evaluated
     * @return false No script is being evaluated
     */
    virtual bool isEvaluating() const = 0;
    //virtual ScriptValue lintScript(const QString& sourceCode, const QString& fileName, const int lineNumber = 1) = 0;

    /**
     * @brief Check a program for syntax errors
     *
     * Returns an object with at least the following properties:
     * * fileName
     * * lineNumber
     * * stack
     * * formatted
     *
     * @param program Program to check
     * @return ScriptValue Result
     *
     * @note It could be a good improvement to redo this to return a struct instead.
     */
    virtual ScriptValue checkScriptSyntax(ScriptProgramPointer program) = 0;

    /**
     * @brief Pointer to the ScriptManager that controls this scripting engine
     *
     * @return ScriptManager* ScriptManager
     */
    ScriptManager* manager() const { return _manager; }

    virtual ScriptValue newArray(uint length = 0) = 0;
    virtual ScriptValue newArrayBuffer(const QByteArray& message) = 0;
    virtual ScriptValue newFunction(FunctionSignature fun, int length = 0) {
        Q_ASSERT(false);
        return ScriptValue();
    }
    virtual ScriptValue newObject() = 0;
    virtual ScriptProgramPointer newProgram(const QString& sourceCode, const QString& fileName) = 0;
    virtual ScriptValue newQObject(QObject *object, ValueOwnership ownership = QtOwnership, const QObjectWrapOptions &options = QObjectWrapOptions()) = 0;
    virtual ScriptValue newValue(bool value) = 0;
    virtual ScriptValue newValue(int value) = 0;
    virtual ScriptValue newValue(uint value) = 0;
    virtual ScriptValue newValue(double value) = 0;
    virtual ScriptValue newValue(const QString& value) = 0;
    virtual ScriptValue newValue(const QLatin1String& value) = 0;
    virtual ScriptValue newValue(const char* value) = 0;
    virtual ScriptValue newVariant(const QVariant& value) = 0;
    virtual ScriptValue nullValue() = 0;


    /**
     * @brief Make a ScriptValue that contains an error
     *
     * This is used to throw an error inside the running script
     *
     * @param other
     * @param type
     * @return ScriptValue ScriptValue containing error
     */
    virtual ScriptValue makeError(const ScriptValue& other, const QString& type = "Error") = 0;


    /**
     * @brief Causes an exception to be raised in the currently executing script
     *
     * @param exception Exception to be thrown in the script
     * @param reason Explanatory text about why the exception happened, for logging
     * @return true Exception was successfully thrown
     * @return false Exception couldn't be thrown because no script is running
     */
    virtual bool raiseException(const ScriptValue& exception, const QString &reason = QString()) = 0;

    /**
     * @brief Causes an exception to be raised in the currently executing script
     *
     * @param error Exception to be thrown in the script
     * @param reason Explanatory text about why the exception happened, for logging
     * @return true Exception was successfully thrown
     * @return false Exception couldn't be thrown because no script is running
     */
    virtual bool raiseException(const QString& error, const QString &reason = QString()) = 0;


    virtual void registerEnum(const QString& enumName, QMetaEnum newEnum) = 0;
    virtual void registerFunction(const QString& name, FunctionSignature fun, int numArguments = -1) = 0;
    virtual void registerFunction(const QString& parent, const QString& name, FunctionSignature fun, int numArguments = -1) = 0;
    virtual void registerGetterSetter(const QString& name, FunctionSignature getter, FunctionSignature setter, const QString& parent = QString("")) = 0;
    virtual void registerGlobalObject(const QString& name, QObject* object, ScriptEngine::ValueOwnership = ScriptEngine::QtOwnership) = 0;
    virtual void setDefaultPrototype(int metaTypeId, const ScriptValue& prototype) = 0;
    virtual void setObjectName(const QString& name) = 0;
    virtual bool setProperty(const char* name, const QVariant& value) = 0;
    virtual void setProcessEventsInterval(int interval) = 0;
    virtual QThread* thread() const = 0;
    virtual void setThread(QThread* thread) = 0;
    //Q_INVOKABLE virtual void enterIsolateOnThisThread() = 0;
    virtual ScriptValue undefinedValue() = 0;

    /**
     * @brief Last uncaught exception, if any.
     *
     * The returned shared pointer is newly allocated by the function,
     * and modifying it has no effect on the internal state of the ScriptEngine.
     *
     * @return std::shared_ptr<ScriptValue> Uncaught exception from the script
     */
    virtual std::shared_ptr<ScriptException> uncaughtException() const = 0;

    virtual void updateMemoryCost(const qint64& deltaSize) = 0;
    virtual void requestCollectGarbage() = 0;

    /**
     * @brief Test the underlying scripting engine
     *
     * This compiles, executes and verifies the execution of a trivial test program
     * to make sure the underlying scripting engine actually works.
     *
     * @deprecated Test function, not part of the API, can be removed
     */
    virtual void compileTest() = 0;
    virtual QString scriptValueDebugDetails(const ScriptValue &value) = 0;
    virtual QString scriptValueDebugListMembers(const ScriptValue &value) = 0;

    /**
     * @brief Log the current backtrace
     *
     * Logs the current backtrace for debugging
     *
     * @param title Informative title for the backtrace
     */
    virtual void logBacktrace(const QString &title) = 0;

    /**
     * @brief Return memory usage statistics data.
     *
     * Returns memory usage statistics data for debugging.
     *
     * @return ScriptEngineMemoryStatistics Object containing memory usage statistics data.
     */
    virtual ScriptEngineMemoryStatistics getMemoryUsageStatistics() = 0;

    /**
     * @brief Start collecting object statistics that can later be reported with dumpHeapObjectStatistics().
     */
    virtual void startCollectingObjectStatistics() = 0;

    /**
     * @brief Prints heap statistics to a file. Collecting needs to first be started with dumpHeapObjectStatistics().
     */
    virtual void dumpHeapObjectStatistics() = 0;

    /**
     * @brief Starts collecting profiling data.
     */
    virtual void startProfiling() = 0;

    /**
     * @brief Stops collecting profiling data and saves it to a CSV file in Logs directory.
     */
    virtual void stopProfilingAndSave() = 0;

public:
    // helper to detect and log warnings when other code invokes QScriptEngine/BaseScriptEngine in thread-unsafe ways
    bool IS_THREADSAFE_INVOCATION(const QString& method);

public:
    template <typename T>
    inline T fromScriptValue(const ScriptValue& value) {
        return scriptvalue_cast<T>(value);
    }

    template <typename T>
    inline ScriptValue toScriptValue(const T& value) {
        return scriptValueFromValue(this, value);
    }

public: // not for public use, but I don't like how Qt strings this along with private friend functions
    virtual ScriptValue create(int type, const void* ptr) = 0;
    virtual QVariant convert(const ScriptValue& value, int type) = 0;
    virtual void registerCustomType(int type, MarshalFunction mf, DemarshalFunction df) = 0;
    virtual QStringList getCurrentScriptURLs() const = 0;
    virtual void perManagerLoopIterationCleanup() = 0;

signals:
    /**
     * @brief The script being run threw an exception
     *
     * @param exception Exception that was thrown
     */
    void exception(std::shared_ptr<ScriptException> exception);

protected:
    virtual ~ScriptEngine() {}  // prevent explicit deletion of base class

    ScriptManager *_manager;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(ScriptEngine::QObjectWrapOptions);

ScriptEnginePointer newScriptEngine(ScriptManager* manager = nullptr);

// Standardized CPS callback helpers (see: http://fredkschott.com/post/2014/03/understanding-error-first-callbacks-in-node-js/)
// These two helpers allow async JS APIs that use a callback parameter to be more friendly to scripters by accepting thisObject
// context and adopting a consistent and intuitable callback signature:
//   function callback(err, result) { if (err) { ... } else { /* do stuff with result */ } }
//
// To use, first pass the user-specified callback args in the same order used with optionally-scoped  Qt signal connections:
//   auto handler = makeScopedHandlerObject(scopeOrCallback, optionalMethodOrName);
// And then invoke the scoped handler later per CPS conventions:
//   auto result = callScopedHandlerObject(handler, err, result);
ScriptValue makeScopedHandlerObject(const ScriptValue& scopeOrCallback, const ScriptValue& methodOrName);
ScriptValue callScopedHandlerObject(const ScriptValue& handler, const ScriptValue& err, const ScriptValue& result);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Inline implementations
/*
QThread* ScriptEngine::thread() const {
    QObject* qobject = toQObject();
    if (qobject == nullptr) {
        return nullptr;
    }
    return qobject->thread();
}
*/

#endif  // hifi_ScriptEngine_h

/// @}
