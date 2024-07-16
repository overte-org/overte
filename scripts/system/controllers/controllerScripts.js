"use strict";

//  controllerScripts.js
//
//  Created by David Rowe on 15 Mar 2017.
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

/* global Script, Menu */

Script.include("controllerDispatcher.js");

var CONTOLLER_SCRIPTS = [
    "controllerDisplayManager.js",
    "grab.js",
    "toggleAdvancedMovementForHandControllers.js",
    // TODO: These are not used currently. It can be reworked re-enabled when we have OpenXR support
    //"handTouch.js",
    "mouseLook.js",
    "controllerModules/nearParentGrabOverlay.js",
    "controllerModules/stylusInput.js",
    "controllerModules/equipEntity.js",
    "controllerModules/nearTrigger.js",
    "controllerModules/webSurfaceLaserInput.js",
    "controllerModules/inEditMode.js",
    "controllerModules/inVREditMode.js",
    "controllerModules/disableOtherModule.js",
    "controllerModules/farTrigger.js",
    "controllerModules/teleport.js",
    "controllerModules/hudOverlayPointer.js",
    "controllerModules/mouseHMD.js",
    "controllerModules/nearGrabHyperLinkEntity.js",
    "controllerModules/nearTabletHighlight.js",
    "controllerModules/nearGrabEntity.js",
    "controllerModules/farGrabEntity.js",
    "controllerModules/pushToTalk.js",
    // TODO: These are not used currently, but have severe impact on performace. They can be re-enabled when we have OpenXR support
    //"controllerModules/trackedHandWalk.js",
    //"controllerModules/trackedHandTablet.js"
];

//Script.include("../../developer/debugging/scriptMemoryReport.js");

var DEBUG_MENU_ITEM = "Debug defaultScripts.js";

function runDefaultsTogether() {
    for (var j in CONTOLLER_SCRIPTS) {
        if (CONTOLLER_SCRIPTS.hasOwnProperty(j)) {
            print("including " + CONTOLLER_SCRIPTS[j]);
            Script.include(CONTOLLER_SCRIPTS[j]);
        }
    }
}

function runDefaultsSeparately() {
    for (var i in CONTOLLER_SCRIPTS) {
        if (CONTOLLER_SCRIPTS.hasOwnProperty(i)) {
            print("loading " + CONTOLLER_SCRIPTS[i]);
            Script.load(CONTOLLER_SCRIPTS[i]);
        }
    }
}

if (Menu.isOptionChecked(DEBUG_MENU_ITEM)) {
    runDefaultsSeparately();
} else {
    runDefaultsTogether();
}
