//
//  ScriptManagerScriptingInterface.h
//  libraries/script-engine/src
//
//  Created by Dale Glass on 24/02/2023.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <QObject>
#include "ScriptManager.h"


/*@jsdoc
 * The <code>Script</code> API provides facilities for working with scripts.
 *
 * @namespace Script
 *
 * @hifi-interface
 * @hifi-client-entity
 * @hifi-avatar
 * @hifi-server-entity
 * @hifi-assignment-client
 *
 * @property {string} context - The context that the script is running in:
 *     <ul>
 *       <li><code>"client"</code>: An Interface or avatar script.</li>
 *       <li><code>"entity_client"</code>: A client entity script.</li>
 *       <li><code>"entity_server"</code>: A server entity script.</li>
 *       <li><code>"agent"</code>: An assignment client script.</li>
 *     </ul>
 *     <em>Read-only.</em>
 * @property {string} type - The type of script that is running:
 *     <ul>
 *       <li><code>"client"</code>: An Interface script.</li>
 *       <li><code>"entity_client"</code>: A client entity script.</li>
 *       <li><code>"avatar"</code>: An avatar script.</li>
 *       <li><code>"entity_server"</code>: A server entity script.</li>
 *       <li><code>"agent"</code>: An assignment client script.</li>
 *     </ul>
 *     <em>Read-only.</em>
 * @property {string} filename - The filename of the script file.
 *     <em>Read-only.</em>
 * @property {Script.ResourceBuckets} ExternalPaths - External resource buckets.
 */

// V8TODO: this should be moved to somewhere test-related
class TestQObject : public QObject {
    Q_OBJECT
public:
    //Q_INVOKABLE virtual void testMethod() { qDebug() << "TestQObject::testMethod"; };
};

class ScriptManagerScriptingInterface : public QObject {
    Q_OBJECT
public:
    ScriptManagerScriptingInterface(ScriptManager *parent);

    virtual ~ScriptManagerScriptingInterface() {

    }
    /*@jsdoc
     * Stops and unloads the current script.
     * <p><strong>Warning:</strong> If an assignment client script, the script gets restarted after stopping.</p>
     * @function Script.stop
     * @example <caption>Stop a script after 5s.</caption>
     * Script.setInterval(function () {
     *     print("Hello");
     * }, 1000);
     *
     * Script.setTimeout(function () {
     *     Script.stop();
     * }, 5000);
     */
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NOTE - this is intended to be a public interface for Agent scripts, and local scripts, but not for EntityScripts
    Q_INVOKABLE void stop(bool marshal = false) { _manager->stop(marshal); }

    /*@jsdoc
     * Gets the context that the script is running in: Interface/avatar, client entity, server entity, or assignment client.
     * @function Script.getContext
     * @returns {string} The context that the script is running in:
     * <ul>
     *   <li><code>"client"</code>: An Interface or avatar script.</li>
     *   <li><code>"entity_client"</code>: A client entity script.</li>
     *   <li><code>"entity_server"</code>: A server entity script.</li>
     *   <li><code>"agent"</code>: An assignment client script.</li>
     * </ul>
     */
    Q_INVOKABLE QString getContext() const { return _manager->getContext(); }


    /*@jsdoc
     * Checks whether the script is running as an Interface or avatar script.
     * @function Script.isClientScript
     * @returns {boolean} <code>true</code> if the script is running as an Interface or avatar script, <code>false</code> if it
     *     isn't.
     */
    Q_INVOKABLE bool isClientScript() const { return _manager->isClientScript(); }

    /*@jsdoc
     * Checks whether the application was compiled as a debug build.
     * @function Script.isDebugMode
     * @returns {boolean} <code>true</code> if the application was compiled as a debug build, <code>false</code> if it was
     *     compiled as a release build.
     */
    Q_INVOKABLE bool isDebugMode() const { return _manager->isDebugMode(); }

    /*@jsdoc
     * Checks whether the script is running as a client entity script.
     * @function Script.isEntityClientScript
     * @returns {boolean} <code>true</code> if the script is running as a client entity script, <code>false</code> if it isn't.
     */
    Q_INVOKABLE bool isEntityClientScript() const { return _manager->isEntityClientScript(); }

    /*@jsdoc
     * Checks whether the script is running as a server entity script.
     * @function Script.isEntityServerScript
     * @returns {boolean} <code>true</code> if the script is running as a server entity script, <code>false</code> if it isn't.
     */
    Q_INVOKABLE bool isEntityServerScript() const { return _manager->isEntityServerScript(); }

    /*@jsdoc
     * Checks whether the script is running as an assignment client script.
     * @function Script.isAgentScript
     * @returns {boolean} <code>true</code> if the script is running as an assignment client script, <code>false</code> if it
     *     isn't.
     */
    Q_INVOKABLE bool isAgentScript() const { return _manager->isAgentScript(); }

    /*@jsdoc
     * registers a global object by name.
     * @function Script.registerValue
     * @param {string} valueName
     * @param {value} value
     */
    /// registers a global object by name
    Q_INVOKABLE void registerValue(const QString& valueName, ScriptValue value) { _manager->registerValue(valueName, value); }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NOTE - these are intended to be public interfaces available to scripts

    // TODO: Deprecated by documentation, please review for accuracy
    Q_INVOKABLE QString formatException(const ScriptValue& exception, bool includeExtendedDetails) { return _manager->formatException(exception, includeExtendedDetails); }

    /*@jsdoc
     * Adds a function to the list of functions called when a particular event occurs on a particular entity.
     * <p>See also, the {@link Entities} API.</p>
     * @function Script.addEventHandler
     * @param {Uuid} entityID - The ID of the entity.
     * @param {Script.EntityEvent} eventName - The name of the event.
     * @param {Script~entityEventCallback|Script~pointerEventCallback|Script~collisionEventCallback} handler - The function to
     *     call when the event occurs on the entity. It can be either the name of a function or an in-line definition.
     * @example <caption>Report when a mouse press occurs on a particular entity.</caption>
     * var entityID = Entities.addEntity({
     *     type: "Box",
     *     position: Vec3.sum(MyAvatar.position, Vec3.multiplyQbyV(MyAvatar.orientation, { x: 0, y: 0, z: -5 })),
     *     dimensions: { x: 0.5, y: 0.5, z: 0.5 },
     *     lifetime: 300  // Delete after 5 minutes.
     * });
     *
     * function reportMousePress(entityID, event) {
     *     print("Mouse pressed on entity: " + JSON.stringify(event));
     * }
     *
     * Script.addEventHandler(entityID, "mousePressOnEntity", reportMousePress);
     */
    Q_INVOKABLE void addEventHandler(const EntityItemID& entityID, const QString& eventName, const ScriptValue& handler) { _manager->addEventHandler(entityID, eventName, handler); }

    /*@jsdoc
     * Removes a function from the list of functions called when an entity event occurs on a particular entity.
     * <p>See also, the {@link Entities} API.</p>
     * @function Script.removeEventHandler
     * @param {Uuid} entityID - The ID of the entity.
     * @param {Script.EntityEvent} eventName - The name of the entity event.
     * @param {function} handler - The name of the function to no longer call when the entity event occurs on the entity.
     */
    Q_INVOKABLE void removeEventHandler(const EntityItemID& entityID, const QString& eventName, const ScriptValue& handler) { _manager->removeEventHandler(entityID, eventName, handler); }

    /*@jsdoc
     * Starts running another script in Interface, if it isn't already running. The script is not automatically loaded next
     * time Interface starts.
     * <p class="availableIn"><strong>Supported Script Types:</strong> Interface Scripts &bull; Avatar Scripts</p>
     * <p>See also, {@link ScriptDiscoveryService.loadScript}.</p>
     * @function Script.load
     * @param {string} filename - The URL of the script to load. This can be relative to the current script's URL.
     * @example <caption>Load a script from another script.</caption>
     * // First file: scriptA.js
     * print("This is script A");
     *
     * // Second file: scriptB.js
     * print("This is script B");
     * Script.load("scriptA.js");
     *
     * // If you run scriptB.js you should see both scripts in the Running Scripts dialog.
     * // And you should see the following output:
     * // This is script B
     * // This is script A
     */
    Q_INVOKABLE void load(const QString& loadfile) { _manager->load(loadfile); }

    /*@jsdoc
     * Includes JavaScript from other files in the current script. If a callback is specified, the files are loaded and
     * included asynchronously, otherwise they are included synchronously (i.e., script execution blocks while the files are
     * included).
     * @function Script.include
     * @variation 0
     * @param {string[]} filenames - The URLs of the scripts to include. Each can be relative to the current script.
     * @param {function} [callback=null] - The function to call back when the scripts have been included. It can be either the
     *     name of a function or an in-line definition.
     */
    Q_INVOKABLE void include(const QStringList& includeFiles, const ScriptValue& callback = ScriptValue()) { _manager->include(includeFiles, callback);};

    /*@jsdoc
     * Includes JavaScript from another file in the current script. If a callback is specified, the file is loaded and included
     * asynchronously, otherwise it is included synchronously (i.e., script execution blocks while the file is included).
     * @function Script.include
     * @param {string} filename - The URL of the script to include. It can be relative to the current script.
     * @param {function} [callback=null] - The function to call back when the script has been included. It can be either the
     *     name of a function or an in-line definition.
     * @example <caption>Include a script file asynchronously.</caption>
     * // First file: scriptA.js
     * print("This is script A");
     *
     * // Second file: scriptB.js
     * print("This is script B");
     * Script.include("scriptA.js", function () {
     *     print("Script A has been included");
     * });
     *
     * // If you run scriptB.js you should see only scriptB.js in the running scripts list.
     * // And you should see the following output:
     * // This is script B
     * // This is script A
     * // Script A has been included
     */
    Q_INVOKABLE void include(const QString& includeFile, const ScriptValue& callback = ScriptValue()) { _manager->include( includeFile, callback); };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // MODULE related methods

    /*@jsdoc
     * Provides access to methods or objects provided in an external JavaScript or JSON file.
     * See {@link https://docs.overte.org/script/js-tips.html} for further details.
     * @function Script.require
     * @param {string} module - The module to use. May be a JavaScript file, a JSON file, or the name of a system module such
     *     as <code>"appUi"</code> (i.e., the "appUi.js" system module JavaScript file).
     * @returns {object|array} The value assigned to <code>module.exports</code> in the JavaScript file, or the value defined
     *     in the JSON file.
     */
    Q_INVOKABLE ScriptValue require(const QString& moduleId) { return _manager->require(moduleId); }

    // TODO: Deprecated by documentation, please review for accuracy
    Q_INVOKABLE void resetModuleCache(bool deleteScriptCache = false) { _manager->resetModuleCache(deleteScriptCache);}

    /*@jsdoc
     * Calls a function repeatedly, at a set interval.
     * @function Script.setInterval
     * @param {function} function - The function to call. This can be either the name of a function or an in-line definition.
     * @param {number} interval - The interval at which to call the function, in ms.
     * @returns {object} A handle to the interval timer. This can be used in {@link Script.clearInterval}.
     * @example <caption>Print a message every second.</caption>
     * Script.setInterval(function () {
     *     print("Interval timer fired");
     * }, 1000);
    */
    Q_INVOKABLE QTimer* setInterval(const ScriptValue& function, int intervalMS) { return _manager->setInterval(function, intervalMS); }

    /*@jsdoc
     * Calls a function once, after a delay.
     * @function Script.setTimeout
     * @param {function} function - The function to call. This can be either the name of a function or an in-line definition.
     * @param {number} timeout - The delay after which to call the function, in ms.
     * @returns {object} A handle to the timeout timer. This can be used in {@link Script.clearTimeout}.
     * @example <caption>Print a message once, after a second.</caption>
     * Script.setTimeout(function () {
     *     print("Timeout timer fired");
     * }, 1000);
     */
    Q_INVOKABLE QTimer* setTimeout(const ScriptValue& function, int timeoutMS) { return _manager->setTimeout(function, timeoutMS); };

    /*@jsdoc
     * Stops an interval timer set by {@link Script.setInterval|setInterval}.
     * @function Script.clearInterval
     * @param {object} timer - The interval timer to stop.
     * @example <caption>Stop an interval timer.</caption>
     * // Print a message every second.
     * var timer = Script.setInterval(function () {
     *     print("Interval timer fired");
     * }, 1000);
     *
     * // Stop the timer after 10 seconds.
     * Script.setTimeout(function () {
     *     print("Stop interval timer");
     *     Script.clearInterval(timer);
     * }, 10000);
     */
    Q_INVOKABLE void clearInterval(QTimer* timer) { _manager->clearInterval(timer); }

    // Overloaded version is needed in case the timer has expired
    Q_INVOKABLE void clearInterval(QVariantMap timer) { ; }

    /*@jsdoc
     * Stops a timeout timer set by {@link Script.setTimeout|setTimeout}.
     * @function Script.clearTimeout
     * @param {object} timer - The timeout timer to stop.
     * @example <caption>Stop a timeout timer.</caption>
     * // Print a message after two seconds.
     * var timer = Script.setTimeout(function () {
     *     print("Timer fired");
     * }, 2000);
     *
     * // Uncomment the following line to stop the timer from firing.
     * //Script.clearTimeout(timer);
     */
    Q_INVOKABLE void clearTimeout(QTimer* timer) { _manager->clearTimeout(timer); }

    // Overloaded version is needed in case the timer has expired
    Q_INVOKABLE void clearTimeout(QVariantMap timer) { ; }

    /*@jsdoc
     * Prints a message to the program log and emits {@link Script.printedMessage}.
     * <p>Alternatively, you can use {@link print} or one of the {@link console} API methods.</p>
     * @function Script.print
     * @param {string} message - The message to print.
     */
    Q_INVOKABLE void print(const QString& message) { _manager->print(message); }

    /*@jsdoc
     * Resolves a relative path to an absolute path. The relative path is relative to the script's location.
     * @function Script.resolvePath
     * @param {string} path - The relative path to resolve.
     * @returns {string} The absolute path.
     * @example <caption>Report the directory and filename of the running script.</caption>
     * print(Script.resolvePath(""));
     * @example <caption>Report the directory of the running script.</caption>
     * print(Script.resolvePath("."));
     * @example <caption>Report the path to a file located relative to the running script.</caption>
     * print(Script.resolvePath("../assets/sounds/hello.wav"));
     */
    Q_INVOKABLE QUrl resolvePath(const QString& path) const { return _manager->resolvePath(path);}

    /*@jsdoc
     * Gets the path to the resources directory for QML files.
     * @function Script.resourcesPath
     * @returns {string} The path to the resources directory for QML files.
     */
    Q_INVOKABLE QUrl resourcesPath() const { return _manager->resourcesPath(); }

    /*@jsdoc
     * Starts timing a section of code in order to send usage data about it to Overte. Shouldn't be used outside of the
     * standard scripts.
     * @function Script.beginProfileRange
     * @param {string} label - A name that identifies the section of code.
     */
    Q_INVOKABLE void beginProfileRange(const QString& label) const { _manager->beginProfileRange(label); }

    /*@jsdoc
     * Finishes timing a section of code in order to send usage data about it to Overte. Shouldn't be used outside of
     * the standard scripts.
     * @function Script.endProfileRange
     * @param {string} label - A name that identifies the section of code.
     */
    Q_INVOKABLE void endProfileRange(const QString& label) const { _manager->endProfileRange(label); }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Entity Script Related methods

    /*@jsdoc
     * Checks whether an entity has an entity script running.
     * @function Script.isEntityScriptRunning
     * @param {Uuid} entityID - The ID of the entity.
     * @returns {boolean} <code>true</code> if the entity has an entity script running, <code>false</code> if it doesn't.
     */
    Q_INVOKABLE bool isEntityScriptRunning(const EntityItemID& entityID) { return _manager->isEntityScriptRunning(entityID); }

    /*@jsdoc
     * Manually runs the JavaScript garbage collector which reclaims memory by disposing of objects that are no longer
     * reachable.
     * @function Script.requestGarbageCollection
     */
    Q_INVOKABLE void requestGarbageCollection() { _manager->requestGarbageCollection(); }

    /*@jsdoc
     * Prints out current backtrace to the log.
     * @function Script.logBacktrace
     * @param {string} title - Title added to the printed out backtrace.
     */
    Q_INVOKABLE void logBacktrace(const QString &title) { _manager->logBacktrace(title); }

    // TODO: Deprecated by documentation, please review for accuracy
    Q_INVOKABLE void loadEntityScript(const EntityItemID& entityID, const QString& entityScript, bool forceRedownload) { _manager->loadEntityScript(entityID, entityScript, forceRedownload); }

    // TODO: Deprecated by documentation, please review for accuracy
    Q_INVOKABLE void unloadEntityScript(const EntityItemID& entityID, bool shouldRemoveFromMap = false) { _manager->unloadEntityScript(entityID, shouldRemoveFromMap); }

    // TODO: Deprecated by documentation, please review for accuracy
    Q_INVOKABLE void unloadAllEntityScripts(bool blockingCall = false) { _manager->unloadAllEntityScripts(blockingCall); }

    /*@jsdoc
     * Calls a method in an entity script.
     * @function Script.callEntityScriptMethod
     * @param {Uuid} entityID - The ID of the entity running the entity script.
     * @param {string} methodName - The name of the method to call.
     * @param {string[]} [parameters=[]] - The parameters to call the specified method with.
     * @param {Uuid} [remoteCallerID=Uuid.NULL] - An ID that identifies the caller.
     */
    Q_INVOKABLE void callEntityScriptMethod(const EntityItemID& entityID, const QString& methodName,
                                            const QStringList& params = QStringList(),
                                            const QUuid& remoteCallerID = QUuid()) { _manager->callEntityScriptMethod(entityID, methodName, params, remoteCallerID); }

    // TODO: Deprecated by documentation, please review for accuracy
    Q_INVOKABLE void callEntityScriptMethod(const EntityItemID& entityID, const QString& methodName, const PointerEvent& event) { _manager->callEntityScriptMethod(entityID, methodName, event); }

    // TODO: Deprecated by documentation, please review for accuracy
    Q_INVOKABLE void callEntityScriptMethod(const EntityItemID& entityID, const QString& methodName, const EntityItemID& otherID, const Collision& collision) { _manager->callEntityScriptMethod(entityID, methodName, otherID, collision);}

    // TODO: Deprecated by documentation, please review for accuracy
    Q_INVOKABLE QUuid generateUUID() { return QUuid::createUuid(); }

    /*@jsdoc
     * Gets the URL for an asset in an external resource bucket. (The location where the bucket is hosted may change over time
     * but this method will return the asset's current URL.)
     * @function Script.getExternalPath
     * @param {Script.ResourceBucket} bucket - The external resource bucket that the asset is in.
     * @param {string} path - The path within the external resource bucket where the asset is located.
     *     <p>Normally, this should start with a path or filename to be appended to the bucket URL.
     *     Alternatively, it can be a relative path starting with <code>./</code> or <code>../</code>, to navigate within the
     *     resource bucket's URL.</p>
     * @Returns {string} The URL of an external asset.
     * @example <caption>Report the URL of a default particle.</caption>
     * print(Script.getExternalPath(Script.ExternalPaths.Assets, "Bazaar/Assets/Textures/Defaults/Interface/default_particle.png"));
     * @example <caption>Report the root directory where the Overte assets are located.</caption>
     * print(Script.getExternalPath(Script.ExternalPaths.Assets, "."));
     */
    Q_INVOKABLE QString getExternalPath(ExternalResource::Bucket bucket, const QString& path) { return _manager->getExternalPath(bucket, path); }

    /*@jsdoc
     * <p>Object containing memory usage statistics data.</p>
     * <table>
     *   <thead>
     *     <tr><th>Name</th><th>Type</th><th>Description</th></tr>
     *   </thead>
     *   <tbody>
     *     <tr><td><code>totalHeapSize</code></td><td>{number}</td><td>Total heap size allocated by scripting engine.</td></tr>
     *     <tr><td><code>usedHeapSize</code></td><td>{number}</td><td></td>Amount of heap memory that is currently in use.</tr>
     *     <tr><td><code>totalAvailableSize</code></td><td>{number}</td><td>Amount of remaining available heap memory</td></tr>
     *     <tr><td><code>totalGlobalHandlesSize</code></td><td>{number}</td><td>V8-specific property</td></tr>
     *     <tr><td><code>usedGlobalHandlesSize</code></td><td>{number}</td><td>V8-specific property</td></tr>
     *   </tbody>
     * </table>
     * @typedef {object} Script.MemoryUsageData
     */

    /*@jsdoc
     * Returns memory usage statistics data.
     * @function Script.getMemoryUsageStatistics
     * @Returns {Script.MemoryUsageData} Object containing statistics about memory usage.
     */
    Q_INVOKABLE QVariantMap getMemoryUsageStatistics();

    /*@jsdoc
     * Start collecting object statistics that can later be reported with Script.dumpHeapObjectStatistics().
     * @function Script.dumpHeapObjectStatistics
     */
    Q_INVOKABLE void startCollectingObjectStatistics();

    /*@jsdoc
     * Prints heap statistics to a file. Collecting needs to first be started with Script.dumpHeapObjectStatistics().
     * @function Script.dumpHeapObjectStatistics
     */
    Q_INVOKABLE void dumpHeapObjectStatistics();

    /*@jsdoc
     * Create test object for garbage collector debugging.
     * @function Script.createGarbageCollectorDebuggingObject()
     * @Returns Test object.
     */
     Q_INVOKABLE ScriptValue createGarbageCollectorDebuggingObject();

     /*@jsdoc
     * Starts collecting profiling data
     * @function Script.startProfiling
     */
     Q_INVOKABLE void startProfiling();

     /*@jsdoc
     * Stops collecting profiling data and writes them to a timestamped CSV file in Logs directory.
     * @function Script.stopProfilingAndSave
     * @example <caption>Collect profiling data from a script.</caption>
     * Script.startProfiling();
     * workFunc1();
     * Script.stopProfilingAndSave();
     *
     * function workFunc1() {
     *     for (var i=0; i<100000; i++) {
     *         var vec1 = {x: i, y: i+1, z: i+2};
     *         var vec2 = {x: i+3, y: i+4, z: i+5};
     *         workFunc2(vec1, vec2);
     *     }
     * };
     * function workFunc2(vec1, vec2) {
     *     var cross = Vec3.cross(vec1, vec2);
     *     var dot = Vec3.dot(vec1, vec2);
     * };
     */
     Q_INVOKABLE void stopProfilingAndSave();

signals:

    // TODO: Deprecated by documentation, please review for accuracy
    void scriptLoaded(const QString& scriptFilename);

    // TODO: Deprecated by documentation, please review for accuracy
    void errorLoadingScript(const QString& scriptFilename);

    /*@jsdoc
     * Triggered frequently at a system-determined interval.
     * @function Script.update
     * @param {number} deltaTime - The time since the last update, in s.
     * @returns {Signal}
     * @example <caption>Report script update intervals.</caption>
     * Script.update.connect(function (deltaTime) {
     *     print("Update: " + deltaTime);
     * });
     */
    void update(float deltaTime);

    /*@jsdoc
     * Triggered when the script is stopping.
     * @function Script.scriptEnding
     * @returns {Signal}
     * @example <caption>Report when a script is stopping.</caption>
     * print("Script started");
     *
     * Script.scriptEnding.connect(function () {
     *     print("Script ending");
     * });
     *
     * Script.setTimeout(function () {
     *     print("Stopping script");
     *     Script.stop();
     * }, 1000);
     */
    void scriptEnding();

    /*@jsdoc
     * Triggered when the script prints a message to the program log via {@link  print}, {@link Script.print},
     * {@link console.log}, {@link console.debug}, {@link console.group}, {@link console.groupEnd}, {@link console.time}, or
     * {@link console.timeEnd}.
     * @function Script.printedMessage
     * @param {string} message - The message.
     * @param {string} scriptName - The name of the script that generated the message.
     * @returns {Signal}
     */
    void printedMessage(const QString& message, const QString& scriptName);

    /*@jsdoc
     * Triggered when the script generates an error, {@link console.error} or {@link console.exception} is called, or
     * {@link console.assert} is called and fails.
     * @function Script.errorMessage
     * @param {string} message - The error message.
     * @param {string} scriptName - The name of the script that generated the error message.
     * @returns {Signal}
     */
    void errorMessage(const QString& message, const QString& scriptName);

    /*@jsdoc
     * Triggered when the script generates a warning or {@link console.warn} is called.
     * @function Script.warningMessage
     * @param {string} message - The warning message.
     * @param {string} scriptName - The name of the script that generated the warning message.
     * @returns {Signal}
     */
    void warningMessage(const QString& message, const QString& scriptName);

    /*@jsdoc
     * Triggered when the script generates an information message or {@link console.info} is called.
     * @function Script.infoMessage
     * @param {string} message - The information message.
     * @param {string} scriptName - The name of the script that generated the information message.
     * @returns {Signal}
     */
    void infoMessage(const QString& message, const QString& scriptName);

    /*@jsdoc
     * Triggered when the running state of the script changes, e.g., from running to stopping.
     * @function Script.runningStateChanged
     * @returns {Signal}
     */
    void runningStateChanged();

    // TODO: Deprecated by documentation, please review for accuracy
    void clearDebugWindow();

    // TODO: Deprecated by documentation, please review for accuracy
    void loadScript(const QString& scriptName, bool isUserLoaded);

    void reloadScript(const QString& scriptName, bool isUserLoaded);

    /*@jsdoc
     * Triggered when the script has stopped.
     * @function Script.doneRunning
     * @returns {Signal}
     */
    void doneRunning();

    // TODO: Deprecated by documentation, please review for accuracy
    void entityScriptDetailsUpdated();

    /*@jsdoc
     * Triggered when the script starts for the user. See also, {@link Entities.preload}.
     * <p class="availableIn"><strong>Supported Script Types:</strong> Client Entity Scripts &bull; Server Entity Scripts</p>
     * @function Script.entityScriptPreloadFinished
     * @param {Uuid} entityID - The ID of the entity that the script is running in.
     * @returns {Signal}
     * @example <caption>Get the ID of the entity that a client entity script is running in.</caption>
     * var entityScript = function () {
     *     this.entityID = Uuid.NULL;
     * };
     *
     * Script.entityScriptPreloadFinished.connect(function (entityID) {
     *     this.entityID = entityID;
     *     print("Entity ID: " + this.entityID);
     * });
     *
     * var entityID = Entities.addEntity({
     *     type: "Box",
     *     position: Vec3.sum(MyAvatar.position, Vec3.multiplyQbyV(MyAvatar.orientation, { x: 0, y: 0, z: -5 })),
     *     dimensions: { x: 0.5, y: 0.5, z: 0.5 },
     *     color: { red: 255, green: 0, blue: 0 },
     *     script: "(" + entityScript + ")",  // Could host the script on a Web server instead.
     *     lifetime: 300  // Delete after 5 minutes.
     * });
     */
    // Emitted when an entity script has finished running preload
    void entityScriptPreloadFinished(const EntityItemID& entityID);

    /*@jsdoc
     * Triggered when a script generates an unhandled exception.
     * @function Script.unhandledException
     * @param {object} exception - The details of the exception.
     * @returns {Signal}
     * @example <caption>Report the details of an unhandled exception.</caption>
     * Script.unhandledException.connect(function (exception) {
     *     print("Unhandled exception: " + JSON.stringify(exception));
     * });
     * var properties = JSON.parse("{ x: 1"); // Invalid JSON string.
     */
    void unhandledException(const ScriptValue& exception);

protected:
    // TODO: Deprecated by documentation, please review for accuracy
    Q_INVOKABLE QString _requireResolve(const QString& moduleId, const QString& relativeTo = QString()) { return _manager->_requireResolve(moduleId, relativeTo); }

    // TODO: Deprecated by documentation, please review for accuracy
    Q_INVOKABLE void entityScriptContentAvailable(const EntityItemID& entityID, const QString& scriptOrURL, const QString& contents, bool isURL, bool success, const QString& status)
        { _manager->entityScriptContentAvailable(entityID, scriptOrURL, contents, isURL, success, status); }

private slots:
    void scriptManagerException(std::shared_ptr<ScriptException> exception);


private:

    ScriptManager *_manager;
};
