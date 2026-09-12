import QtQuick
import QtWebChannel
import QtWebEngine

import controlsUit

WebView {
    id: webview
    url: "https://overte.org/"
    profile: FileTypeProfile;
    property var parentRoot: null

    userScripts.collection: [
        // Create a global EventBridge object for raiseAndLowerKeyboard.
        {
            sourceCode: eventBridgeJavaScriptToInject,
            injectionPoint: WebEngineScript.Deferred,
            worldId: WebEngineScript.MainWorld,
        },
        // Detect when may want to raise and lower keyboard.
        {
            injectionPoint: WebEngineScript.Deferred,
            sourceUrl: resourceDirectoryUrl + "/html/raiseAndLowerKeyboard.js",
            worldId: WebEngineScript.MainWorld,
        }
    ]

    onLoadingChanged: {
        if (loadRequest.status === WebEngineView.LoadSucceededStatus) {
            addressBar.text = loadRequest.url
        }
        parentRoot.loadingChanged(loadRequest.status);
    }

    onWindowCloseRequested: {
        parentRoot.destroy();
    }

    Component.onCompleted: {
        webChannel.registerObject("eventBridge", eventBridge);
        webChannel.registerObject("eventBridgeWrapper", eventBridgeWrapper);
        desktop.initWebviewProfileHandlers(webview.profile);
    }
}
