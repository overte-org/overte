/*
mouseLook.js – mouse look switching script
by rampa3 (https://github.com/rampa3) and vegaslon (https://github.com/vegaslon)
*/
(function() { // BEGIN LOCAL_SCOPE

    var away;

    var hmd = HMD.active;

    var mouseLookEnabled = Camera.getMouseLook();

    var tablet = Tablet.getTablet("com.highfidelity.interface.tablet.system");

    var tabletUp;
    
    var keysOnOverlay = Desktop.isOverlayWindowFocused();

    var tempOff = false;

    Camera.mouseLookChanged.connect(onMouseLookChanged);

    function onMouseLookChanged(newMouseLook) {
        mouseLookEnabled = newMouseLook;
    }

    if (!hmd){
        if (mouseLookEnabled) {
            if (!keysOnOverlay) {
                if (!tablet.tabletShown){
                    Window.displayAnnouncement("Mouse look: ON");
                    mouseLookOn();
                } else {
                    Window.displayAnnouncement("Tablet is up – mouse look temporarily OFF.");
                }
            }
        }
    }

    Controller.keyPressEvent.connect(onKeyPressEvent);

    function onKeyPressEvent(event) {
        if (!hmd){
            if (event.text.toLowerCase() === 'm') {
                if (!keysOnOverlay) {
                    if (mouseLookEnabled) {
                        if (!Camera.getCaptureMouse()){
                            tempOff = false;
                            Window.displayAnnouncement("Mouse look: ON");
                            mouseLookOn();
                        } else {
                            tempOff = true;
                            Window.displayAnnouncement("Mouse look: Temporarily OFF");
                            mouseLookOff();
                        }
                    }
                }
            }
        }
    }

    tablet.tabletShownChanged.connect(onTabletShownChanged);

    function onTabletShownChanged() {
        if (!hmd) {
            if (mouseLookEnabled) {
                if (!tablet.toolbarMode) {
                    if (!keysOnOverlay) {
                        if (tablet.tabletShown) {
                            tabletUp = true;
                            if (!tempOff) {
                                if (!away) {
                                    Window.displayAnnouncement("Tablet is up – mouse look temporarily OFF.");
                                    mouseLookOff();
                                }
                            }
                        } else if (!tablet.tabletShown) {
                            tabletUp = false;
                            if (!tempOff) {
                                if (!away && !keysOnOverlay) {
                                    Window.displayAnnouncement("Tablet hidden – mouse look ON.");
                                    mouseLookOn();
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    MyAvatar.wentAway.connect(onWentAway);

    function onWentAway() {
        if (!hmd) {
            if (mouseLookEnabled) {
                away = true;
                if (!keysOnOverlay) {
                    if (!tabletUp){
                        Window.displayAnnouncement("Away state ON – mouse look temporarily OFF.")
                        tempOff = false;
                        mouseLookOff()
                    }
                }
            }
        }
    }

    MyAvatar.wentActive.connect(onWentActive);

    function onWentActive() {
        if (!hmd) {
            if (mouseLookEnabled) {
                away = false;
                if (!keysOnOverlay) {
                    if (!tabletUp) {
                        Window.displayAnnouncement("Away state OFF – mouse look ON.");
                        mouseLookOn();
                    }
                }
            }
        }
    }

    HMD.displayModeChanged.connect(onDisplayModeChanged);

    function onDisplayModeChanged() {
        if (mouseLookEnabled) {
            if (HMD.active) {
                hmd = true;
                mouseLookOff();
            } else {
                hmd = false;
                if (!tempOff) {
                    if (!keysOnOverlay) {
                        if (!tabletUp) {
                            mouseLookOn();
                        }
                    }
                }
            }
        }
    }

    function mouseLookOn() {
        Camera.captureMouse = true;
    }

    function mouseLookOff() {
        Camera.captureMouse = false;
    }
    
    Desktop.uiFocusChanged.connect(onUiFocusChanged);
    
    function onUiFocusChanged(keyFocus) {
        if (!hmd) {
            if (mouseLookEnabled) {
                if (keyFocus) {
                    keysOnOverlay = true;
                    if (Camera.captureMouse) {
                        mouseLookOff();
                    }
                } else {
                    keysOnOverlay = false;
                    if (!tablet.tabletShown) {
                        if (!tempOff) {
                            if (!away) {
                                mouseLookOn();
                            }
                        }
                    }
                }
            }
        }
    }

    Script.scriptEnding.connect(onScriptEnding);

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

}()); // END LOCAL_SCOPE
