//
//  ScriptManager.cpp
//  libraries/script-engine/src
//
//  Created by Brad Hefta-Gaub on 12/14/13.
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "ScriptManager.h"

#include <chrono>
#include <thread>

#include <QtCore/QCoreApplication>
#include <QtCore/QEventLoop>
#include <QtCore/QFileInfo>
#include <QtCore/QTimer>
#include <QtCore/QThread>
#include <QtCore/QRegularExpression>

#include <QtCore/QFuture>
#include <QtConcurrent/QtConcurrentRun>

#include <shared/LocalFileAccessGate.h>
#include <shared/AbstractLoggerInterface.h>
#include <DebugDraw.h>
#include <MessagesClient.h>
#include <OctreeConstants.h>
#include <PathUtils.h>
#include <PortableHighResolutionClock.h>
#include <ResourceCache.h>
#include <ResourceManager.h>
#include <ResourceScriptingInterface.h>
#include <UserActivityLoggerScriptingInterface.h>
#include <NodeList.h>

#include "AssetScriptingInterface.h"
#include "BatchLoader.h"
#include "EventTypes.h"
#include "FileScriptingInterface.h" // unzip project
#include "MenuItemProperties.h"
#include "ScriptCache.h"
#include "ScriptContext.h"
#include "XMLHttpRequestClass.h"
#include "WebSocketClass.h"
#include "ScriptEngine.h"
#include "ScriptEngineCast.h"
#include "ScriptEngineLogging.h"
#include "ScriptEngines.h"
#include "StackTestScriptingInterface.h"
#include "ScriptValue.h"
#include "ScriptProgram.h"
#include "ScriptValueIterator.h"
#include "ScriptValueUtils.h"
#include "ScriptManagerScriptingInterface.h"

#include <Profile.h>

#include "MIDIEvent.h"

#include "SettingHandle.h"
#include <AddressManager.h>
#include <NetworkingConstants.h>
#include <ThreadHelpers.h>

const QString ScriptManager::_SETTINGS_ENABLE_EXTENDED_EXCEPTIONS {
    "com.highfidelity.experimental.enableExtendedJSExceptions"
};

const QString ScriptManager::SCRIPT_EXCEPTION_FORMAT{ "[%0] %1 in %2:%3" };
const QString ScriptManager::SCRIPT_BACKTRACE_SEP{ "\n    " };

static const int MAX_MODULE_ID_LENGTH { 4096 };
static const int MAX_DEBUG_VALUE_LENGTH { 80 };

static const ScriptValue::PropertyFlags READONLY_PROP_FLAGS{ ScriptValue::ReadOnly | ScriptValue::Undeletable };
static const ScriptValue::PropertyFlags READONLY_HIDDEN_PROP_FLAGS{ READONLY_PROP_FLAGS | ScriptValue::SkipInEnumeration };

static const bool HIFI_AUTOREFRESH_FILE_SCRIPTS { true };

int scriptManagerPointerMetaID = qRegisterMetaType<ScriptManagerPointer>();

Q_DECLARE_METATYPE(ExternalResource::Bucket);

Q_DECLARE_METATYPE(ScriptValue);

// --- Static script initialization registry

static ScriptManager::StaticInitializerNode* rootInitializer = nullptr;
static ScriptManager::StaticTypesInitializerNode* rootTypesInitializer = nullptr;


using ScriptableResourceRawPtr = ScriptableResource*;
ScriptValue externalResourceBucketToScriptValue(ScriptEngine* engine, ExternalResource::Bucket const& in);
bool externalResourceBucketFromScriptValue(const ScriptValue& object, ExternalResource::Bucket& out);
static ScriptValue scriptableResourceToScriptValue(ScriptEngine* engine,
                                                    const ScriptableResourceRawPtr& resource);
static bool scriptableResourceFromScriptValue(const ScriptValue& value, ScriptableResourceRawPtr& resource);
STATIC_SCRIPT_TYPES_INITIALIZER((+[](ScriptManager* manager){
    auto scriptEngine = manager->engine().get();

    scriptRegisterMetaType<ExternalResource::Bucket, externalResourceBucketToScriptValue, externalResourceBucketFromScriptValue>(scriptEngine);

    scriptRegisterMetaType<ScriptableResourceRawPtr, scriptableResourceToScriptValue, scriptableResourceFromScriptValue>(scriptEngine);
}));

void ScriptManager::registerNewStaticInitializer(StaticInitializerNode* dest) {
    // this function is assumed to be called on LoadLibrary, where we are explicitly operating in single-threaded mode
    // Therefore there is no mutex or threadsafety here and the structure is assumed not to change after loading
    dest->prev = rootInitializer;
    rootInitializer = dest;
}
static void runStaticInitializers(ScriptManager* manager) {
    ScriptManager::StaticInitializerNode* here = rootInitializer;
    while (here != nullptr) {
        (*here->init)(manager);
        here = here->prev;
    }
}
void ScriptManager::registerNewStaticTypesInitializer(StaticTypesInitializerNode* dest) {
    // this function is assumed to be called on LoadLibrary, where we are explicitly operating in single-threaded mode
    // Therefore there is no mutex or threadsafety here and the structure is assumed not to change after loading
    dest->prev = rootTypesInitializer;
    rootTypesInitializer = dest;
}
static void runStaticTypesInitializers(ScriptManager* manager) {
    ScriptManager::StaticTypesInitializerNode* here = rootTypesInitializer;
    while (here != nullptr) {
        (*here->init)(manager);
        here = here->prev;
    }
}

// ---

static ScriptValue debugPrint(ScriptContext* context, ScriptEngine* engine) {
    // assemble the message by concatenating our arguments
    QString message = "";
    for (int i = 0; i < context->argumentCount(); i++) {
        if (i > 0) {
            message += " ";
        }
        message += context->argument(i).toString();
    }

    // was this generated by a script engine? If we don't recognize it then send the message and exit
    ScriptManager* scriptManager = engine->manager();
    if (!scriptManager) {
        qCDebug(scriptengine_script, "%s", qUtf8Printable(message));
        return ScriptValue();
    }

    // This message was sent by one of our script engines, let's try to see if we can find the source.
    // Note that the first entry in the backtrace should be "print" and is somewhat useless to us
    AbstractLoggerInterface* loggerInterface = AbstractLoggerInterface::get();
    if (loggerInterface && loggerInterface->showSourceDebugging()) {
        ScriptContext* userContext = context;
        ScriptContextPointer parentContext; // using this variable to maintain parent variable lifespan
        while (userContext && userContext->functionContext()->functionType() == ScriptFunctionContext::NativeFunction) {
            parentContext = userContext->parentContext();
            userContext = parentContext.get();
        }
        QString location;
        if (userContext) {
            auto contextInfo = userContext->functionContext();
            QString fileName = contextInfo->fileName();
            int lineNumber = contextInfo->lineNumber();
            QString functionName = contextInfo->functionName();

            location = functionName;
            if (!fileName.isEmpty()) {
                if (location.isEmpty()) {
                    location = fileName;
                } else {
                    location = QString("%1 at %2").arg(location).arg(fileName);
                }
            }
            if (lineNumber != -1) {
                location = QString("%1:%2").arg(location).arg(lineNumber);
            }
        }
        if (location.isEmpty()) {
            location = scriptManager->getFilename();
        }

        // give the script engine a chance to notify the system about this message
        scriptManager->print(message);

        // send the message to debug log
        qCDebug(scriptengine_script, "[%s] %s", qUtf8Printable(location), qUtf8Printable(message));
    } else {
        scriptManager->print(message);
        // prefix the script engine name to help disambiguate messages in the main debug log
        qCDebug(scriptengine_script, "[%s] %s", qUtf8Printable(scriptManager->getFilename()), qUtf8Printable(message));
    }

    return ScriptValue();
}

// FIXME Come up with a way to properly encode entity IDs in filename
// The purpose of the following two function is to embed entity ids into entity script filenames
// so that they show up in stacktraces
//
// Extract the url portion of a url that has been encoded with encodeEntityIdIntoEntityUrl(...)
QString extractUrlFromEntityUrl(const QString& url) {
    auto parts = url.split(' ', Qt::SkipEmptyParts);
    if (parts.length() > 0) {
        return parts[0];
    } else {
        return "";
    }
}

// Encode an entity id into an entity url
// Example: http://www.example.com/some/path.js [EntityID:{9fdd355f-d226-4887-9484-44432d29520e}]
QString encodeEntityIdIntoEntityUrl(const QString& url, const QString& entityID) {
    return url + " [EntityID:" + entityID + "]";
}

QString ScriptManager::logException(const ScriptValue& exception) {
    auto message = formatException(exception, _enableExtendedJSExceptions.get());
    scriptErrorMessage(message);
    return message;
}

std::shared_ptr<ScriptException> ScriptManager::getUncaughtException() const {
    return _engine->uncaughtException();
}

ScriptManagerPointer scriptManagerFactory(ScriptManager::Context context,
                                                 const QString& scriptContents,
                                                 const QString& fileNameString) {
    ScriptManagerPointer manager = newScriptManager(context, scriptContents, fileNameString);
    auto scriptEngines = DependencyManager::get<ScriptEngines>();
    scriptEngines->addScriptEngine(manager);
    manager->setScriptEngines(scriptEngines);
    return manager;
}

ScriptManagerPointer newScriptManager(ScriptManager::Context context,
                                      const QString& scriptContents,
                                      const QString& fileNameString) {
    ScriptManagerPointer manager(new ScriptManager(context, scriptContents, fileNameString),
                                 [](ScriptManager* obj) { obj->deleteLater(); });
    return manager;
}

int ScriptManager::processLevelMaxRetries { ScriptRequest::MAX_RETRIES };

ScriptManager::ScriptManager(Context context, const QString& scriptContents, const QString& fileNameString) :
    QObject(),
    _context(context),
    _engine(newScriptEngine(this)),
    _scriptContents(scriptContents),
    _timerFunctionMap(),
    _fileNameString(fileNameString),
    _assetScriptingInterface(new AssetScriptingInterface(this))
{

    switch (_context) {
        case Context::CLIENT_SCRIPT:
            _type = Type::CLIENT;
            break;
        case Context::ENTITY_CLIENT_SCRIPT:
            _type = Type::ENTITY_CLIENT;
            break;
        case Context::ENTITY_SERVER_SCRIPT:
            _type = Type::ENTITY_SERVER;
            break;
        case Context::AGENT_SCRIPT:
            _type = Type::AGENT;
            break;
        case Context::NETWORKLESS_TEST_SCRIPT:
            _type = Type::NETWORKLESS_TEST;
            break;
    }

    qRegisterMetaType<ScriptValue>();
    qRegisterMetaType<std::function<void()>>();

    _scriptingInterface = std::make_shared<ScriptManagerScriptingInterface>(this);

    if (isEntityServerScript()) {
        qCDebug(scriptengine) << "isEntityServerScript() -- limiting maxRetries to 1";
        processLevelMaxRetries = 1;
    }


#if 0
    // V8TODO: Is this actually needed? Exceptions will already be logged on the
    // script engine side.

    // this is where all unhandled exceptions end up getting logged
    connect(this, &ScriptManager::unhandledException, this, [this](const ScriptValue& err) {
        qCCritical(scriptengine) << "Caught exception";

        auto output = err.engine() == _engine ? err : _engine->makeError(err);
        if (!output.property("detail").isValid()) {
            output.setProperty("detail", "UnhandledException");
        }

        // Unhandled exception kills the running script
        if (_abortOnUncaughtException) {
            stop(false);
            logException(output);
        }
    });
#endif

    // Forward exceptions from the scripting engine
    connect(_engine.get(), &ScriptEngine::exception, this, &ScriptManager::unhandledException);

    if (_type == Type::ENTITY_CLIENT || _type == Type::ENTITY_SERVER) {
        QObject::connect(this, &ScriptManager::update, this, [this]() {
            // process pending entity script content
            if (!_contentAvailableQueue.empty() && !(_isFinished || _isStopping)) {
                EntityScriptContentAvailableMap pending;
                std::swap(_contentAvailableQueue, pending);
                for (auto& pair : pending) {
                    auto& args = pair.second;
                    entityScriptContentAvailable(args.entityID, args.scriptOrURL, args.contents, args.isURL, args.success, args.status);
                }
            }
        });
    }

    if (!_areMetaTypesInitialized) {
        initMetaTypes();
    }
}

QString ScriptManager::getTypeAsString() const {
    auto value = QVariant::fromValue(_type).toString();
    return value.isEmpty() ? "unknown" : value.toLower();
}

QString ScriptManager::getContext() const {
    switch (_context) {
        case CLIENT_SCRIPT:
            return "client";
        case ENTITY_CLIENT_SCRIPT:
            return "entity_client";
        case ENTITY_SERVER_SCRIPT:
            return "entity_server";
        case AGENT_SCRIPT:
            return "agent";
        case NETWORKLESS_TEST_SCRIPT:
            return "networkless_test";
        default:
            return "unknown";
    }
    return "unknown";
}

bool ScriptManager::isDebugMode() const {
#if defined(DEBUG)
    return true;
#else
    return false;
#endif
}

ScriptManager::~ScriptManager() {}

void ScriptManager::disconnectNonEssentialSignals() {
    disconnect();
    QThread* workerThread;
    // Ensure the thread should be running, and does exist
    if (_isRunning && _isThreaded && (workerThread = thread())) {
        connect(this, &QObject::destroyed, workerThread, &QThread::quit);
        connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);
    }
}

void ScriptManager::runInThread() {
    Q_ASSERT_X(!_isThreaded, "ScriptManager::runInThread()", "runInThread should not be called more than once");

    if (_isThreaded) {
        return;
    }

    _isThreaded = true;

    // The thread interface cannot live on itself, and we want to move this into the thread, so
    // the thread cannot have this as a parent.
    QThread* workerThread = new QThread();
    QString name = QString("js:") + getFilename().replace("about:","");
    workerThread->setObjectName(name);
    _engine->setThread(workerThread);
    moveToThread(workerThread);

    // NOTE: If you connect any essential signals for proper shutdown or cleanup of
    // the script engine, make sure to add code to "reconnect" them to the
    // disconnectNonEssentialSignals() method
    connect(workerThread, &QThread::started, this, [this, name] {
        setThreadName(name.toStdString());
        run();
    });
    connect(this, &QObject::destroyed, workerThread, &QThread::quit);
    connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);

    workerThread->start();
}

void ScriptManager::executeOnScriptThread(std::function<void()> function, const Qt::ConnectionType& type ) {
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, "executeOnScriptThread", type, Q_ARG(std::function<void()>, function));
        return;
    }

    function();
}

void ScriptManager::waitTillDoneRunning(bool shutdown) {
    // Engine should be stopped already, but be defensive
    stop();

    auto workerThread = thread();

    if (workerThread == QThread::currentThread()) {
        qCWarning(scriptengine) << "ScriptManager::waitTillDoneRunning called, but the script is on the same thread:" << getFilename();
        return;
    }

    if (_isThreaded && workerThread) {
        // We should never be waiting (blocking) on our own thread
        assert(workerThread != QThread::currentThread());

#if 0
        // 26 Feb 2021 - Disabled this OSX-specific code because it causes OSX to crash on shutdown; without this code, OSX
        // doesn't crash on shutdown. Qt 5.12.3 and Qt 5.15.2.
        //
        // On mac, don't call QCoreApplication::processEvents() here. This is to prevent
        // [NSApplication terminate:] from prematurely destroying the static destructors
        // while we are waiting for the scripts to shutdown. We will pump the message
        // queue later in the Application destructor.
        if (workerThread->isRunning()) {
            workerThread->quit();

            if (_engine->isEvaluating()) {
                qCWarning(scriptengine) << "Script Engine has been running too long, aborting:" << getFilename();
                _engine->abortEvaluation();
            } else {
                auto context = _engine->currentContext();
                if (context) {
                    qCWarning(scriptengine) << "Script Engine has been running too long, throwing:" << getFilename();
                    context->throwError("Timed out during shutdown");
                }
            }

            // Wait for the scripting thread to stop running, as
            // flooding it with aborts/exceptions will persist it longer
            static const auto MAX_SCRIPT_QUITTING_TIME = 0.5 * MSECS_PER_SECOND;
            if (!workerThread->wait(MAX_SCRIPT_QUITTING_TIME)) {
                workerThread->terminate();
            }
        }
#else
        auto startedWaiting = usecTimestampNow();
        while (workerThread->isRunning()) {
            // If the final evaluation takes too long, then tell the script engine to stop running
            auto elapsedUsecs = usecTimestampNow() - startedWaiting;
            // V8TODO: temporarily increased script timeout. Maybe use different timeouts for release and unoptimized debug?
            static const auto MAX_SCRIPT_EVALUATION_TIME = USECS_PER_SECOND;
            if (elapsedUsecs > MAX_SCRIPT_EVALUATION_TIME) {
                workerThread->quit();

                if (_engine->isEvaluating()) {
                    qCWarning(scriptengine) << "Script Engine has been running too long (evaluation), aborting:" << getFilename();
                    _engine->abortEvaluation();
                } else {
                    auto context = _engine->currentContext();
                    if (context) {
                        qCWarning(scriptengine) << "Script Engine has been running too long (event loop), throwing:" << getFilename();
                        context->throwError("Timed out during shutdown");
                    }
                }

                // Wait for the scripting thread to stop running, as
                // flooding it with aborts/exceptions will persist it longer
                static const auto MAX_SCRIPT_QUITTING_TIME = 0.5 * MSECS_PER_SECOND;
                if (!workerThread->wait(MAX_SCRIPT_QUITTING_TIME)) {
                    Q_ASSERT(false);
                    workerThread->terminate();
                }
            }

            if (shutdown) {
                // NOTE: This will be called on the main application thread (among other threads) from stopAllScripts.
                //       The thread will need to continue to process events, because
                //       the scripts will likely need to marshall messages across to the main thread, e.g.
                //       if they access Settings or Menu in any of their shutdown code. So:
                // Process events for this thread, allowing invokeMethod calls to pass between threads.
                QCoreApplication::processEvents();
            }

            // Avoid a pure busy wait
            QThread::yieldCurrentThread();
        }
#endif

        scriptInfoMessage("Script Engine has stopped:" + getFilename());
    }
}

QString ScriptManager::getFilename() const {
    QStringList fileNameParts = _fileNameString.split("/");
    QString lastPart;
    if (!fileNameParts.isEmpty()) {
        lastPart = fileNameParts.last();
    }
    return lastPart;
}

bool ScriptManager::hasValidScriptSuffix(const QString& scriptFileName) {
    QFileInfo fileInfo(scriptFileName);
    QString scriptSuffixToLower = fileInfo.completeSuffix().toLower();
    return scriptSuffixToLower.contains(QString("js"), Qt::CaseInsensitive);
}

void ScriptManager::loadURL(const QUrl& scriptURL, bool reload) {
    if (_isRunning) {
        return;
    }

    QUrl url = expandScriptUrl(scriptURL);
    _fileNameString = url.toString();
    _isReloading = reload;

    // Check that script has a supported file extension
    if (!hasValidScriptSuffix(_fileNameString)) {
        scriptErrorMessage("File extension of file: " + _fileNameString + " is not a currently supported script type");
        emit errorLoadingScript(_fileNameString);
        return;
    }

    const auto maxRetries = 0; // for consistency with previous scriptCache->getScript() behavior
    auto scriptCache = DependencyManager::get<ScriptCache>();
    scriptCache->getScriptContents(url.toString(), [this](const QString& url, const QString& scriptContents, bool isURL, bool success, const QString&status) {
        qCDebug(scriptengine) << "loadURL" << url << status << QThread::currentThread();
        if (!success) {
            scriptErrorMessage("ERROR Loading file (" + status + "):" + url);
            emit errorLoadingScript(_fileNameString);
            return;
        }

        _scriptContents = scriptContents;

        emit scriptLoaded(url);
    }, reload, maxRetries);
}

void ScriptManager::scriptErrorMessage(const QString& message) {
    qCCritical(scriptengine, "[%s] %s", qUtf8Printable(getFilename()), qUtf8Printable(message));
    emit errorMessage(message, getFilename());
}

void ScriptManager::scriptWarningMessage(const QString& message) {
    qCWarning(scriptengine, "[%s] %s", qUtf8Printable(getFilename()), qUtf8Printable(message));
    emit warningMessage(message, getFilename());
}

void ScriptManager::scriptInfoMessage(const QString& message) {
    qCInfo(scriptengine, "[%s] %s", qUtf8Printable(getFilename()), qUtf8Printable(message));
    emit infoMessage(message, getFilename());
}

void ScriptManager::scriptPrintedMessage(const QString& message) {
    qCDebug(scriptengine, "[%s] %s", qUtf8Printable(getFilename()), qUtf8Printable(message));
    emit printedMessage(message, getFilename());
}

void ScriptManager::clearDebugLogWindow() {
    emit clearDebugWindow();
}

// Templated qScriptRegisterMetaType fails to compile with raw pointers
using ScriptableResourceRawPtr = ScriptableResource*;

static ScriptValue scriptableResourceToScriptValue(ScriptEngine* engine,
                                                    const ScriptableResourceRawPtr& resource) {
    if (!resource) {
        return ScriptValue();  // probably shutting down
    }

    // The first script to encounter this resource will track its memory.
    // In this way, it will be more likely to GC.
    // This fails in the case that the resource is used across many scripts, but
    // in that case it would be too difficult to tell which one should track the memory, and
    // this serves the common case (use in a single script).
    auto data = resource->getResource();
    auto manager = engine->manager();
    if (data && manager && !resource->isInScript()) {
        resource->setInScript(true);
        QObject::connect(data.data(), &Resource::updateSize, manager, &ScriptManager::updateMemoryCost);
    }

    auto object = engine->newQObject(const_cast<ScriptableResourceRawPtr>(resource), ScriptEngine::ScriptOwnership);
    return object;
}

static bool scriptableResourceFromScriptValue(const ScriptValue& value, ScriptableResourceRawPtr& resource) {
    resource = static_cast<ScriptableResourceRawPtr>(value.toQObject());
    return true;
}

/*@jsdoc
 * The <code>Resource</code> API provides values that define the possible loading states of a resource.
 *
 * @namespace Resource
 *
 * @hifi-interface
 * @hifi-client-entity
 * @hifi-avatar
 * @hifi-server-entity
 * @hifi-assignment-client
 *
 * @property {Resource.State} State - The possible loading states of a resource. <em>Read-only.</em>
 */
static ScriptValue createScriptableResourcePrototype(ScriptManagerPointer manager) {
    auto engine = manager->engine();
    auto prototype = engine->newObject();

    auto state = engine->newObject();
    auto metaEnum = QMetaEnum::fromType<ScriptableResource::State>();
    for (int i = 0; i < metaEnum.keyCount(); ++i) {
        state.setProperty(metaEnum.key(i), metaEnum.value(i));
    }

    prototype.setProperty("State", state);

    return prototype;
}

ScriptValue externalResourceBucketToScriptValue(ScriptEngine* engine, ExternalResource::Bucket const& in) {
    return engine->newValue((int)in);
}

bool externalResourceBucketFromScriptValue(const ScriptValue& object, ExternalResource::Bucket& out) {
    out = static_cast<ExternalResource::Bucket>(object.toInt32());
    return true;
}

void ScriptManager::resetModuleCache(bool deleteScriptCache) {
    if (QThread::currentThread() != thread()) {
        executeOnScriptThread([=]() { resetModuleCache(deleteScriptCache); });
        return;
    }
    auto jsRequire = _engine->globalObject().property("Script").property("require");
    auto cache = jsRequire.property("cache");
    auto cacheMeta = jsRequire.data();

    if (deleteScriptCache) {
        auto it = cache.newIterator();
        while (it->hasNext()) {
            it->next();
            if (it->flags() & ScriptValue::SkipInEnumeration) {
                continue;
            }
            qCDebug(scriptengine) << "resetModuleCache(true) -- staging " << it->name() << " for cache reset at next require";
            cacheMeta.setProperty(it->name(), true);
        }
    }
    cache = _engine->newObject();
    if (!cacheMeta.isObject()) {
        cacheMeta = _engine->newObject();
        cacheMeta.setProperty("id", "Script.require.cacheMeta");
        cacheMeta.setProperty("type", "cacheMeta");
        jsRequire.setData(cacheMeta);
    }
    cache.setProperty("__created__", (double)QDateTime::currentMSecsSinceEpoch(), ScriptValue::SkipInEnumeration);
#if DEBUG_JS_MODULES
    cache.setProperty("__meta__", cacheMeta, READONLY_HIDDEN_PROP_FLAGS);
#endif
    jsRequire.setProperty("cache", cache, READONLY_PROP_FLAGS);
}

void ScriptManager::initMetaTypes() {
    if (_areMetaTypesInitialized) {
        return;
    }
    _areMetaTypesInitialized = true;
    runStaticTypesInitializers(this);
}

void ScriptManager::init() {
    if (_isInitialized) {
        return; // only initialize once
    }

    _isInitialized = true;

    if (_context != NETWORKLESS_TEST_SCRIPT) {
        // This initializes a bunch of systems that want network access. We
        // want to avoid it in test script mode.
        runStaticInitializers(this);
    }

    auto scriptEngine = _engine.get();

    if (_context != NETWORKLESS_TEST_SCRIPT) {
        // For test scripts we want to minimize the amount of functionality available, for the least
        // amount of dependencies and faster test system startup.

        ScriptValue xmlHttpRequestConstructorValue = scriptEngine->newFunction(XMLHttpRequestClass::constructor);
        scriptEngine->globalObject().setProperty("XMLHttpRequest", xmlHttpRequestConstructorValue);

        ScriptValue webSocketConstructorValue = scriptEngine->newFunction(WebSocketClass::constructor);
        scriptEngine->globalObject().setProperty("WebSocket", webSocketConstructorValue);
    }

    /*@jsdoc
     * Prints a message to the program log and emits {@link Script.printedMessage}.
     * The message logged is the message values separated by spaces.
     * <p>Alternatively, you can use {@link Script.print} or one of the {@link console} API methods.</p>
     * @function print
     * @param {...*} [message] - The message values to print.
     */
    scriptEngine->globalObject().setProperty("print", scriptEngine->newFunction(debugPrint));

    // NOTE: You do not want to end up creating new instances of singletons here. They will be on the ScriptManager thread
    // and are likely to be unusable if we "reset" the ScriptManager by creating a new one (on a whole new thread).

    scriptEngine->registerGlobalObject("Script", _scriptingInterface.get());

    {
        // set up Script.require.resolve and Script.require.cache
        auto Script = scriptEngine->globalObject().property("Script");
        auto require = Script.property("require");
        auto resolve = Script.property("_requireResolve");
        require.setProperty("resolve", resolve, READONLY_PROP_FLAGS);
        resetModuleCache();
    }

    scriptEngine->registerEnum("Script.ExternalPaths", QMetaEnum::fromType<ExternalResource::Bucket>());

    scriptEngine->registerGlobalObject("Quat", &_quatLibrary);
    scriptEngine->registerGlobalObject("Vec3", &_vec3Library);
    scriptEngine->registerGlobalObject("Mat4", &_mat4Library);
    scriptEngine->registerGlobalObject("Uuid", &_uuidLibrary);

    if (_context != NETWORKLESS_TEST_SCRIPT) {
        // This requires networking, we want to avoid the need for it in test scripts
        scriptEngine->registerGlobalObject("Messages", DependencyManager::get<MessagesClient>().data());
    }

    scriptEngine->registerGlobalObject("File", new FileScriptingInterface(this));
    scriptEngine->registerGlobalObject("console", &_consoleScriptingInterface);
    scriptEngine->registerFunction("console", "info", ConsoleScriptingInterface::info, scriptEngine->currentContext()->argumentCount());
    scriptEngine->registerFunction("console", "log", ConsoleScriptingInterface::log, scriptEngine->currentContext()->argumentCount());
    scriptEngine->registerFunction("console", "debug", ConsoleScriptingInterface::debug, scriptEngine->currentContext()->argumentCount());
    scriptEngine->registerFunction("console", "warn", ConsoleScriptingInterface::warn, scriptEngine->currentContext()->argumentCount());
    scriptEngine->registerFunction("console", "error", ConsoleScriptingInterface::error, scriptEngine->currentContext()->argumentCount());
    scriptEngine->registerFunction("console", "exception", ConsoleScriptingInterface::exception, scriptEngine->currentContext()->argumentCount());
    scriptEngine->registerFunction("console", "assert", ConsoleScriptingInterface::assertion, scriptEngine->currentContext()->argumentCount());
    scriptEngine->registerFunction("console", "group", ConsoleScriptingInterface::group, 1);
    scriptEngine->registerFunction("console", "groupCollapsed", ConsoleScriptingInterface::groupCollapsed, 1);
    scriptEngine->registerFunction("console", "groupEnd", ConsoleScriptingInterface::groupEnd, 0);

    // constants
    scriptEngine->globalObject().setProperty("TREE_SCALE", scriptEngine->newValue(TREE_SCALE));

    if (_context != NETWORKLESS_TEST_SCRIPT) {
        // Scriptable cache access
        auto resourcePrototype = createScriptableResourcePrototype(shared_from_this());
        scriptEngine->globalObject().setProperty("Resource", resourcePrototype);
        scriptEngine->setDefaultPrototype(qMetaTypeId<ScriptableResource*>(), resourcePrototype);

        scriptEngine->registerGlobalObject("Assets", _assetScriptingInterface);
        scriptEngine->registerGlobalObject("Resources", DependencyManager::get<ResourceScriptingInterface>().data());

        scriptEngine->registerGlobalObject("DebugDraw", &DebugDraw::getInstance());

        scriptEngine->registerGlobalObject("UserActivityLogger", DependencyManager::get<UserActivityLoggerScriptingInterface>().data());
    }

#if DEV_BUILD || PR_BUILD
    scriptEngine->registerGlobalObject("StackTest", new StackTestScriptingInterface(this));
#endif

    qCDebug(scriptengine) << "Engine initialized";
}

// registers a global object by name
void ScriptManager::registerValue(const QString& valueName, ScriptValue value) {
    _engine->globalObject().setProperty(valueName, value);
}

// Unregister the handlers for this eventName and entityID.
void ScriptManager::removeEventHandler(const EntityItemID& entityID, const QString& eventName, const ScriptValue& handler) {
    if (QThread::currentThread() != thread()) {
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine) << "*** WARNING *** ScriptManager::removeEventHandler() called on wrong thread [" << QThread::currentThread() << "], invoking on correct thread [" << thread() << "] "
            "entityID:" << entityID << " eventName:" << eventName;
#endif
        QMetaObject::invokeMethod(this, "removeEventHandler",
                                  Q_ARG(const EntityItemID&, entityID),
                                  Q_ARG(const QString&, eventName),
                                  Q_ARG(const ScriptValue&, handler));
        return;
    }
#ifdef THREAD_DEBUGGING
    qCDebug(scriptengine) << "ScriptManager::removeEventHandler() called on thread [" << QThread::currentThread() << "] entityID:" << entityID << " eventName : " << eventName;
#endif

    if (!_registeredHandlers.contains(entityID)) {
        return;
    }
    RegisteredEventHandlers& handlersOnEntity = _registeredHandlers[entityID];
    CallbackList& handlersForEvent = handlersOnEntity[eventName];
    // ScriptValue does not have operator==(), so we can't use QList::removeOne and friends. So iterate.
    for (int i = 0; i < handlersForEvent.count(); ++i) {
        if (handlersForEvent[i].function.equals(handler)) {
            handlersForEvent.removeAt(i);
            return; // Design choice: since comparison is relatively expensive, just remove the first matching handler.
        }
    }
}

// Unregister all event handlers for the specified entityID (i.e. the entity is being removed)
void ScriptManager::removeAllEventHandlers(const EntityItemID& entityID) {
    if (QThread::currentThread() != thread()) {
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine) << "*** WARNING *** ScriptManager::removeAllEventHandlers() called on wrong thread [" << QThread::currentThread() << ", correct thread is " << thread() << " ], ignoring "
            "entityID:" << entityID;
#endif
        return;
    }

    if (_registeredHandlers.contains(entityID)) {
        _registeredHandlers.remove(entityID);
    }
}

// Register the handler.
void ScriptManager::addEventHandler(const EntityItemID& entityID, const QString& eventName, const ScriptValue& handler) {
    if (QThread::currentThread() != thread()) {
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine) << "*** WARNING *** ScriptManager::addEventHandler() called on wrong thread [" << QThread::currentThread() << "], invoking on correct thread [" << thread() << "] "
        "entityID:" << entityID << " eventName:" << eventName;
#endif

        QMetaObject::invokeMethod(this, "addEventHandler",
                                  Q_ARG(const EntityItemID&, entityID),
                                  Q_ARG(const QString&, eventName),
                                  Q_ARG(const ScriptValue&, handler));
        return;
    }
#ifdef THREAD_DEBUGGING
    qCDebug(scriptengine) << "ScriptManager::addEventHandler() called on thread [" << QThread::currentThread() << "] entityID:" << entityID << " eventName : " << eventName;
#endif

    if (_registeredHandlers.count() == 0) {
        // First time any per-entity handler has been added in this script...
        emit attachDefaultEventHandlers();
    }
    if (!_registeredHandlers.contains(entityID)) {
        _registeredHandlers[entityID] = RegisteredEventHandlers();
    }
    CallbackList& handlersForEvent = _registeredHandlers[entityID][eventName];
    CallbackData handlerData = { handler, currentEntityIdentifier, currentSandboxURL };
    handlersForEvent << handlerData; // Note that the same handler can be added many times. See removeEntityEventHandler().
}

bool ScriptManager::isStopped() const {
    if (_context == NETWORKLESS_TEST_SCRIPT) {
        return false;
    }

    QSharedPointer<ScriptEngines> scriptEngines(_scriptEngines);
    return !scriptEngines || scriptEngines->isStopped();
}

void ScriptManager::run() {
    if (QThread::currentThread() != qApp->thread() && _context == Context::CLIENT_SCRIPT) {
        // Flag that we're allowed to access local HTML files on UI created from C++ calls on this thread
        // (because we're a client script)
        hifi::scripting::setLocalAccessSafeThread(true);
    }


    //_engine->enterIsolateOnThisThread();

    _engine->compileTest();

    auto filenameParts = _fileNameString.split("/");
    auto name = filenameParts.size() > 0 ? filenameParts[filenameParts.size() - 1] : "unknown";
    PROFILE_SET_THREAD_NAME("Script: " + name);

    if (isStopped()) {
        qCCritical(scriptengine) << "ScriptManager is stopped or ScriptEngines is not available, refusing to run script";
        return; // bail early - avoid setting state in init(), as evaluate() will bail too
    }

    scriptInfoMessage("Script Engine starting:" + getFilename());

    if (!_isInitialized) {
        init();
    }

    _isRunning = true;
    emit runningStateChanged();

    {
        PROFILE_RANGE(script, _fileNameString);
        _returnValue = _engine->evaluate(_scriptContents, _fileNameString);

        if (_engine->hasUncaughtException() && _abortOnUncaughtException) {

            qCWarning(scriptengine) << "Engine has uncaught exception, stopping";
            stop();
            // V8TODO: Is clearing needed here?
            //_engine->clearExceptions();
        }
    }
#ifdef _WIN32
    // VS13 does not sleep_until unless it uses the system_clock, see:
    // https://www.reddit.com/r/cpp_questions/comments/3o71ic/sleep_until_not_working_with_a_time_pointsteady/
    using clock = std::chrono::system_clock;
#else
    using clock = std::chrono::high_resolution_clock;
#endif

    clock::time_point startTime = clock::now();
    int thisFrame = 0;

    _lastUpdate = usecTimestampNow();

    std::chrono::microseconds totalUpdates(0);

    qCDebug(scriptengine) << "Waiting for finish";

    // TODO: Integrate this with signals/slots instead of reimplementing throttling for ScriptManager
    while (!_isFinished) {
        //qCDebug(scriptengine) << "In script event loop";

        auto beforeSleep = clock::now();

        // Throttle to SCRIPT_FPS
        // We'd like to try to keep the script at a solid SCRIPT_FPS update rate. And so we will
        // calculate a sleepUntil to be the time from our start time until the original target
        // sleepUntil for this frame. This approach will allow us to "catch up" in the event
        // that some of our script udpates/frames take a little bit longer than the target average
        // to execute.
        // NOTE: if we go to variable SCRIPT_FPS, then we will need to reconsider this approach
        const std::chrono::microseconds TARGET_SCRIPT_FRAME_DURATION(USECS_PER_SECOND / SCRIPT_FPS + 1);
        clock::time_point targetSleepUntil(startTime + (thisFrame++ * TARGET_SCRIPT_FRAME_DURATION));

        // However, if our sleepUntil is not at least our average update and timer execution time
        // into the future it means our script is taking too long in its updates, and we want to
        // punish the script a little bit. So we will force the sleepUntil to be at least our
        // averageUpdate + averageTimerPerFrame time into the future.
        auto averageUpdate = totalUpdates / thisFrame;
        auto averageTimerPerFrame = _totalTimerExecution / thisFrame;
        auto averageTimerAndUpdate = averageUpdate + averageTimerPerFrame;
        auto sleepUntil = std::max(targetSleepUntil, beforeSleep + averageTimerAndUpdate);

        // We don't want to actually sleep for too long, because it causes our scripts to hang
        // on shutdown and stop... so we want to loop and sleep until we've spent our time in
        // purgatory, constantly checking to see if our script was asked to end
        bool processedEvents = false;
        if (!_isFinished) {
            PROFILE_RANGE(script, "processEvents-sleep");
            std::chrono::milliseconds sleepFor =
                std::chrono::duration_cast<std::chrono::milliseconds>(sleepUntil - clock::now());
            if (sleepFor > std::chrono::milliseconds(0)) {
                QEventLoop loop;
                QTimer timer;
                timer.setSingleShot(true);
                connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
                timer.start(sleepFor.count());
                loop.exec();
            } else {
                QCoreApplication::processEvents();
            }
            processedEvents = true;
        }

        PROFILE_RANGE(script, "ScriptMainLoop");

#ifdef SCRIPT_DELAY_DEBUG
        {
            auto actuallySleptUntil = clock::now();
            uint64_t seconds = std::chrono::duration_cast<std::chrono::seconds>(actuallySleptUntil - startTime).count();
            if (seconds > 0) { // avoid division by zero and time travel
                uint64_t fps = thisFrame / seconds;
                // Overreporting artificially reduces the reported rate
                if (thisFrame % SCRIPT_FPS == 0) {
                    qCDebug(scriptengine) <<
                        "Frame:" << thisFrame <<
                        "Slept (us):" << std::chrono::duration_cast<std::chrono::microseconds>(actuallySleptUntil - beforeSleep).count() <<
                        "Avg Updates (us):" << averageUpdate.count() <<
                        "FPS:" << fps;
                }
            }
        }
#endif
        if (_isFinished) {
            break;
        }

        // Only call this if we didn't processEvents as part of waiting for next frame
        if (!processedEvents) {
            PROFILE_RANGE(script, "processEvents");
            QCoreApplication::processEvents();
        }

        if (_isFinished) {
            break;
        }

        if (!_isFinished) {
            emit releaseEntityPacketSenderMessages(false);
        }

        qint64 now = usecTimestampNow();

        // we check for 'now' in the past in case people set their clock back
        if (_emitScriptUpdates() && _lastUpdate < now) {
            float deltaTime = (float) (now - _lastUpdate) / (float) USECS_PER_SECOND;
            if (!_isFinished) {
                auto preUpdate = clock::now();
                {
                    PROFILE_RANGE(script, "ScriptUpdate");
                    emit update(deltaTime);
                }
                auto postUpdate = clock::now();
                auto elapsed = (postUpdate - preUpdate);
                totalUpdates += std::chrono::duration_cast<std::chrono::microseconds>(elapsed);
            }
        }
        _lastUpdate = now;

        // only clear exceptions if we are not in the middle of evaluating
        if (!_engine->isEvaluating() && _engine->hasUncaughtException()) {
            qCWarning(scriptengine) << __FUNCTION__ << "---------- UNCAUGHT EXCEPTION --------";
            qCWarning(scriptengine) << "runInThread" << _engine->uncaughtException();
            emit unhandledException(_engine->uncaughtException());
            _engine->clearExceptions();
        }
    }
    scriptInfoMessage("Script Engine stopping:" + getFilename());

    stopAllTimers(); // make sure all our timers are stopped if the script is ending
    emit scriptEnding();

    emit releaseEntityPacketSenderMessages(true);

    emit finished(_fileNameString, shared_from_this());

    // Don't leave our local-file-access flag laying around, reset it to false when the scriptengine
    // thread is finished
    hifi::scripting::setLocalAccessSafeThread(false);
    _isRunning = false;
    emit runningStateChanged();
    emit doneRunning();
}

// NOTE: This is private because it must be called on the same thread that created the timers, which is why
// we want to only call it in our own run "shutdown" processing.
void ScriptManager::stopAllTimers() {
    QMutableHashIterator<QTimer*, CallbackData> i(_timerFunctionMap);
    int j {0};
    while (i.hasNext()) {
        i.next();
        QTimer* timer = i.key();
        qCDebug(scriptengine) << getFilename() << "stopAllTimers[" << j++ << "]";
        stopTimer(timer);
    }
}

void ScriptManager::stopAllTimersForEntityScript(const EntityItemID& entityID) {
     // We could maintain a separate map of entityID => QTimer, but someone will have to prove to me that it's worth the complexity. -HRS
    QVector<QTimer*> toDelete;
    QMutableHashIterator<QTimer*, CallbackData> i(_timerFunctionMap);
    while (i.hasNext()) {
        i.next();
        if (i.value().definingEntityIdentifier != entityID) {
            continue;
        }
        QTimer* timer = i.key();
        toDelete << timer; // don't delete while we're iterating. save it.
    }
    for (auto timer:toDelete) { // now reap 'em
        stopTimer(timer);
    }

}

void ScriptManager::stop(bool marshal) {
    _isStopping = true; // this can be done on any thread

    if (marshal) {
        QMetaObject::invokeMethod(this, "stop");
        return;
    }

    if (!_isFinished) {
        _isFinished = true;
        emit runningStateChanged();
    }
}

void ScriptManager::updateMemoryCost(const qint64& deltaSize) {
    _engine->updateMemoryCost(deltaSize);
}

void ScriptManager::timerFired() {
    if (isStopped()) {
        scriptWarningMessage("Script.timerFired() while shutting down is ignored... parent script:" + getFilename());
        return; // bail early
    }

    _timerCallCounter++;
    if (_timerCallCounter % 100 == 0) {
        qDebug() << "Script engine: " << _engine->manager()->getFilename()
                 << "timer call count: " << _timerCallCounter << " total time: " << _totalTimeInTimerEvents_s;
    }
    QElapsedTimer callTimer;
    callTimer.start();

    QTimer* callingTimer = reinterpret_cast<QTimer*>(sender());
    CallbackData timerData = _timerFunctionMap.value(callingTimer);

    if (!callingTimer->isActive()) {
        // this timer is done, we can kill it
        _timerFunctionMap.remove(callingTimer);
        delete callingTimer;
    }

    // call the associated JS function, if it exists
    if (timerData.function.isValid()) {
        PROFILE_RANGE(script, __FUNCTION__);
        auto preTimer = p_high_resolution_clock::now();
        callWithEnvironment(timerData.definingEntityIdentifier, timerData.definingSandboxURL, timerData.function, timerData.function, ScriptValueList());
        auto postTimer = p_high_resolution_clock::now();
        auto elapsed = (postTimer - preTimer);
        _totalTimerExecution += std::chrono::duration_cast<std::chrono::microseconds>(elapsed);
    } else {
        qCWarning(scriptengine) << "timerFired -- invalid function" << timerData.function.toVariant().toString();
    }

    _totalTimeInTimerEvents_s += callTimer.elapsed() / 1000.0;
}

QTimer* ScriptManager::setupTimerWithInterval(const ScriptValue& function, int intervalMS, bool isSingleShot) {
    // create the timer, add it to the map, and start it
    QTimer* newTimer = new QTimer(this);
    newTimer->setSingleShot(isSingleShot);

    // The default timer type is not very accurate below about 200ms http://doc.qt.io/qt-5/qt.html#TimerType-enum
    static const int MIN_TIMEOUT_FOR_COARSE_TIMER = 200;
    if (intervalMS < MIN_TIMEOUT_FOR_COARSE_TIMER) {
        newTimer->setTimerType(Qt::PreciseTimer);
    }

    connect(newTimer, &QTimer::timeout, this, &ScriptManager::timerFired);

    // make sure the timer stops when the script does
    connect(this, &ScriptManager::scriptEnding, newTimer, &QTimer::stop);


    CallbackData timerData = { function, currentEntityIdentifier, currentSandboxURL };
    _timerFunctionMap.insert(newTimer, timerData);

    newTimer->start(intervalMS);
    return newTimer;
}

QTimer* ScriptManager::setInterval(const ScriptValue& function, int intervalMS) {
    if (isStopped()) {
        scriptWarningMessage("Script.setInterval() while shutting down is ignored... parent script:" + getFilename());
        return NULL; // bail early
    }

    return setupTimerWithInterval(function, intervalMS, false);
}

QTimer* ScriptManager::setTimeout(const ScriptValue& function, int timeoutMS) {
    if (isStopped()) {
        scriptWarningMessage("Script.setTimeout() while shutting down is ignored... parent script:" + getFilename());
        return NULL; // bail early
    }

    return setupTimerWithInterval(function, timeoutMS, true);
}

void ScriptManager::stopTimer(QTimer *timer) {
    if (_timerFunctionMap.contains(timer)) {
        timer->stop();
        _timerFunctionMap.remove(timer);
        delete timer;
    } else {
        qCDebug(scriptengine) << "stopTimer -- not in _timerFunctionMap" << timer;
    }
}

QUrl ScriptManager::resolvePath(const QString& include) const {
    //qDebug(scriptengine) << "ScriptManager::resolvePath: getCurrentScriptURLs: " << _engine->getCurrentScriptURLs();
    QUrl url(include);
    // first lets check to see if it's already a full URL -- or a Windows path like "c:/"
    if (include.startsWith("/") || url.scheme().length() == 1) {
        url = QUrl::fromLocalFile(include);
    }
    if (!url.isRelative()) {
        return expandScriptUrl(url);
    }

    // we apparently weren't a fully qualified url, so, let's assume we're relative
    // to the first absolute URL in the JS scope chain
    QUrl parentURL;
    auto context = _engine->currentContext();
    ScriptContextPointer parentContext;  // using this variable to maintain parent variable lifespan
    do {
        auto contextInfo = context->functionContext();
        parentURL = QUrl(contextInfo->fileName());
        //qDebug(scriptengine) << "ScriptManager::resolvePath: URL get: " << parentURL << " backtrace: " << context->backtrace() << " " << _engine->getCurrentScriptURLs();
        parentContext = context->parentContext();
        context = parentContext.get();
    } while (parentURL.isRelative() && context);

    if (parentURL.isRelative()) {
        // fallback to the "include" parent (if defined, this will already be absolute)
        parentURL = QUrl(_parentURL);
    }

    if (parentURL.isRelative()) {
        // fallback to the original script engine URL
        parentURL = QUrl(_fileNameString);

        // if still relative and path-like, then this is probably a local file...
        if (parentURL.isRelative() && url.path().contains("/")) {
            parentURL = QUrl::fromLocalFile(_fileNameString);
        }
    }

    // at this point we should have a legitimate fully qualified URL for our parent
    url = expandScriptUrl(parentURL.resolved(url));
    return url;
}

QUrl ScriptManager::resourcesPath() const {
    return QUrl(PathUtils::resourcesUrl());
}

void ScriptManager::print(const QString& message) {
    emit printedMessage(message, getFilename());
}


void ScriptManager::beginProfileRange(const QString& label) const {
    PROFILE_SYNC_BEGIN(script, label.toStdString().c_str(), label.toStdString().c_str());
}

void ScriptManager::endProfileRange(const QString& label) const {
    PROFILE_SYNC_END(script, label.toStdString().c_str(), label.toStdString().c_str());
}

// Script.require.resolve -- like resolvePath, but performs more validation and throws exceptions on invalid module identifiers (for consistency with Node.js)
QString ScriptManager::_requireResolve(const QString& moduleId, const QString& relativeTo) {
    if (!_engine->IS_THREADSAFE_INVOCATION(__FUNCTION__)) {
        return QString();
    }
    QUrl defaultScriptsLoc = PathUtils::defaultScriptsLocation();
    QUrl url(moduleId);

    auto displayId = moduleId;
    if (displayId.length() > MAX_DEBUG_VALUE_LENGTH) {
        displayId = displayId.mid(0, MAX_DEBUG_VALUE_LENGTH) + "...";
    }
    auto message = QString("Cannot find module '%1' (%2)").arg(displayId);

    auto throwResolveError = [&](const QString& error, const QString &details = QString()) -> QString {
        _engine->raiseException(error, "require.resolve: " + details);
        return QString();
    };

    // de-fuzz the input a little by restricting to rational sizes
    auto idLength = url.toString().length();
    if (idLength < 1 || idLength > MAX_MODULE_ID_LENGTH) {
        auto details = QString("rejecting invalid module id size (%1 chars [1,%2])")
            .arg(idLength).arg(MAX_MODULE_ID_LENGTH);
        return throwResolveError(details, "RangeError");
    }

    // this regex matches: absolute, dotted or path-like URLs
    // (ie: the kind of stuff ScriptManager::resolvePath already handles)
    QRegularExpression qualified ("^\\w+:|^/|^[.]{1,2}(/|$)");

    // this is for module.require (which is a bound version of require that's always relative to the module path)
    if (!relativeTo.isEmpty()) {
        url = QUrl(relativeTo).resolved(moduleId);
        url = resolvePath(url.toString());
    } else if (qualified.match(moduleId).hasMatch()) {
        url = resolvePath(moduleId);
    } else {
        // check if the moduleId refers to a "system" module
        QString systemPath = defaultScriptsLoc.path();
        QString systemModulePath = QString("%1/modules/%2.js").arg(systemPath).arg(moduleId);
        url = defaultScriptsLoc;
        url.setPath(systemModulePath);
        if (!QFileInfo(url.toLocalFile()).isFile()) {
            if (!moduleId.contains("./")) {
                // the user might be trying to refer to a relative file without anchoring it
                // let's do them a favor and test for that case -- offering specific advice if detected
                auto unanchoredUrl = resolvePath("./" + moduleId);
                if (QFileInfo(unanchoredUrl.toLocalFile()).isFile()) {
                    auto msg = QString("relative module ids must be anchored; use './%1' instead")
                        .arg(moduleId);
                    return throwResolveError(message.arg(msg));
                }
            }
            return throwResolveError(message.arg("system module not found"));
        }
    }

    if (url.isRelative()) {
        return throwResolveError(message.arg("could not resolve module id"));
    }

    // if it looks like a local file, verify that it's an allowed path and really a file
    if (url.isLocalFile()) {
        QFileInfo file(url.toLocalFile());
        QUrl canonical = url;
        if (file.exists()) {
            canonical.setPath(file.canonicalFilePath());
        }

        bool disallowOutsideFiles = !PathUtils::defaultScriptsLocation().isParentOf(canonical) && !currentSandboxURL.isLocalFile();
        if (disallowOutsideFiles && !PathUtils::isDescendantOf(canonical, currentSandboxURL)) {
            return throwResolveError(message.arg(
                QString("path '%1' outside of origin script '%2' '%3'")
                    .arg(PathUtils::stripFilename(url))
                    .arg(PathUtils::stripFilename(currentSandboxURL))
                    .arg(canonical.toString())
            ));
        }
        if (!file.exists()) {
            return throwResolveError(message.arg("path does not exist: " + url.toLocalFile()));
        }
        if (!file.isFile()) {
            return throwResolveError(message.arg("path is not a file: " + url.toLocalFile()));
        }
    }

    return url.toString();
}

// retrieves the current parent module from the JS scope chain
ScriptValue ScriptManager::currentModule() {
    if (!_engine->IS_THREADSAFE_INVOCATION(__FUNCTION__)) {
        return _engine->nullValue();
    }
    auto jsRequire = _engine->globalObject().property("Script").property("require");
    auto cache = jsRequire.property("cache");
    ScriptValue candidate;
    ScriptContextPointer parentContext;  // using this variable to maintain parent variable lifespan
    for (auto context = _engine->currentContext(); context && !candidate.isObject();
         parentContext = context->parentContext(), context = parentContext.get()) {
        auto contextInfo = context->functionContext();
        candidate = cache.property(contextInfo->fileName());
    }
    if (!candidate.isObject()) {
        return ScriptValue();
    }
    return candidate;
}

// replaces or adds "module" to "parent.children[]" array
// (for consistency with Node.js and userscript cache invalidation without "cache busters")
bool ScriptManager::registerModuleWithParent(const ScriptValue& module, const ScriptValue& parent) {
    auto children = parent.property("children");
    if (children.isArray()) {
        auto key = module.property("id");
        auto length = children.property("length").toInt32();
        for (int i = 0; i < length; i++) {
            if (children.property(i).property("id").strictlyEquals(key)) {
                qCDebug(scriptengine_module) << key.toString() << " updating parent.children[" << i << "] = module";
                children.setProperty(i, module);
                return true;
            }
        }
        qCDebug(scriptengine_module) << key.toString() << " appending parent.children[" << length << "] = module";
        children.setProperty(length, module);
        return true;
    } else if (parent.isValid()) {
        qCDebug(scriptengine_module) << "registerModuleWithParent -- unrecognized parent" << parent.toVariant().toString();
    }
    return false;
}

// creates a new JS "module" Object with default metadata properties
ScriptValue ScriptManager::newModule(const QString& modulePath, const ScriptValue& parent) {
    auto closure = _engine->newObject();
    auto exports = _engine->newObject();
    auto module = _engine->newObject();
    //qCDebug(scriptengine_module) << "newModule" << parent.property("filename").toString();

    closure.setProperty("module", module, READONLY_PROP_FLAGS);

    // note: this becomes the "exports" free variable, so should not be set read only
    closure.setProperty("exports", exports);

    // make the closure available to module instantiation
    module.setProperty("__closure__", closure, READONLY_HIDDEN_PROP_FLAGS);

    // for consistency with Node.js Module
    module.setProperty("id", modulePath, READONLY_PROP_FLAGS);
    module.setProperty("filename", modulePath, READONLY_PROP_FLAGS);
    module.setProperty("exports", exports); // not readonly
    module.setProperty("loaded", false, READONLY_PROP_FLAGS);
    module.setProperty("parent", parent, READONLY_PROP_FLAGS);
    module.setProperty("children", _engine->newArray(), READONLY_PROP_FLAGS);

    // module.require is a bound version of require that always resolves relative to that module's path
    auto boundRequire = _engine->evaluate("(function(id) { return Script.require(Script.require.resolve(id, this.filename)); })", "(boundRequire)");
    module.setProperty("require", boundRequire, READONLY_PROP_FLAGS);
    //qDebug() << "Module object contents" << _engine->scriptValueDebugListMembers(module);
    return module;
}

// synchronously fetch a module's source code using BatchLoader
QVariantMap ScriptManager::fetchModuleSource(const QString& modulePath, const bool forceDownload) {
    using UrlMap = QMap<QUrl, QString>;
    auto scriptCache = DependencyManager::get<ScriptCache>();
    QVariantMap req;
    qCDebug(scriptengine_module) << "require.fetchModuleSource: " << QUrl(modulePath).fileName() << QThread::currentThread();

    auto onload = [=, &req](const UrlMap& data, const UrlMap& _status) {
        auto url = modulePath;
        auto status = _status[url];
        auto contents = data[url];
        if (isStopping()) {
            req["status"] = "Stopped";
            req["success"] = false;
        } else {
            req["url"] = url;
            req["status"] = status;
            req["success"] = ScriptCache::isSuccessStatus(status);
            req["contents"] = contents;
        }
    };

    if (forceDownload) {
        qCDebug(scriptengine_module) << "require.requestScript -- clearing cache for" << modulePath;
        scriptCache->deleteScript(modulePath);
    }
    BatchLoader* loader = new BatchLoader(QList<QUrl>({ modulePath }));
    connect(loader, &BatchLoader::finished, this, onload);
    connect(this, &QObject::destroyed, loader, &QObject::deleteLater);
    // fail faster? (since require() blocks the engine thread while resolving dependencies)
    const int MAX_RETRIES = 1;

    loader->start(MAX_RETRIES);

    if (!loader->isFinished()) {
        // This lambda can get called AFTER this local scope has completed.
        // This is why we pass smart ptrs to the lambda instead of references to local variables.
        auto monitor = std::make_shared<QTimer>();
        auto loop = std::make_shared<QEventLoop>();
        QObject::connect(loader, &BatchLoader::finished, this, [monitor, loop] {
            monitor->stop();
            loop->quit();
        });

        // this helps detect the case where stop() is invoked during the download
        //  but not seen in time to abort processing in onload()...
        connect(monitor.get(), &QTimer::timeout, this, [this, loop] {
            if (isStopping()) {
                loop->exit(-1);
            }
        });
        monitor->start(500);
        loop->exec();
    }
    loader->deleteLater();
    return req;
}

// evaluate a pending module object using the fetched source code
ScriptValue ScriptManager::instantiateModule(const ScriptValue& module, const QString& sourceCode) {
    ScriptValue result;
    auto modulePath = module.property("filename").toString();
    auto closure = module.property("__closure__");

    qCDebug(scriptengine_module) << QString("require.instantiateModule: %1 / %2 bytes")
        .arg(QUrl(modulePath).fileName()).arg(sourceCode.length());

    if (module.property("content-type").toString() == "application/json") {
        qCDebug(scriptengine_module) << "... parsing as JSON";
        closure.setProperty("__json", sourceCode);
        result = _engine->evaluateInClosure(closure, _engine->newProgram( "module.exports = JSON.parse(__json)", modulePath ));
    } else {
        // scoped vars for consistency with Node.js
        closure.setProperty("require", module.property("require"));
        closure.setProperty("__filename", modulePath, READONLY_HIDDEN_PROP_FLAGS);
        closure.setProperty("__dirname", QString(modulePath).replace(QRegExp("/[^/]*$"), ""), READONLY_HIDDEN_PROP_FLAGS);
        //_engine->scriptValueDebugDetails(module);
        result = _engine->evaluateInClosure(closure, _engine->newProgram( sourceCode, modulePath ));
    }

    return result;
}

// CommonJS/Node.js like require/module support
ScriptValue ScriptManager::require(const QString& moduleId) {
    qCDebug(scriptengine_module) << "ScriptManager::require(" << moduleId.left(MAX_DEBUG_VALUE_LENGTH) << ")";
    if (!_engine->IS_THREADSAFE_INVOCATION(__FUNCTION__)) {
        return _engine->nullValue();
    }

    auto jsRequire = _engine->globalObject().property("Script").property("require");
    auto cacheMeta = jsRequire.data();
    auto cache = jsRequire.property("cache");
    auto parent = currentModule();

    auto throwModuleError = [&](const QString& modulePath, const ScriptValue& error) {
        cache.setProperty(modulePath, _engine->nullValue());
        if (!error.isNull()) {
#ifdef DEBUG_JS_MODULES
            qCWarning(scriptengine_module) << "throwing module error:" << error.toString() << modulePath << error.property("stack").toString();
#endif
            _engine->raiseException(error, "module error");
        }
        return _engine->nullValue();
    };

    // start by resolving the moduleId into a fully-qualified path/URL
    QString modulePath = _requireResolve(moduleId);
    if (modulePath.isNull() || _engine->hasUncaughtException()) {
        // the resolver already threw an exception -- bail early
        return _engine->nullValue();
    }

    // check the resolved path against the cache
    auto module = cache.property(modulePath);

    // modules get cached in `Script.require.cache` and (similar to Node.js) users can access it
    // to inspect particular entries and invalidate them by deleting the key:
    //   `delete Script.require.cache[Script.require.resolve(moduleId)];`

    // Check to see if we should invalidate the cache based on a user setting.
    Setting::Handle<bool> getCachebustSetting {"cachebustScriptRequire", false };

    // cacheMeta is just used right now to tell deleted keys apart from undefined ones
    bool invalidateCache = getCachebustSetting.get() || (module.isUndefined() && cacheMeta.property(moduleId).isValid());

    // reset the cacheMeta record so invalidation won't apply next time, even if the module fails to load
    cacheMeta.setProperty(modulePath, ScriptValue());

    auto exports = module.property("exports");
    if (!invalidateCache && exports.isObject()) {
        // we have found a cached module -- just need to possibly register it with current parent
        qCDebug(scriptengine_module) << QString("require - using cached module for '%1' (loaded: %2)")
            .arg(moduleId).arg(module.property("loaded").toString());
        registerModuleWithParent(module, parent);
        return exports;
    }

    // bootstrap / register new empty module
    module = newModule(modulePath, parent);
    registerModuleWithParent(module, parent);

    // add it to the cache (this is done early so any cyclic dependencies pick up)
    cache.setProperty(modulePath, module);

    // download the module source
    auto req = fetchModuleSource(modulePath, invalidateCache);

    if (!req.contains("success") || !req["success"].toBool()) {
        auto error = QString("error retrieving script (%1)").arg(req["status"].toString());
        return throwModuleError(modulePath, _engine->newValue(error));
    }

#if DEBUG_JS_MODULES
    qCDebug(scriptengine_module) << "require.loaded: " <<
        QUrl(req["url"].toString()).fileName() << req["status"].toString();
#endif

    auto sourceCode = req["contents"].toString();

    if (QUrl(modulePath).fileName().endsWith(".json", Qt::CaseInsensitive)) {
        module.setProperty("content-type", "application/json");
    } else {
        module.setProperty("content-type", "application/javascript");
    }

    // evaluate the module
    auto result = instantiateModule(module, sourceCode);

    if (result.isError() && !result.strictlyEquals(module.property("exports"))) {
        qCWarning(scriptengine_module) << "-- result.isError --" << result.toString();
        return throwModuleError(modulePath, result);
    }

    // mark as fully-loaded
    module.setProperty("loaded", true, READONLY_PROP_FLAGS);

    // set up a new reference point for detecting cache key deletion
    cacheMeta.setProperty(modulePath, module);

    //qCDebug(scriptengine_module) << "//ScriptManager::require(" << moduleId << ")";

    //qCDebug(scriptengine_module) << "Exports: " << _engine->scriptValueDebugDetails(module.property("exports"));
    return module.property("exports");
}

// If a callback is specified, the included files will be loaded asynchronously and the callback will be called
// when all of the files have finished loading.
// If no callback is specified, the included files will be loaded synchronously and will block execution until
// all of the files have finished loading.
void ScriptManager::include(const QStringList& includeFiles, const ScriptValue& callback) {
    if (!_engine->IS_THREADSAFE_INVOCATION(__FUNCTION__)) {
        return;
    }
    if (isStopped()) {
        scriptWarningMessage("Script.include() while shutting down is ignored... includeFiles:"
                + includeFiles.join(",") + "parent script:" + getFilename());
        return; // bail early
    }
    QList<QUrl> urls;

    for (QString includeFile : includeFiles) {
        QString file = DependencyManager::get<ResourceManager>()->normalizeURL(includeFile);
        QUrl thisURL;
        bool isStandardLibrary = false;
        if (file.startsWith("/~/")) {
            thisURL = expandScriptUrl(QUrl::fromLocalFile(expandScriptPath(file)));
            QUrl defaultScriptsLoc = PathUtils::defaultScriptsLocation();
            if (!defaultScriptsLoc.isParentOf(thisURL)) {
                //V8TODO this probably needs to be done per context, otherwise file cannot be included again in a module
                scriptWarningMessage("Script.include() -- skipping" + file + "-- outside of standard libraries");
                continue;
            }
            isStandardLibrary = true;
        } else {
            thisURL = resolvePath(file);
        }

        bool disallowOutsideFiles = thisURL.isLocalFile() && !isStandardLibrary && !currentSandboxURL.isLocalFile();
        if (disallowOutsideFiles && !PathUtils::isDescendantOf(thisURL, currentSandboxURL)) {
            scriptWarningMessage("Script.include() ignoring file path" + thisURL.toString()
                                + "outside of original entity script" + currentSandboxURL.toString());
        } else {
            // We could also check here for CORS, but we don't yet.
            // It turns out that QUrl.resolve will not change hosts and copy authority, so we don't need to check that here.
            urls.append(thisURL);
        }
    }

    // If there are no URLs left to download, don't bother attempting to download anything and return early
    if (urls.size() == 0) {
        return;
    }

    BatchLoader* loader = new BatchLoader(urls);
    EntityItemID capturedEntityIdentifier = currentEntityIdentifier;
    QUrl capturedSandboxURL = currentSandboxURL;

    auto evaluateScripts = [=](const QMap<QUrl, QString>& data, const QMap<QUrl, QString>& status) {
        auto parentURL = _parentURL;
        for (QUrl url : urls) {
            QString contents = data[url];
            if (contents.isNull()) {
                scriptErrorMessage("Error loading file (" + status[url] +"): " + url.toString());
            } else {
                std::lock_guard<std::recursive_mutex> lock(_lock);
                if (!_includedURLs.contains(url)) {
                    _includedURLs << url;
                    // Set the parent url so that path resolution will be relative
                    // to this script's url during its initial evaluation
                    _parentURL = url.toString();
                    auto operation = [&]() {
                        _engine->evaluate(contents, url.toString());
                    };

                    doWithEnvironment(capturedEntityIdentifier, capturedSandboxURL, operation);
                    if(_engine->hasUncaughtException()) {
                        auto ex = _engine->uncaughtException();
                        ex->additionalInfo += "; evaluateInClosure";
                        emit unhandledException(ex);
                        _engine->clearExceptions();
                    }
                } else {
                    scriptPrintedMessage("Script.include() skipping evaluation of previously included url:" + url.toString());
                }
            }
        }
        _parentURL = parentURL;

        if (callback.isFunction()) {
            callWithEnvironment(capturedEntityIdentifier, capturedSandboxURL, callback, ScriptValue(), ScriptValueList());
        }

        loader->deleteLater();
    };

    connect(loader, &BatchLoader::finished, this, evaluateScripts);

    // If we are destroyed before the loader completes, make sure to clean it up
    connect(this, &QObject::destroyed, loader, &QObject::deleteLater);

    loader->start(processLevelMaxRetries);

    if (!callback.isFunction() && !loader->isFinished()) {
        QEventLoop loop;
        QObject::connect(loader, &BatchLoader::finished, &loop, &QEventLoop::quit);
        loop.exec();
    }
}

void ScriptManager::include(const QString& includeFile, const ScriptValue& callback) {
    if (isStopped()) {
        scriptWarningMessage("Script.include() while shutting down is ignored...  includeFile:"
                    + includeFile + "parent script:" + getFilename());
        return; // bail early
    }

    QStringList urls;
    urls.append(includeFile);
    include(urls, callback);
}

// NOTE: The load() command is similar to the include() command except that it loads the script
// as a stand-alone script. To accomplish this, the ScriptManager class just emits a signal which
// the Application or other context will connect to in order to know to actually load the script
void ScriptManager::load(const QString& loadFile) {
    if (!_engine->IS_THREADSAFE_INVOCATION(__FUNCTION__)) {
        return;
    }
    if (isStopped()) {
        scriptWarningMessage("Script.load() while shutting down is ignored... loadFile:"
                + loadFile + "parent script:" + getFilename());
        return; // bail early
    }
    if (!currentEntityIdentifier.isInvalidID()) {
        scriptWarningMessage("Script.load() from entity script is ignored...  loadFile:"
                + loadFile + "parent script:" + getFilename() + "entity: " + currentEntityIdentifier.toString());
        return; // bail early
    }

    QUrl url = resolvePath(loadFile);
    if (_isReloading) {
        auto scriptCache = DependencyManager::get<ScriptCache>();
        scriptCache->deleteScript(url.toString());
        emit reloadScript(url.toString(), false);
    } else {
        emit loadScript(url.toString(), false);
    }
}

// Look up the handler associated with eventName and entityID. If found, evalute the argGenerator thunk and call the handler with those args
void ScriptManager::forwardHandlerCall(const EntityItemID& entityID, const QString& eventName, const ScriptValueList& eventHandlerArgs) {
    if (QThread::currentThread() != thread()) {
        qCDebug(scriptengine) << "*** ERROR *** ScriptManager::forwardHandlerCall() called on wrong thread [" << QThread::currentThread() << "], invoking on correct thread [" << thread() << "]";
        assert(false);
        return ;
    }
    if (!_registeredHandlers.contains(entityID)) {
        return;
    }
    const RegisteredEventHandlers& handlersOnEntity = _registeredHandlers[entityID];
    if (!handlersOnEntity.contains(eventName)) {
        return;
    }
    CallbackList handlersForEvent = handlersOnEntity[eventName];
    if (!handlersForEvent.isEmpty()) {
        for (int i = 0; i < handlersForEvent.count(); ++i) {
            // handlersForEvent[i] can contain many handlers that may have each been added by different interface or entity scripts,
            // and the entity scripts may be for entities other than the one this is a handler for.
            // Fortunately, the definingEntityIdentifier captured the entity script id (if any) when the handler was added.
            CallbackData& handler = handlersForEvent[i];
            callWithEnvironment(handler.definingEntityIdentifier, handler.definingSandboxURL, handler.function, ScriptValue(), eventHandlerArgs);
        }
    }
}

int ScriptManager::getNumRunningEntityScripts() const {
    QReadLocker locker { &_entityScriptsLock };
    int sum = 0;
    for (const auto& st : _entityScripts) {
        if (st.status == EntityScriptStatus::RUNNING) {
            ++sum;
        }
    }
    return sum;
}

void ScriptManager::setEntityScriptDetails(const EntityItemID& entityID, const EntityScriptDetails& details) {
    {
        QWriteLocker locker { &_entityScriptsLock };
        _entityScripts[entityID] = details;
    }
    emit entityScriptDetailsUpdated();
}

void ScriptManager::updateEntityScriptStatus(const EntityItemID& entityID, const EntityScriptStatus &status, const QString& errorInfo) {
    {
        QWriteLocker locker { &_entityScriptsLock };
        EntityScriptDetails& details = _entityScripts[entityID];
        details.status = status;
        details.errorInfo = errorInfo;
    }
    emit entityScriptDetailsUpdated();
}

QVariant ScriptManager::cloneEntityScriptDetails(const EntityItemID& entityID) {
    static const QVariant NULL_VARIANT = QVariant::fromValue(nullptr);
    QVariantMap map;
    if (entityID.isNull()) {
        // TODO: find better way to report JS Error across thread/process boundaries
        map["isError"] = true;
        map["errorInfo"] = "Error: getEntityScriptDetails -- invalid entityID";
    } else {
#ifdef DEBUG_ENTITY_STATES
        qDebug() << "cloneEntityScriptDetails" << entityID << QThread::currentThread();
#endif
        EntityScriptDetails scriptDetails;
        if (getEntityScriptDetails(entityID, scriptDetails)) {
#ifdef DEBUG_ENTITY_STATES
            qDebug() << "gotEntityScriptDetails" << scriptDetails.status << QThread::currentThread();
#endif
            map["isRunning"] = isEntityScriptRunning(entityID);
            map["status"] = EntityScriptStatus_::valueToKey(scriptDetails.status).toLower();
            map["errorInfo"] = scriptDetails.errorInfo;
            map["entityID"] = entityID.toString();
#ifdef DEBUG_ENTITY_STATES
            {
                auto debug = QVariantMap();
                debug["script"] = scriptDetails.scriptText;
                debug["scriptObject"] = scriptDetails.scriptObject.toVariant();
                debug["lastModified"] = (qlonglong)scriptDetails.lastModified;
                debug["sandboxURL"] = scriptDetails.definingSandboxURL;
                map["debug"] = debug;
            }
#endif
        } else {
#ifdef DEBUG_ENTITY_STATES
            qDebug() << "!gotEntityScriptDetails" <<  QThread::currentThread();
#endif
            map["isError"] = true;
            map["errorInfo"] = "Entity script details unavailable";
            map["entityID"] = entityID.toString();
        }
    }
    return map;
}

QFuture<QVariant> ScriptManager::getLocalEntityScriptDetails(const EntityItemID& entityID) {
    return QtConcurrent::run(this, &ScriptManager::cloneEntityScriptDetails, entityID);
}

bool ScriptManager::getEntityScriptDetails(const EntityItemID& entityID, EntityScriptDetails &details) const {
    QReadLocker locker { &_entityScriptsLock };
    auto it = _entityScripts.constFind(entityID);
    if (it == _entityScripts.constEnd()) {
        return false;
    }
    details = it.value();
    return true;
}

bool ScriptManager::hasEntityScriptDetails(const EntityItemID& entityID) const {
    QReadLocker locker { &_entityScriptsLock };
    return _entityScripts.contains(entityID);
}

void ScriptManager::loadEntityScript(const EntityItemID& entityID, const QString& entityScript, bool forceRedownload) {
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, "loadEntityScript",
            Q_ARG(const EntityItemID&, entityID),
            Q_ARG(const QString&, entityScript),
            Q_ARG(bool, forceRedownload)
        );
        return;
    }
    PROFILE_RANGE(script, __FUNCTION__);

    QSharedPointer<ScriptEngines> scriptEngines(_scriptEngines);
    if (isStopping() || !scriptEngines || scriptEngines->isStopped()) {
        qCDebug(scriptengine) << "loadEntityScript.start " << entityID.toString()
                                     << " but isStopping==" << isStopping()
                                     << " || engines->isStopped==" << scriptEngines->isStopped();
        return;
    }

    if (!hasEntityScriptDetails(entityID)) {
        // make sure EntityScriptDetails has an entry for this UUID right away
        // (which allows bailing from the loading/provisioning process early if the Entity gets deleted mid-flight)
        updateEntityScriptStatus(entityID, EntityScriptStatus::PENDING, "...pending...");
    }

#ifdef DEBUG_ENTITY_STATES
    {
        EntityScriptDetails details;
        bool hasEntityScript = getEntityScriptDetails(entityID, details);
        qCDebug(scriptengine) << "loadEntityScript.LOADING: " << entityID.toString()
            << "(previous: " << (hasEntityScript ? details.status : EntityScriptStatus::PENDING) << ")";
    }
#endif

    EntityScriptDetails newDetails;
    newDetails.scriptText = entityScript;
    newDetails.status = EntityScriptStatus::LOADING;
    newDetails.definingSandboxURL = currentSandboxURL;
    setEntityScriptDetails(entityID, newDetails);

    auto scriptCache = DependencyManager::get<ScriptCache>();
    // note: see EntityTreeRenderer.cpp for shared pointer lifecycle management
    std::weak_ptr<ScriptManager> weakRef(shared_from_this());
    scriptCache->getScriptContents(entityScript,
        [this, weakRef, entityScript, entityID](const QString& url, const QString& contents, bool isURL, bool success, const QString& status) {
            std::shared_ptr<ScriptManager> strongRef(weakRef);
            if (!strongRef) {
                qCWarning(scriptengine) << "loadEntityScript.contentAvailable -- ScriptManager was deleted during getScriptContents!!";
                return;
            }
            if (isStopping()) {
#ifdef DEBUG_ENTITY_STATES
                qCDebug(scriptengine) << "loadEntityScript.contentAvailable -- stopping";
#endif
                return;
            }
            executeOnScriptThread([=]{
#ifdef DEBUG_ENTITY_STATES
                qCDebug(scriptengine) << "loadEntityScript.contentAvailable" << status << entityID.toString();
#endif
                if (!isStopping() && hasEntityScriptDetails(entityID)) {
                    _contentAvailableQueue[entityID] = { entityID, url, contents, isURL, success, status };
                } else {
#ifdef DEBUG_ENTITY_STATES
                    qCDebug(scriptengine) << "loadEntityScript.contentAvailable -- aborting";
#endif
                }
            });
    }, forceRedownload);
}

// The JSDoc is for the callEntityScriptMethod() call in this method.
// since all of these operations can be asynch we will always do the actual work in the response handler
// for the download
void ScriptManager::entityScriptContentAvailable(const EntityItemID& entityID, const QString& scriptOrURL, const QString& contents, bool isURL, bool success , const QString& status) {
    if (QThread::currentThread() != thread()) {
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine) << "*** WARNING *** ScriptManager::entityScriptContentAvailable() called on wrong thread ["
            << QThread::currentThread() << "], invoking on correct thread [" << thread()
            << "]  " "entityID:" << entityID << "scriptOrURL:" << scriptOrURL << "contents:"
            << contents << "isURL:" << isURL << "success:" << success;
#endif

        QMetaObject::invokeMethod(this, "entityScriptContentAvailable",
                                  Q_ARG(const EntityItemID&, entityID),
                                  Q_ARG(const QString&, scriptOrURL),
                                  Q_ARG(const QString&, contents),
                                  Q_ARG(bool, isURL),
                                  Q_ARG(bool, success),
                                  Q_ARG(const QString&, status));
        return;
    }

#ifdef THREAD_DEBUGGING
    qCDebug(scriptengine) << "ScriptManager::entityScriptContentAvailable() thread [" << QThread::currentThread() << "] expected thread [" << thread() << "]";
#endif

    auto scriptCache = DependencyManager::get<ScriptCache>();
    bool isFileUrl = isURL && scriptOrURL.startsWith("file://");
    auto fileName = isURL ? scriptOrURL : "about:EmbeddedEntityScript";

    QString entityScript;
    {
        QWriteLocker locker { &_entityScriptsLock };
        entityScript = _entityScripts[entityID].scriptText;
    }

    EntityScriptDetails newDetails;
    newDetails.scriptText = scriptOrURL;

    // If an error happens below, we want to update newDetails with the new status info
    // and also abort any pending Entity loads that are waiting on the exact same script URL.
    auto setError = [&](const QString &errorInfo, const EntityScriptStatus& status) {
        newDetails.errorInfo = errorInfo;
        newDetails.status = status;
        setEntityScriptDetails(entityID, newDetails);
    };

    // NETWORK / FILESYSTEM ERRORS
    if (!success) {
        setError("Failed to load script (" + status + ")", EntityScriptStatus::ERROR_LOADING_SCRIPT);
        return;
    }

    // SYNTAX ERRORS
    //auto syntaxError = _engine->lintScript(contents, fileName);
    auto program = _engine->newProgram( contents, fileName );
    auto syntaxCheck = program->checkSyntax();
    if (syntaxCheck->state() != ScriptSyntaxCheckResult::Valid) {
        auto message = syntaxCheck->errorMessage();
        //syntaxError.property("formatted").toString();
        //if (message.isEmpty()) {
        //    message = syntaxError.toString();
        //}
        setError(QString("Bad syntax (%1)").arg(message), EntityScriptStatus::ERROR_RUNNING_SCRIPT);
        //syntaxError.setProperty("detail", entityID.toString());
        //V8TODO
        //emit unhandledException(syntaxError);

        return;
    }
    if (!program) {
        setError("Bad program (isNull)", EntityScriptStatus::ERROR_RUNNING_SCRIPT);
        std::shared_ptr<ScriptException> ex = std::make_shared<ScriptEngineException>("Program is Null", "Bad program in entityScriptContentAvailable");
        emit unhandledException(ex);

        return; // done processing script
    }

    if (isURL) {
        setParentURL(scriptOrURL);
    }

    // SANITY/PERFORMANCE CHECK USING SANDBOX
    // V8TODO: can be skipped for now but needs to be implemented before release

    /*const int SANDBOX_TIMEOUT = 0.25 * MSECS_PER_SECOND;
    ScriptEnginePointer sandbox = newScriptEngine();
    sandbox->setProcessEventsInterval(SANDBOX_TIMEOUT);
    ScriptValue testConstructor, exception;
    if (atoi(getenv("UNSAFE_ENTITY_SCRIPTS") ? getenv("UNSAFE_ENTITY_SCRIPTS") : "0"))
    {
        QTimer timeout;
        timeout.setSingleShot(true);
        timeout.start(SANDBOX_TIMEOUT);
        connect(&timeout, &QTimer::timeout, [=, &sandbox]{
                qCDebug(scriptengine) << "ScriptManager::entityScriptContentAvailable timeout";

                // Guard against infinite loops and non-performant code
                sandbox->raiseException(
                    sandbox->makeError(sandbox->newValue(QString("Timed out (entity constructors are limited to %1ms)").arg(SANDBOX_TIMEOUT)))
                );
        });

        testConstructor = sandbox->evaluate(program);

        if (sandbox->hasUncaughtException()) {
            exception = sandbox->cloneUncaughtException(QString("(preflight %1)").arg(entityID.toString()));
            sandbox->clearExceptions();
        } else if (testConstructor.isError()) {
            exception = testConstructor;
        }
    } else {
        // ENTITY SCRIPT WHITELIST STARTS HERE
        auto nodeList = DependencyManager::get<NodeList>();
        bool passList = false;  // assume unsafe
        QString whitelistPrefix = "[WHITELIST ENTITY SCRIPTS]";
        QList<QString> safeURLPrefixes = { "file:///", "atp:", "cache:" };
        safeURLPrefixes += qEnvironmentVariable("EXTRA_WHITELIST").trimmed().split(QRegExp("\\s*,\\s*"), Qt::SkipEmptyParts);

        // Entity Script Whitelist toggle check.
        Setting::Handle<bool> whitelistEnabled {"private/whitelistEnabled", false };

        if (!whitelistEnabled.get()) {
            passList = true;
        }

        // Pull SAFEURLS from the Interface.JSON settings.
        QVariant raw = Setting::Handle<QVariant>("private/settingsSafeURLS").get();
        QStringList settingsSafeURLS = raw.toString().trimmed().split(QRegExp("\\s*[,\r\n]+\\s*"), Qt::SkipEmptyParts);
        safeURLPrefixes += settingsSafeURLS;
        // END Pull SAFEURLS from the Interface.JSON settings.

        // Get current domain whitelist bypass, in case an entire domain is whitelisted.
        QString currentDomain = DependencyManager::get<AddressManager>()->getDomainURL().host();

        QString domainSafeIP = nodeList->getDomainHandler().getHostname();
        QString domainSafeURL = URL_SCHEME_OVERTE + "://" + currentDomain;
        for (const auto& str : safeURLPrefixes) {
            if (domainSafeURL.startsWith(str) || domainSafeIP.startsWith(str)) {
                qCDebug(scriptengine) << whitelistPrefix << "Whitelist Bypassed, entire domain is whitelisted. Current Domain Host: "
                    << nodeList->getDomainHandler().getHostname()
                    << "Current Domain: " << currentDomain;
                passList = true;
            }
        }
        // END bypass whitelist based on current domain.

        // Start processing scripts through the whitelist.
        if (ScriptManager::getContext() == "entity_server") { // If running on the server, do not engage whitelist.
            passList = true;
        } else if (!passList) { // If waved through, do not engage whitelist.
            for (const auto& str : safeURLPrefixes) {
                qCDebug(scriptengine) << whitelistPrefix << "Script URL: " << scriptOrURL << "TESTING AGAINST" << str << "RESULTS IN"
                    << scriptOrURL.startsWith(str);
                if (!str.isEmpty() && scriptOrURL.startsWith(str)) {
                    passList = true;
                    qCDebug(scriptengine) << whitelistPrefix << "Script approved.";
                    break; // Bail early since we found a match.
                }
            }
        }
        // END processing of scripts through the whitelist.

        if (!passList) { // If the entity failed to pass for any reason, it's blocked and an error is thrown.
            qCDebug(scriptengine) << whitelistPrefix << "(disabled entity script)" << entityID.toString() << scriptOrURL;
            exception = _engine->makeError(_engine->newValue("UNSAFE_ENTITY_SCRIPTS == 0"));
        } else {
            QTimer timeout;
            timeout.setSingleShot(true);
            timeout.start(SANDBOX_TIMEOUT);
            connect(&timeout, &QTimer::timeout, [=, &sandbox] {
                qCDebug(scriptengine) << "ScriptManager::entityScriptContentAvailable timeout";

                // Guard against infinite loops and non-performant code
                sandbox->raiseException(
                    sandbox->makeError(sandbox->newValue(QString("Timed out (entity constructors are limited to %1ms)").arg(SANDBOX_TIMEOUT))));
            });

            testConstructor = sandbox->evaluate(program);

            if (sandbox->hasUncaughtException()) {
                exception = sandbox->cloneUncaughtException(QString("(preflight %1)").arg(entityID.toString()));
                sandbox->clearExceptions();
            } else if (testConstructor.isError()) {
                exception = testConstructor;
            }
        }
      // ENTITY SCRIPT WHITELIST ENDS HERE, uncomment below for original full disabling.

      // qDebug() << "(disabled entity script)" << entityID.toString() << scriptOrURL;
      // exception = makeError("UNSAFE_ENTITY_SCRIPTS == 0");
    }

    if (exception.isError()) {
      // create a local copy using makeError to decouple from the sandbox engine
      exception = _engine->makeError(exception);
      setError(formatException(exception, _enableExtendedJSExceptions.get()), EntityScriptStatus::ERROR_RUNNING_SCRIPT);
      emit unhandledException(exception);
      return;
    }

    // CONSTRUCTOR VIABILITY
    if (!testConstructor.isFunction()) {
        QString testConstructorType = QString(testConstructor.toVariant().typeName());
        if (testConstructorType == "") {
            testConstructorType = "empty";
        }
        QString testConstructorValue = testConstructor.toString();
        if (testConstructorValue.size() > MAX_DEBUG_VALUE_LENGTH) {
            testConstructorValue = testConstructorValue.mid(0, MAX_DEBUG_VALUE_LENGTH) + "...";
        }
        auto message = QString("failed to load entity script -- expected a function, got %1, %2")
            .arg(testConstructorType).arg(testConstructorValue);

        auto err = _engine->makeError(_engine->newValue(message));
        err.setProperty("fileName", scriptOrURL);
        err.setProperty("detail", "(constructor " + entityID.toString() + ")");

        setError("Could not find constructor (" + testConstructorType + ")", EntityScriptStatus::ERROR_RUNNING_SCRIPT);
        emit unhandledException(err);
        return; // done processing script
    }*/

    // (this feeds into refreshFileScript)
    int64_t lastModified = 0;
    if (isFileUrl) {
        QString file = QUrl(scriptOrURL).toLocalFile();
        lastModified = (quint64)QFileInfo(file).lastModified().toMSecsSinceEpoch();
    }

    // THE ACTUAL EVALUATION AND CONSTRUCTION
    ScriptValue entityScriptConstructor, entityScriptObject;
    QUrl sandboxURL = currentSandboxURL.isEmpty() ? scriptOrURL : currentSandboxURL;
    auto initialization = [&]{
        entityScriptConstructor = _engine->evaluate(contents, fileName);
        entityScriptObject = entityScriptConstructor.construct();

        if (_engine->hasUncaughtException()) {
            // V8TODO: Why were we copying the uncaught exception here? Does anything
            // actually make use of that?
            //  entityScriptObject = _engine->cloneUncaughtException("(construct " + entityID.toString() + ")");
            entityScriptObject = _engine->nullValue();
            _engine->clearExceptions();
        }
    };

    doWithEnvironment(entityID, sandboxURL, initialization);

    if (entityScriptObject.isError()) {
       // auto exception = entityScriptObject;
       // setError(formatException(exception, _enableExtendedJSExceptions.get()), EntityScriptStatus::ERROR_RUNNING_SCRIPT);
       // emit unhandledException(exception);
       // V8TODO: Is this needed? Wouldn't the ScriptManager have already emitted the exception?
        return;
    }

    // ... AND WE HAVE LIFTOFF
    newDetails.status = EntityScriptStatus::RUNNING;
    newDetails.scriptObject = entityScriptObject;
    newDetails.lastModified = lastModified;
    newDetails.definingSandboxURL = sandboxURL;
    setEntityScriptDetails(entityID, newDetails);

    if (isURL) {
        setParentURL("");
    }

    // if we got this far, then call the preload method
    callEntityScriptMethod(entityID, "preload");

    emit entityScriptPreloadFinished(entityID);
}

/*@jsdoc
 * Triggered when the script terminates for a user.
 * <p>Note: Can only be connected to via <code>this.unoad = function () { ... }</code> in the entity script.</p>
 * <p class="availableIn"><strong>Supported Script Types:</strong> Client Entity Scripts &bull; Server Entity Scripts</p>
 * @function Entities.unload
 * @param {Uuid} entityID - The ID of the entity that the script is running in.
 * @returns {Signal}
 */
// The JSDoc is for the callEntityScriptMethod() call in this method.
void ScriptManager::unloadEntityScript(const EntityItemID& entityID, bool shouldRemoveFromMap) {
    if (QThread::currentThread() != thread()) {
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine) << "*** WARNING *** ScriptManager::unloadEntityScript() called on wrong thread [" << QThread::currentThread() << "], invoking on correct thread [" << thread() << "]  "
            "entityID:" << entityID;
#endif

        QMetaObject::invokeMethod(this, "unloadEntityScript",
                                  Q_ARG(const EntityItemID&, entityID),
                                  Q_ARG(bool, shouldRemoveFromMap));
        return;
    }
#ifdef THREAD_DEBUGGING
    qCDebug(scriptengine) << "ScriptManager::unloadEntityScript() called on correct thread [" << thread() << "]  "
        "entityID:" << entityID;
#endif

    EntityScriptDetails oldDetails;
    if (getEntityScriptDetails(entityID, oldDetails)) {
        auto scriptText = oldDetails.scriptText;

        if (isEntityScriptRunning(entityID)) {
            callEntityScriptMethod(entityID, "unload");
        }
#ifdef DEBUG_ENTITY_STATES
        else {
            qCDebug(scriptengine) << "unload called while !running" << entityID << oldDetails.status;
        }
#endif
        if (shouldRemoveFromMap) {
            // this was a deleted entity, we've been asked to remove it from the map
            {
                QWriteLocker locker { &_entityScriptsLock };
                _entityScripts.remove(entityID);
            }
            emit entityScriptDetailsUpdated();
        } else if (oldDetails.status != EntityScriptStatus::UNLOADED) {
            EntityScriptDetails newDetails;
            newDetails.status = EntityScriptStatus::UNLOADED;
            newDetails.lastModified = QDateTime::currentMSecsSinceEpoch();
            // keep scriptText populated for the current need to "debouce" duplicate calls to unloadEntityScript
            newDetails.scriptText = scriptText;
            setEntityScriptDetails(entityID, newDetails);
        }

        stopAllTimersForEntityScript(entityID);
    }
}

QList<EntityItemID> ScriptManager::getListOfEntityScriptIDs() {
    QReadLocker locker{ &_entityScriptsLock };
    return _entityScripts.keys();
}

void ScriptManager::unloadAllEntityScripts(bool blockingCall) {
    if (QThread::currentThread() != thread()) {
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine) << "*** WARNING *** ScriptManager::unloadAllEntityScripts() called on wrong thread [" << QThread::currentThread() << "], invoking on correct thread [" << thread() << "]";
#endif

        QMetaObject::invokeMethod(this, "unloadAllEntityScripts",
            blockingCall ? Qt::BlockingQueuedConnection : Qt::QueuedConnection);
        return;
    }
#ifdef THREAD_DEBUGGING
    qCDebug(scriptengine) << "ScriptManager::unloadAllEntityScripts() called on correct thread [" << thread() << "]";
#endif

    QList<EntityItemID> keys;
    {
        QReadLocker locker{ &_entityScriptsLock };
        keys = _entityScripts.keys();
    }
    foreach(const EntityItemID& entityID, keys) {
        unloadEntityScript(entityID);
    }
    {
        QWriteLocker locker{ &_entityScriptsLock };
        _entityScripts.clear();
    }
    emit entityScriptDetailsUpdated();

#ifdef DEBUG_ENGINE_STATE
    _debugDump(
        "---- CURRENT STATE OF ENGINE: --------------------------",
        globalObject(),
        "--------------------------------------------------------"
    );
#endif // DEBUG_ENGINE_STATE
}

void ScriptManager::refreshFileScript(const EntityItemID& entityID) {
    if (!HIFI_AUTOREFRESH_FILE_SCRIPTS || !hasEntityScriptDetails(entityID)) {
        return;
    }

    static bool recurseGuard = false;
    if (recurseGuard) {
        return;
    }
    recurseGuard = true;

    EntityScriptDetails details;
    {
        QWriteLocker locker { &_entityScriptsLock };
        details = _entityScripts[entityID];
    }
    // Check to see if a file based script needs to be reloaded (easier debugging)
    if (details.lastModified > 0) {
        QString filePath = QUrl(details.scriptText).toLocalFile();
        auto lastModified = QFileInfo(filePath).lastModified().toMSecsSinceEpoch();
        if (lastModified > details.lastModified) {
            scriptInfoMessage("Reloading modified script " + details.scriptText);
            loadEntityScript(entityID, details.scriptText, true);
        }
    }
    recurseGuard = false;
}

// Execute operation in the appropriate context for (the possibly empty) entityID.
// Even if entityID is supplied as currentEntityIdentifier, this still documents the source
// of the code being executed (e.g., if we ever sandbox different entity scripts, or provide different
// global values for different entity scripts).
void ScriptManager::doWithEnvironment(const EntityItemID& entityID, const QUrl& sandboxURL, std::function<void()> operation) {
    EntityItemID oldIdentifier = currentEntityIdentifier;
    QUrl oldSandboxURL = currentSandboxURL;
    currentEntityIdentifier = entityID;
    currentSandboxURL = sandboxURL;

#if DEBUG_CURRENT_ENTITY
    ScriptValue oldData = this->globalObject().property("debugEntityID");
    this->globalObject().setProperty("debugEntityID", entityID.toScriptValue(this)); // Make the entityID available to javascript as a global.
    operation();
    this->globalObject().setProperty("debugEntityID", oldData);
#else
    operation();
#endif
    currentEntityIdentifier = oldIdentifier;
    currentSandboxURL = oldSandboxURL;
}

void ScriptManager::callWithEnvironment(const EntityItemID& entityID, const QUrl& sandboxURL, const ScriptValue& function, const ScriptValue& thisObject, const ScriptValueList& args) {
    auto operation = [&]() {
        function.call(thisObject, args);
    };
    doWithEnvironment(entityID, sandboxURL, operation);
}

void ScriptManager::callEntityScriptMethod(const EntityItemID& entityID, const QString& methodName, const QStringList& params, const QUuid& remoteCallerID) {
    if (QThread::currentThread() != thread()) {
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine) << "*** WARNING *** ScriptManager::callEntityScriptMethod() called on wrong thread [" << QThread::currentThread() << "], invoking on correct thread [" << thread() << "]  "
            "entityID:" << entityID << "methodName:" << methodName;
#endif

        QMetaObject::invokeMethod(this, "callEntityScriptMethod",
                                  Q_ARG(const EntityItemID&, entityID),
                                  Q_ARG(const QString&, methodName),
                                  Q_ARG(const QStringList&, params),
                                  Q_ARG(const QUuid&, remoteCallerID));
        return;
    }
#ifdef THREAD_DEBUGGING
    qCDebug(scriptengine) << "ScriptManager::callEntityScriptMethod() called on correct thread [" << thread() << "]  "
        "entityID:" << entityID << "methodName:" << methodName;
#endif

    if (HIFI_AUTOREFRESH_FILE_SCRIPTS && methodName != "unload") {
        refreshFileScript(entityID);
    }
    if (isEntityScriptRunning(entityID)) {
        EntityScriptDetails details;
        {
            QWriteLocker locker { &_entityScriptsLock };
            details = _entityScripts[entityID];
        }
        ScriptValue entityScript = details.scriptObject;  // previously loaded

        // If this is a remote call, we need to check to see if the function is remotely callable
        // we do this by checking for the existance of the 'remotelyCallable' property on the
        // entityScript. And we confirm that the method name is included. If this fails, the
        // function will not be called.
        bool callAllowed = false;
        if (remoteCallerID == QUuid()) {
            callAllowed = true;
        } else {
            if (entityScript.property("remotelyCallable").isArray()) {
                auto callables = entityScript.property("remotelyCallable");
                auto callableCount = callables.property("length").toInteger();
                for (int i = 0; i < callableCount; i++) {
                    auto callable = callables.property(i).toString();
                    if (callable == methodName) {
                        callAllowed = true;
                        break;
                    }
                }
            }
            if (!callAllowed) {
                qDebug() << "Method [" << methodName << "] not remotely callable.";
            }
        }

        if (callAllowed && entityScript.property(methodName).isFunction()) {
            auto scriptEngine = engine().get();

            ScriptValueList args;
            args << EntityItemIDtoScriptValue(scriptEngine, entityID);
            args << scriptValueFromSequence(scriptEngine, params);

            ScriptValue oldData = scriptEngine->globalObject().property("Script").property("remoteCallerID");
            scriptEngine->globalObject().property("Script").setProperty("remoteCallerID", remoteCallerID.toString()); // Make the remoteCallerID available to javascript as a global.
            callWithEnvironment(entityID, details.definingSandboxURL, entityScript.property(methodName), entityScript, args);
            scriptEngine->globalObject().property("Script").setProperty("remoteCallerID", oldData);
        }
    }
}

void ScriptManager::callEntityScriptMethod(const EntityItemID& entityID, const QString& methodName, const PointerEvent& event) {
    if (QThread::currentThread() != thread()) {
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine) << "*** WARNING *** ScriptManager::callEntityScriptMethod() called on wrong thread [" << QThread::currentThread() << "], invoking on correct thread [" << thread() << "]  "
            "entityID:" << entityID << "methodName:" << methodName << "event: mouseEvent";
#endif

        QMetaObject::invokeMethod(this, "callEntityScriptMethod",
                                  Q_ARG(const EntityItemID&, entityID),
                                  Q_ARG(const QString&, methodName),
                                  Q_ARG(const PointerEvent&, event));
        return;
    }
#ifdef THREAD_DEBUGGING
    qCDebug(scriptengine) << "ScriptManager::callEntityScriptMethod() called on correct thread [" << thread() << "]  "
        "entityID:" << entityID << "methodName:" << methodName << "event: pointerEvent";
#endif

    if (HIFI_AUTOREFRESH_FILE_SCRIPTS) {
        refreshFileScript(entityID);
    }
    if (isEntityScriptRunning(entityID)) {
        EntityScriptDetails details;
        {
            QWriteLocker locker { &_entityScriptsLock };
            details = _entityScripts[entityID];
        }
        ScriptValue entityScript = details.scriptObject;  // previously loaded
        if (entityScript.property(methodName).isFunction()) {
            auto scriptEngine = engine().get();

            ScriptValueList args;
            args << EntityItemIDtoScriptValue(scriptEngine, entityID);
            args << event.toScriptValue(scriptEngine);
            callWithEnvironment(entityID, details.definingSandboxURL, entityScript.property(methodName), entityScript, args);
        }
    }
}

void ScriptManager::callEntityScriptMethod(const EntityItemID& entityID, const QString& methodName, const EntityItemID& otherID, const Collision& collision) {
    if (QThread::currentThread() != thread()) {
#ifdef THREAD_DEBUGGING
        qCDebug(scriptengine) << "*** WARNING *** ScriptManager::callEntityScriptMethod() called on wrong thread [" << QThread::currentThread() << "], invoking on correct thread [" << thread() << "]  "
            "entityID:" << entityID << "methodName:" << methodName << "otherID:" << otherID << "collision: collision";
#endif

        QMetaObject::invokeMethod(this, "callEntityScriptMethod",
                                  Q_ARG(const EntityItemID&, entityID),
                                  Q_ARG(const QString&, methodName),
                                  Q_ARG(const EntityItemID&, otherID),
                                  Q_ARG(const Collision&, collision));
        return;
    }
#ifdef THREAD_DEBUGGING
    qCDebug(scriptengine) << "ScriptManager::callEntityScriptMethod() called on correct thread [" << thread() << "]  "
        "entityID:" << entityID << "methodName:" << methodName << "otherID:" << otherID << "collision: collision";
#endif

    if (HIFI_AUTOREFRESH_FILE_SCRIPTS) {
        refreshFileScript(entityID);
    }
    if (isEntityScriptRunning(entityID)) {
        EntityScriptDetails details;
        {
            QWriteLocker locker { &_entityScriptsLock };
            details = _entityScripts[entityID];
        }
        ScriptValue entityScript = details.scriptObject;  // previously loaded
        if (entityScript.property(methodName).isFunction()) {
            auto scriptEngine = engine().get();

            ScriptValueList args;
            args << EntityItemIDtoScriptValue(scriptEngine, entityID);
            args << EntityItemIDtoScriptValue(scriptEngine, otherID);
            args << collisionToScriptValue(scriptEngine, collision);
            callWithEnvironment(entityID, details.definingSandboxURL, entityScript.property(methodName), entityScript, args);
        }
    }
}

QString ScriptManager::getExternalPath(ExternalResource::Bucket bucket, const QString& path) {
    return ExternalResource::getInstance()->getUrl(bucket, path);
}

QString ScriptManager::formatException(const ScriptValue& exception, bool includeExtendedDetails) {
    if (!_engine->IS_THREADSAFE_INVOCATION(__FUNCTION__)) {
        return QString();
    }
    QString note{ "UncaughtException" };
    QString result;

    if (!exception.isObject()) {
        return result;
    }
    const auto message = exception.toString();
    const auto fileName = exception.property("fileName").toString();
    const auto lineNumber = exception.property("lineNumber").toString();
    const auto stacktrace = exception.property("stack").toString();

    if (includeExtendedDetails) {
        // Display additional exception / troubleshooting hints that can be added via the custom Error .detail property
        // Example difference:
        //   [UncaughtExceptions] Error: Can't find variable: foobar in atp:/myentity.js\n...
        //   [UncaughtException (construct {1eb5d3fa-23b1-411c-af83-163af7220e3f})] Error: Can't find variable: foobar in atp:/myentity.js\n...
        if (exception.property("detail").isValid()) {
            note += " " + exception.property("detail").toString();
        }
    }

    result = QString(SCRIPT_EXCEPTION_FORMAT).arg(note, message, fileName, lineNumber);
    if (!stacktrace.isEmpty()) {
        result += QString("\n[Backtrace]%1%2").arg(SCRIPT_BACKTRACE_SEP).arg(stacktrace);
    }
    return result;
}

ScriptValue ScriptManager::evaluate(const QString& program, const QString& fileName) {
    return _engine->evaluate(program, fileName);
}

void ScriptManager::requestGarbageCollection() {
    _engine->requestCollectGarbage();
}

void ScriptManager::logBacktrace(const QString &title) {
    _engine->logBacktrace(title);
}
