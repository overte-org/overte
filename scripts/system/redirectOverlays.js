"use strict";
//
//  redirectOverlays.js
//
//  Created by Wayne Chen on September 25th, 2018
//  Copyright 2018 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Overlays
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

(function() {

    var ERROR_MESSAGE_MAP = [
        "Oops! Protocol version mismatch.",
        "Oops! Not authorized to join domain.",
        "Oops! Connection timed out.",
        "Oops! The domain is full.",
        "Oops! Something went wrong."
    ];

    var PROTOCOL_VERSION_MISMATCH = 1;
    var NOT_AUTHORIZED = 3;
    var DOMAIN_FULL = 4;
    var TIMEOUT = 5;
    var hardRefusalErrors = [
        PROTOCOL_VERSION_MISMATCH,
        NOT_AUTHORIZED,
        TIMEOUT,
        DOMAIN_FULL
    ];
    var timer = null;
    var isErrorState = false;

    function getOopsText() {
        var error = Window.getLastDomainConnectionError();
        var errorMessageMapIndex = hardRefusalErrors.indexOf(error);
        if (error === -1) {
            // not an error.
            return "";
        } else if (errorMessageMapIndex >= 0) {
            return ERROR_MESSAGE_MAP[errorMessageMapIndex];
        } else {
            // some other text.
            return ERROR_MESSAGE_MAP[ERROR_MESSAGE_MAP.length - 1];
        }
    };

    var oopsDimensions = {x: 4.2, y: 0.8};

    var redirectOopsText = Entities.addEntity({
        "type": "Text",
        "name": "oopsText",
        "position": {"x": 0, "y": 1.6763916015625, "z": 1.45927095413208},
        "rotation": {"x": -4.57763671875e-05, "y": 0.4957197904586792, "z": -7.62939453125e-05, "w": 0.8684672117233276},
        "text": getOopsText(),
        "textAlpha": 1,
        "backgroundColor": {"x": 0, "y": 0, "z":0},
        "backgroundAlpha": 0,
        "lineHeight": 0.10,
        "leftMargin": 0.538373570564886,
        "visible": false,
        "unlit": true,
        "ignorePickIntersection": true,
        "dimensions": oopsDimensions,
        "grab": {
            "grabbable": false
        }
    }, "local");

    var tryAgainImageNeutral = Entities.addEntity({
        "type": "Image",
        "name": "tryAgainImage",
        "localPosition": {"x": -0.6, "y": -0.6, "z": 0.0},
        "imageURL": Script.resourcesPath() + "images/interstitialPage/button.png",
        "alpha": 1,
        "visible": false,
        "emissive": true,
        "ignorePickIntersection": false,
        "grab": {
            "grabbable": false
        },
        "rotation": Entities.getEntityProperties(redirectOopsText, ["rotation"]).rotation,
        "parentID": redirectOopsText
    }, "local");

    var tryAgainImageHover = Entities.addEntity({
        "type": "Image",
        "name": "tryAgainImageHover",
        "localPosition": {"x": -0.6, "y": -0.6, "z": 0.0},
        "imageURL": Script.resourcesPath() + "images/interstitialPage/button_hover.png",
        "alpha": 1,
        "visible": false,
        "emissive": true,
        "ignorePickIntersection": false,
        "grab": {
            "grabbable": false
        },
        "rotation": Entities.getEntityProperties(redirectOopsText, ["rotation"]).rotation,
        "parentID": redirectOopsText
    }, "local");

    var tryAgainText = Entities.addEntity({
        "type": "Text",
        "name": "tryAgainText",
        "localPosition": {"x": -0.6, "y": -0.962, "z": 0.0},
        "text": "Try Again",
        "textAlpha": 1,
        "backgroundAlpha": 0.00393,
        "lineHeight": 0.08,
        "visible": false,
        "unlit": true,
        "ignorePickIntersection": true,
        "grab": {
            "grabbable": false
        },
        "rotation": Entities.getEntityProperties(redirectOopsText, ["rotation"]).rotation,
        "parentID": redirectOopsText
    }, "local");

    var backImageNeutral = Entities.addEntity({
        "type": "Image",
        "name": "backImage",
        "localPosition": {"x": 0.6, "y": -0.6, "z": 0.0},
        "imageURL": Script.resourcesPath() + "images/interstitialPage/button.png",
        "alpha": 1,
        "visible": false,
        "emissive": true,
        "ignorePickIntersection": false,
        "grab": {
            "grabbable": false
        },
        "rotation": Entities.getEntityProperties(redirectOopsText, ["rotation"]).rotation,
        "parentID": redirectOopsText
    }, "local");

    var backImageHover = Entities.addEntity({
        "type": "Image",
        "name": "backImageHover",
        "localPosition": {"x": 0.6, "y": -0.6, "z": 0.0},
        "imageURL": Script.resourcesPath() + "images/interstitialPage/button_hover.png",
        "alpha": 1,
        "visible": false,
        "emissive": true,
        "ignorePickIntersection": false,
        "grab": {
            "grabbable": false
        },
        "rotation": Entities.getEntityProperties(redirectOopsText, ["rotation"]).rotation,
        "parentID": redirectOopsText
    }, "local");

    var backText = Entities.addEntity({
        "type": "Text",
        "name": "backText",
        "localPosition": {"x": 0.6, "y": -0.962, "z": 0.0},
        "text": "Back",
        "textAlpha": 1,
        "backgroundAlpha": 0.00393,
        "lineHeight": 0.08,
        "visible": false,
        "unlit": true,
        "ignorePickIntersection": true,
        "grab": {
            "grabbable": false
        },
        "rotation": Entities.getEntityProperties(redirectOopsText, ["rotation"]).rotation,
        "parentID": redirectOopsText
    }, "local");

    function toggleOverlays(isInErrorState) {
        isErrorState = isInErrorState;
        if (!isInErrorState) {
            var properties = {
                "visible": false
            };

            Entities.editEntity(redirectOopsText, properties);
            Entities.editEntity(tryAgainImageNeutral, properties);
            Entities.editEntity(tryAgainImageHover, properties);
            Entities.editEntity(backImageNeutral, properties);
            Entities.editEntity(backImageHover, properties);
            Entities.editEntity(tryAgainText, properties);
            Entities.editEntity(backText, properties);
            return;
        }
        var oopsText = getOopsText();
        // if oopsText === "", it was a success.
        var overlaysVisible = (oopsText !== "");
        // for catching init or if error state were to be different.
        isErrorState = overlaysVisible;
        var properties = {
            "visible": overlaysVisible
        };

        var textWidth = Entities.textSize(redirectOopsText, oopsText).width;
        var textOverlayWidth = oopsDimensions.x;

        var oopsTextProperties = {
            "visible": overlaysVisible,
            "text": oopsText,
            "textAlpha": overlaysVisible,
            // either visible or invisible. 0 doesn't work in Mac.
            "backgroundAlpha": overlaysVisible * 0.00393,
            "leftMargin": (textOverlayWidth - textWidth) / 2
        };

        var tryAgainTextWidth = Entities.textSize(tryAgainText, "Try Again").width;
        var tryAgainImageWidth = Entities.getEntityProperties(tryAgainImageNeutral, ["dimensions"]).dimensions.x;

        var tryAgainTextProperties = {
            "visible": overlaysVisible,
            "leftMargin": (tryAgainImageWidth - tryAgainTextWidth) / 2
        };

        var backTextWidth = Entities.textSize(backText, "Back").width;
        var backImageWidth = Entities.getEntityProperties(backImageNeutral, ["dimensions"]).dimensions.x;

        var backTextProperties = {
            "visible": overlaysVisible,
            "leftMargin": (backImageWidth - backTextWidth) / 2
        };

        Entities.editEntity(redirectOopsText, oopsTextProperties);
        Entities.editEntity(tryAgainImageNeutral, properties);
        Entities.editEntity(backImageNeutral, properties);
        Entities.editEntity(tryAgainImageHover, {"visible": false});
        Entities.editEntity(backImageHover, {"visible": false});
        Entities.editEntity(tryAgainText, tryAgainTextProperties);
        Entities.editEntity(backText, backTextProperties);

    }

    function clickedOnOverlay(overlayID, event) {
        if (event.isRightButton) {
            // don't allow right-clicks.
            return;
        }
        if (tryAgainImageHover === overlayID) {
            location.goToLastAddress();
        } else if (backImageHover === overlayID && location.canGoBack()) {
            location.goBack();
        }
    }

    function cleanup() {
        Script.clearInterval(timer);
        timer = null;
        Entities.deleteEntity(redirectOopsText);
        Entities.deleteEntity(tryAgainImageNeutral);
        Entities.deleteEntity(backImageNeutral);
        Entities.deleteEntity(tryAgainImageHover);
        Entities.deleteEntity(backImageHover);
        Entities.deleteEntity(tryAgainText);
        Entities.deleteEntity(backText);
    }

    toggleOverlays(true);

    Overlays.mouseReleaseOnOverlay.connect(clickedOnOverlay);
    Overlays.hoverEnterOverlay.connect(function(overlayID, event) {
        if (!isErrorState) {
            // don't allow hover overlay events to get caught if it's not in error state anymore.
            return;
        }
        if (overlayID === backImageNeutral && location.canGoBack()) {
            Entities.editEntity(backImageNeutral, {"visible": false});
            Entities.editEntity(backImageHover, {"visible": true});
        }
        if (overlayID === tryAgainImageNeutral) {
            Entities.editEntity(tryAgainImageNeutral, {"visible": false});
            Entities.editEntity(tryAgainImageHover, {"visible": true});
        }
    });

    Overlays.hoverLeaveOverlay.connect(function(overlayID, event) {
        if (!isErrorState) {
            // don't allow hover overlay events to get caught if it's not in error state anymore.
            return;
        }
        if (overlayID === backImageHover) {
            Entities.editEntity(backImageHover, {"visible": false});
            Entities.editEntity(backImageNeutral, {"visible": true});
        }
        if (overlayID === tryAgainImageHover) {
            Entities.editEntity(tryAgainImageHover, {"visible": false});
            Entities.editEntity(tryAgainImageNeutral, {"visible": true});
        }
    });

    Window.redirectErrorStateChanged.connect(toggleOverlays);

    Script.scriptEnding.connect(cleanup);
}());
