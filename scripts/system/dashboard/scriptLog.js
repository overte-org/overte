// dashIPC.js
// Created by Ada <ada@thingvellir.net> on 2026-02-26
// Copyright 2026 Overte e.V.
// SPDX-License-Identifier: Apache-2.0
"use strict";

const { DashWindow } = require("dashIPC");

const PRIORITY_DEBUG = 0;
const PRIORITY_INFO = 1;
const PRIORITY_WARN = 2;
const PRIORITY_ERROR = 3;

const window = new DashWindow({
    title: "Script Log",
    appID: "system scriptLog",
    sourceURL: `${Script.resourcesPath()}qml/overte/dash/ScriptLog.qml`,
});

window.closed.connect(() => Script.stop());

function sendLog(priority, text, source) {
    window.sendEvent(JSON.stringify({ priority, text, source }));
}

ScriptDiscoveryService.printedMessage.connect((msg, script) => sendLog(PRIORITY_DEBUG, msg, script));
ScriptDiscoveryService.infoMessage.connect((msg, script) => sendLog(PRIORITY_INFO, msg, script));
ScriptDiscoveryService.warningMessage.connect((msg, script) => sendLog(PRIORITY_WARN, msg, script));
ScriptDiscoveryService.errorMessage.connect((msg, script) => sendLog(PRIORITY_ERROR, msg, script));

Script.setTimeout(() => {
    console.debug("Debug message");
    console.info("Info message");
    console.warn("Warning message");
    console.error("Error message");
}, 1000);

Script.scriptEnding.connect(() => window.close());
