//
//  ScriptManager.h
//  libraries/script-engine/src
//
//  Created by Brad Hefta-Gaub on 12/14/13.
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors.
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef hifi_ScriptManager_h
#define hifi_ScriptManager_h

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <unordered_map>
#include <mutex>

#include <QtCore/QFuture>
#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QReadWriteLock>
#include <QtCore/QSet>
#include <QtCore/QSharedPointer>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtCore/QVariant>

#include "EntityItemID.h"
#include "EntitiesScriptEngineProvider.h"
#include "EntityScriptUtils.h"
#include <ExternalResource.h>
#include <SettingHandle.h>

#include "AssetScriptingInterface.h"
#include "ConsoleScriptingInterface.h"
#include "Mat4.h"
#include "PointerEvent.h"
#include "Quat.h"
#include "ScriptUUID.h"
#include "ScriptValue.h"
#include "ScriptException.h"
#include "Vec3.h"

static const QString NO_SCRIPT("");

static const int SCRIPT_FPS = 60;
static const int DEFAULT_MAX_ENTITY_PPS = 9000;
static const int DEFAULT_ENTITY_PPS_PER_SCRIPT = 900;

class ScriptEngine;
class ScriptEngines;
class ScriptManager;
class ScriptManagerScriptingInterface;

using ScriptEnginePointer = std::shared_ptr<ScriptEngine>;
using ScriptManagerPointer = std::shared_ptr<ScriptManager>;
using ScriptManagerScriptingInterfacePointer = std::shared_ptr<ScriptManagerScriptingInterface>;
using ScriptValueList = QList<ScriptValue>;

Q_DECLARE_METATYPE(ScriptManagerPointer)

const int QTREGISTER_QTimerStar = qRegisterMetaType<QTimer*>();


/**
 * @brief Callback data for addEventHandler
 *
 */
class CallbackData {
public:
    /**
     * @brief Function to call
     *
     */
    ScriptValue function;

    /**
     * @brief Entity ID
     *
     */
    EntityItemID definingEntityIdentifier;

    /**
     * @brief Sandbox URL for the script
     *
     */
    QUrl definingSandboxURL;
};


/**
 * @brief DeferredLoadEntity
 * @deprecated This appars unused
 *
 */
class DeferredLoadEntity {
public:
    EntityItemID entityID;
    QString entityScript;
    //bool forceRedownload;
};

/**
 * @brief Entity with available script contents
 *
 */
struct EntityScriptContentAvailable {
    /**
     * @brief Entity ID
     *
     */
    EntityItemID entityID;

    /**
     * @brief URL to the script, or the actual script if embedded in the URL field
     *
     */
    QString scriptOrURL;

    /**
     * @brief Contents of the script
     *
     */
    QString contents;

    /**
     * @brief Whether scriptOrURL contains an URL
     *
     */
    bool isURL;

    /**
     * @brief Whether the request has been successful
     *
     */
    bool success;

    /**
     * @brief Status as text
     *
     */
    QString status;
};

typedef std::unordered_map<EntityItemID, EntityScriptContentAvailable> EntityScriptContentAvailableMap;

typedef QList<CallbackData> CallbackList;
typedef QHash<QString, CallbackList> RegisteredEventHandlers;


/**
 * @brief Details about an entity script
 *
 */
class EntityScriptDetails {
public:

    /**
     * @brief Current status
     *
     */
    EntityScriptStatus status { EntityScriptStatus::PENDING };

    /**
     * @brief Error information
     *
     * If status indicates an error, this contains a human-readable string giving more information about the error.
     *
     */
    QString errorInfo { "" };


    /**
     * @brief The source code of the script
     *
     */
    QString scriptText { "" };

    /**
     * @brief The return value of the script
     *
     */
    ScriptValue scriptObject{ ScriptValue() };

    /**
     * @brief Last modified time of the underlying script file
     *
     * This is used to determine if the script needs reloading when it changes on disk.
     */
    int64_t lastModified { 0 };

    /**
     * @brief URL under which the script is allowed to have access
     *
     * The script is allowed access below this URL (eg, sub-directories), but
     * not to the parent context.
     */
    QUrl definingSandboxURL { QUrl("about:EntityScript") };
};

// declare a static script initializers
#define STATIC_SCRIPT_TYPES_INITIALIZER(init)                                     \
    static ScriptManager::StaticTypesInitializerNode static_script_types_initializer_(init);

#define STATIC_SCRIPT_INITIALIZER(init)                                     \
    static ScriptManager::StaticInitializerNode static_script_initializer_(init);




/**
 * @brief Manages a single scripting engine
 *
 * This class manages and sets up a single scripting engine to make it execute scripts.
 *
 * It passes the objects needed to expose the public API, provides console access and error
 * reporting and event management.
 *
 * This manipulates a single underlying instance of ScriptEngine.
 *
 * Part of this class' functionality exists only to provide helper functions to the scripts that are
 * run by the scripting engine, and shouldn't be considered part of the C++ API. Those are the functions
 * in the "Script support methods", "Module support methods", "Entity Script methods", and "Scripting signals" sections.
 *
 * The script-facing interface is in ScriptManagerScriptingInterface and documented in JSDoc
 * as the <a href="https://apidocs.overte.org/Script.html">Script</a> class.
 *
 * The ScriptManager provides the following functionality to scripts:
 *
 * * A math library: Quat, Vec3, Mat4
 * * UUID generation: Uuid
 * * Filesystem access: File
 * * Console access: console, print
 * * Resource access: Resource, Asset, Resources, ExternalPaths
 * * Scripting system management: Script
 * * Module loading: require
 * * Web access: XMLHttpRequest, WebSocket
 * * Other miscellaneous functionality.
 *
 * Example:
 *
 * @code {.cpp}
 * #include "ScriptManager.h"
 *
 * // Scripts only stop running when Script.stop() is called.
 * // In the normal environment this isn't needed, but for things like unit tests we need
 * // to use it to make the ScriptManager return from run().
 *
 * QString scriptSource = "print(\"Hello, world!\"); Script.stop(true);";
 * QString scriptFilename = "test.js";
 *
 * ScriptManagerPointer sm = newScriptManager(ScriptManager::NETWORKLESS_TEST_SCRIPT, scriptSource, scriptFilename);
 * connect(sm.get(), &ScriptManager::printedMessage, [](const QString& message, const QString& engineName){
 *     qCDebug(scriptengine) << "Printed message from engine" << engineName << ": " << message;
 * });
 *
 * qInfo() << "Running script!";
 * sm->run();
 * qInfo() << "Done!"
 * @endcode
 *
 * @note
 * Technically, the ScriptManager isn't generic enough -- it implements things that imitate
 * Node.js for examine in the module loading code, which makes it JS specific. This code
 * should probably be moved into the JS ScriptEngine class instead.
 *
 * The EntityScript functionality might also benefit from being split off into a separate
 * class, for better organization.
 *
 * Some more functionality can be shifted to ScriptManagerScriptingInterface, since
 * it only provides services to scripts and isn't called from C++.
 */
class ScriptManager : public QObject, public EntitiesScriptEngineProvider, public std::enable_shared_from_this<ScriptManager> {
    Q_OBJECT
    Q_PROPERTY(QString context READ getContext)
    Q_PROPERTY(QString type READ getTypeAsString)
    Q_PROPERTY(QString fileName MEMBER _fileNameString CONSTANT)
public:
    static const QString SCRIPT_EXCEPTION_FORMAT;
    static const QString SCRIPT_BACKTRACE_SEP;

    /**
     * @brief Context of the script
     *
     */
    enum Context {
        /**
         * @brief Client script.
         * Allowed to access local HTML files on UI created from C++ calls.
         *
         */
        CLIENT_SCRIPT,

        /**
         * @brief Entity client script
         *
         */
        ENTITY_CLIENT_SCRIPT,

        /**
         * @brief Entity server script
         *
         */
        ENTITY_SERVER_SCRIPT,

        /**
         * @brief Agent script
         *
         */
        AGENT_SCRIPT,

        /**
         * @brief Network-less test system context.
         * This is used for the QTest self-tests, and minimizes the API that is made available to
         * the running script. It removes the need for network access, which makes for much faster
         * test execution.
         *
         *
         * @warning This is a development-targeted bit of functionality.
         *
         * @warning This is going to break functionality like loadURL and require
         */
        NETWORKLESS_TEST_SCRIPT
    };

    /**
     * @brief Type of the script
     *
     */
    enum Type {
        /**
         * @brief Client
         *
         */
        CLIENT,

        /**
         * @brief Entity client
         * Receives the update event.
         */
        ENTITY_CLIENT,

        /**
         * @brief Entity server
         * Receives the update event
         *
         */
        ENTITY_SERVER,

        /**
         * @brief Agent script
         *
         */
        AGENT,

        /**
         * @brief Avatar script
         *
         */
        AVATAR,

        /**
         * @brief Test system script
         *
         * This is used for the QTest self-tests, and minimizes the API that is made available to
         * the running script. It removes the need for network access, which makes for much faster
         * test execution.
         *
         * @warning This is a development-targeted bit of functionality.
         */
        NETWORKLESS_TEST
    };
    Q_ENUM(Type);

    static int processLevelMaxRetries;
    ScriptManager(Context context, const QString& scriptContents = NO_SCRIPT, const QString& fileNameString = QString("about:ScriptEngine"));
    ~ScriptManager();

    // static initialization support
    typedef void (*ScriptManagerInitializer)(ScriptManager*);
    class StaticInitializerNode {
    public:
        ScriptManagerInitializer init;
        StaticInitializerNode* prev;
        inline StaticInitializerNode(ScriptManagerInitializer&& pInit) : init(std::move(pInit)),prev(nullptr) { registerNewStaticInitializer(this); }
    };
    static void registerNewStaticInitializer(StaticInitializerNode* dest);

    class StaticTypesInitializerNode {
    public:
        ScriptManagerInitializer init;
        StaticTypesInitializerNode* prev;
        inline StaticTypesInitializerNode(ScriptManagerInitializer&& pInit) : init(std::move(pInit)),prev(nullptr) { registerNewStaticTypesInitializer(this); }
    };
    static void registerNewStaticTypesInitializer(StaticTypesInitializerNode* dest);

    /**
     * @brief Run the script in a dedicated thread
     *
     * This will have the side effect of evaluating the current script contents and calling run().
     * Callers will likely want to register the script with external services before calling this.
     *
     * This function will return immediately, and work will continue on the newly created thread.
     *
     * @note Can't be called twice.
     * @note The underlying thread is not accessible.
     */
    void runInThread();

    /**
     * @brief Run the script in the caller's thread, exit when Script.stop() is called.
     *
     * Most scripts never stop running, so this function will never return for them.
     */
    void run();


    /**
     * @brief Get the filename of the running script, without the path.
     *
     * @return QString Filename
     */
    QString getFilename() const;

    /**
     * @brief Underlying scripting engine
     *
     * @return ScriptEnginePointer Scripting engine
     */
    inline ScriptEnginePointer engine() { return _engine; }

    QList<EntityItemID> getListOfEntityScriptIDs();

    bool isStopped() const;


    /**
     * @name Script support functions
     *
     * These functions exist to support the scripting API
     */

    ///@{

    /**
     * @brief Stops and unloads the current script.
     *
     * @note This is part of the public scripting API for Agent scripts and local scripts, but not for EntityScripts
     * @param marshal Deprecated
     */
    Q_INVOKABLE void stop(bool marshal = false);

    ///@}

    /**
     * @brief Stop any evaluating scripts and wait for the scripting thread to finish.
     *
     * @param shutdown True if we are currently shutting down. Setting this to true will allow
     * processing events emitted during the script's shutdown, such as scripts saving settings.
     *
     * @note This function has an internal timeout, and will forcefully abort the script if it
     * takes too long.
     */
    void waitTillDoneRunning(bool shutdown = false);

    /**
     * @brief Load a script from a given URL
     *
     * If the script engine is not already running, this will download the URL and start the process of seting it up
     * to run.
     *
     *
     * @param scriptURL URL where to load the script from. Can be http, https, atp, or file protocol. The file extension
     * has to pass hasValidScriptSuffix().
     * @param reload Load the script again even if it's in the cache.
     *
     * @note For file:// URLs, only URLs under the default scripts location are allowed.
     * @see PathUtils::defaultScriptsLocation
     */
    void loadURL(const QUrl& scriptURL, bool reload);

    /**
     * @brief Determines whether a script filename has the right suffix
     *
     *
     * @param scriptFileName
     * @return true When the script has the right file extension (eg, .js)
     * @return false Otherwise
     */
    bool hasValidScriptSuffix(const QString& scriptFileName);


    /**
     * @name Script support methods
     *
     * These functions exist to support the scripting API
     */

    ///@{

    /**
     * @brief Gets the context that the script is running in: Interface/avatar, client entity, server entity, or assignment client.
     *
     * @note This is part of the public JS API
     * @return QString
     */
    Q_INVOKABLE QString getContext() const;

    /**
     * @brief Checks whether the script is running as an Interface or avatar script.
     * @note This is part of the public JS API
     * @return bool
     */
    Q_INVOKABLE bool isClientScript() const { return _context == CLIENT_SCRIPT; }

    /**
     * @brief Checks whether the application was compiled as a debug build.
     * @note This is part of the public JS API
     * @return bool
     */
    Q_INVOKABLE bool isDebugMode() const;

    /**
     * @brief Checks whether the script is running as a client entity script.
     *
     * @return bool
     */
    Q_INVOKABLE bool isEntityClientScript() const { return _context == ENTITY_CLIENT_SCRIPT; }


    /**
     * @brief  Checks whether the script is running as a server entity script.
     *
     * @return bool
     */
    Q_INVOKABLE bool isEntityServerScript() const { return _context == ENTITY_SERVER_SCRIPT; }

    /**
     * @brief Checks whether the script is running as an assignment client script.
     *
     * @return bool
     */
    Q_INVOKABLE bool isAgentScript() const { return _context == AGENT_SCRIPT; }

    /**
     * @brief Registers a global object by name.
     *
     * @param valueName
     * @param value
     */
    Q_INVOKABLE void registerValue(const QString& valueName, ScriptValue value);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NOTE - these are intended to be public interfaces available to scripts

    /**
     * @brief Format an exception and return it as a string
     *
     * @param exception Exception object, containing the exception information.
     * @param includeExtendedDetails Include additional troubleshooting information from the "detail" property, if there's one
     * @return QString A multi-line string containing the formatted exception
     */
    Q_INVOKABLE QString formatException(const ScriptValue& exception, bool includeExtendedDetails);



    /**
     * @brief Adds a function to the list of functions called when a particular event occurs on a particular entity.
     *
     * @param entityID Entity ID
     * @param eventName Name of the event
     * @param handler Event handler
     *
     * @note The same handler can be added multiple times.
     */
    Q_INVOKABLE void addEventHandler(const EntityItemID& entityID, const QString& eventName, const ScriptValue& handler);

    /**
     * @brief Removes a function from the list of functions called when an entity event occurs on a particular entity.
     *
     * @param entityID Entity ID
     * @param eventName Name if the event
     * @param handler Event handler
     */
    Q_INVOKABLE void removeEventHandler(const EntityItemID& entityID, const QString& eventName, const ScriptValue& handler);


    /**
     * @brief Starts running another script in Interface, if it isn't already running. The script is not automatically loaded next
     * time Interface starts.
     *
     * The script is loaded as a stand-alone script.
     *
     * @param loadfile File to load
     * @warning In practice this seems equivalent to calling loadScript or reloadScript. It reacts to _isReloading in an odd-looking manner.
     * Is this function superfluous?
     */
    Q_INVOKABLE void load(const QString& loadfile);

    /**
     * @brief Includes JavaScript from other files in the current script.
     *
     * If a callback is specified, the included files will be loaded asynchronously and the callback will be called
     * when all of the files have finished loading.
     * If no callback is specified, the included files will be loaded synchronously and will block execution until
     * all of the files have finished loading.
     *
     * @param includeFiles List of files to include
     * @param callback Callback to call when the files have finished loading.
     */
    Q_INVOKABLE void include(const QStringList& includeFiles, const ScriptValue& callback = ScriptValue());

    /**
     * @brief Includes JavaScript from another file in the current script.
     *
     * If a callback is specified, the included files will be loaded asynchronously and the callback will be called
     * when all of the files have finished loading.
     * If no callback is specified, the included files will be loaded synchronously and will block execution until
     * all of the files have finished loading.
     *
     * @param includeFile
     * @param callback
     */
    Q_INVOKABLE void include(const QString& includeFile, const ScriptValue& callback = ScriptValue());

    ///@}



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    /**
     * @name Module support methods
     *
     */
    ///@{

    /**
     * @brief Provides access to methods or objects provided in an external JavaScript or JSON file.
     *
     * Implements CommonJS/Node.js like require/module support
     *
     * @param moduleId Module to load
     * @return ScriptValue
     */
    Q_INVOKABLE ScriptValue require(const QString& moduleId);

    /**
     * @brief Resets the module cache
     *
     * @deprecated
     * @param deleteScriptCache
     */
    Q_INVOKABLE void resetModuleCache(bool deleteScriptCache = false);

    /**
     * @brief The current parent module from the running JS script
     *
     *
     * @return ScriptValue Module. May be null or empty.
     */
    ScriptValue currentModule();

    /**
     * @brief Replaces or adds "module" to "parent.children[]" array
     *
     * This is an internal use function used as a part of the 'require' implementation.
     *
     * @param module Module to register
     * @param parent Parent
     * @return true Registration successful
     * @return false Registration failed, if the parent isn't a valid module
     */
    bool registerModuleWithParent(const ScriptValue& module, const ScriptValue& parent);

    /**
     * @brief creates a new JS "module" Object with default metadata properties
     *
     * This imitates what is provided by https://nodejs.org/api/modules.html
     *
     * @param modulePath File path to the module
     * @param parent Parent module
     * @return ScriptValue Created module object
     */
    ScriptValue newModule(const QString& modulePath, const ScriptValue& parent = ScriptValue());

    /**
     * @brief Synchronously fetch a module's source code
     *
     * The return value is a map containing the following fields:
     *
     * * "status" -- A string indicating the status of the operation
     * * "success" -- A true or false value indicating success or failure
     * * "url" -- The URL of the source. May not be present.
     * * "contents" -- The contents of the source. May not be present.
     *
     * @param modulePath Path to the module's source code
     * @param forceDownload Force a redownload even if the source is already in the cache
     * @return QVariantMap The result of the operation
     */
    QVariantMap fetchModuleSource(const QString& modulePath, const bool forceDownload = false);

    /**
     * @brief Evaluate a pending module object using the fetched source code
     *
     * @param module Module object
     * @param sourceCode Source code to evaluate
     * @return ScriptValue The result of evaluating the source code
     */
    ScriptValue instantiateModule(const ScriptValue& module, const QString& sourceCode);

    /**
     * @brief Evaluate a program in the underlying scripting engine
     *
     * This simply calls to ScriptEngine::evaluate()
     *
     * @param program Source of the program
     * @param fileName Filename it was obtained from
     * @return ScriptValue Result of the evaluation
     */
    ScriptValue evaluate(const QString& program, const QString& fileName = QString());

   /**
    * @brief Calls a function repeatedly, at a set interval.
    *
    * @note This is a JS API service.
    *
    * @param function Function to call
    * @param intervalMS Interval at which to call the function, in ms
    * @return QTimer* A pointer to the timer
    */
    Q_INVOKABLE QTimer* setInterval(const ScriptValue& function, int intervalMS);


    /**
     * @brief  Calls a function once, after a delay.
     *
     * @note This is a JS API service.
     *
     * @param function Function to call
     * @param timeoutMS How long to wait before calling the function, in ms
     * @return QTimer* A pointer to the timer
     */
    Q_INVOKABLE QTimer* setTimeout(const ScriptValue& function, int timeoutMS);

    /**
     * @brief Stops an interval timer
     *
     * @param timer Timer to stop
     */
    Q_INVOKABLE void clearInterval(QTimer* timer) { stopTimer(timer); }

    /**
     * @brief Stops an interval timer
     *
     * Overloaded version is needed in case the timer has expired
     *
     * @param timer Timer to stop
     */
    Q_INVOKABLE void clearInterval(QVariantMap timer) { ; }

    /**
     * @brief Stops a timeout timer
     *
     * @param timer Timer to stop
     */
    Q_INVOKABLE void clearTimeout(QTimer* timer) { stopTimer(timer); }

    /**
     * @brief Stops a timeout timer
     * Overloaded version is needed in case the timer has expired
     *
     * @param timer Timer to stop
     */

    Q_INVOKABLE void clearTimeout(QVariantMap timer) { ; }


    /**
     * @brief Prints a message to the program log
     *
     * @param message
     */
    Q_INVOKABLE void print(const QString& message);

    /**
     * @brief Resolves a relative path to an absolute path. The relative path is relative to the script's location.
     *
     * @param path
     * @return QUrl
     */
    Q_INVOKABLE QUrl resolvePath(const QString& path) const;

    /**
     * @brief Gets the path to the resources directory for QML files.
     *
     * @return QUrl
     */
    Q_INVOKABLE QUrl resourcesPath() const;

    /**
     * @brief Starts timing a section of code in order to send usage data about it to Overte. Shouldn't be used outside of the
     * standard scripts.
     * @param label
     */
    Q_INVOKABLE void beginProfileRange(const QString& label) const;

    /**
     * @brief Finishes timing a section of code in order to send usage data about it to Overte. Shouldn't be used outside of
     * the standard scripts
     * @param label
     */
    Q_INVOKABLE void endProfileRange(const QString& label) const;


    ///@}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Entity Script Related methods


    /**
     * @name Entity Script methods
     *
     */

    ///@{

    /**
     * @brief Checks whether an entity has an entity script running.
     *
     * @param entityID
     * @return bool
     */
    Q_INVOKABLE bool isEntityScriptRunning(const EntityItemID& entityID) {
        QReadLocker locker { &_entityScriptsLock };
        auto it = _entityScripts.constFind(entityID);
        return it != _entityScripts.constEnd() && it->status == EntityScriptStatus::RUNNING;
    }

    /**
     * @brief Clone the details of an entity script
     *
     * @param entityID Entity ID
     * @return QVariant Copy of the details
     */
    QVariant cloneEntityScriptDetails(const EntityItemID& entityID);


    /**
     * @brief Get the details of a local entity script
     *
     * Same as cloneEntityScriptDetails, only as a QFuture.
     *
     * @param entityID Entity ID
     * @return QFuture<QVariant>
     */
    QFuture<QVariant> getLocalEntityScriptDetails(const EntityItemID& entityID) override;


    /**
     * @brief Manually runs the JavaScript garbage collector which reclaims memory by disposing of objects that are no longer reachable
     *
     */
    Q_INVOKABLE void requestGarbageCollection();

    /**
     * @brief Prints out current backtrace to the log.
     *
     * @param title
     */
    Q_INVOKABLE void logBacktrace(const QString &title);

    /**
     * @brief Load an entity script
     *
     * @param entityID
     * @param entityScript
     * @param forceRedownload
     */
    Q_INVOKABLE void loadEntityScript(const EntityItemID& entityID, const QString& entityScript, bool forceRedownload);

    /**
     * @brief Unload an entity script
     *
     * @param entityID
     * @param shouldRemoveFromMap
     */
    Q_INVOKABLE void unloadEntityScript(const EntityItemID& entityID, bool shouldRemoveFromMap = false); // will call unload method


    /**
     * @brief Unload all entity scripts
     *
     * @param blockingCall
     */
    Q_INVOKABLE void unloadAllEntityScripts(bool blockingCall = false);

    /**
     * @brief Call a method on an entity script
     *
     * @param entityID
     * @param methodName
     * @param params
     * @param remoteCallerID
     */
    Q_INVOKABLE void callEntityScriptMethod(const EntityItemID& entityID, const QString& methodName,
                                            const QStringList& params = QStringList(),
                                            const QUuid& remoteCallerID = QUuid()) override;

    /**
     * @brief Call a method on an entity script
     *
     * @param entityID
     * @param methodName
     * @param event
     */
    Q_INVOKABLE void callEntityScriptMethod(const EntityItemID& entityID, const QString& methodName, const PointerEvent& event);


   /**
    * @brief Call a method on an entity script
    *
    * @param entityID
    * @param methodName
    * @param otherID
    * @param collision
    */
    Q_INVOKABLE void callEntityScriptMethod(const EntityItemID& entityID, const QString& methodName, const EntityItemID& otherID, const Collision& collision);


    /**
     * @brief Set the script type
     *
     * @param type Type of this script
     */
    void setType(Type type) { _type = type; };

    /**
     * @brief Returns the script type
     *
     * @return Type of this script
     */
    Type getType() { return _type; };

    /**
     * @brief Returns the type of the script as a string
     *
     * @return QString A string describing the script's type
     */
    QString getTypeAsString() const;

    /**
     * @brief Whether the script has finished running
     *
     * The finished status is set by stop()
     * @return true The script has finished
     * @return false The script is running
     */
    bool isFinished() const { return _isFinished; } // used by Application and ScriptWidget

    /**
     * @brief Whether the script is running
     *
     * @return true The script is running
     * @return false The script is not running
     */
    bool isRunning() const { return _isRunning; } // used by ScriptWidget

    // this is used by code in ScriptEngines.cpp during the "reload all" operation
    /**
     * @brief Whether this ScriptManager is stopping. Once this is true, it stays true.
     *
     * @return true We are stopping
     * @return false We are not stopping
     */
    bool isStopping() const { return _isStopping; }

    /**
     * @brief Disconnect all signals, except essential ones
     *
     * This disconnects all signals, except the destroyed() and finished() handlers that
     * are needed for cleanup.
     */
    void disconnectNonEssentialSignals();


    ///@}


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Set whether this script was user-loaded
     *
     * This is used by Application to track if a script is user loaded or not.
     * @note Consider finding a solution inside of Application so that the ScriptManager class is not polluted by this notion
     *
     * @param isUserLoaded Script is user-loaded.
     */
    void setUserLoaded(bool isUserLoaded) { _isUserLoaded = isUserLoaded; }

    /**
     * @brief Whether the script was user-loaded.
     *
     * This is used by Application to track if a script is user loaded or not.
     * @note Consider finding a solution inside of Application so that the ScriptManager class is not polluted by this notion
     *
     * @return true
     * @return false
     */
    bool isUserLoaded() const { return _isUserLoaded; }

    /**
     * @brief Set whether to quit when finished
     *
     * This is used by ScriptEngines
     *
     * @param quitWhenFinished
     */
    void setQuitWhenFinished(const bool quitWhenFinished) { _quitWhenFinished = quitWhenFinished; }

    /**
     * @brief Whether to quit when finished
     *
     * This is read by ScriptEngines
     *
     * @return true
     * @return false
     */
    bool isQuitWhenFinished() const { return _quitWhenFinished; }

    /**
     * @brief Set a function that determines whether to emit update events
     *
     * Function func will be called from run() to determine whether update() will be issued.
     * The update() event will be called if func() returns true.
     *
     * @param func Function that determines whether update() events will be issued.
     */
    void setEmitScriptUpdatesFunction(std::function<bool()> func) { _emitScriptUpdates = func; }

    /**
     * @brief Logs a script error message and emits an errorMessage event
     *
     * Emits errorMessage()
     *
     * @param message Message to send to the log
     */
    void scriptErrorMessage(const QString& message);

    /**
     * @brief Logs a script warning message and emits an warningMessage event
     *
     * Emits warningMessage()
     *
     * @param message Message to send to the log
     */
    void scriptWarningMessage(const QString& message);

    /**
     * @brief Logs a script info message and emits an infoMessage event
     *
     * Emits infoMessage()
     *
     * @param message Message to send to the log
     */
    void scriptInfoMessage(const QString& message);

    /**
     * @brief Logs a script printed message and emits an printedMessage event
     *
     * These are messages scripts provide by calling the print function.
     * Emits printedMessage()
     *
     * @param message Message to send to the log
     */

    void scriptPrintedMessage(const QString& message);

    /**
     * @brief Clears the debug log window
     *
     * This only emits clearDebugWindow()
     *
     */
    void clearDebugLogWindow();

    /**
     * @brief Get the number of running entity scripts
     *
     * @return int Number of scripts with the status EntityScriptStatus::RUNNING
     */
    int getNumRunningEntityScripts() const;

    /**
     * @brief Retrieves the details about an entity script
     *
     * @param entityID Entity ID
     * @param details Returned details
     * @return true If the entity ID was found
     * @return false If the entity ID wasn't found. details will be unmodified.
     */
    bool getEntityScriptDetails(const EntityItemID& entityID, EntityScriptDetails &details) const;

    /**
     * @brief Whether there are script details for a given entity ID
     *
     * @param entityID Entity ID
     * @return true There is an entity script for this entity
     * @return false There's no entity script
     */
    bool hasEntityScriptDetails(const EntityItemID& entityID) const;

    /**
     * @brief Set a shared pointer to the ScriptEngines class
     *
     * This is used to ask ScriptEngines whether the system is being stopped.
     * Setting this is optional.
     *
     * isStopped() is implemented by asking ScriptEngines.
     *
     * @param scriptEngines ScriptEngines class
     */
    void setScriptEngines(QSharedPointer<ScriptEngines>& scriptEngines) { _scriptEngines = scriptEngines; }


    /**
     * @brief Call all the registered event handlers on an entity for the specified name.
     *
     * Look up the handler associated with eventName and entityID. If found, evalute the argGenerator thunk and call the handler with those args
     *
     * @param entityID
     * @param eventName
     * @param eventHanderArgs
     */
    void forwardHandlerCall(const EntityItemID& entityID, const QString& eventName, const ScriptValueList& eventHanderArgs);

    /**
     * @brief Remove all event handlers for the specified entityID (i.e. the entity is being removed)
     *
     * @param entityID Entity ID
     */
    void removeAllEventHandlers(const EntityItemID& entityID);


    /**
     * @brief Return value of the script that finished running
     *
     * This should only be used after the script terminates.
     *
     * @return ScriptValue
     */
    ScriptValue getReturnValue() const { return _returnValue; }


    /**
     * @brief Gets the URL for an asset in an external resource bucket.
     *
     * @param bucket
     * @param path
     * @return Q_INVOKABLE
     */
    Q_INVOKABLE QString getExternalPath(ExternalResource::Bucket bucket, const QString& path);


    /**
     * @brief Get the uncaught exception from the underlying script engine
     *
     * @return std::shared_ptr<ScriptException> Exception
     */
    std::shared_ptr<ScriptException> getUncaughtException() const;

    /**
     * @brief Whether this engine will abort on an uncaught exception
     *
     * @warning This probably should be refactored into a more comprehensive per-script flags system
     * @return true
     * @return false
     */
    bool getAbortOnUncaughtException() const { return _abortOnUncaughtException; }

    /**
     * @brief Whether to abort on an uncaught exception
     *
     * @warning This probably should be refactored into a more comprehensive per-script flags system
     * @param value
     */
    void setAbortOnUncaughtException(bool value) { _abortOnUncaughtException = value; }

    /**
     * @brief Returns true after script finished running and doneRunning signal was called
     *
     * @return true If the script and doneRunning signal was called
     * @return false If the script has not finished running yet
     */
    bool isDoneRunning() { return _isDoneRunning; };

public slots:

    /**
     * @brief Script.updateMemoryCost
     *
     * Sends a memory cost update to the underlying scripting engine
     *
     * @param deltaSize Difference in memory usage
     * @deprecated Deprecated
     */
    void updateMemoryCost(const qint64 &deltaSize);

signals:


    /**
     * @name Scripting events
     *
     */
    ///@{

    /**
     * @brief Script.scriptLoaded
     * @deprecated
     * @param scriptFilename
     */
    void scriptLoaded(const QString& scriptFilename);


    /**
     * @brief Script.errorLoadingScript
     * @deprecated
     * @param scriptFilename
     */
    void errorLoadingScript(const QString& scriptFilename);


    /**
     * @brief Triggered frequently at a system-determined interval.
     *
     * @param deltaTime
     */
    void update(float deltaTime);



    /**
     * @brief Triggered when the script is stopping.
     *
     */
    void scriptEnding();



    /**
     * @brief Script.finished
     *
     * @param fileNameString
     */
    void finished(const QString& fileNameString, ScriptManagerPointer);

    /**
     * @brief Triggered when the script prints a message to the program log
     *
     * @param message
     * @param scriptName
     */
    void printedMessage(const QString& message, const QString& scriptName);


    /**
     * @brief Triggered when the script generates an error
     *
     * @param message
     * @param scriptName
     */
    void errorMessage(const QString& message, const QString& scriptName);



    /**
     * @brief  Triggered when the script generates a warning
     *
     * @param message
     * @param scriptName
     */
    void warningMessage(const QString& message, const QString& scriptName);


    /**
     * @brief Triggered when the script generates an information message
     *
     * @param message
     * @param scriptName
     */
    void infoMessage(const QString& message, const QString& scriptName);


    /**
     * @brief Triggered when the running state of the script changes, e.g., from running to stopping.
     *
     */
    void runningStateChanged();


    /**
     * @brief Script.clearDebugWindow
     * @deprecated
     */
    void clearDebugWindow();


    /**
     * @brief Script.loadScript
     * @deprecated
     * @param scriptName
     * @param isUserLoaded
     */
    void loadScript(const QString& scriptName, bool isUserLoaded);


    /**
     * @brief  Script.reloadScript
     *
     * @param scriptName
     * @param isUserLoaded
     */
    void reloadScript(const QString& scriptName, bool isUserLoaded);

    /**
     * @brief Triggered when the script has stopped.
     *
     */
    void doneRunning();

    /**
     * @brief Emitted when an entity script is added or removed, or when the status of an entity
     * script is updated (goes from RUNNING to ERROR_RUNNING_SCRIPT, for example)
     *
     */
    void entityScriptDetailsUpdated();

    /**
     * @brief Emitted when an entity script has finished running preload
     *
     * @param entityID
     */
    void entityScriptPreloadFinished(const EntityItemID& entityID);


    /**
     * @brief Triggered when a script generates an unhandled exception.
     *
     * @param exception
     */
    void unhandledException(std::shared_ptr<ScriptException> exception);

    ///@}

    /**
     * @brief Triggered once before the first call to Script.addEventHandler happens on this ScriptManager
     * connections assumed to use Qt::DirectConnection
     *
     * @note not for use by scripts
     */
    void attachDefaultEventHandlers();

    /**
     * @brief Triggered repeatedly in the scripting loop to ensure entity edit messages get processed properly
     * connections assumed to use Qt::DirectConnection
     *
     * @note Not for use by scripts
     * @param wait
     */
    void releaseEntityPacketSenderMessages(bool wait);

protected:
    // Is called by the constructor, bceause all types need to be registered before method discovery with ScriptObjectV8Proxy::investigate()
    void initMetaTypes();

    /**
     * @brief Initializes the underlying scripting engine
     *
     * This sets up the scripting engine with the default APIs
     */
    void init();

    /**
     * @brief executeOnScriptThread
     *
     * @deprecated
     * @param function
     * @param type
     */
    Q_INVOKABLE void executeOnScriptThread(std::function<void()> function, const Qt::ConnectionType& type = Qt::QueuedConnection );

    /**
     * @brief Script._requireResolve
     *
     * @note this is not meant to be called directly, but just to have QMetaObject take care of wiring it up in general;
     * then inside of init() we just have to do "Script.require.resolve = Script._requireResolve;"
     *
     * @deprecated
     * @param moduleId
     * @param relativeTo
     * @return QString
     */
    Q_INVOKABLE QString _requireResolve(const QString& moduleId, const QString& relativeTo = QString());

    /**
     * @brief Log an exception
     *
     * This both sends an exception to the log as an error message, and returns the formatted
     * text as a string.
     *
     * @param exception Exception
     * @return QString Exception formatted as a string
     */
    QString logException(const ScriptValue& exception);
    void timerFired();
    void stopAllTimers();
    void stopAllTimersForEntityScript(const EntityItemID& entityID);
    void refreshFileScript(const EntityItemID& entityID);

    /**
     * @brief Updates the status of an entity script
     *
     * Emits entityScriptDetailsUpdated()
     *
     * @param entityID Entity ID
     * @param status Status
     * @param errorInfo Description of the error, if any
     */
    void updateEntityScriptStatus(const EntityItemID& entityID, const EntityScriptStatus& status, const QString& errorInfo = QString());


    /**
     * @brief Set the details for an entity script
     *
     * @param entityID Entity ID
     * @param details Details
     */
    void setEntityScriptDetails(const EntityItemID& entityID, const EntityScriptDetails& details);

    /**
     * @brief Set the parent URL, used to resolve relative paths
     *
     * Relative paths are resolved respect of this URL
     *
     * @param parentURL Parent URL
     */
    void setParentURL(const QString& parentURL) { _parentURL = parentURL; }

    /**
     * @brief Creates a timer with the specified interval
     *
     * @param function Function to call when the interval elapses
     * @param intervalMS Interval in milliseconds
     * @param isSingleShot Whether the timer happens continuously or a single time
     * @return QTimer*
     */
    QTimer* setupTimerWithInterval(const ScriptValue& function, int intervalMS, bool isSingleShot);

    /**
     * @brief Stops a timer
     *
     * @param timer Timer to stop
     */
    void stopTimer(QTimer* timer);

    QHash<EntityItemID, RegisteredEventHandlers> _registeredHandlers;


    /**
     * @brief Script.entityScriptContentAvailable
     *
     * @param entityID
     * @param scriptOrURL
     * @param contents
     * @param isURL
     * @param success
     * @param status
     */
    Q_INVOKABLE void entityScriptContentAvailable(const EntityItemID& entityID, const QString& scriptOrURL, const QString& contents, bool isURL, bool success, const QString& status);

    EntityItemID currentEntityIdentifier; // Contains the defining entity script entity id during execution, if any. Empty for interface script execution.
    QUrl currentSandboxURL; // The toplevel url string for the entity script that loaded the code being executed, else empty.

    /**
     * @brief Execute operation in the appropriate context for (the possibly empty) entityID.
     * Even if entityID is supplied as currentEntityIdentifier, this still documents the source
     * of the code being executed (e.g., if we ever sandbox different entity scripts, or provide different
     * global values for different entity scripts).
     *
     * @param entityID Entity ID, may be null
     * @param sandboxURL Sandbox URL
     * @param operation Operation to call
     */
    void doWithEnvironment(const EntityItemID& entityID, const QUrl& sandboxURL, std::function<void()> operation);

    /**
     * @brief Execute operation in the appropriate context for (the possibly empty) entityID.
     *
     * This is equivalent to doWithEnvironment(), only with separate arguments for the function, this object and arguments.
     *
     * This is a convenience function, which performs:
     *
     * @code {.cpp}
     * auto operation = [&]() {
     *   function.call(thisObject, args);
     * };
    *  doWithEnvironment(entityID, sandboxURL, operation);
     * @endcode
     *
     * @param entityID Entity ID, may be null
     * @param sandboxURL Sandbox URL
     * @param function  Function to call
     * @param thisObject "this" object to use for the call
     * @param args Arguments
     */
    void callWithEnvironment(const EntityItemID& entityID, const QUrl& sandboxURL, const ScriptValue& function, const ScriptValue& thisObject, const ScriptValueList& args);

    Context _context;
    Type _type;
    ScriptEnginePointer _engine;
    QString _scriptContents;
    QString _parentURL;
    std::atomic<bool> _isFinished { false };
    std::atomic<bool> _isRunning { false };
    std::atomic<bool> _isStopping { false };
    std::atomic<bool> _isDoneRunning { false };
    bool _areMetaTypesInitialized { false };
    bool _isInitialized { false };
    QHash<QTimer*, CallbackData> _timerFunctionMap;
    QSet<QUrl> _includedURLs;
    mutable QReadWriteLock _entityScriptsLock { QReadWriteLock::Recursive };
    QHash<EntityItemID, EntityScriptDetails> _entityScripts;
    EntityScriptContentAvailableMap _contentAvailableQueue;
    ScriptValue _returnValue;

    bool _isThreaded { false };
    qint64 _lastUpdate;

    QString _fileNameString;
    Quat _quatLibrary;
    Vec3 _vec3Library;
    Mat4 _mat4Library;
    ScriptUUID _uuidLibrary;
    ConsoleScriptingInterface _consoleScriptingInterface;
    std::atomic<bool> _isUserLoaded { false };
    bool _isReloading { false };

    std::atomic<bool> _quitWhenFinished;

    AssetScriptingInterface* _assetScriptingInterface;

    std::function<bool()> _emitScriptUpdates{ []() { return true; }  };

    std::recursive_mutex _lock;

    std::chrono::microseconds _totalTimerExecution { 0 };

    static const QString _SETTINGS_ENABLE_EXTENDED_EXCEPTIONS;

    Setting::Handle<bool> _enableExtendedJSExceptions { _SETTINGS_ENABLE_EXTENDED_EXCEPTIONS, true };

    QWeakPointer<ScriptEngines> _scriptEngines;



    // For debugging performance issues
    int _timerCallCounter{ 0 };
    double _totalTimeInTimerEvents_s{ 0.0 };

    ScriptManagerScriptingInterfacePointer _scriptingInterface;

    bool _abortOnUncaughtException{ false };

    friend ScriptManagerPointer newScriptManager(Context context, const QString& scriptContents, const QString& fileNameString);
    friend class ScriptManagerScriptingInterface;

};

/**
 * @brief Creates a new ScriptManager
 *
 * @param context Context in which scripts will run
 * @param scriptContents Contents of the script to run
 * @param fileNameString Filename for the script
 * @return ScriptManagerPointer
 */
ScriptManagerPointer newScriptManager(ScriptManager::Context context,
                                      const QString& scriptContents,
                                      const QString& fileNameString);

/**
 * @brief Creates a new ScriptManager and adds it to ScriptEngines
 *
 * Same as newScriptManager, but it additionally registers the new
 * ScriptManager with ScriptEngines.
 *
 * @param context Context in which scripts will run
 * @param scriptContents Contents of the script
 * @param fileNameString Filename of the script
 * @return ScriptManagerPointer
 */
ScriptManagerPointer scriptManagerFactory(ScriptManager::Context context,
                                        const QString& scriptContents,
                                        const QString& fileNameString);

#endif // hifi_ScriptManager_h
