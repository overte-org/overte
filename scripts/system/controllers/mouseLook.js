//
// mouseLook.js
//
// By Armored Dragon (June 6). Refactored from Rampa3 & Vegaslon work
//  Copyright 2024 Overte e.V.
//
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

(() => {
    // States ----
    let mouseLookActive = Settings.getValue("mouselook-active", false);
    let mouseLookEnabled = Camera.getMouseLook();
    let hmdActive = HMD.active;
    let overlayActive = Desktop.isOverlayWindowFocused();

    // Resources ----
    let tablet = Tablet.getTablet("com.highfidelity.interface.tablet.system");

    // Events ----
    Camera.mouseLookChanged.connect(onMouseLookChanged);
    Controller.keyPressEvent.connect(onKeyPressEvent);
    Desktop.uiFocusChanged.connect(onUiFocusChanged);
    HMD.displayModeChanged.connect(onDisplayModeChanged);
    MyAvatar.wentActive.connect(onWentActive);
    MyAvatar.wentAway.connect(onWentAway);
    tablet.tabletShownChanged.connect(onTabletShownChanged);
    Script.scriptEnding.connect(onScriptEnding);

    // Program ----
    function onMouseLookChanged(newMouseLook) {
        mouseLookEnabled = newMouseLook;
    }

    function onKeyPressEvent(event) {
        // Toggle using the m key
        if (event.text.toLowerCase() === "m") {
            if (Camera.captureMouse) {
                mouseLookActive = false;
                Settings.setValue("mouselook-active", false);
                disableMouseLook();
            } else {
                mouseLookActive = true;
                Settings.setValue("mouselook-active", true);
                enableMouseLook();
            }
        }
    }

    function onTabletShownChanged() {
        if (tablet.tabletShown) disableMouseLook();
        else enableMouseLook();
    }

    function onWentAway() {
        disableMouseLook();
    }

    function onWentActive() {
        enableMouseLook();
    }

    function onDisplayModeChanged() {
        hmdActive = HMD.active;
        if (hmdActive) disableMouseLook();
        else enableMouseLook();
    }

    function onUiFocusChanged(keyFocus) {
        if (keyFocus) {
            overlayActive = true;
            disableMouseLook();
        } else {
            overlayActive = false;
            enableMouseLook();
        }
    }

    function enableMouseLook() {
        if (hmdActive) return;
        if (tablet.tabletShown) return;
        if (overlayActive) return;
        if (!mouseLookActive) return; // Mouse look disabled via the hotkey

        Camera.captureMouse = true;
    }

    function disableMouseLook() {
        Camera.captureMouse = false;
    }

    function onScriptEnding() {
        Camera.captureMouse = false;
        Camera.mouseLookChanged.disconnect(onMouseLookChanged);
        Controller.keyPressEvent.disconnect(onKeyPressEvent);
        tablet.tabletShownChanged.disconnect(onTabletShownChanged);
        MyAvatar.wentAway.disconnect(onWentAway);
        MyAvatar.wentActive.disconnect(onWentActive);
        HMD.displayModeChanged.disconnect(onDisplayModeChanged);
        Desktop.uiFocusChanged.disconnect(onUiFocusChanged);
        Script.scriptEnding.disconnect(onScriptEnding);
    }
})();
