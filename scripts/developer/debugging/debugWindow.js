//
//  debugWindow.js
//
//  Brad Hefta-Gaub, created on 12/19/2016.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

(function() { // BEGIN LOCAL_SCOPE
"use strict"
//check if script already running.
const scriptData = ScriptDiscoveryService.getRunning();
const scripts = scriptData.filter(function (datum) { return datum.name === 'debugWindow.js'; });
if (scripts.length >= 2) {
    //2nd instance of the script is too much
    ScriptDiscoveryService.stopScript(scripts[1].url);
    return;
}

const SUPPRESS_DEFAULT_SCRIPTS_MENU_NAME  = "Developer"
const SUPPRESS_DEFAULT_SCRIPTS_ITEM_NAME = "Suppress messages from default scripts in Debug Window";
const DEBUG_WINDOW_SUPPRESS_DEFAULTS_SCRIPTS = 'debugWindowSuppressDefaultScripts';
let suppressDefaultScripts = Settings.getValue(DEBUG_WINDOW_SUPPRESS_DEFAULTS_SCRIPTS, false)
Menu.addMenuItem({
    menuName: SUPPRESS_DEFAULT_SCRIPTS_MENU_NAME,
    menuItemName: SUPPRESS_DEFAULT_SCRIPTS_ITEM_NAME,
    isCheckable: true,
    isChecked: suppressDefaultScripts
});

Menu.menuItemEvent.connect(function(menuItem) {
    if (menuItem === SUPPRESS_DEFAULT_SCRIPTS_ITEM_NAME) {
        suppressDefaultScripts = Menu.isOptionChecked(SUPPRESS_DEFAULT_SCRIPTS_ITEM_NAME);
    }
});

// Set up the qml ui
const qml = Script.resolvePath('debugWindow.qml');

const HMD_DEBUG_WINDOW_GEOMETRY_KEY = 'hmdDebugWindowGeometry';
const hmdDebugWindowGeometryValue = Settings.getValue(HMD_DEBUG_WINDOW_GEOMETRY_KEY);

let windowWidth = 400;
let windowHeight = 900;

let hasPosition = false;
let windowX = 0;
let windowY = 0;

if (hmdDebugWindowGeometryValue !== '') {
    const geometry = JSON.parse(hmdDebugWindowGeometryValue);
    if ((geometry.x !== 0) && (geometry.y !== 0)) {
        windowWidth = geometry.width;
        windowHeight = geometry.height;
        windowX = geometry.x;
        windowY = geometry.y;

        // Constrain window position to within the viewport
        const viewportDimensions = Controller.getViewportDimensions();
        if (windowX > viewportDimensions.x) windowX = viewportDimensions.x - windowWidth;
        if (windowY > viewportDimensions.y) windowX = viewportDimensions.y - windowHeight;

        hasPosition = true;
    }
}

const window = new OverlayWindow({
    title: 'Debug Window',
    source: qml,
    width: windowWidth, height: windowHeight,
});

if (hasPosition) {
    window.setPosition(windowX, windowY);
};

function recenterWindow() {
    window.setPosition(100, 100);
}

window.visibleChanged.connect(function() {
    if (!window.visible) {
        window.setVisible(true);
        recenterWindow();
    }
});

HMD.displayModeChanged.connect(function(isHMDMode) {
    recenterWindow();
});

window.closed.connect(function () { Script.stop(); });

function shouldLogMessage(scriptFileName) {
    return !suppressDefaultScripts
        || (scriptFileName !== "defaultScripts.js" && scriptFileName != "controllerScripts.js");
}

const getFormattedDate = function() {
    const date = new Date();
    return date.getMonth() + "/" + date.getDate() + " " + date.getHours() + ":" + date.getMinutes() + ":" + date.getSeconds();
};

const sendToLogWindow = function(type, message, scriptFileName) {
    if (shouldLogMessage(scriptFileName)) {
        let typeFormatted = "";
        if (type) {
            typeFormatted = type + " - ";
        }
        window.sendToQml({
            date: getFormattedDate(),
            scriptFileName: scriptFileName,
            type: type,
            message: message,
        });
    }
};

ScriptDiscoveryService.printedMessage.connect(function(message, scriptFileName) {
    sendToLogWindow("", message, scriptFileName);
});

ScriptDiscoveryService.warningMessage.connect(function(message, scriptFileName) {
    sendToLogWindow("WARNING", message, scriptFileName);
});

ScriptDiscoveryService.errorMessage.connect(function(message, scriptFileName) {
    sendToLogWindow("ERROR", message, scriptFileName);
});

ScriptDiscoveryService.infoMessage.connect(function(message, scriptFileName) {
    sendToLogWindow("INFO", message, scriptFileName);
});

ScriptDiscoveryService.clearDebugWindow.connect(function() {
    window.clearDebugWindow();
});

Script.scriptEnding.connect(function () {
    Settings.setValue(DEBUG_WINDOW_SUPPRESS_DEFAULTS_SCRIPTS,
                      Menu.isOptionChecked(SUPPRESS_DEFAULT_SCRIPTS_ITEM_NAME));
    Menu.removeMenuItem(SUPPRESS_DEFAULT_SCRIPTS_MENU_NAME, SUPPRESS_DEFAULT_SCRIPTS_ITEM_NAME);

    const geometry = JSON.stringify({
        x: window.position.x,
        y: window.position.y,
        width: window.size.x,
        height: window.size.y
    });

    Settings.setValue(HMD_DEBUG_WINDOW_GEOMETRY_KEY, geometry);
    window.close();
});

}());
