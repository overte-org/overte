import QtQuick
import QtWebEngine
import QtWebChannel

import controlsUit as Controls
import stylesUit
Controls.WebView {
    id: webview
    url: "about:blank"
    anchors.fill: parent
    focus: true
    profile: HFWebEngineProfile;

    property string userScriptUrl: ""

    // Create a global EventBridge object for raiseAndLowerKeyboard.

    userScripts.collection: [ {
            name: "createGlobalEventBridge",
            id: createGlobalEventBridge,
            sourceCode: eventBridgeJavaScriptToInject,
            injectionPoint: WebEngineScript.DocumentCreation,
            worldId: WebEngineScript.MainWorld
        },
        {
            name: "raiseAndLowerKeyboardScript",
            id: raiseAndLowerKeyboard,
            injectionPoint: WebEngineScript.Deferred,
            sourceUrl: resourceDirectoryUrl + "/html/raiseAndLowerKeyboard.js",
            worldId: WebEngineScript.MainWorld
        },
        {
            name: "userWebEngineScript",
            id: userScript,
            sourceUrl: webview.userScriptUrl,
            injectionPoint: WebEngineScript.DocumentReady,  // DOM ready but page load may not be finished.
            worldId: WebEngineScript.MainWorld
    } ]

    function onWebEventReceived(event) {
        if (typeof event === "string" && event.slice(0, 17) === "CLARA.IO DOWNLOAD") {
            ApplicationInterface.addAssetToWorldFromURL(event.slice(18));
        }
    }

    Component.onCompleted: {
        webChannel.registerObject("eventBridge", eventBridge);
        webChannel.registerObject("eventBridgeWrapper", eventBridgeWrapper);
        eventBridge.webEventReceived.connect(onWebEventReceived);
    }
}
