//
//  ScriptEngineV8.cpp
//  libraries/script-engine/src/v8
//
//  Created by Brad Hefta-Gaub on 12/14/13.
//  Modified for V8 by dr Karol Suprynowicz on 2022/10/08
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "ScriptEngineV8.h"

#include <chrono>
#include <mutex>
#include <thread>

#include <QtCore/QCoreApplication>
#include <QtCore/QEventLoop>
#include <QtCore/QFileInfo>
#include <QtCore/QTimer>
#include <QtCore/QThread>
#include <QtCore/QRegularExpression>

#include <QtCore/QFuture>
#include <QtConcurrent/QtConcurrentRun>

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMenu>

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <shared/LocalFileAccessGate.h>
#include <shared/QtHelpers.h>
#include <shared/AbstractLoggerInterface.h>

#include <Profile.h>

#include <v8-profiler.h>

#include "../ScriptEngineLogging.h"
#include "../ScriptProgram.h"
#include "../ScriptEngineCast.h"
#include "../ScriptValue.h"
#include "../ScriptManagerScriptingInterface.h"

#include "ScriptContextV8Wrapper.h"
#include "ScriptObjectV8Proxy.h"
#include "ScriptProgramV8Wrapper.h"
#include "ScriptValueV8Wrapper.h"
#include "ScriptEngineLoggingV8.h"
#include "ScriptValueIteratorV8Wrapper.h"
#include "shared/FileUtils.h"

static const int MAX_DEBUG_VALUE_LENGTH { 80 };

std::once_flag ScriptEngineV8::_v8InitOnceFlag;
QMutex ScriptEngineV8::_v8InitMutex;

bool ScriptEngineV8::IS_THREADSAFE_INVOCATION(const QThread* thread, const QString& method) {
    const QThread* currentThread = QThread::currentThread();
    if (currentThread == thread) {
        return true;
    }
    qCCritical(scriptengine_v8) << QString("Scripting::%1 @ %2 -- ignoring thread-unsafe call from %3")
                              .arg(method)
                              .arg(thread ? thread->objectName() : "(!thread)")
                              .arg(QThread::currentThread()->objectName());
    qCDebug(scriptengine_v8) << "(please resolve on the calling side by using invokeMethod, executeOnScriptThread, etc.)";
    Q_ASSERT(false);
    return false;
}

QString getFileNameFromTryCatch(v8::TryCatch &tryCatch, v8::Isolate *isolate, v8::Local<v8::Context> &context ) {
    v8::Local<v8::Message> exceptionMessage = tryCatch.Message();
    QString errorFileName;
    auto resource = exceptionMessage->GetScriptResourceName();
    v8::Local<v8::String> v8resourceString;
    if (resource->ToString(context).ToLocal(&v8resourceString)) {
        errorFileName = QString(*v8::String::Utf8Value(isolate, v8resourceString));
    }
    return errorFileName;
}

ScriptValue ScriptEngineV8::makeError(const ScriptValue& _other, const QString& type) {
    if (!IS_THREADSAFE_INVOCATION(thread(), __FUNCTION__)) {
        return nullValue();
    }

    auto other = _other;
    if (_other.isString()) {
        other = newObject();
        other.setProperty("message", _other.toString());
    }
    auto proto = globalObject().property(type);
    if (!proto.isFunction()) {
        proto = globalObject().property(other.prototype().property("constructor").property("name").toString());
    }
    if (!proto.isFunction()) {
#ifdef DEBUG_JS_EXCEPTIONS
        qCDebug(shared) << "BaseScriptEngine::makeError -- couldn't find constructor for" << type << " -- using Error instead";
#endif
        proto = globalObject().property("Error");
    }
    if (other.engine().get() != this) {
        // JS Objects are parented to a specific script engine instance
        // -- this effectively ~clones it locally by routing through a QVariant and back
        other = toScriptValue(other.toVariant());
    }
    // ~ var err = new Error(other.message)
    auto err = proto.construct(ScriptValueList({ other.property("message") }));

    // transfer over any existing properties
    auto it = other.newIterator();
    while (it->hasNext()) {
        it->next();
        err.setProperty(it->name(), it->value());
    }
    return err;
}



// check syntax and when there are issues returns an actual "SyntaxError" with the details
ScriptValue ScriptEngineV8::checkScriptSyntax(ScriptProgramPointer program) {
    if (!IS_THREADSAFE_INVOCATION(thread(), __FUNCTION__)) {
        return nullValue();
    }
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Context::Scope contextScope(getContext());
    ScriptSyntaxCheckResultPointer syntaxCheck = program->checkSyntax();
    if (syntaxCheck->state() != ScriptSyntaxCheckResult::Valid) {
        auto err = globalObject().property("SyntaxError").construct(ScriptValueList({ newValue(syntaxCheck->errorMessage()) }));
        err.setProperty("fileName", program->fileName());
        err.setProperty("lineNumber", syntaxCheck->errorLineNumber());
        err.setProperty("expressionBeginOffset", syntaxCheck->errorColumnNumber());
        err.setProperty("stack", syntaxCheck->errorBacktrace());
        {
            const auto error = syntaxCheck->errorMessage();
            const auto line = QString::number(syntaxCheck->errorLineNumber());
            const auto column = QString::number(syntaxCheck->errorColumnNumber());
            // for compatibility with legacy reporting
            const auto message = QString("[SyntaxError] %1 in %2:%3(%4)").arg(error, program->fileName(), line, column);
            err.setProperty("formatted", message);
        }
        return err;
    }
    return undefinedValue();
}

#ifdef DEBUG_JS
void ScriptEngineV8::_debugDump(const QString& header, const V8ScriptValue& object, const QString& footer) {
    // V8TODO
    /*if (!IS_THREADSAFE_INVOCATION(thread(), __FUNCTION__)) {
        return;
    }
    if (!header.isEmpty()) {
        qCDebug(shared) << header;
    }
    if (!object.isObject()) {
        qCDebug(shared) << "(!isObject)" << object.toVariant().toString() << object.toString();
        return;
    }
    V8ScriptValueIterator it(object);
    while (it.hasNext()) {
        it.next();
        qCDebug(shared) << it.name() << ":" << it.value().toString();
    }
    if (!footer.isEmpty()) {
        qCDebug(shared) << footer;
    }*/
}
#endif

v8::Platform* ScriptEngineV8::getV8Platform() {
    static std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    return platform.get();
}

ScriptEngineV8::ScriptEngineV8(ScriptManager *manager) : ScriptEngine(manager), _evaluatingCounter(0)
    //V8TODO _arrayBufferClass(new ArrayBufferClass(this))
{
    _v8InitMutex.lock();
    std::call_once ( _v8InitOnceFlag, [ ]{
        v8::V8::InitializeExternalStartupData("");

        // Experimentally determined that the maximum size that works on Linux with a stack size of 8192K is 8182.
        // That would seem to be the overhead of our code and V8.
        //
        // Windows stacks are 1MB.
        //
        // Based on that, going with 256K for stacks for now. That seems like a reasonable value.
        // We'll probably need a more complex system on the longer term, with configurable limits.
        // Flags to try:
        // --single-threaded is to check if it fixes random crashes
        // --jitless - might improve debugging performance due to no JIT?
        // --assert-types

        v8::V8::InitializeICU();
#ifdef OVERTE_V8_MEMORY_DEBUG
        v8::V8::SetFlagsFromString("--stack-size=256 --track_gc_object_stats --assert-types");
#else
        v8::V8::SetFlagsFromString("--stack-size=256");
#endif
        v8::Platform* platform = getV8Platform();
        v8::V8::InitializePlatform(platform);
        v8::V8::Initialize(); qCDebug(scriptengine_v8) << "V8 platform initialized";
    } );
    _v8InitMutex.unlock();
    qCDebug(scriptengine_v8) << "Creating new script engine";
    {
        v8::Isolate::CreateParams isolateParams;
        isolateParams.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        _v8Isolate = v8::Isolate::New(isolateParams);
        v8::Locker locker(_v8Isolate);
        v8::Isolate::Scope isolateScope(_v8Isolate);
        v8::HandleScope handleScope(_v8Isolate);
        v8::Local<v8::Context> context = v8::Context::New(_v8Isolate);
        Q_ASSERT(!context.IsEmpty());
        v8::Context::Scope contextScope(context);
        _contexts.append(std::make_shared<ScriptContextV8Wrapper>(this,context, ScriptContextPointer()));

        V8ScriptValue nullScriptValue(this, v8::Null(_v8Isolate));
        _nullValue = ScriptValue(new ScriptValueV8Wrapper(this, nullScriptValue));

        V8ScriptValue undefined(this, v8::Undefined(_v8Isolate));
        _undefinedValue = ScriptValue(new ScriptValueV8Wrapper(this, undefined));

        registerSystemTypes();

        // V8TODO: dispose of isolate on ScriptEngineV8 destruction
        // V8TODO:
        //QScriptEngine::setProcessEventsInterval(MSECS_PER_SECOND);
    }

    //if (_scriptManager) {
        // V8TODO: port to V8
        /*connect(this, &QScriptEngine::signalHandlerException, this, [this](const V8ScriptValue& exception) {
            if (hasUncaughtException()) {
                // the engine's uncaughtException() seems to produce much better stack traces here
                emit _scriptManager->unhandledException(cloneUncaughtException("signalHandlerException"));
                clearExceptions();
            } else {
                // ... but may not always be available -- so if needed we fallback to the passed exception
                V8ScriptValue thrown = makeError(exception);
                emit _scriptManager->unhandledException(ScriptValue(new ScriptValueV8Wrapper(this, std::move(thrown))));
            }
        }, Qt::DirectConnection);*/
    //}
}

ScriptEngineV8::~ScriptEngineV8() {
    // Process remaining events to avoid problems with `deleteLater` calling destructor of script proxies after script engine has been deleted:
    {
        QEventLoop loop;
        loop.processEvents();
    }
    // This is necessary for script engines that don't run in ScriptManager::run(), for example entity scripts:
    disconnectSignalProxies();
    deleteUnusedValueWrappers();
#ifdef OVERTE_SCRIPT_USE_AFTER_DELETE_GUARD
    _wasDestroyed = true;
#endif
    qDebug() << "ScriptEngineV8::~ScriptEngineV8: script engine destroyed";
}

void ScriptEngineV8::perManagerLoopIterationCleanup() {
    deleteUnusedValueWrappers();
}

void ScriptEngineV8::disconnectSignalProxies() {
    _signalProxySetLock.lockForRead();
    while (!_signalProxySet.empty()) {
        auto proxy = *_signalProxySet.begin();
        _signalProxySetLock.unlock();
        delete proxy;
        _signalProxySetLock.lockForRead();
    }
    _signalProxySetLock.unlock();
}

void ScriptEngineV8::deleteUnusedValueWrappers() {
    while (!_scriptValueWrappersToDelete.empty()) {
        auto wrapper = _scriptValueWrappersToDelete.dequeue();
        wrapper->release();
    }
}

void ScriptEngineV8::registerEnum(const QString& enumName, QMetaEnum newEnum) {
    if (!newEnum.isValid()) {
        qCCritical(scriptengine_v8) << "registerEnum called on invalid enum with name " << enumName;
        return;
    }
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Context::Scope contextScope(getContext());

    for (int i = 0; i < newEnum.keyCount(); i++) {
        const char* keyName = newEnum.key(i);
        QString fullName = enumName + "." + keyName;
        registerValue(fullName, V8ScriptValue(this, v8::Integer::New(_v8Isolate, newEnum.keyToValue(keyName))));
    }
}

void ScriptEngineV8::registerValue(const QString& valueName, V8ScriptValue value) {
    if (QThread::currentThread() != thread()) {
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine_v8) << "*** WARNING *** ScriptEngineV8::registerValue() called on wrong thread [" << QThread::currentThread() << "], invoking on correct thread [" << thread() << "]";
#endif
        QMetaObject::invokeMethod(this, "registerValue",
                                  Q_ARG(const QString&, valueName),
                                  Q_ARG(V8ScriptValue, value));
        return;
    }
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Local<v8::Context> context = getContext();
    v8::Context::Scope contextScope(context);
    QStringList pathToValue = valueName.split(".");
    int partsToGo = pathToValue.length();
    v8::Local<v8::Object> partObject = context->Global();

    for (const auto& pathPart : pathToValue) {
        partsToGo--;
        v8::Local<v8::String> pathPartV8 = v8::String::NewFromUtf8(_v8Isolate, pathPart.toStdString().c_str(),v8::NewStringType::kNormal).ToLocalChecked();
        v8::Local<v8::Value> currentPath;
        bool createProperty = false;
        if (!partObject->Get(context, pathPartV8).ToLocal(&currentPath)) {
            createProperty = true;
        }
        if (currentPath->IsUndefined()) {
            createProperty = true;
        }
        if (createProperty) {
            if (partsToGo > 0) {
                v8::Local<v8::Object> partValue = v8::Object::New(_v8Isolate);
                if (!partObject->Set(context, pathPartV8, partValue).FromMaybe(false)) {
                    Q_ASSERT(false);
                }
            } else {
                if (!partObject->Set(context, pathPartV8, value.constGet()).FromMaybe(false)) {
                    Q_ASSERT(false);
                }
            }
        }

        v8::Local<v8::Value> child;
        if (!partObject->Get(context, pathPartV8).ToLocal(&child)) {
            Q_ASSERT(false);
        }
        if (partsToGo > 0) {
            if (!child->IsObject()) {
                QString details = *v8::String::Utf8Value(_v8Isolate, child->ToDetailString(context).ToLocalChecked());
                qCDebug(scriptengine_v8) << "ScriptEngineV8::registerValue: Part of path is not an object: " << pathPart << " details: " << details;
                Q_ASSERT(false);
            }
            partObject = v8::Local<v8::Object>::Cast(child);
        }
    }
}

void ScriptEngineV8::registerGlobalObject(const QString& name, QObject* object, ScriptEngine::ValueOwnership) {
    if (QThread::currentThread() != thread()) {
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine_v8) << "*** WARNING *** ScriptEngineV8::registerGlobalObject() called on wrong thread [" << QThread::currentThread() << "], invoking on correct thread [" << thread() << "]  name:" << name;
#endif
        QMetaObject::invokeMethod(this, "registerGlobalObject",
                                  Q_ARG(const QString&, name),
                                  Q_ARG(QObject*, object));
        return;
    }
#ifdef THREAD_DEBUGGING
    qCDebug(scriptengine_v8) << "ScriptEngineV8::registerGlobalObject() called on thread [" << QThread::currentThread() << "] name:" << name;
#endif
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    Q_ASSERT(_v8Isolate->IsCurrent());
    v8::Local<v8::Context> context = getContext();
    v8::Context::Scope contextScope(context);
    v8::Local<v8::Object> v8GlobalObject = context->Global();
    v8::Local<v8::String> v8Name = v8::String::NewFromUtf8(_v8Isolate, name.toStdString().c_str()).ToLocalChecked();

    if (!v8GlobalObject->Get(context, v8Name).IsEmpty()) {
        if (object) {
            V8ScriptValue value = ScriptObjectV8Proxy::newQObject(this, object, ScriptEngine::QtOwnership);
            if(!v8GlobalObject->Set(context, v8Name, value.get()).FromMaybe(false)) {
                Q_ASSERT(false);
            }
        } else {
            if(!v8GlobalObject->Set(context, v8Name, v8::Null(_v8Isolate)).FromMaybe(false)) {
                Q_ASSERT(false);
            }
        }
    }
}

void ScriptEngineV8::registerFunction(const QString& name, ScriptEngine::FunctionSignature functionSignature, int numArguments) {
    if (QThread::currentThread() != thread()) {
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine_v8) << "*** WARNING *** ScriptEngineV8::registerFunction() called on wrong thread [" << QThread::currentThread() << "], invoking on correct thread [" << thread() << "] name:" << name;
#endif
        QMetaObject::invokeMethod(this, "registerFunction",
                                  Q_ARG(const QString&, name),
                                  Q_ARG(ScriptEngine::FunctionSignature, functionSignature),
                                  Q_ARG(int, numArguments));
        return;
    }
#ifdef THREAD_DEBUGGING
    qCDebug(scriptengine_v8) << "ScriptEngineV8::registerFunction() called on thread [" << QThread::currentThread() << "] name:" << name;
#endif

    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Context::Scope contextScope(getContext());
    auto scriptFun = newFunction(functionSignature, numArguments);

    globalObject().setProperty(name, scriptFun);
}

void ScriptEngineV8::registerFunction(const QString& parent, const QString& name, ScriptEngine::FunctionSignature functionSignature, int numArguments) {
    if (QThread::currentThread() != thread()) {
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine_v8) << "*** WARNING *** ScriptEngineV8::registerFunction() called on wrong thread [" << QThread::currentThread() << "], invoking on correct thread [" << thread() << "] parent:" << parent << "name:" << name;
#endif
        QMetaObject::invokeMethod(this, "registerFunction",
                                  Q_ARG(const QString&, name),
                                  Q_ARG(ScriptEngine::FunctionSignature, functionSignature),
                                  Q_ARG(int, numArguments));
        return;
    }
#ifdef THREAD_DEBUGGING
    qCDebug(scriptengine_v8) << "ScriptEngineV8::registerFunction() called on thread [" << QThread::currentThread() << "] parent:" << parent << "name:" << name;
#endif

    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Context::Scope contextScope(getContext());
    ScriptValue object = globalObject().property(parent);
    if (object.isValid()) {
        ScriptValue scriptFun = newFunction(functionSignature, numArguments);
        object.setProperty(name, scriptFun);
    }
}

void ScriptEngineV8::registerGetterSetter(const QString& name, ScriptEngine::FunctionSignature getter,
                                        ScriptEngine::FunctionSignature setter, const QString& parent) {
    if (QThread::currentThread() != thread()) {
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine_v8) << "*** WARNING *** ScriptEngineV8::registerGetterSetter() called on wrong thread [" << QThread::currentThread() << "], invoking on correct thread [" << thread() << "] "
            " name:" << name << "parent:" << parent;
#endif
        QMetaObject::invokeMethod(this, "registerGetterSetter",
                                  Q_ARG(const QString&, name),
                                  Q_ARG(ScriptEngine::FunctionSignature, getter),
                                  Q_ARG(ScriptEngine::FunctionSignature, setter),
                                  Q_ARG(const QString&, parent));
        return;
    }
#ifdef THREAD_DEBUGGING
    qCDebug(scriptengine_v8) << "ScriptEngineV8::registerGetterSetter() called on thread [" << QThread::currentThread() << "] name:" << name << "parent:" << parent;
#endif

        v8::Locker locker(_v8Isolate);
        v8::Isolate::Scope isolateScope(_v8Isolate);
        v8::HandleScope handleScope(_v8Isolate);
        auto context = getContext();
        v8::Context::Scope contextScope(context);

        ScriptValue setterFunction = newFunction(setter, 1);
        ScriptValue getterFunction = newFunction(getter);
        V8ScriptValue unwrappedGetter = ScriptValueV8Wrapper::fullUnwrap(this, getterFunction);
        V8ScriptValue unwrappedSetter = ScriptValueV8Wrapper::fullUnwrap(this, setterFunction);
        v8::PropertyDescriptor propertyDescriptor(unwrappedGetter.get(), unwrappedSetter.get());

        if (!parent.isNull() && !parent.isEmpty()) {
            ScriptValue object = globalObject().property(parent);
            if (object.isValid()) {
                V8ScriptValue v8parent = ScriptValueV8Wrapper::fullUnwrap(this, object);
                Q_ASSERT(v8parent.get()->IsObject());
                v8::Local<v8::Object> v8ParentObject = v8::Local<v8::Object>::Cast(v8parent.get());
                v8::Local<v8::String> v8propertyName =
                    v8::String::NewFromUtf8(_v8Isolate, name.toStdString().c_str()).ToLocalChecked();
                v8::Local<v8::Object> v8ObjectToSetProperty;
                ScriptObjectV8Proxy *proxy = ScriptObjectV8Proxy::unwrapProxy(V8ScriptValue(this, v8ParentObject));
                // If object is ScriptObjectV8Proxy, then setting property needs to be handled differently
                if (proxy) {
                    v8ObjectToSetProperty = v8ParentObject->GetInternalField(2).As<v8::Object>();
                } else {
                    v8ObjectToSetProperty = v8ParentObject;
                }
                    if (!v8ObjectToSetProperty->DefineProperty(context, v8propertyName, propertyDescriptor).FromMaybe(false)) {
                    qCDebug(scriptengine_v8) << "DefineProperty failed for registerGetterSetter \"" << name << "\" for parent: \""
                                          << parent << "\"";
                }
            } else {
                qCDebug(scriptengine_v8) << "Parent object \"" << parent << "\" for registerGetterSetter \"" << name
                                      << "\" is not valid: ";
            }
        } else {
            v8::Local<v8::String> v8propertyName =
                v8::String::NewFromUtf8(_v8Isolate, name.toStdString().c_str()).ToLocalChecked();
            if (!context->Global()->DefineProperty(context, v8propertyName, propertyDescriptor).FromMaybe(false)) {
                qCDebug(scriptengine_v8) << "DefineProperty failed for registerGetterSetter \"" << name << "\" for global object";
            }
        }
}

v8::Local<v8::Context> ScriptEngineV8::getContext() {
#ifdef OVERTE_SCRIPT_USE_AFTER_DELETE_GUARD
    Q_ASSERT(!_wasDestroyed);
#endif
    v8::EscapableHandleScope handleScope(_v8Isolate);
    Q_ASSERT(!_contexts.isEmpty());
    return handleScope.Escape(_contexts.last().get()->toV8Value());
}

const v8::Local<v8::Context> ScriptEngineV8::getConstContext() const {
    v8::EscapableHandleScope handleScope(_v8Isolate);
    Q_ASSERT(!_contexts.isEmpty());
    return handleScope.Escape(_contexts.last().get()->toV8Value());
}

// Stored objects are used to create global objects for evaluateInClosure
void ScriptEngineV8::storeGlobalObjectContents() {
    if (areGlobalObjectContentsStored) {
        return;
    }
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    auto context = getContext();
    v8::Context::Scope contextScope(context);
    v8::Local<v8::Object> globalMemberObjects = v8::Object::New(_v8Isolate);

    auto globalMemberNames = context->Global()->GetPropertyNames(context).ToLocalChecked();
    for (uint32_t i = 0; i < globalMemberNames->Length(); i++) {
        auto name = globalMemberNames->Get(context, i).ToLocalChecked();
        if(!globalMemberObjects->Set(context, name, context->Global()->Get(context, name).ToLocalChecked()).FromMaybe(false)) {
            Q_ASSERT(false);
        }
    }

    _globalObjectContents.Reset(_v8Isolate, globalMemberObjects);
    qCDebug(scriptengine_v8) << "ScriptEngineV8::storeGlobalObjectContents: " << globalMemberNames->Length() << " objects stored";
    areGlobalObjectContentsStored = true;
}

ScriptValue ScriptEngineV8::evaluateInClosure(const ScriptValue& _closure,
                                                           const ScriptProgramPointer& _program) {
    PROFILE_RANGE(script, "evaluateInClosure");
    if (!IS_THREADSAFE_INVOCATION(thread(), __FUNCTION__)) {
        return nullValue();
    }
    _evaluatingCounter++;
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    storeGlobalObjectContents();

    v8::Local<v8::Object> closureObject;
    v8::Local<v8::Value> closureGlobal;
    ScriptValueV8Wrapper* unwrappedClosure;
    ScriptProgramV8Wrapper* unwrappedProgram;

    {
        auto context = getContext();
        v8::Context::Scope contextScope(context);
        unwrappedProgram = ScriptProgramV8Wrapper::unwrap(_program);
        if (unwrappedProgram == nullptr) {
            _evaluatingCounter--;
            qCDebug(scriptengine_v8) << "Cannot unwrap program for closure";
            Q_ASSERT(false);
            return nullValue();
        }

        const auto fileName = unwrappedProgram->fileName();
        const auto shortName = QUrl(fileName).fileName();

        unwrappedClosure = ScriptValueV8Wrapper::unwrap(_closure);
        if (unwrappedClosure == nullptr) {
            _evaluatingCounter--;
            qCDebug(scriptengine_v8) << "Cannot unwrap closure";
            Q_ASSERT(false);
            return nullValue();
        }

        const V8ScriptValue& closure = unwrappedClosure->toV8Value();
        if (!closure.constGet()->IsObject()) {
            _evaluatingCounter--;
            qCDebug(scriptengine_v8) << "Unwrapped closure is not an object";
            Q_ASSERT(false);
            return nullValue();
        }
        Q_ASSERT(closure.constGet()->IsObject());
        closureObject = v8::Local<v8::Object>::Cast(closure.constGet());
        qCDebug(scriptengine_v8) << "Closure object members:" << scriptValueDebugListMembersV8(closure);
        v8::Local<v8::Object> testObject = v8::Object::New(_v8Isolate);
        if(!testObject->Set(context, v8::String::NewFromUtf8(_v8Isolate, "test_value").ToLocalChecked(), closureObject).FromMaybe(false)) {
            Q_ASSERT(false);
        }
        qCDebug(scriptengine_v8) << "Test object members:" << scriptValueDebugListMembersV8(V8ScriptValue(this, testObject));

        if (!closureObject->Get(closure.constGetContext(), v8::String::NewFromUtf8(_v8Isolate, "global").ToLocalChecked())
                 .ToLocal(&closureGlobal)) {
            _evaluatingCounter--;
            qCDebug(scriptengine_v8) << "Cannot get global from unwrapped closure";
            Q_ASSERT(false);
            return nullValue();
        }
    }
    v8::Local<v8::Context> closureContext;

    closureContext = v8::Context::New(_v8Isolate);
    pushContext(closureContext);

    ScriptValue result;
    // V8TODO: a lot of functions rely on _v8Context, which was not updated here
    // It might cause trouble
    {
        v8::Context::Scope contextScope(closureContext);
        //const V8ScriptValue& closure = unwrappedClosure->toV8Value();
        if (!unwrappedProgram->compile()) {
            qCDebug(scriptengine_v8) << "Can't compile script for evaluating in closure";
            Q_ASSERT(false);
            popContext();
            return nullValue();
        }
        const V8ScriptProgram& program = unwrappedProgram->toV8Value();

        v8::Local<v8::Value> thiz;
#ifdef DEBUG_JS
        qCDebug(shared) << QString("[%1] evaluateInClosure %2").arg(isEvaluating()).arg(shortName);
#endif
        {
            ScriptContextV8Wrapper scriptContext(this, getContext(), currentContext()->parentContext());
            ScriptContextGuard scriptContextGuard(&scriptContext);

            v8::TryCatch tryCatch(getIsolate());
            // Since V8 cannot use arbitrary object as global object, objects from main global need to be copied to closure's global object
            auto globalObjectContents = _globalObjectContents.Get(_v8Isolate);
            auto globalMemberNames = globalObjectContents->GetPropertyNames(globalObjectContents->CreationContext()).ToLocalChecked();
            for (uint32_t i = 0; i < globalMemberNames->Length(); i++) {
                auto name = globalMemberNames->Get(closureContext, i).ToLocalChecked();
                if(!closureContext->Global()->Set(closureContext, name, globalObjectContents->Get(globalObjectContents->CreationContext(), name).ToLocalChecked()).FromMaybe(false)) {
                    Q_ASSERT(false);
                }
            }
            qCDebug(scriptengine_v8) << "ScriptEngineV8::evaluateInClosure: " << globalMemberNames->Length() << " objects added to global";

            // Objects from closure need to be copied to global object too
            // V8TODO: I'm not sure which context to use with Get
            auto closureMemberNames = closureObject->GetPropertyNames(closureContext).ToLocalChecked();
            for (uint32_t i = 0; i < closureMemberNames->Length(); i++) {
                auto name = closureMemberNames->Get(closureContext, i).ToLocalChecked();
                if(!closureContext->Global()->Set(closureContext, name, closureObject->Get(closureContext, name).ToLocalChecked()).FromMaybe(false)) {
                    Q_ASSERT(false);
                }
            }
            // "Script" API is context-dependent, so it needs to be recreated for each new context
            registerGlobalObject("Script", new ScriptManagerScriptingInterface(_manager), ScriptEngine::ScriptOwnership);
            auto Script = globalObject().property("Script");
            auto require = Script.property("require");
            auto resolve = Script.property("_requireResolve");
            require.setProperty("resolve", resolve, ScriptValue::ReadOnly | ScriptValue::Undeletable);
            globalObject().setProperty("require", require, ScriptValue::ReadOnly | ScriptValue::Undeletable);

            // Script.require properties need to be copied, since that's where the Script.require cache is
            // Get source and destination Script.require objects
            try {
                v8::Local<v8::Value> oldScriptObjectValue;
                if (!globalObjectContents
                         ->Get(closureContext, v8::String::NewFromUtf8(_v8Isolate, "Script").ToLocalChecked())
                         .ToLocal(&oldScriptObjectValue)) {
                    throw(QString("evaluateInClosure: Script API object does not exist in calling script"));
                }
                if (!oldScriptObjectValue->IsObject()) {
                    throw(QString("evaluateInClosure: Script API object invalid in calling script"));
                }
                v8::Local<v8::Object> oldScriptObject = v8::Local<v8::Object>::Cast(oldScriptObjectValue);

                v8::Local<v8::Value> oldRequireObjectValue;
                if (!oldScriptObject->Get(closureContext, v8::String::NewFromUtf8(_v8Isolate, "require").ToLocalChecked())
                         .ToLocal(&oldRequireObjectValue)) {
                    throw(QString("evaluateInClosure: Script.require API object does not exist in calling script"));
                }
                if (!oldRequireObjectValue->IsObject()) {
                    throw(QString("evaluateInClosure: Script.require API object invalid in calling script"));
                }
                v8::Local<v8::Object> oldRequireObject = v8::Local<v8::Object>::Cast(oldRequireObjectValue);

                v8::Local<v8::Value> newScriptObjectValue;
                if (!closureContext->Global()
                         ->Get(closureContext, v8::String::NewFromUtf8(_v8Isolate, "Script").ToLocalChecked())
                         .ToLocal(&newScriptObjectValue)) {
                    Q_ASSERT(false);  // This should never happen
                }
                if (!newScriptObjectValue->IsObject()) {
                    Q_ASSERT(false);  // This should never happen
                }
                v8::Local<v8::Object> newScriptObject = v8::Local<v8::Object>::Cast(newScriptObjectValue);

                v8::Local<v8::Value> newRequireObjectValue;
                if (!newScriptObject->Get(closureContext, v8::String::NewFromUtf8(_v8Isolate, "require").ToLocalChecked())
                         .ToLocal(&newRequireObjectValue)) {
                    Q_ASSERT(false);  // This should never happen
                }
                if (!newRequireObjectValue->IsObject()) {
                    Q_ASSERT(false);  // This should never happen
                }
                v8::Local<v8::Object> newRequireObject = v8::Local<v8::Object>::Cast(newRequireObjectValue);

                auto requireMemberNames =
                    oldRequireObject->GetPropertyNames(oldRequireObject->CreationContext()).ToLocalChecked();
                for (uint32_t i = 0; i < requireMemberNames->Length(); i++) {
                    auto name = requireMemberNames->Get(closureContext, i).ToLocalChecked();
                    v8::Local<v8::Value> oldObject;
                    if (!oldRequireObject->Get(oldRequireObject->CreationContext(), name).ToLocal(&oldObject)) {
                        Q_ASSERT(false);  // This should never happen, the property has been reported as existing
                    }
                    if (!newRequireObject->Set(closureContext, name,oldObject).FromMaybe(false)) {
                        Q_ASSERT(false);
                    }
                }
            } catch (QString exception) {
                raiseException(exception);
                popContext();
                _evaluatingCounter--;
                return nullValue();
            }

            auto maybeResult = program.constGet()->GetUnboundScript()->BindToCurrentContext()->Run(closureContext);
            v8::Local<v8::Value> v8Result;
            if (!maybeResult.ToLocal(&v8Result)) {
                v8::String::Utf8Value utf8Value(getIsolate(), tryCatch.Exception());
                QString errorMessage = QString(__FUNCTION__) + " hasCaught:" + QString(*utf8Value) + "\n"
                    + "tryCatch details:" + formatErrorMessageFromTryCatch(tryCatch);
                v8Result = v8::Null(_v8Isolate);
                if (_manager) {
                    v8::Local<v8::Message> exceptionMessage = tryCatch.Message();
                    int errorLineNumber = -1;
                    if (!exceptionMessage.IsEmpty()) {
                        errorLineNumber = exceptionMessage->GetLineNumber(closureContext).FromJust();
                    }
                    _manager->scriptErrorMessage(errorMessage, getFileNameFromTryCatch(tryCatch, _v8Isolate, closureContext),
                                                          errorLineNumber);
                } else {
                    qWarning(scriptengine_v8) << errorMessage;
                }
            }

            if (hasUncaughtException()) {
#ifdef DEBUG_JS_EXCEPTIONS
                qCWarning(shared) << __FUNCTION__ << "---------- hasCaught:" << err.toString() << result.toString();
                err.setProperty("_result", result);
#endif
                result = nullValue();
            } else {
                result = ScriptValue(new ScriptValueV8Wrapper(this, V8ScriptValue(this, v8Result)));
            }
        }
#ifdef DEBUG_JS
        qCDebug(shared) << QString("[%1] //evaluateInClosure %2").arg(isEvaluating()).arg(shortName);
#endif
        popContext();
    }

    _evaluatingCounter--;
    return result;
}

ScriptValue ScriptEngineV8::evaluate(const QString& sourceCode, const QString& fileName) {

    if (QThread::currentThread() != thread()) {
        ScriptValue result;
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine_v8) << "*** WARNING *** ScriptEngineV8::evaluate() called on wrong thread [" << QThread::currentThread() << "], invoking on correct thread [" << thread() << "] "
            "sourceCode:" << sourceCode << " fileName:" << fileName;
#endif
        BLOCKING_INVOKE_METHOD(this, "evaluate",
                                  Q_RETURN_ARG(ScriptValue, result),
                                  Q_ARG(const QString&, sourceCode),
                                  Q_ARG(const QString&, fileName));
        return result;
    }
    // Compile and check syntax
    Q_ASSERT(!_v8Isolate->IsDead());
    _evaluatingCounter++;
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    auto context = getContext();
    v8::Context::Scope contextScope(context);
    v8::ScriptOrigin scriptOrigin(getIsolate(), v8::String::NewFromUtf8(getIsolate(), fileName.toStdString().c_str()).ToLocalChecked());
    v8::Local<v8::Script> script;
    {
        v8::TryCatch tryCatch(getIsolate());
        if (!v8::Script::Compile(context, v8::String::NewFromUtf8(getIsolate(), sourceCode.toStdString().c_str()).ToLocalChecked(), &scriptOrigin).ToLocal(&script)) {
            QString errorMessage(QString("Error while compiling script: \"") + fileName + QString("\" ") + formatErrorMessageFromTryCatch(tryCatch));
            if (_manager) {
                v8::Local<v8::Message> exceptionMessage = tryCatch.Message();
                int errorLineNumber = -1;
                if (!exceptionMessage.IsEmpty()) {
                    errorLineNumber = exceptionMessage->GetLineNumber(context).FromJust();
                }
                _manager->scriptErrorMessage(errorMessage, getFileNameFromTryCatch(tryCatch, _v8Isolate, context),
                                                      errorLineNumber);
            } else {
                qDebug(scriptengine_v8) << errorMessage;
            }
            setUncaughtException(tryCatch, "Error while compiling script");
            _evaluatingCounter--;
            return nullValue();
        }
    }

    v8::Local<v8::Value> result;
    v8::TryCatch tryCatchRun(getIsolate());
    if (!script->Run(context).ToLocal(&result)) {
        Q_ASSERT(tryCatchRun.HasCaught());
        auto runError = tryCatchRun.Message();
        ScriptValue errorValue(new ScriptValueV8Wrapper(this, V8ScriptValue(this, runError->Get())));
        QString errorMessage(QString("Running script: \"") + fileName + QString("\" ") + formatErrorMessageFromTryCatch(tryCatchRun));
        if (_manager) {
            v8::Local<v8::Message> exceptionMessage = tryCatchRun.Message();
            int errorLineNumber = -1;
            if (!exceptionMessage.IsEmpty()) {
                errorLineNumber = exceptionMessage->GetLineNumber(context).FromJust();
            }
            _manager->scriptErrorMessage(errorMessage, getFileNameFromTryCatch(tryCatchRun, _v8Isolate, context),
                                                  errorLineNumber);
        } else {
            qDebug(scriptengine_v8) << errorMessage;
        }
        setUncaughtException(tryCatchRun, "script evaluation");

        _evaluatingCounter--;
        return errorValue;
    }
    V8ScriptValue resultValue(this, result);
    _evaluatingCounter--;
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(resultValue)));
}


void ScriptEngineV8::setUncaughtEngineException(const QString &reason, const QString& info) {
    auto ex = std::make_shared<ScriptEngineException>(reason, info);
    setUncaughtException(ex);
}

void ScriptEngineV8::setUncaughtException(const v8::TryCatch &tryCatch, const QString& info) {
    if (!tryCatch.HasCaught()) {
        qCWarning(scriptengine_v8) << "setUncaughtException called without exception";
        clearExceptions();
        return;
    }

    auto ex = std::make_shared<ScriptRuntimeException>();
    ex->additionalInfo = info;

    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Local<v8::Context> context = getContext();
    v8::Context::Scope contextScope(context);
    QString result("");

    QString errorMessage = "";
    QString errorBacktrace = "";
    v8::String::Utf8Value utf8Value(getIsolate(), tryCatch.Message()->Get());

    ex->errorMessage = QString(*utf8Value);

    auto exceptionValue = tryCatch.Exception();
    ex->thrownValue =  ScriptValue(new ScriptValueV8Wrapper(this, V8ScriptValue(this, exceptionValue)));


    v8::Local<v8::Message> exceptionMessage = tryCatch.Message();
    if (!exceptionMessage.IsEmpty()) {
        ex->errorLine = exceptionMessage->GetLineNumber(context).FromJust();
        ex->errorColumn = exceptionMessage->GetStartColumn(context).FromJust();
        v8::Local<v8::Value> backtraceV8String;
        if (tryCatch.StackTrace(context).ToLocal(&backtraceV8String)) {
            if (backtraceV8String->IsString()) {
                if (v8::Local<v8::String>::Cast(backtraceV8String)->Length() > 0) {
                    v8::String::Utf8Value backtraceUtf8Value(getIsolate(), backtraceV8String);
                    QString errorBacktrace = QString(*backtraceUtf8Value).replace("\\n","\n");
                    ex->backtrace = errorBacktrace.split("\n");

                }
            }
        }
    }

    setUncaughtException(ex);
}

void ScriptEngineV8::setUncaughtException(std::shared_ptr<ScriptException> uncaughtException) {
    qCDebug(scriptengine_v8) << "Emitting exception:" << uncaughtException;
    _uncaughtException = uncaughtException;

    auto copy = uncaughtException->clone();
    emit exception(copy);
}


QString ScriptEngineV8::formatErrorMessageFromTryCatch(v8::TryCatch &tryCatch) {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    auto context = getContext();
    v8::Context::Scope contextScope(context);
    QString result("");
    int errorColumnNumber = 0;
    int errorLineNumber = 0;
    QString errorMessage = "";
    QString errorBacktrace = "";
    v8::String::Utf8Value utf8Value(getIsolate(), tryCatch.Message()->Get());
    errorMessage = QString(*utf8Value);
    v8::Local<v8::Message> exceptionMessage = tryCatch.Message();
    if (!exceptionMessage.IsEmpty()) {
        errorLineNumber = exceptionMessage->GetLineNumber(context).FromJust();
        errorColumnNumber = exceptionMessage->GetStartColumn(context).FromJust();
        v8::Local<v8::Value> backtraceV8String;
        if (tryCatch.StackTrace(context).ToLocal(&backtraceV8String)) {
            if (backtraceV8String->IsString()) {
                if (v8::Local<v8::String>::Cast(backtraceV8String)->Length() > 0) {
                    v8::String::Utf8Value backtraceUtf8Value(getIsolate(), backtraceV8String);
                    errorBacktrace = QString(*backtraceUtf8Value).replace("\\n","\n");
                }
            }
        }
        QTextStream resultStream(&result);
        resultStream << "failed on line " << errorLineNumber << " column " << errorColumnNumber << " with message: \"" << errorMessage <<"\" backtrace: " << errorBacktrace;
    }
    return result.replace("\\n", "\n");
}

v8::Local<v8::ObjectTemplate> ScriptEngineV8::getObjectProxyTemplate() {
    v8::EscapableHandleScope handleScope(_v8Isolate);
    if (_objectProxyTemplate.IsEmpty()) {
        auto objectTemplate = v8::ObjectTemplate::New(_v8Isolate);
        objectTemplate->SetInternalFieldCount(3);
        objectTemplate->SetHandler(v8::NamedPropertyHandlerConfiguration(ScriptObjectV8Proxy::v8Get, ScriptObjectV8Proxy::v8Set, nullptr, nullptr, ScriptObjectV8Proxy::v8GetPropertyNames));
        _objectProxyTemplate.Reset(_v8Isolate, objectTemplate);
    }

    return handleScope.Escape(_objectProxyTemplate.Get(_v8Isolate));
}

v8::Local<v8::ObjectTemplate> ScriptEngineV8::getMethodDataTemplate() {
    v8::EscapableHandleScope handleScope(_v8Isolate);
    if (_methodDataTemplate.IsEmpty()) {
        auto methodDataTemplate = v8::ObjectTemplate::New(_v8Isolate);
        methodDataTemplate->SetInternalFieldCount(2);
        _methodDataTemplate.Reset(_v8Isolate, methodDataTemplate);
    }

    return handleScope.Escape(_methodDataTemplate.Get(_v8Isolate));
}

v8::Local<v8::ObjectTemplate> ScriptEngineV8::getFunctionDataTemplate() {
    v8::EscapableHandleScope handleScope(_v8Isolate);
    if (_functionDataTemplate.IsEmpty()) {
        auto functionDataTemplate = v8::ObjectTemplate::New(_v8Isolate);
        functionDataTemplate->SetInternalFieldCount(2);
        _functionDataTemplate.Reset(_v8Isolate, functionDataTemplate);
    }

    return handleScope.Escape(_functionDataTemplate.Get(_v8Isolate));
}

v8::Local<v8::ObjectTemplate> ScriptEngineV8::getVariantDataTemplate() {
    v8::EscapableHandleScope handleScope(_v8Isolate);
    if (_variantDataTemplate.IsEmpty()) {
        auto variantDataTemplate = v8::ObjectTemplate::New(_v8Isolate);
        variantDataTemplate->SetInternalFieldCount(2);
        _variantDataTemplate.Reset(_v8Isolate, variantDataTemplate);
    }

    return handleScope.Escape(_variantDataTemplate.Get(_v8Isolate));
}

v8::Local<v8::ObjectTemplate> ScriptEngineV8::getVariantProxyTemplate() {
    v8::EscapableHandleScope handleScope(_v8Isolate);
    if (_variantProxyTemplate.IsEmpty()) {
        auto variantProxyTemplate = v8::ObjectTemplate::New(_v8Isolate);
        variantProxyTemplate->SetInternalFieldCount(2);
        variantProxyTemplate->SetHandler(v8::NamedPropertyHandlerConfiguration(ScriptVariantV8Proxy::v8Get, ScriptVariantV8Proxy::v8Set, nullptr, nullptr, ScriptVariantV8Proxy::v8GetPropertyNames));
        _variantProxyTemplate.Reset(_v8Isolate, variantProxyTemplate);
    }

    return handleScope.Escape(_variantProxyTemplate.Get(_v8Isolate));
}


ScriptContextV8Pointer ScriptEngineV8::pushContext(v8::Local<v8::Context> context) {
    v8::HandleScope handleScope(_v8Isolate);
    Q_ASSERT(!_contexts.isEmpty());
    ScriptContextPointer parent = _contexts.last();
    _contexts.append(std::make_shared<ScriptContextV8Wrapper>(this, context, parent));
    v8::Context::Scope contextScope(context);
    /*static volatile int debug_context_id = 1;
    if (!context->Global()->Set(context, v8::String::NewFromUtf8(_v8Isolate, "debug_context_id").ToLocalChecked(), v8::Integer::New(_v8Isolate, debug_context_id)).FromMaybe(false)) {
        Q_ASSERT(false);
    }
    debug_context_id++;*/
    return _contexts.last();
}

void ScriptEngineV8::popContext() {
    Q_ASSERT(!_contexts.isEmpty());
    _contexts.pop_back();
}

Q_INVOKABLE ScriptValue ScriptEngineV8::evaluate(const ScriptProgramPointer& program) {

    if (QThread::currentThread() != thread()) {
        ScriptValue result;
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine_v8) << "*** WARNING *** ScriptEngineV8::evaluate() called on wrong thread [" << QThread::currentThread() << "], invoking on correct thread [" << thread() << "] "
            "sourceCode:" << sourceCode << " fileName:" << fileName;
#endif
        BLOCKING_INVOKE_METHOD(this, "evaluate",
                                  Q_RETURN_ARG(ScriptValue, result),
                                  Q_ARG(const ScriptProgramPointer&, program));
        return result;
    }
    _evaluatingCounter++;
    ScriptValue errorValue;
    ScriptValue resultValue;
    bool hasFailed = false;
    {
        v8::Locker locker(_v8Isolate);
        v8::Isolate::Scope isolateScope(_v8Isolate);
        v8::HandleScope handleScope(_v8Isolate);
        auto context = getContext();
        v8::Context::Scope contextScope(context);
        ScriptProgramV8Wrapper* unwrapped = ScriptProgramV8Wrapper::unwrap(program);
        if (!unwrapped) {
            setUncaughtEngineException("Could not unwrap program", "Compile error");
            hasFailed = true;
        }

        if(!hasFailed) {
            ScriptSyntaxCheckResultPointer syntaxCheck = unwrapped->checkSyntax();
            if (syntaxCheck->state() == ScriptSyntaxCheckResult::Error) {
                setUncaughtEngineException(syntaxCheck->errorMessage(), "Compile error");
                hasFailed = true;
            }
        }

        v8::Local<v8::Value> result;
        if(!hasFailed) {
            const V8ScriptProgram& v8Program = unwrapped->toV8Value();

            v8::TryCatch tryCatchRun(getIsolate());
            if (!v8Program.constGet()->Run(context).ToLocal(&result)) {
                Q_ASSERT(tryCatchRun.HasCaught());
                auto runError = tryCatchRun.Message();
                errorValue = ScriptValue(new ScriptValueV8Wrapper(this, V8ScriptValue(this, runError->Get())));
                raiseException(errorValue, "evaluation error");
                hasFailed = true;
            } else {
                // V8TODO this is just to check if run will always return false for uncaught exception
                Q_ASSERT(!tryCatchRun.HasCaught());
            }
        }
        if(!hasFailed) {
            V8ScriptValue resultValueV8(this, result);
            resultValue = ScriptValue(new ScriptValueV8Wrapper(this, std::move(resultValueV8)));
        }
    }
    _evaluatingCounter--;
    if (hasFailed) {
        return errorValue;
    } else {
        return resultValue;
    }
}


void ScriptEngineV8::updateMemoryCost(const qint64& deltaSize) {
    if (deltaSize > 0) {
        // We've patched qt to fix https://highfidelity.atlassian.net/browse/BUGZ-46 on mac and windows only.
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
        // V8TODO: it seems to be broken in V8 branch on Windows for some reason
        //reportAdditionalMemoryCost(deltaSize);
#endif
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ScriptEngine implementation

ScriptValue ScriptEngineV8::globalObject() {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Context::Scope contextScope(getConstContext());
    V8ScriptValue global(this, getConstContext()->Global());// = QScriptEngine::globalObject(); // can't cache the value as it may change
    return ScriptValue(new ScriptValueV8Wrapper(const_cast<ScriptEngineV8*>(this), std::move(global)));
}

ScriptValue ScriptEngineV8::newArray(uint length) {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Context::Scope contextScope(getContext());
    V8ScriptValue result(this, v8::Array::New(_v8Isolate, static_cast<int>(length)));
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptValue ScriptEngineV8::newArrayBuffer(const QByteArray& message) {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Context::Scope contextScope(getContext());
    std::shared_ptr<v8::BackingStore> backingStore(v8::ArrayBuffer::NewBackingStore(_v8Isolate, message.size()));
    std::memcpy(backingStore.get()->Data(), message.constData(), message.size());
    auto arrayBuffer = v8::ArrayBuffer::New(_v8Isolate, backingStore);
    //V8TODO: this needs to be finished and tested
    /*V8ScriptValue data = QScriptEngine::newVariant(QVariant::fromValue(message));
    V8ScriptValue ctor = QScriptEngine::globalObject().property("ArrayBuffer");
    auto array = qscriptvalue_cast<ArrayBufferClass*>(ctor.data());
    if (!array) {
        return undefinedValue();
    }*/
    V8ScriptValue result(this, arrayBuffer);//QScriptEngine::newObject(array, data);
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptValue ScriptEngineV8::newObject() {
    ScriptValue result;
    {
        v8::Locker locker(_v8Isolate);
        v8::Isolate::Scope isolateScope(_v8Isolate);
        v8::HandleScope handleScope(_v8Isolate);
        v8::Context::Scope contextScope(getContext());
        V8ScriptValue resultV8 = V8ScriptValue(this, v8::Object::New(_v8Isolate));
        result = ScriptValue(new ScriptValueV8Wrapper(this, std::move(resultV8)));
    }
    return result;
}

ScriptValue ScriptEngineV8::newMethod(QObject* object, V8ScriptValue lifetime,
                               const QList<QMetaMethod>& metas, int numMaxParams) {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Context::Scope contextScope(getContext());
    V8ScriptValue result(ScriptMethodV8Proxy::newMethod(this, object, lifetime, metas, numMaxParams));
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptProgramPointer ScriptEngineV8::newProgram(const QString& sourceCode, const QString& fileName) {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Context::Scope contextScope(getContext());
    return std::make_shared<ScriptProgramV8Wrapper>(this, sourceCode, fileName);
}

ScriptValue ScriptEngineV8::newQObject(QObject* object,
                                                    ScriptEngine::ValueOwnership ownership,
                                                    const ScriptEngine::QObjectWrapOptions& options) {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Context::Scope contextScope(getContext());
    V8ScriptValue result = ScriptObjectV8Proxy::newQObject(this, object, ownership, options);
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptValue ScriptEngineV8::newValue(bool value) {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Context::Scope contextScope(getContext());
    V8ScriptValue result(this, v8::Boolean::New(_v8Isolate, value));
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptValue ScriptEngineV8::newValue(int value) {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Context::Scope contextScope(getContext());
    V8ScriptValue result(this, v8::Integer::New(_v8Isolate, value));
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptValue ScriptEngineV8::newValue(uint value) {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Context::Scope contextScope(getContext());
    V8ScriptValue result(this, v8::Uint32::New(_v8Isolate, value));
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptValue ScriptEngineV8::newValue(double value) {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Context::Scope contextScope(getContext());
    V8ScriptValue result(this, v8::Number::New(_v8Isolate, value));
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptValue ScriptEngineV8::newValue(const QString& value) {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Context::Scope contextScope(getContext());
    v8::Local<v8::String> valueV8 = v8::String::NewFromUtf8(_v8Isolate, value.toStdString().c_str(), v8::NewStringType::kNormal).ToLocalChecked();
    V8ScriptValue result(this, valueV8);
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptValue ScriptEngineV8::newValue(const QLatin1String& value) {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Context::Scope contextScope(getContext());
    v8::Local<v8::String> valueV8 = v8::String::NewFromUtf8(_v8Isolate, value.latin1(), v8::NewStringType::kNormal).ToLocalChecked();
    V8ScriptValue result(this, valueV8);
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptValue ScriptEngineV8::newValue(const char* value) {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Context::Scope contextScope(getContext());
    v8::Local<v8::String> valueV8 = v8::String::NewFromUtf8(_v8Isolate, value, v8::NewStringType::kNormal).ToLocalChecked();
    V8ScriptValue result(this, valueV8);
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptValue ScriptEngineV8::newVariant(const QVariant& value) {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Context::Scope contextScope(getContext());
    V8ScriptValue result = castVariantToValue(value);
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptValue ScriptEngineV8::nullValue() {
    return _nullValue;
}

ScriptValue ScriptEngineV8::undefinedValue() {
    return _undefinedValue;
}

void ScriptEngineV8::abortEvaluation() {
    //V8TODO
    //QScriptEngine::abortEvaluation();
}

void ScriptEngineV8::clearExceptions() {
    _uncaughtException.reset();
}

ScriptContext* ScriptEngineV8::currentContext() const {
    // V8TODO: add FunctionCallbackInfo or PropertyCallbackInfo when necessary
    // Is it needed?
    return _contexts.last().get();
}

bool ScriptEngineV8::hasUncaughtException() const {
    return _uncaughtException != nullptr;
}

bool ScriptEngineV8::isEvaluating() const {
    //return QScriptEngine::isEvaluating();
    return _evaluatingCounter > 0;
    return false;
}

ScriptValue ScriptEngineV8::newFunction(ScriptEngine::FunctionSignature fun, int length) {
    //V8TODO is callee() used for anything?

    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    auto context = getContext();
    v8::Context::Scope contextScope(context);

    auto v8FunctionCallback = [](const v8::FunctionCallbackInfo<v8::Value>& info) {
        //V8TODO: is using GetCurrentContext ok, or context wrapper needs to be added?
        v8::HandleScope handleScope(info.GetIsolate());
        auto context = info.GetIsolate()->GetCurrentContext();
        v8::Context::Scope contextScope(context);
        Q_ASSERT(info.Data()->IsObject());
        auto object = v8::Local<v8::Object>::Cast(info.Data());
        Q_ASSERT(object->InternalFieldCount() == 2);
        auto function = reinterpret_cast<ScriptEngine::FunctionSignature>
            (object->GetAlignedPointerFromInternalField(0));
        ScriptEngineV8 *scriptEngine = reinterpret_cast<ScriptEngineV8*>
            (object->GetAlignedPointerFromInternalField(1));
        ContextScopeV8 contextScopeV8(scriptEngine);
        ScriptContextV8Wrapper scriptContext(scriptEngine, &info, scriptEngine->getContext(), scriptEngine->currentContext()->parentContext());
        ScriptContextGuard scriptContextGuard(&scriptContext);
        ScriptValue result = function(&scriptContext, scriptEngine);
        ScriptValueV8Wrapper* unwrapped = ScriptValueV8Wrapper::unwrap(result);
        if (unwrapped) {
            info.GetReturnValue().Set(unwrapped->toV8Value().constGet());
        }
    };
    auto functionDataTemplate = getFunctionDataTemplate();
    auto functionData = functionDataTemplate->NewInstance(context).ToLocalChecked();
    functionData->SetAlignedPointerInInternalField(0, reinterpret_cast<void*>(fun));
    functionData->SetAlignedPointerInInternalField(1, reinterpret_cast<void*>(this));
    auto v8Function = v8::Function::New(context, v8FunctionCallback, functionData, length).ToLocalChecked();
    V8ScriptValue result(this, v8Function);
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

//V8TODO
void ScriptEngineV8::setObjectName(const QString& name) {
    QObject::setObjectName(name);
}

bool ScriptEngineV8::setProperty(const char* name, const QVariant& value) {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Local<v8::Context> context = getContext();
    v8::Context::Scope contextScope(context);
    v8::Local<v8::Object> global = context->Global();
    auto v8Name = v8::String::NewFromUtf8(getIsolate(), name).ToLocalChecked();
    V8ScriptValue v8Value = castVariantToValue(value);
    return global->Set(context, v8Name, v8Value.get()).FromMaybe(false);
}

void ScriptEngineV8::setProcessEventsInterval(int interval) {
    //V8TODO
    //QScriptEngine::setProcessEventsInterval(interval);
}

QThread* ScriptEngineV8::thread() const {
    return QObject::thread();
}

void ScriptEngineV8::setThread(QThread* thread) {
    if (_v8Isolate->IsCurrent()) {
        _v8Isolate->Exit();
        qCDebug(scriptengine_v8) << "Script engine " << objectName() << " exited isolate";
    }
    Q_ASSERT(QObject::thread() == QThread::currentThread());
    moveToThread(thread);
    qCDebug(scriptengine_v8) << "Moved script engine " << objectName() << " to different thread";
}

std::shared_ptr<ScriptException> ScriptEngineV8::uncaughtException() const {
    if (_uncaughtException) {
        return _uncaughtException->clone();
    } else {
        return std::shared_ptr<ScriptException>();
    }
}

bool ScriptEngineV8::raiseException(const QString& error, const QString &reason) {
    return raiseException(newValue(error), reason);
}

bool ScriptEngineV8::raiseException(const ScriptValue& exception, const QString &reason) {
    //V8TODO Im not sure how to finish these
    qCCritical(scriptengine_v8) << "Script exception occurred: " << exception.toString();
    ScriptValueV8Wrapper* unwrapped = ScriptValueV8Wrapper::unwrap(exception);
    ScriptValue qException = unwrapped ? exception : newVariant(exception.toVariant());

    return raiseException(ScriptValueV8Wrapper::fullUnwrap(this, exception));
}

bool ScriptEngineV8::raiseException(const V8ScriptValue& exception) {
    if (!IS_THREADSAFE_INVOCATION(thread(), __FUNCTION__)) {
        return false;
    }

    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Context::Scope contextScope(getContext());

    auto trace = v8::StackTrace::CurrentStackTrace(_v8Isolate, 2);
    if (trace->GetFrameCount() > 0) {
        // we have an active context / JS stack frame so throw the exception per usual
        //currentContext()->throwValue(makeError(exception));
        auto thrown = makeError(ScriptValue(new ScriptValueV8Wrapper(this, exception)));
        auto thrownV8 = ScriptValueV8Wrapper::fullUnwrap(this, thrown);
        _v8Isolate->ThrowException(thrownV8.get());
        return true;
    } else if (_manager) {
        // we are within a pure C++ stack frame (ie: being called directly by other C++ code)
        // in this case no context information is available so just emit the exception for reporting
        ScriptValue thrown = makeError(ScriptValue(new ScriptValueV8Wrapper(this, exception)));
        auto scriptRuntimeException = std::make_shared<ScriptRuntimeException>();
        ScriptValue message = thrown.property("stack"); //This contains more details along with the error message
        scriptRuntimeException->errorMessage = message.toString();
        scriptRuntimeException->thrownValue = thrown;
        emit _manager->unhandledException(scriptRuntimeException);
    }

    return false;
}


ScriptValue ScriptEngineV8::create(int type, const void* ptr) {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Context::Scope contextScope(getContext());
    QVariant variant(type, ptr);
    V8ScriptValue scriptValue = castVariantToValue(variant);
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(scriptValue)));
}

QVariant ScriptEngineV8::convert(const ScriptValue& value, int typeId) {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Context::Scope contextScope(getContext());
    ScriptValueV8Wrapper* unwrapped = ScriptValueV8Wrapper::unwrap(value);
    if (unwrapped == nullptr) {
        return QVariant();
    }

    QVariant var;
    if (!castValueToVariant(unwrapped->toV8Value(), var, typeId)) {
        return QVariant();
    }

    int destType = var.userType();
    if (destType != typeId) {
        var.convert(typeId);  // if conversion fails then var is set to QVariant()
    }

    return var;
    return QVariant();
}

void ScriptEngineV8::compileTest() {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    auto context = getContext();
    v8::Context::Scope contextScope(context);
    v8::Local<v8::Script> script;
    v8::ScriptOrigin scriptOrigin(getIsolate(), v8::String::NewFromUtf8(getIsolate(),"test").ToLocalChecked());
    if (v8::Script::Compile(context, v8::String::NewFromUtf8(getIsolate(), "print(\"hello world\");").ToLocalChecked(), &scriptOrigin).ToLocal(&script)) {
        qCDebug(scriptengine_v8) << "Compile test successful";
    } else {
        qCDebug(scriptengine_v8) << "Compile test failed";
        Q_ASSERT(false);
    }
}

QString ScriptEngineV8::scriptValueDebugDetails(const ScriptValue &value) {
    V8ScriptValue v8Value = ScriptValueV8Wrapper::fullUnwrap(this, value);
    return scriptValueDebugDetailsV8(v8Value);
}

QString ScriptEngineV8::scriptValueDebugListMembers(const ScriptValue &value) {
    V8ScriptValue v8Value = ScriptValueV8Wrapper::fullUnwrap(this, value);
    return scriptValueDebugDetailsV8(v8Value);
}

QString ScriptEngineV8::scriptValueDebugListMembersV8(const V8ScriptValue &v8Value) {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Local<v8::Context> context = getContext();
    v8::Context::Scope contextScope(context);

    QString membersString("");
    if (v8Value.constGet()->IsObject()) {
        v8::Local<v8::String> membersStringV8;
        v8::Local<v8::Object> object = v8::Local<v8::Object>::Cast(v8Value.constGet());
        auto names = object->GetPropertyNames(context).ToLocalChecked();
        if (v8::JSON::Stringify(context, names).ToLocal(&membersStringV8)) {
            membersString = QString(*v8::String::Utf8Value(_v8Isolate, membersStringV8));
        }
        membersString = QString(*v8::String::Utf8Value(_v8Isolate, membersStringV8));
    } else {
        membersString = QString(" Is not an object");
    }
    return membersString;
}

QString ScriptEngineV8::scriptValueDebugDetailsV8(const V8ScriptValue &v8Value) {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    v8::HandleScope handleScope(_v8Isolate);
    v8::Local<v8::Context> context = getContext();
    v8::Context::Scope contextScope(context);

    QString parentValueQString("");
    v8::Local<v8::String> parentValueString;
    if (v8Value.constGet()->ToDetailString(context).ToLocal(&parentValueString)) {
        parentValueQString = QString(*v8::String::Utf8Value(_v8Isolate, parentValueString));
    }
    QString JSONQString;
    v8::Local<v8::String> JSONString;
    if (v8::JSON::Stringify(context, v8Value.constGet()).ToLocal(&JSONString)) {
        JSONQString = QString(*v8::String::Utf8Value(_v8Isolate, JSONString));
    }
    return parentValueQString + QString(" JSON: ") + JSONQString;
}

void ScriptEngineV8::logBacktrace(const QString &title) {
    QStringList backtrace = currentContext()->backtrace();
    qCDebug(scriptengine_v8) << title;
    for (int n = 0; n < backtrace.length(); n++) {
        qCDebug(scriptengine_v8) << backtrace[n];
    }
}

QStringList ScriptEngineV8::getCurrentScriptURLs() const {
    auto isolate = _v8Isolate;
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Context::Scope contextScope(_v8Isolate->GetCurrentContext());
    v8::Local<v8::StackTrace> stackTrace = v8::StackTrace::CurrentStackTrace(isolate, 100);
    QStringList scriptURLs;
    //V8TODO nicer formatting
    for (int i = 0; i < stackTrace->GetFrameCount(); i++) {
        v8::Local<v8::StackFrame> stackFrame = stackTrace->GetFrame(isolate, i);
        scriptURLs.append(QString(*v8::String::Utf8Value(isolate, stackFrame->GetScriptNameOrSourceURL())));
    }
    return scriptURLs;
}

ScriptEngineMemoryStatistics ScriptEngineV8::getMemoryUsageStatistics() {
    v8::Locker locker(_v8Isolate);
    v8::Isolate::Scope isolateScope(_v8Isolate);
    ScriptEngineMemoryStatistics statistics;
    v8::HeapStatistics heapStatistics;
    _v8Isolate->GetHeapStatistics(&heapStatistics);
    statistics.totalHeapSize = heapStatistics.total_available_size();
    statistics.usedHeapSize = heapStatistics.used_heap_size();
    statistics.totalAvailableSize = heapStatistics.total_available_size();
    statistics.totalGlobalHandlesSize = heapStatistics.total_global_handles_size();
    statistics.usedGlobalHandlesSize = heapStatistics.used_global_handles_size();
#ifdef OVERTE_V8_MEMORY_DEBUG
    statistics.scriptValueCount = scriptValueCount;
    statistics.scriptValueProxyCount = scriptValueProxyCount;
    statistics.qObjectCount = _qobjectWrapperMapV8.size();
#endif
    return statistics;
}

void ScriptEngineV8::startCollectingObjectStatistics() {
    auto heapProfiler = _v8Isolate->GetHeapProfiler();
    heapProfiler->StartTrackingHeapObjects();
}

void ScriptEngineV8::dumpHeapObjectStatistics() {
    // V8TODO: this is not very elegant, but very convenient
    QFile dumpFile("/tmp/heap_objectStatistics_dump.csv");
    if (!dumpFile.open(QFile::WriteOnly | QFile::Truncate)) {
        return;
    }
    QTextStream dump(&dumpFile);
    size_t objectTypeCount = _v8Isolate->NumberOfTrackedHeapObjectTypes();
    for (size_t i = 0; i < objectTypeCount; i++) {
        v8::HeapObjectStatistics statistics;
        if (_v8Isolate->GetHeapObjectStatisticsAtLastGC(&statistics, i)) {
            dump << statistics.object_type() << " " << statistics.object_sub_type() << " " << statistics.object_count() << " "
                 << statistics.object_size() << "\n";
        }
    }
}

void ScriptEngineV8::startProfiling() {
    if (_profiler) {
        qWarning(scriptengine_v8) << "ScriptEngineV8::startProfiling: Profiler is already running";
        return;
    }
    _profiler = v8::CpuProfiler::New(_v8Isolate);
    v8::CpuProfilingResult result = _profiler->Start(v8::CpuProfilingOptions());
    if (!result.id) {
        qWarning(scriptengine_v8) << "ScriptEngineV8::startProfiling: Profiler failed to start";
        _profiler->Dispose();
        _profiler = nullptr;
        return;
    }
    qDebug(scriptengine_v8) << "Script profiler started";
    _profilerId = result.id;
};

// Helper function for ScriptEngineV8::stopProfilingAndSave

int getTotalNodeHitCount(const v8::CpuProfileNode *node) {
    int hitCount = node->GetHitCount();
    for (int i = 0; i < node->GetChildrenCount(); i++) {
        hitCount += getTotalNodeHitCount(node->GetChild(i));
    }
    return hitCount;
}

QString getLogFileName() {
    static const QString FILENAME_FORMAT = "overte-profile_%1.csv";
    static const QString DATETIME_FORMAT = "yyyy-MM-dd_hh.mm.ss";
    static const QString LOGS_DIRECTORY = "Logs";

    QString result = FileUtils::standardPath(LOGS_DIRECTORY);
    QDateTime now = QDateTime::currentDateTime();

    result.append(QString(FILENAME_FORMAT).arg(now.toString(DATETIME_FORMAT)));
    return result;
}

void ScriptEngineV8::stopProfilingAndSave() {
    if (!_profiler || !_profilerId) {
        qWarning(scriptengine_v8) << "ScriptEngineV8::stopProfilingAndSave: Profiler is not running";
        return;
    }
    v8::CpuProfile *profile = _profiler->Stop(_profilerId);
    QString filename(getLogFileName());
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        QStringList samples;
        for (int i = 0; i < profile->GetSamplesCount(); i++) {
            QString line;
            QTextStream stream(&line);
            const v8::CpuProfileNode *node = profile->GetSample(i);
            stream << getTotalNodeHitCount(node) << ";"
                   << node->GetHitCount() << ";"
                   << node->GetFunctionNameStr() << ";"
                   << node->GetScriptResourceNameStr() << ";"
                   << node->GetLineNumber() << "\n";
            samples.append(line);
        }
        samples.sort();
        QStringList deduplicated;
        deduplicated.append(samples[0]);
        for (int i=1; i < samples.size(); i++) {
            if (samples[i] != samples[i-1]) {
                deduplicated.append(samples[i]);
            }
        }
        QTextStream fileStream(&file);
        for (QString line : deduplicated) {
            fileStream << line;
        }
    } else {
        qWarning(scriptengine_v8) << "ScriptEngineV8::stopProfilingAndSave: Cannot open output file";
    }
    profile->Delete();
    _profiler->Dispose();
    _profiler = nullptr;
    qDebug(scriptengine_v8) << "Script profiler stopped, results written to: " << filename;
};

ContextScopeV8::ContextScopeV8(ScriptEngineV8 *engine) :
    _engine(engine) {
    Q_ASSERT(engine);
    _isContextChangeNeeded = engine->getContext() != engine->getIsolate()->GetCurrentContext();
    if (_isContextChangeNeeded) {
        engine->pushContext(engine->getIsolate()->GetCurrentContext());
    }
}

ContextScopeV8::~ContextScopeV8() {
    if (_isContextChangeNeeded) {
        _engine->popContext();
    }
}
