//
//  OSCScriptingInterface.h
//  interface/src/scripting
//
//  Created by Ada <ada@thingvellir.net> on 2025-12-13
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_OSCScriptingInterface_h
#define hifi_OSCScriptingInterface_h

#include <QObject>
#include <QUdpSocket>

#include <DependencyManager.h>

#include "SettingHandle.h"

class ScriptValue;
class ScriptEngine;
class ScriptContext;

/*@jsdoc
 * <p>Primarily for differentiating between OSC integers and floats,
 * since <code>Number</code> in JavaScript is always a 64-bit float.</p>
 * Supported OSC types are:
 * <ul>
 *   <li><code>i</code> (32-bit integer)</li>
 *   <li><code>f</code> (32-bit float)</li>
 *   <li><code>s</code> (UTF-8 string)</li>
 *   <li><code>b</code> (<code>ArrayBuffer</code>)</li>
 *   <li><code>F</code> (boolean <code>false</code>)</li>
 *   <li><code>T</code> (boolean <code>true</code>)</li>
 *   <li><code>N</code> (<code>null</code>)</li>
 * </ul>
 * @typedef {Object} OSCSocket.TypedArgument
 * @property {string} type
 * @property {number|string|ArrayBuffer|boolean|null} value
 */

/*@jsdoc
 * The <code>OSCSocket</code> API lets you send and receive Open Sound Control packets.
 * Despite the name, OSC is commonly used with VR applications too. It is used by some
 * tracking and haptic systems that aren't exposed through OpenVR or OpenXR.
 *
 * <p>Overte doesn't have any built-in OSC functionality; OSC is only useful with scripts.</p>
 *
 * {@link https://opensoundcontrol.stanford.edu/spec-1_0.html}
 *
 * @namespace OSCSocket
 *
 * @hifi-interface
 * @hifi-client-entity
 * @hifi-avatar
 *
 * @property {string} receiveHost - IP address for receiving OSC packets.
 *      <code>127.0.0.1</code> by default, and shouldn't be changed unless
 *      you know what you're doing and need to use something different.
 * @property {number} receivePort - IP port for receiving OSC packets.
 *      <code>9000</code> by default.
 * @property {string} sendHost - IP address to send OSC packets to.
 *      <code>127.0.0.1</code> by default, and shouldn't be changed unless
 *      you know what you're doing and need to use something different.
 * @property {number} receivePort - IP port to send OSC packets to.
 *      <code>9001</code> by default.
 */
class OSCScriptingInterface : public QObject, public Dependency {
    Q_OBJECT

    Q_PROPERTY(QString receiveHost READ getReceiveHost WRITE setReceiveHost)
    Q_PROPERTY(int receivePort READ getReceivePort WRITE setReceivePort)

    Q_PROPERTY(QString sendHost READ getSendHost WRITE setSendHost)
    Q_PROPERTY(int sendPort READ getSendPort WRITE setSendPort)

public:
    OSCScriptingInterface(QObject* parent = nullptr);
    ~OSCScriptingInterface();

    /*@jsdoc
     * Sends an encoded OSC packet. OSC bundles are not supported.
     * @function OSCSocket.sendPacket
     * @param {string} address - OSC address, starting with <code>/</code>
     * @param {...(OSCSocket.TypedArgument|number|string|ArrayBuffer|boolean|null)} [arguments]
     */
    static ScriptValue sendPacket(ScriptContext* context, ScriptEngine* engine);

signals:
    /*@jsdoc
     * Triggered when an OSC packet is received. OSC bundles are not supported.
     * @function OSCSocket.packetReceived
     * @param {string} address - OSC address starting with <code>/</code>
     * @param {Array.<OSCSocket.TypedArgument>} arguments
     * @returns {Signal}
     */
    void packetReceived(const QString& address, const QVariantList& arguments);

private:
    void readPacket();
    void rebindSocket();

    void setReceiveHost(QString host);
    QString getReceiveHost() const { return _receiveHost.get(); }

    void setReceivePort(int port);
    int getReceivePort() const { return _receivePort.get(); }

    void setSendHost(QString host);
    QString getSendHost() const { return _sendHost.get(); }

    void setSendPort(int port);
    int getSendPort() const { return _sendPort.get(); }

    Setting::Handle<int> _receivePort;
    Setting::Handle<QString> _receiveHost;

    Setting::Handle<int> _sendPort;
    Setting::Handle<QString> _sendHost;

    std::shared_ptr<QUdpSocket> _socket;
};

#endif
