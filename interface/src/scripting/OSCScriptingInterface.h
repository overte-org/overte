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

class OSCScriptingInterface : public QObject, public Dependency {
    Q_OBJECT

    void readPacket();
public:
    OSCScriptingInterface(QObject* parent = nullptr);
    ~OSCScriptingInterface();

    /*@jsdoc
     * Sends an encoded OSC message. The target host address and port are accessible as the <code>osc/sendHost</code> and <code>osc/sendPort</code> settings. Changes to these settings do not apply until a restart.
     * @function OSCSocket.sendMessage
     * @param {string} address - The OSC address, starting with <code>/</code>
     * @param {Array.<*>} [arguments] - OSC arguments. May be plain values, or in an object like <code>{ type: "i", value: 123 }</code>.
     * Supported OSC types are <code>i</code> (32-bit int), <code>f</code> (32-bit float), <code>s</code> (UTF-8 string), <code>b</code> (<code>ArrayBuffer</code>), <code>F</code> (boolean false), <code>T</code> (boolean true), and <code>N</code> (<code>null</code>). OSC bundles are not supported.
     */
    Q_INVOKABLE void sendPacket(const QString& address, const QVariantList& arguments = QVariantList());

signals:
    /*@jsdoc
     * Triggered when an OSC packet is received. The receiving host address and port are accessible as the <code>osc/receiveHost</code> and <code>osc/receivePort</code> settings. Changes to these settings do not apply until a restart.
     * @function OSCSocket.messageReceived
     * @param {string} address - OSC address starting with <code>/</code>
     * @param {Array.<Object.<string,*>>} arguments
     * @returns {Signal}
     */
    void packetReceived(const QString& address, const QVariantList& arguments);

private:
    Setting::Handle<int> _receivePort;
    Setting::Handle<QString> _receiveHost;

    Setting::Handle<int> _sendPort;
    Setting::Handle<QString> _sendHost;

    std::shared_ptr<QUdpSocket> _socket;
};

#endif
