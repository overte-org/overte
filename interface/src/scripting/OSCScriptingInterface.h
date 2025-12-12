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

#include "SettingHandle.h"
#include "ScriptManager.h"

class OSCScriptingInterface : public QObject {
    Q_OBJECT

    void receivePacket();
public:
    OSCScriptingInterface(ScriptManager* parent);
    ~OSCScriptingInterface();

    /*@jsdoc
     * Sends an encoded OSC message. The target host address and port are visible as the <code>osc/sendHost</code> and <code>osc/sendPort</code> settings.
     * @function OSCSocket.sendMessage
     * @param {string} address - The OSC address, starting with <code>/</code>
     * @param {string} argumentTypes - A string of the data types contained in <code>arguments</code>.
     * Supports the <code>i</code> (32-bit integer), <code>f</code> (32-bit float),
     * <code>s</code> (null-terminated string), <code>b</code> (ArrayBuffer), <code>T</code> (boolean true), and <code>F</code> (boolean false) tag types. The string should not be prefixed with a comma. May be left empty for no arguments.
     * @param {Array.<*>} [arguments] - OSC arguments.
     */
    Q_INVOKABLE void sendMessage(const QString& address, const QString& argumentTypes = QString(), const QVariantList& arguments = QVariantList());

signals:
    /*@jsdoc
     * Triggered when an OSC datagram is received. The receiving host address and port are visible as the <code>osc/receiveHost</code> and <code>osc/receivePort</code> settings.
     * @function OSCSocket.messageReceived
     * @param {string} address - OSC address starting with <code>/</code>
     * @param {string} argumentTypes - String of OSC data types contained by <code>arguments</code>
     * @param {Array.<*>} arguments
     * @returns {Signal}
     */
    void messageReceived(const QString& address, const QString& argumentTypes, const QVariantList& arguments);

private:
    ScriptManager* _scriptManager;

    Setting::Handle<int> _receivePort;
    Setting::Handle<QString> _receiveHost;

    Setting::Handle<int> _sendPort;
    Setting::Handle<QString> _sendHost;

    std::shared_ptr<QUdpSocket> _socket;
};

#endif
