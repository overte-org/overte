import QtQuick
import QtWebChannel
import QtWebEngine

import "controls"
import controlsUit as HifiControls
import "styles" as HifiStyles
import stylesUit
import "windows"

Item {
    id: root
    HifiConstants { id: hifi }
    HifiStyles.HifiConstants { id: hifistyles }

    height: 600
    property alias url: webview.url

    property bool canGoBack: webview.canGoBack
    property bool canGoForward: webview.canGoForward


    signal loadingChanged(int status)

    x: 0
    y: 0

    function setProfile(profile) {
        webview.profile = profile;
    }

    onUrlChanged: {
        permissionPopupBackground.visible = false;
    }

    WebEngineView {
        id: webview
        objectName: "webEngineView"
        x: 0
        y: 0
        width: parent.width
        height: keyboardEnabled && keyboardRaised ? parent.height - keyboard.height : parent.height

        profile: HFWebEngineProfile;

        property string userScriptUrl: ""

        // creates a global EventBridge object.
        WebEngineScript {
            id: createGlobalEventBridge
            sourceCode: eventBridgeJavaScriptToInject
            injectionPoint: WebEngineScript.DocumentCreation
            worldId: WebEngineScript.MainWorld
        }

        // detects when to raise and lower virtual keyboard
        WebEngineScript {
            id: raiseAndLowerKeyboard
            injectionPoint: WebEngineScript.Deferred
            sourceUrl: resourceDirectoryUrl + "/html/raiseAndLowerKeyboard.js"
            worldId: WebEngineScript.MainWorld
        }

        // User script.
        WebEngineScript {
            id: userScript
            sourceUrl: webview.userScriptUrl
            injectionPoint: WebEngineScript.DocumentReady  // DOM ready but page load may not be finished.
            worldId: WebEngineScript.MainWorld
        }

        userScripts: [ createGlobalEventBridge, raiseAndLowerKeyboard, userScript ]

        property string newUrl: ""

        Component.onCompleted: {
            webChannel.registerObject("eventBridge", eventBridge);
            webChannel.registerObject("eventBridgeWrapper", eventBridgeWrapper);

            // Ensure the JS from the web-engine makes it to our logging
            webview.javaScriptConsoleMessage.connect(function(level, message, lineNumber, sourceID) {
                console.log("Web Entity JS message: " + sourceID + " " + lineNumber + " " +  message);
            });

            webview.profile.httpUserAgent = "Mozilla/5.0 (Linux; Android 6.0; Nexus 5 Build/MRA58N) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/56.0.2924.87 Mobile Safari/537.36";
            web.address = url;
        }

        onFeaturePermissionRequested: {
            if (permissionPopupBackground.visible === true) {
                console.log("Browser engine requested a new permission, but user is already being presented with a different permission request. Aborting request for new permission...");
                return;
            }
            permissionPopupBackground.securityOrigin = securityOrigin;
            permissionPopupBackground.feature = feature;

            permissionPopupBackground.visible = true;
        }

        onLoadingChanged: {
            keyboardRaised = false;
            punctuationMode = false;
            keyboard.resetShiftMode(false);

            // Required to support clicking on "hifi://" links
            if (WebEngineView.LoadStartedStatus == loadRequest.status) {
                urlAppend(loadRequest.url.toString())
                var url = loadRequest.url.toString();
                if (urlHandler.canHandleUrl(url)) {
                    if (urlHandler.handleUrl(url)) {
                        root.stop();
                    }
                }
            }
        }

        onNewViewRequested: {
            request.openIn(webView);
        }

        HifiControls.WebSpinner { }
    }

    Keys.onPressed: {
        switch(event.key) {
        case Qt.Key_L:
            if (event.modifiers == Qt.ControlModifier) {
                event.accepted = true
                addressBar.selectAll()
                addressBar.forceActiveFocus()
            }
            break;
        }
    }

    HifiControls.PermissionPopupBackground {
        id: permissionPopupBackground
        onSendPermission: {
            webview.grantFeaturePermission(securityOrigin, feature, shouldGivePermission);
        }
    }
}
