//
//  ScriptEngineV8.cpp
//  libraries/script-engine/src/v8
//
//  Created by Brad Hefta-Gaub on 12/14/13.
//  Modified for V8 by dr Karol Suprynowicz on 2022/10/08
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
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

#include "../ScriptEngineLogging.h"
#include "../ScriptProgram.h"
#include "../ScriptValue.h"

#include "ScriptContextV8Wrapper.h"
#include "ScriptObjectV8Proxy.h"
#include "ScriptProgramV8Wrapper.h"
#include "ScriptValueV8Wrapper.h"

static const int MAX_DEBUG_VALUE_LENGTH { 80 };

std::once_flag ScriptEngineV8::_v8InitOnceFlag;
QMutex ScriptEngineV8::_v8InitMutex;

bool ScriptEngineV8::IS_THREADSAFE_INVOCATION(const QThread* thread, const QString& method) {
    const QThread* currentThread = QThread::currentThread();
    if (currentThread == thread) {
        return true;
    }
    qCCritical(scriptengine) << QString("Scripting::%1 @ %2 -- ignoring thread-unsafe call from %3")
                              .arg(method)
                              .arg(thread ? thread->objectName() : "(!thread)")
                              .arg(QThread::currentThread()->objectName());
    qCDebug(scriptengine) << "(please resolve on the calling side by using invokeMethod, executeOnScriptThread, etc.)";
    Q_ASSERT(false);
    return false;
}

// engine-aware JS Error copier and factory
V8ScriptValue ScriptEngineV8::makeError(const V8ScriptValue& _other, const QString& type) {
    if (!IS_THREADSAFE_INVOCATION(thread(), __FUNCTION__)) {
        return V8ScriptValue(_v8Isolate, v8::Null(_v8Isolate));
    }
    return V8ScriptValue(_v8Isolate, v8::Null(_v8Isolate));
    //V8TODO
    /*
    auto other = _other;
    if (_other.constGet()->IsString()) {
        other = QScriptEngine::newObject();
        other.setProperty("message", _other.toString());
    }
    auto proto = QScriptEngine::globalObject().property(type);
    if (!proto.isFunction()) {
        proto = QScriptEngine::globalObject().property(other.prototype().property("constructor").property("name").toString());
    }
    if (!proto.isFunction()) {
#ifdef DEBUG_JS_EXCEPTIONS
        qCDebug(shared) << "BaseScriptEngine::makeError -- couldn't find constructor for" << type << " -- using Error instead";
#endif
        proto = QScriptEngine::globalObject().property("Error");
    }
    if (other.engine() != this) {
        // JS Objects are parented to a specific script engine instance
        // -- this effectively ~clones it locally by routing through a QVariant and back
        other = QScriptEngine::toScriptValue(other.toVariant());
    }
    // ~ var err = new Error(other.message)
    auto err = proto.construct(V8ScriptValueList({ other.property("message") }));

    // transfer over any existing properties
    V8ScriptValueIterator it(other);
    while (it.hasNext()) {
        it.next();
        err.setProperty(it.name(), it.value());
    }
    return err;*/
}

ScriptValue ScriptEngineV8::makeError(const ScriptValue& _other, const QString& type) {
    if (!IS_THREADSAFE_INVOCATION(thread(), __FUNCTION__)) {
        return nullValue();
    }
    return nullValue();
    
    //V8TODO
    //what does makeError actually do?
    /*ScriptValueV8Wrapper* unwrapped = ScriptValueV8Wrapper::unwrap(_other);
    V8ScriptValue other;
    if (_other.isString()) {
        other = QScriptEngine::newObject();
        other.setProperty("message", _other.toString());
    } else if (unwrapped) {
        other = unwrapped->toV8Value();
    } else {
        other = QScriptEngine::newVariant(_other.toVariant());
    }
    V8ScriptValue result = makeError(other, type);
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));*/
}

// check syntax and when there are issues returns an actual "SyntaxError" with the details
ScriptValue ScriptEngineV8::cheskScriptSyntax(ScriptProgramPointer program) {
    if (!IS_THREADSAFE_INVOCATION(thread(), __FUNCTION__)) {
        return nullValue();
    }
    ScriptSyntaxCheckResultPointer syntaxCheck = program->checkSyntax();
    //V8TODO
    if (syntaxCheck->state() != ScriptSyntaxCheckResult::Valid) {
        auto err = globalObject().property("SyntaxError").construct(ScriptValueList({ newValue(syntaxCheck->errorMessage()) }));
        err.setProperty("fileName", program->fileName());
        err.setProperty("lineNumber", syntaxCheck->errorLineNumber());
        err.setProperty("expressionBeginOffset", syntaxCheck->errorColumnNumber());
        err.setProperty("stack", syntaxCheck->errorBacktrace());
        //err.setProperty("stack", currentContext()->backtrace().join(ScriptManager::SCRIPT_BACKTRACE_SEP));
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
/*ScriptValue ScriptEngineV8::lintScript(const QString& sourceCode, const QString& fileName, const int lineNumber) {
    if (!IS_THREADSAFE_INVOCATION(thread(), __FUNCTION__)) {
        return nullValue();
    }
    //V8TODO
    const auto syntaxCheck = checkSyntax(sourceCode);
    if (syntaxCheck.state() != QScriptSyntaxCheckResult::Valid) {
        auto err = QScriptEngine::globalObject().property("SyntaxError").construct(V8ScriptValueList({ syntaxCheck.errorMessage() }));
        err.setProperty("fileName", fileName);
        err.setProperty("lineNumber", syntaxCheck.errorLineNumber());
        err.setProperty("expressionBeginOffset", syntaxCheck.errorColumnNumber());
        err.setProperty("stack", currentContext()->backtrace().join(ScriptManager::SCRIPT_BACKTRACE_SEP));
        {
            const auto error = syntaxCheck.errorMessage();
            const auto line = QString::number(syntaxCheck.errorLineNumber());
            const auto column = QString::number(syntaxCheck.errorColumnNumber());
            // for compatibility with legacy reporting
            const auto message = QString("[SyntaxError] %1 in %2:%3(%4)").arg(error, fileName, line, column);
            err.setProperty("formatted", message);
        }
        return ScriptValue(new ScriptValueV8Wrapper(this, std::move(err)));
    }
    return undefinedValue();
}*/

// this pulls from the best available information to create a detailed snapshot of the current exception
ScriptValue ScriptEngineV8::cloneUncaughtException(const QString& extraDetail) {
    if (!IS_THREADSAFE_INVOCATION(thread(), __FUNCTION__)) {
        return nullValue();
    }
    if (!hasUncaughtException()) {
        return nullValue();
    }
    return nullValue();
    //V8TODO
    /*
    auto exception = uncaughtException();
    // ensure the error object is engine-local
    auto err = makeError(exception);

    // not sure why Qt does't offer uncaughtExceptionFileName -- but the line number
    // on its own is often useless/wrong if arbitrarily married to a filename.
    // when the error object already has this info, it seems to be the most reliable
    auto fileName = exception.property("fileName").toString();
    auto lineNumber = exception.property("lineNumber").toInt32();

    // the backtrace, on the other hand, seems most reliable taken from uncaughtExceptionBacktrace
    auto backtrace = uncaughtExceptionBacktrace();
    if (backtrace.isEmpty()) {
        // fallback to the error object
        backtrace = exception.property("stack").toString().split(ScriptManager::SCRIPT_BACKTRACE_SEP);
    }
    // the ad hoc "detail" property can be used now to embed additional clues
    auto detail = exception.property("detail").toString();
    if (detail.isEmpty()) {
        detail = extraDetail;
    } else if (!extraDetail.isEmpty()) {
        detail += "(" + extraDetail + ")";
    }
    if (lineNumber <= 0) {
        lineNumber = uncaughtExceptionLineNumber();
    }
    if (fileName.isEmpty()) {
        // climb the stack frames looking for something useful to display
        for (auto c = QScriptEngine::currentContext(); c && fileName.isEmpty(); c = c->parentContext()) {
            V8ScriptContextInfo info{ c };
            if (!info.fileName().isEmpty()) {
                // take fileName:lineNumber as a pair
                fileName = info.fileName();
                lineNumber = info.lineNumber();
                if (backtrace.isEmpty()) {
                    backtrace = c->backtrace();
                }
                break;
            }
        }
    }
    err.setProperty("fileName", fileNlintame);
    err.setProperty("lineNumber", lineNumber);
    err.setProperty("detail", detail);
    err.setProperty("stack", backtrace.join(ScriptManager::SCRIPT_BACKTRACE_SEP));

#ifdef DEBUG_JS_EXCEPTIONS
    err.setProperty("_fileName", exception.property("fileName").toString());
    err.setProperty("_stack", uncaughtExceptionBacktrace().join(SCRIPT_BACKTRACE_SEP));
    err.setProperty("_lineNumber", uncaughtExceptionLineNumber());
#endif
    return err;
    */
}

bool ScriptEngineV8::raiseException(const V8ScriptValue& exception) {
    if (!IS_THREADSAFE_INVOCATION(thread(), __FUNCTION__)) {
        return false;
    }
    //V8TODO
    _v8Isolate->ThrowException(makeError(exception).get());
    /*if (QScriptEngine::currentContext()) {
        // we have an active context / JS stack frame so throw the exception per usual
        QScriptEngine::currentContext()->throwValue(makeError(exception));
        return true;
    } else if (_scriptManager) {
        // we are within a pure C++ stack frame (ie: being called directly by other C++ code)
        // in this case no context information is available so just emit the exception for reporting
        V8ScriptValue thrown = makeError(exception);
        emit _scriptManager->unhandledException(ScriptValue(new ScriptValueV8Wrapper(this, std::move(thrown))));
    }*/
    //emit _scriptManager->unhandledException(ScriptValue(new ScriptValueV8Wrapper(this, std::move(thrown))));
    return false;
}

bool ScriptEngineV8::maybeEmitUncaughtException(const QString& debugHint) {
    if (!IS_THREADSAFE_INVOCATION(thread(), __FUNCTION__)) {
        return false;
    }
    if (!isEvaluating() && hasUncaughtException() && _scriptManager) {
        emit _scriptManager->unhandledException(cloneUncaughtException(debugHint));
        clearExceptions();
        return true;
    }
    return false;
}

// Lambda
ScriptValue ScriptEngineV8::newLambdaFunction(std::function<V8ScriptValue(V8ScriptContext*, ScriptEngineV8*)> operation,
                                                 const V8ScriptValue& data,
                                                 const ValueOwnership& ownership) {
    auto lambda = new Lambda(this, operation, data);
    auto object = newQObject(lambda, ownership);
    //V8TODO?
    auto call = object.property("call");
    call.setPrototype(object);  // context->callee().prototype() === Lambda QObject
    call.setData(ScriptValue(new ScriptValueV8Wrapper(this, data)));         // context->callee().data() will === data param
    return call;
}
QString Lambda::toString() const {
    v8::Local<v8::String> string;
    QString qString("");
    if (_data.constGet()->ToString(_engine->getContext()).ToLocal(&string)) {
        v8::String::Utf8Value utf8Value(_engine->getIsolate(), string);
        qString = QString(*utf8Value);
    }
    //V8TODO it was data.isValid() originally
    //I have no idea what happens here
    return QString("[Lambda%1]").arg((!_data.constGet()->IsNullOrUndefined()) ? " " + qString : qString);
}

Lambda::~Lambda() {
#ifdef DEBUG_JS_LAMBDA_FUNCS
    qDebug() << "~Lambda"
             << "this" << this;
#endif
}

Lambda::Lambda(ScriptEngineV8* engine,
               std::function<V8ScriptValue(V8ScriptContext*, ScriptEngineV8*)> operation,
               V8ScriptValue data) :
    _engine(engine),
    _operation(operation), _data(data) {
#ifdef DEBUG_JS_LAMBDA_FUNCS
    qDebug() << "Lambda" << data.toString();
#endif
}
V8ScriptValue Lambda::call() {
    if (!_engine->IS_THREADSAFE_INVOCATION(__FUNCTION__)) {
        return V8ScriptValue(_engine->getIsolate(), v8::Null(_engine->getIsolate()));
    }
    return V8ScriptValue(_engine->getIsolate(), v8::Null(_engine->getIsolate()));
    // V8TODO: something is obviously wrong here, there's no call at all?
    //return operation(static_cast<QScriptEngine*>(engine)->currentContext(), engine);
}

#ifdef DEBUG_JS
void ScriptEngineV8::_debugDump(const QString& header, const V8ScriptValue& object, const QString& footer) {
    if (!IS_THREADSAFE_INVOCATION(thread(), __FUNCTION__)) {
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
    }
}
#endif

v8::Platform* ScriptEngineV8::getV8Platform() {
    static std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    return platform.get();
}

ScriptEngineV8::ScriptEngineV8(ScriptManager* scriptManager) :
    _scriptManager(scriptManager)
    //V8TODO
    //_arrayBufferClass(new ArrayBufferClass(this))
{
    _v8InitMutex.lock();
    std::call_once ( _v8InitOnceFlag, [ ]{ 
        v8::Platform* platform = getV8Platform();
        v8::V8::InitializePlatform(platform);
        v8::V8::Initialize();
    } );
    _v8InitMutex.unlock();
    v8::Isolate::CreateParams isolateParams;
    isolateParams.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    _v8Isolate = v8::Isolate::New(isolateParams);
    v8::Local<v8::Context> context = v8::Context::New(_v8Isolate);
    _v8Context = v8::UniquePersistent<v8::Context>(_v8Isolate, context);

    registerSystemTypes();

    _currentThread = QThread::currentThread();
    // V8TODO: port to V8
    /*
    if (_scriptManager) {
        connect(this, &QScriptEngine::signalHandlerException, this, [this](const V8ScriptValue& exception) {
            if (hasUncaughtException()) {
                // the engine's uncaughtException() seems to produce much better stack traces here
                emit _scriptManager->unhandledException(cloneUncaughtException("signalHandlerException"));
                clearExceptions();
            } else {
                // ... but may not always be available -- so if needed we fallback to the passed exception
                V8ScriptValue thrown = makeError(exception);
                emit _scriptManager->unhandledException(ScriptValue(new ScriptValueV8Wrapper(this, std::move(thrown))));
            }
        }, Qt::DirectConnection);
        moveToThread(scriptManager->thread());
    }*/

    // V8TODO: dispose of isolate on ScriptEngineV8 destruction
    //v8::UniquePersistent<v8::Value> null = v8::UniquePersistent<v8::Value>(_v8Isolate, v8::Null(_v8Isolate));
    //_nullValue = ScriptValue(new ScriptValueV8Wrapper(this, std::move(null)));
    V8ScriptValue nullScriptValue(_v8Isolate, v8::Null(_v8Isolate));
    _nullValue = ScriptValue(new ScriptValueV8Wrapper(this, nullScriptValue));

    //V8ScriptValue undefined = v8::UniquePersistent<v8::Value>(_v8Isolate,v8::Undefined(_v8Isolate));
    //_undefinedValue = ScriptValue(new ScriptValueV8Wrapper(this, std::move(undefined)));
    V8ScriptValue undefined(_v8Isolate, v8::Undefined(_v8Isolate));
    _undefinedValue = ScriptValue(new ScriptValueV8Wrapper(this, undefined));

    // V8TODO: 
    //QScriptEngine::setProcessEventsInterval(MSECS_PER_SECOND);
}

void ScriptEngineV8::registerEnum(const QString& enumName, QMetaEnum newEnum) {
    if (!newEnum.isValid()) {
        qCCritical(scriptengine) << "registerEnum called on invalid enum with name " << enumName;
        return;
    }

    for (int i = 0; i < newEnum.keyCount(); i++) {
        const char* keyName = newEnum.key(i);
        QString fullName = enumName + "." + keyName;
        registerValue(fullName, V8ScriptValue(_v8Isolate, v8::Integer::New(_v8Isolate, newEnum.keyToValue(keyName))));
    }
}

void ScriptEngineV8::registerValue(const QString& valueName, V8ScriptValue value) {
    //V8TODO
    /*if (QThread::currentThread() != QScriptEngine::thread()) {
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine) << "*** WARNING *** ScriptEngineV8::registerValue() called on wrong thread [" << QThread::currentThread() << "], invoking on correct thread [" << thread() << "]";
#endif
        QMetaObject::invokeMethod(this, "registerValue",
                                  Q_ARG(const QString&, valueName),
                                  Q_ARG(V8ScriptValue, value));
        return;
    }*/

    QStringList pathToValue = valueName.split(".");
    int partsToGo = pathToValue.length();
    v8::Local<v8::Object> partObject = _v8Context.Get(_v8Isolate)->Global();

    for (const auto& pathPart : pathToValue) {
        partsToGo--;
        v8::Local<v8::String> pathPartV8 = v8::String::NewFromUtf8(_v8Isolate, pathPart.toStdString().c_str(),v8::NewStringType::kNormal).ToLocalChecked();
        v8::Local<v8::Value> currentPath;
        if (!partObject->Get(_v8Context.Get(_v8Isolate), pathPartV8).ToLocal(&currentPath)) {
            if (partsToGo > 0) {
                //This was commented out
                //QObject *object = new QObject;
                v8::Local<v8::Object> partValue = v8::Object::New(_v8Isolate);  //newQObject(object, QScriptEngine::ScriptOwnership);
                //V8ScriptValue partValue = QScriptEngine::newArray();  //newQObject(object, QScriptEngine::ScriptOwnership);
                Q_ASSERT(partObject->Set(_v8Context.Get(_v8Isolate), pathPartV8, partValue).FromMaybe(false));
            } else {
                //partObject = currentPath->ToObject();
                //V8TODO: do these still happen if asserts are disabled?
                Q_ASSERT(partObject->Set(_v8Context.Get(_v8Isolate), pathPartV8, value.constGet()).FromMaybe(false));
            }
        }
        v8::Local<v8::Value> child;
        Q_ASSERT(partObject->Get(_v8Context.Get(_v8Isolate), pathPartV8).ToLocal(&child));
    }
}

void ScriptEngineV8::registerGlobalObject(const QString& name, QObject* object) {
    //V8TODO
    /*if (QThread::currentThread() != QScriptEngine::thread()) {
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine) << "*** WARNING *** ScriptEngineV8::registerGlobalObject() called on wrong thread [" << QThread::currentThread() << "], invoking on correct thread [" << thread() << "]  name:" << name;
#endif
        QMetaObject::invokeMethod(this, "registerGlobalObject",
                                  Q_ARG(const QString&, name),
                                  Q_ARG(QObject*, object));
        return;
    }
#ifdef THREAD_DEBUGGING
    qCDebug(scriptengine) << "ScriptEngineV8::registerGlobalObject() called on thread [" << QThread::currentThread() << "] name:" << name;
#endif*/
    v8::Local<v8::Object> globalObject = getContext()->Global();
    v8::Local<v8::String> v8Name = v8::String::NewFromUtf8(_v8Isolate, name.toStdString().c_str()).ToLocalChecked();

    // V8TODO: Is IsEmpty check enough or IsValid is needed too?
    if (!globalObject->Get(getContext(), v8Name).IsEmpty()) {
        if (object) {
            V8ScriptValue value = ScriptObjectV8Proxy::newQObject(this, object, ScriptEngine::QtOwnership);
            globalObject->Set(getContext(), v8Name, value.get());
        } else {
            globalObject->Set(getContext(), v8Name, v8::Null(_v8Isolate));
        }
    }
}

void ScriptEngineV8::registerFunction(const QString& name, ScriptEngine::FunctionSignature functionSignature, int numArguments) {
    //V8TODO
    /*if (QThread::currentThread() != QScriptEngine::thread()) {
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine) << "*** WARNING *** ScriptEngineV8::registerFunction() called on wrong thread [" << QThread::currentThread() << "], invoking on correct thread [" << thread() << "] name:" << name;
#endif
        QMetaObject::invokeMethod(this, "registerFunction",
                                  Q_ARG(const QString&, name),
                                  Q_ARG(QScriptEngine::FunctionSignature, functionSignature),
                                  Q_ARG(int, numArguments));
        return;
    }
#ifdef THREAD_DEBUGGING
    qCDebug(scriptengine) << "ScriptEngineV8::registerFunction() called on thread [" << QThread::currentThread() << "] name:" << name;
#endif*/

    //auto scriptFun = static_cast<ScriptValueV8Wrapper*>(newFunction(functionSignature, numArguments).ptr())->toV8Value().constGet();
    auto scriptFun = newFunction(functionSignature, numArguments);
    
    //getContext()->Global().Set();
    globalObject().setProperty(name, scriptFun);
}

void ScriptEngineV8::registerFunction(const QString& parent, const QString& name, ScriptEngine::FunctionSignature functionSignature, int numArguments) {
    //V8TODO
    /*if (QThread::currentThread() != QScriptEngine::thread()) {
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine) << "*** WARNING *** ScriptEngineV8::registerFunction() called on wrong thread [" << QThread::currentThread() << "], invoking on correct thread [" << thread() << "] parent:" << parent << "name:" << name;
#endif
        QMetaObject::invokeMethod(this, "registerFunction",
                                  Q_ARG(const QString&, name),
                                  Q_ARG(QScriptEngine::FunctionSignature, functionSignature),
                                  Q_ARG(int, numArguments));
        return;
    }
#ifdef THREAD_DEBUGGING
    qCDebug(scriptengine) << "ScriptEngineV8::registerFunction() called on thread [" << QThread::currentThread() << "] parent:" << parent << "name:" << name;
#endif*/

    ScriptValue object = globalObject().property(parent);
    if (object.isValid()) {
        ScriptValue scriptFun = ScriptEngine::newFunction(functionSignature, numArguments);
        object.setProperty(name, scriptFun);
    }
}

void ScriptEngineV8::registerGetterSetter(const QString& name, ScriptEngine::FunctionSignature getter,
                                        ScriptEngine::FunctionSignature setter, const QString& parent) {
    //V8TODO
    /*if (QThread::currentThread() != QScriptEngine::thread()) {
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine) << "*** WARNING *** ScriptEngineV8::registerGetterSetter() called on wrong thread [" << QThread::currentThread() << "], invoking on correct thread [" << thread() << "] "
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
    qCDebug(scriptengine) << "ScriptEngineV8::registerGetterSetter() called on thread [" << QThread::currentThread() << "] name:" << name << "parent:" << parent;
#endif*/

    ScriptValue setterFunction = newFunction(setter, 1);
    ScriptValue getterFunction = newFunction(getter);

    if (!parent.isNull() && !parent.isEmpty()) {
        ScriptValue object = ScriptEngine::globalObject().property(parent);
        if (object.isValid()) {
            object.setProperty(name, setterFunction, ScriptValue::PropertySetter);
            object.setProperty(name, getterFunction, ScriptValue::PropertyGetter);
        }
    } else {
        globalObject().setProperty(name, setterFunction, ScriptValue::PropertySetter);
        globalObject().setProperty(name, getterFunction, ScriptValue::PropertyGetter);
    }
}

ScriptValue ScriptEngineV8::evaluateInClosure(const ScriptValue& _closure,
                                                           const ScriptProgramPointer& _program) {
    PROFILE_RANGE(script, "evaluateInClosure");
    if (!IS_THREADSAFE_INVOCATION(thread(), __FUNCTION__)) {
        return nullValue();
    }
    ScriptProgramV8Wrapper* unwrappedProgram = ScriptProgramV8Wrapper::unwrap(_program);
    if (unwrappedProgram == nullptr) {
        return nullValue();
    }
    const V8ScriptProgram& program = unwrappedProgram->toV8Value();

    const auto fileName = unwrappedProgram->fileName();
    const auto shortName = QUrl(fileName).fileName();

    ScriptValueV8Wrapper* unwrappedClosure = ScriptValueV8Wrapper::unwrap(_closure);
    if (unwrappedClosure == nullptr) {
        return nullValue();
    }
    const V8ScriptValue& closure = unwrappedClosure->toV8Value();
    if (!closure.constGet()->IsObject()) {
        return nullValue();
    }
    const v8::Local<v8::Object> closureObject = v8::Local<v8::Object>::Cast(closure.constGet());

    v8::Local<v8::Value> oldGlobal;
    v8::Local<v8::Value> closureGlobal;
    if (!closureObject->Get(closure.constGetContext() ,v8::String::NewFromUtf8(_v8Isolate, "global").ToLocalChecked()).ToLocal(&closureGlobal)) {
        return nullValue();
    }
    
    _v8Context.Get(_v8Isolate)->Exit();
    _v8Context.Get(_v8Isolate)->DetachGlobal();
    oldGlobal = _v8Context.Get(_v8Isolate)->Global();
    v8::Local<v8::Context> closureContext;
    
    if (closureGlobal->IsObject()) {
#ifdef DEBUG_JS
        qCDebug(shared) << " setting global = closure.global" << shortName;
#endif
        closureContext = v8::Context::New(_v8Isolate, nullptr, v8::Local<v8::ObjectTemplate>(), closureGlobal);
        //setGlobalObject(global);
    } else {
        closureContext = v8::Context::New(_v8Isolate, nullptr, v8::Local<v8::ObjectTemplate>(), oldGlobal);
    }

    //auto context = pushContext();

    v8::Local<v8::Value> thiz;
    // V8TODO: not sure if "this" doesn't exist or is empty in some cases
    if (!closureObject->Get(closure.constGetContext() ,v8::String::NewFromUtf8(_v8Isolate, "this").ToLocalChecked()).ToLocal(&thiz)) {
        return nullValue();
    }
    //thiz = closure.property("this");
    if (thiz->IsObject()) {
#ifdef DEBUG_JS
        qCDebug(shared) << " setting this = closure.this" << shortName;
#endif
        //V8TODO I don't know how to do this in V8, will adding "this" to global object work?
        closureContext->Global()->Set(closureContext, v8::String::NewFromUtf8(_v8Isolate, "this").ToLocalChecked(), thiz);
        //context->setThisObject(thiz);
    }

    //context->pushScope(closure);
#ifdef DEBUG_JS
    qCDebug(shared) << QString("[%1] evaluateInClosure %2").arg(isEvaluating()).arg(shortName);
#endif
    ScriptValue result;
    {
        v8::TryCatch tryCatch(getIsolate());
        auto maybeResult = program.constGet()->Run(closureContext);
        v8::Local<v8::Value> v8Result;
        if (!maybeResult.ToLocal(&v8Result)) {
            v8::String::Utf8Value utf8Value(getIsolate(), tryCatch.Exception());
            QString errorMessage = QString(*utf8Value);
            qWarning() << __FUNCTION__ << "---------- hasCaught:" << errorMessage;
            //V8TODO: better error reporting
        }

        if (hasUncaughtException()) {
            auto err = cloneUncaughtException(__FUNCTION__);
#ifdef DEBUG_JS_EXCEPTIONS
            qCWarning(shared) << __FUNCTION__ << "---------- hasCaught:" << err.toString() << result.toString();
            err.setProperty("_result", result);
#endif
            result = err;
        } else {
            result = ScriptValue(new ScriptValueV8Wrapper(this, V8ScriptValue(_v8Isolate, v8Result)));
        }
    }
#ifdef DEBUG_JS
    qCDebug(shared) << QString("[%1] //evaluateInClosure %2").arg(isEvaluating()).arg(shortName);
#endif
    //popContext();
    closureContext->Exit();
    _v8Context.Get(_v8Isolate)->Enter();

    //This is probably unnecessary in V8
    /*if (oldGlobal.isValid()) {
#ifdef DEBUG_JS
        qCDebug(shared) << " restoring global" << shortName;
#endif
        setGlobalObject(oldGlobal);
    }*/

    _v8Context.Get(_v8Isolate)->Enter();
    return result;
}

ScriptValue ScriptEngineV8::evaluate(const QString& sourceCode, const QString& fileName) {
    if (_scriptManager && _scriptManager->isStopped()) {
        return undefinedValue(); // bail early
    }
    
    //V8TODO

    /*if (QThread::currentThread() != QScriptEngine::thread()) {
        ScriptValue result;
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine) << "*** WARNING *** ScriptEngineV8::evaluate() called on wrong thread [" << QThread::currentThread() << "], invoking on correct thread [" << thread() << "] "
            "sourceCode:" << sourceCode << " fileName:" << fileName;
#endif
        BLOCKING_INVOKE_METHOD(this, "evaluate",
                                  Q_RETURN_ARG(ScriptValue, result),
                                  Q_ARG(const QString&, sourceCode),
                                  Q_ARG(const QString&, fileName));
        return result;
    }*/
    // Compile and check syntax
    // V8TODO: Could these all be replaced with checkSyntax function from wrapper?
    v8::TryCatch tryCatch(getIsolate());
    v8::ScriptOrigin scriptOrigin(getIsolate(), v8::String::NewFromUtf8(getIsolate(), fileName.toStdString().c_str()).ToLocalChecked());
    v8::Local<v8::Script> script;
    if (!v8::Script::Compile(getContext(), v8::String::NewFromUtf8(getIsolate(), sourceCode.toStdString().c_str()).ToLocalChecked(), &scriptOrigin).ToLocal(&script)) {
        int errorColumnNumber = 0;
        int errorLineNumber = 0;
        QString errorMessage = "";
        QString errorBacktrace = "";
        v8::String::Utf8Value utf8Value(getIsolate(), tryCatch.Exception());
        errorMessage = QString(*utf8Value);
        v8::Local<v8::Message> exceptionMessage = tryCatch.Message();
        if (!exceptionMessage.IsEmpty()) {
            errorLineNumber = exceptionMessage->GetLineNumber(getContext()).FromJust();
            errorColumnNumber = exceptionMessage->GetStartColumn(getContext()).FromJust();
            v8::Local<v8::Value> backtraceV8String;
            if (tryCatch.StackTrace(getContext()).ToLocal(&backtraceV8String) && backtraceV8String->IsString() &&
                    v8::Local<v8::String>::Cast(backtraceV8String)->Length() > 0) {
                v8::String::Utf8Value backtraceUtf8Value(getIsolate(), backtraceV8String);
                errorBacktrace = *backtraceUtf8Value;
            }
        }
        auto err = makeError(newValue(errorMessage));
        raiseException(err);
        maybeEmitUncaughtException("compile");
        return err;
    }
    
    //V8TODO
    /*auto syntaxError = lintScript(sourceCode, fileName);
    if (syntaxError.isError()) {
        if (!isEvaluating()) {
            syntaxError.setProperty("detail", "evaluate");
        }
        raiseException(syntaxError);
        maybeEmitUncaughtException("lint");
        return syntaxError;
    }*/
    //V8TODO
    /*if (script->IsNull()) {
        // can this happen?
        auto err = makeError(newValue("could not create V8ScriptProgram for " + fileName));
        raiseException(err);
        maybeEmitUncaughtException("compile");
        return err;
    }*/

    v8::Local<v8::Value> result;
    v8::TryCatch tryCatchRun(getIsolate());
    if (!script->Run(getContext()).ToLocal(&result)) {
        Q_ASSERT(tryCatchRun.HasCaught());
        auto runError = tryCatchRun.Message();
        ScriptValue errorValue(new ScriptValueV8Wrapper(this, V8ScriptValue(_v8Isolate, runError->Get())));
        raiseException(errorValue);
        maybeEmitUncaughtException("evaluate");
        return errorValue;
    }
    V8ScriptValue resultValue(_v8Isolate, result);
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(resultValue)));
}

Q_INVOKABLE ScriptValue ScriptEngineV8::evaluate(const ScriptProgramPointer& program) {
    if (_scriptManager && _scriptManager->isStopped()) {
        return undefinedValue(); // bail early
    }

    //V8TODO
    /*if (QThread::currentThread() != QScriptEngine::thread()) {
        ScriptValue result;
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine) << "*** WARNING *** ScriptEngineV8::evaluate() called on wrong thread [" << QThread::currentThread() << "], invoking on correct thread [" << thread() << "] "
            "sourceCode:" << sourceCode << " fileName:" << fileName;
#endif
        BLOCKING_INVOKE_METHOD(this, "evaluate",
                                  Q_RETURN_ARG(ScriptValue, result),
                                  Q_ARG(const ScriptProgramPointer&, program));
        return result;
    }*/

    ScriptProgramV8Wrapper* unwrapped = ScriptProgramV8Wrapper::unwrap(program);
    if (!unwrapped) {
        auto err = makeError(newValue("could not unwrap program"));
        raiseException(err);
        maybeEmitUncaughtException("compile");
        return err;
    }
    ScriptSyntaxCheckResultPointer syntaxCheck = unwrapped->checkSyntax();
    if (syntaxCheck->state() == ScriptSyntaxCheckResult::Error) {
        auto err = makeError(newValue(syntaxCheck->errorMessage()));
        raiseException(err);
        maybeEmitUncaughtException("compile");
        return err;
    }

    const V8ScriptProgram& v8Program = unwrapped->toV8Value();
    // V8TODO
    /*if (qProgram.isNull()) {
        // can this happen?
        auto err = makeError(newValue("requested program is empty"));
        raiseException(err);
        maybeEmitUncaughtException("compile");
        return err;
    }*/

    v8::Local<v8::Value> result;
    v8::TryCatch tryCatchRun(getIsolate());
    if (!v8Program.constGet()->Run(getContext()).ToLocal(&result)) {
        Q_ASSERT(tryCatchRun.HasCaught());
        auto runError = tryCatchRun.Message();
        ScriptValue errorValue(new ScriptValueV8Wrapper(this, V8ScriptValue(_v8Isolate, runError->Get())));
        raiseException(errorValue);
        maybeEmitUncaughtException("evaluate");
        return errorValue;
    }
    V8ScriptValue resultValue(_v8Isolate, result);
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(resultValue)));
}


void ScriptEngineV8::updateMemoryCost(const qint64& deltaSize) {
    if (deltaSize > 0) {
        // We've patched qt to fix https://highfidelity.atlassian.net/browse/BUGZ-46 on mac and windows only.
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
        reportAdditionalMemoryCost(deltaSize);
#endif
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ScriptEngine implementation

ScriptValue ScriptEngineV8::globalObject() const {
    V8ScriptValue global(_v8Isolate, getConstContext()->Global());// = QScriptEngine::globalObject(); // can't cache the value as it may change
    return ScriptValue(new ScriptValueV8Wrapper(const_cast<ScriptEngineV8*>(this), std::move(global)));
}

ScriptManager* ScriptEngineV8::manager() const {
    return _scriptManager;
}

ScriptValue ScriptEngineV8::newArray(uint length) {
    V8ScriptValue result(_v8Isolate, v8::Array::New(_v8Isolate, static_cast<int>(length)));
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptValue ScriptEngineV8::newArrayBuffer(const QByteArray& message) {
    //V8TODO: this will leak memory
    std::shared_ptr<v8::BackingStore> backingStore(v8::ArrayBuffer::NewBackingStore(_v8Isolate, message.size()));
    std::memcpy(backingStore.get()->Data(), message.constData(), message.size());
    auto arrayBuffer = v8::ArrayBuffer::New(_v8Isolate, backingStore);
    /*V8ScriptValue data = QScriptEngine::newVariant(QVariant::fromValue(message));
    V8ScriptValue ctor = QScriptEngine::globalObject().property("ArrayBuffer");
    auto array = qscriptvalue_cast<ArrayBufferClass*>(ctor.data());
    if (!array) {
        return undefinedValue();
    }*/
    V8ScriptValue result(_v8Isolate, arrayBuffer);//QScriptEngine::newObject(array, data);
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptValue ScriptEngineV8::newObject() {
    V8ScriptValue result(_v8Isolate, v8::Object::New(_v8Isolate));
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptValue ScriptEngineV8::newMethod(QObject* object, V8ScriptValue lifetime,
                               const QList<QMetaMethod>& metas, int numMaxParams) {
    V8ScriptValue result(ScriptMethodV8Proxy::newMethod(this, object, lifetime, metas, numMaxParams));
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptProgramPointer ScriptEngineV8::newProgram(const QString& sourceCode, const QString& fileName) {
    //V8TODO: is it used between isolates?
    //V8TODO: should it be compiled on creation?
    //V8ScriptProgram result(sourceCode, fileName);
    return std::make_shared<ScriptProgramV8Wrapper>(this, sourceCode, fileName);
}

ScriptValue ScriptEngineV8::newQObject(QObject* object,
                                                    ScriptEngine::ValueOwnership ownership,
                                                    const ScriptEngine::QObjectWrapOptions& options) {
    V8ScriptValue result = ScriptObjectV8Proxy::newQObject(this, object, ownership, options);
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptValue ScriptEngineV8::newValue(bool value) {
    V8ScriptValue result(_v8Isolate, v8::Boolean::New(_v8Isolate, value));
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptValue ScriptEngineV8::newValue(int value) {
    V8ScriptValue result(_v8Isolate, v8::Integer::New(_v8Isolate, value));
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptValue ScriptEngineV8::newValue(uint value) {
    V8ScriptValue result(_v8Isolate, v8::Uint32::New(_v8Isolate, value));
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptValue ScriptEngineV8::newValue(double value) {
    V8ScriptValue result(_v8Isolate, v8::Number::New(_v8Isolate, value));
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptValue ScriptEngineV8::newValue(const QString& value) {
    v8::Local<v8::String> valueV8 = v8::String::NewFromUtf8(_v8Isolate, value.toStdString().c_str(), v8::NewStringType::kNormal).ToLocalChecked();
    V8ScriptValue result(_v8Isolate, valueV8);
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptValue ScriptEngineV8::newValue(const QLatin1String& value) {
    v8::Local<v8::String> valueV8 = v8::String::NewFromUtf8(_v8Isolate, value.latin1(), v8::NewStringType::kNormal).ToLocalChecked();
    V8ScriptValue result(_v8Isolate, valueV8);
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptValue ScriptEngineV8::newValue(const char* value) {
    v8::Local<v8::String> valueV8 = v8::String::NewFromUtf8(_v8Isolate, value, v8::NewStringType::kNormal).ToLocalChecked();
    V8ScriptValue result(_v8Isolate, valueV8);
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

ScriptValue ScriptEngineV8::newVariant(const QVariant& value) {
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
    //V8TODO
    //QScriptEngine::clearExceptions();
}

ScriptContext* ScriptEngineV8::currentContext() const {
    //V8TODO
    /*V8ScriptContext* localCtx = QScriptEngine::currentContext();
    if (!localCtx) {
        return nullptr;
    }
    if (!_currContext || _currContext->toV8Value() != localCtx) {
        _currContext = std::make_shared<ScriptContextV8Wrapper>(const_cast<ScriptEngineV8*>(this), localCtx);
    }*/
    //_currContext = std::make_shared<ScriptContextV8Wrapper>(const_cast<ScriptEngineV8*>(this), localCtx);
    if (!_currContext) {
        // I'm not sure how to do this without discardin const
        _currContext = std::make_shared<ScriptContextV8Wrapper>(const_cast<ScriptEngineV8*>(this), getConstContext());
    }
    return _currContext.get();
}

bool ScriptEngineV8::hasUncaughtException() const {
    //V8TODO
    //return QScriptEngine::hasUncaughtException();
}

bool ScriptEngineV8::isEvaluating() const {
    //V8TODO
    //return QScriptEngine::isEvaluating();
}

ScriptValue ScriptEngineV8::newFunction(ScriptEngine::FunctionSignature fun, int length) {
    /*auto innerFunc = [](V8ScriptContext* _context, QScriptEngine* _engine) -> V8ScriptValue {
        auto callee = _context->callee();
        QVariant funAddr = callee.property("_func").toVariant();
        ScriptEngine::FunctionSignature fun = reinterpret_cast<ScriptEngine::FunctionSignature>(funAddr.toULongLong());
        ScriptEngineV8* engine = static_cast<ScriptEngineV8*>(_engine);
        ScriptContextV8Wrapper context(engine, _context);
        ScriptValue result = fun(&context, engine);
        ScriptValueV8Wrapper* unwrapped = ScriptValueV8Wrapper::unwrap(result);
        return unwrapped ? unwrapped->toV8Value() : V8ScriptValue();
    };*/
    
    auto v8FunctionCallback = [](const v8::FunctionCallbackInfo<v8::Value>& info) {
        //V8TODO: is using GetCurrentContext ok, or context wrapper needs to be added?
        auto context = info.GetIsolate()->GetCurrentContext();
        auto function = reinterpret_cast<ScriptEngine::FunctionSignature>
            (info.Data()->ToObject(context).ToLocalChecked()->GetAlignedPointerFromInternalField(0));
        ScriptContext *scriptContext = reinterpret_cast<ScriptContext*>
            (info.Data()->ToObject(context).ToLocalChecked()->GetAlignedPointerFromInternalField(1));;
        ScriptEngineV8 *scriptEngine = reinterpret_cast<ScriptEngineV8*>
            (info.Data()->ToObject(context).ToLocalChecked()->GetAlignedPointerFromInternalField(2));;
        ScriptValue result = function(scriptContext, scriptEngine);
        ScriptValueV8Wrapper* unwrapped = ScriptValueV8Wrapper::unwrap(result);
        info.GetReturnValue().Set(unwrapped->toV8Value().constGet());
    };
    //auto functionTemplate = v8::FunctionTemplate::New(_v8Isolate, v8FunctionCallback, v8::Local<v8::Value>(), v8::Local<v8::Signature>(), length);
    //auto functionData = v8::Object::New(_v8Isolate);
    //functionData->setIn
    auto functionDataTemplate = v8::ObjectTemplate::New(_v8Isolate);
    functionDataTemplate->SetInternalFieldCount(3);
    auto functionData = functionDataTemplate->NewInstance(getContext()).ToLocalChecked();
    functionData->SetAlignedPointerInInternalField(0, reinterpret_cast<void*>(fun));
    functionData->SetAlignedPointerInInternalField(1, reinterpret_cast<void*>(this));
    functionData->SetAlignedPointerInInternalField(2, reinterpret_cast<void*>(currentContext()));
    //functionData->SetInternalField(3, v8::Null(_v8Isolate));
    auto v8Function = v8::Function::New(getContext(), v8FunctionCallback, functionData, length).ToLocalChecked();
    //auto functionObjectTemplate = functionTemplate->InstanceTemplate();
    //auto function = 
    V8ScriptValue result(_v8Isolate, v8Function); // = QScriptEngine::newFunction(innerFunc, length);
    //auto funAddr = QScriptEngine::newVariant(QVariant(reinterpret_cast<qulonglong>(fun)));
    // V8TODO
    //result.setProperty("_func", funAddr, V8ScriptValue::PropertyFlags(V8ScriptValue::ReadOnly + V8ScriptValue::Undeletable + V8ScriptValue::SkipInEnumeration));
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(result)));
}

//V8TODO
/*void ScriptEngineV8::setObjectName(const QString& name) {
    QScriptEngine::setObjectName(name);
}*/

//V8TODO
bool ScriptEngineV8::setProperty(const char* name, const QVariant& value) {
    v8::Local<v8::Object> global = getContext()->Global();
    auto v8Name = v8::String::NewFromUtf8(getIsolate(), name).ToLocalChecked();
    V8ScriptValue v8Value = castVariantToValue(value);
    return global->Set(getContext(), v8Name, v8Value.get()).FromMaybe(false);
}

void ScriptEngineV8::setProcessEventsInterval(int interval) {
    //V8TODO
    //QScriptEngine::setProcessEventsInterval(interval);
}

QThread* ScriptEngineV8::thread() const {
    return _currentThread;
}

void ScriptEngineV8::setThread(QThread* thread) {
    moveToThread(thread);
}

ScriptValue ScriptEngineV8::uncaughtException() const {
    //V8TODO
    //V8ScriptValue result = QScriptEngine::uncaughtException();
    //return ScriptValue(new ScriptValueV8Wrapper(const_cast<ScriptEngineV8*>(this), std::move(result)));
}

QStringList ScriptEngineV8::uncaughtExceptionBacktrace() const {
    //V8TODO
    //return QScriptEngine::uncaughtExceptionBacktrace();
}

int ScriptEngineV8::uncaughtExceptionLineNumber() const {
    //V8TODO
    //return QScriptEngine::uncaughtExceptionLineNumber();
}

bool ScriptEngineV8::raiseException(const ScriptValue& exception) {
    //V8TODO
    Q_ASSERT(false);
    /*ScriptValueV8Wrapper* unwrapped = ScriptValueV8Wrapper::unwrap(exception);
    V8ScriptValue qException = unwrapped ? unwrapped->toV8Value() : QScriptEngine::newVariant(exception.toVariant());
    return raiseException(qException);*/
}

ScriptValue ScriptEngineV8::create(int type, const void* ptr) {
    QVariant variant(type, ptr);
    V8ScriptValue scriptValue = castVariantToValue(variant);
    return ScriptValue(new ScriptValueV8Wrapper(this, std::move(scriptValue)));
}

QVariant ScriptEngineV8::convert(const ScriptValue& value, int typeId) {
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
}
