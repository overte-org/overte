//
//  Web3DSurface.qml
//
//  Created by David Rowe on 16 Dec 2016.
//  Copyright 2016 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.5
import controlsUit 1.0 as Controls
import "controls"

Item {
    id: root
    anchors.fill: parent
    property string url: ""
    property string scriptUrl: null
    property bool useBackground: true
    property string userAgent: ""

    onUrlChanged: {
        load(root.url, root.scriptUrl, root.useBackground, root.userAgent);
    }

    onScriptUrlChanged: {
        if (loader.item) {
            if (root.webViewLoaded) {
                loader.item.scriptUrl = root.scriptUrl;
            }
        } else {
            load(root.url, root.scriptUrl, root.useBackground, root.userAgent);
        }
    }

    onUseBackgroundChanged: {
        if (loader.item) {
            if (root.webViewLoaded) {
                loader.item.useBackground = root.useBackground;
            }
        } else {
            load(root.url, root.scriptUrl, root.useBackground, root.userAgent);
        }
    }

    onUserAgentChanged: {
        if (loader.item) {
            if (root.webViewLoaded) {
                loader.item.userAgent = root.userAgent;
            }
        } else {
            load(root.url, root.scriptUrl, root.useBackground, root.userAgent);
        }
    }

    // Handle message traffic from our loaded QML to the script that launched us
    onItemChanged: {
        if (loader.item && loader.item.sendToScript) {
            loader.item.sendToScript.connect(sendToScript);
        }
    }

    property var item: null
    property bool webViewLoaded: false

    // Handle message traffic from the script that launched us to the loaded QML
    function fromScript(message) {
        if (loader.item && loader.item.fromScript) {
            loader.item.fromScript(message);
        }
    }

    Loader {
        id: loader
        anchors.fill: parent
    }

    function load(url, scriptUrl, useBackground, userAgent) {
        // Ensure we reset any existing item to "about:blank" to ensure web audio stops: DEV-2375
        if (loader.item && root.webViewLoaded) {
            if (root.webViewLoaded) {
                loader.item.url = "about:blank"
            }
            loader.setSource(undefined);
        }

        if (url.match(/\.qml$/)) {
            root.webViewLoaded = false;
            loader.setSource(url);
        } else {
            root.webViewLoaded = true;
            loader.setSource("./controls/WebView.qml", {
                url: url,
                scriptUrl: scriptUrl,
                useBackground: useBackground,
                userAgent: userAgent
            });
        }
    }

    Component.onCompleted: {
        load(root.url, root.scriptUrl, root.useBackground, root.userAgent);
    }

    signal sendToScript(var message);
}
