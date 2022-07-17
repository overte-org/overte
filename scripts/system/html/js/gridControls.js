//  gridControls.js
//
//  Created by Ryan Huffman on 6 Nov 2014
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

var createAppModeValue = "";
var voxelEditModeValue = "";

function loaded() {
    openEventBridge(function() {
        elCreateAppModeMenu = document.getElementById("create-app-mode");
        elEditModeObject = document.getElementById("edit-mode-object");
        elEditModeVoxel = document.getElementById("edit-mode-voxel");
        elVoxelEditModeMenu = document.getElementById("voxel-edit-mode");
        elVoxelEditModeSingle = document.getElementById("voxel-edit-mode-single");
        elVoxelEditModeSphere = document.getElementById("voxel-edit-mode-sphere");
        elVoxelEditModeCube = document.getElementById("voxel-edit-mode-cube");
        elMenuBackgroundOverlay = document.getElementById("menuBackgroundOverlay");
        elVoxelHelpButton = document.getElementById("voxel-help-button");
        elVoxelHelpPopup = document.getElementById("voxel-help-popup");

        elVoxelSphereSize = document.getElementById("voxel-sphere-size");
        elVoxelRemove = document.getElementById("voxel-remove");
        
        elPosY = document.getElementById("horiz-y");
        elMinorSpacing = document.getElementById("minor-spacing");
        elMajorSpacing = document.getElementById("major-spacing");
        elSnapToGrid = document.getElementById("snap-to-grid");
        elHorizontalGridVisible = document.getElementById("horiz-grid-visible");
        elMoveToSelection = document.getElementById("move-to-selection");
        elMoveToAvatar = document.getElementById("move-to-avatar");

        elCreateAppModeMenu.onclick = function() {
            document.getElementById("menuBackgroundOverlay").style.display = "block";
            document.getElementById("edit-mode-menu").style.display = "block";
        };

        elVoxelEditModeMenu.onclick = function() {
            document.getElementById("menuBackgroundOverlay").style.display = "block";
            document.getElementById("voxel-edit-mode-menu").style.display = "block";
        };

        elVoxelHelpButton.onclick = function() {
            document.getElementById("menuBackgroundOverlay").style.display = "block";
            document.getElementById("voxel-help-popup").style.display = "block";
        };

        elMenuBackgroundOverlay.onclick = function() {
            closeAllEntityListMenu();
        };

        elEditModeObject.onclick = function() {
            createAppModeValue = "object";
            elCreateAppModeMenu.value = "Object mode\u25BE";
            emitUpdateEditTools();
            closeAllEntityListMenu();
        };

        elEditModeVoxel.onclick = function() {
            createAppModeValue = "voxel";
            elCreateAppModeMenu.value = "Voxel edit mode\u25BE";
            emitUpdateEditTools();
            closeAllEntityListMenu();
        };
        
        elVoxelEditModeSingle.onclick = function() {
            voxelEditModeValue = "single";
            elVoxelEditModeMenu.value = "Single voxels\u25BE";
            emitUpdateEditTools();
            closeAllEntityListMenu();
        };

        elVoxelEditModeSphere.onclick = function() {
            voxelEditModeValue = "sphere";
            elVoxelEditModeMenu.value = "Spheres\u25BE";
            emitUpdateEditTools();
            closeAllEntityListMenu();
        };

        elVoxelEditModeCube.onclick = function() {
            voxelEditModeValue = "cube";
            elVoxelEditModeMenu.value = "Cubes\u25BE";
            emitUpdateEditTools();
            closeAllEntityListMenu();
        };

        elVoxelHelpPopup.onclick = function() {
            closeAllEntityListMenu();
        };

        if (window.EventBridge !== undefined) {
            EventBridge.scriptEventReceived.connect(function(data) {
                data = JSON.parse(data);

                if (data.createAppMode !== undefined) {
                    if (data.createAppMode === "object") {
                        createAppModeValue = data.createAppMode;
                        elCreateAppModeMenu.value = "Object mode\u25BE";
                    }
                    if (data.createAppMode === "voxel") {
                        createAppModeValue = data.createAppMode;
                        elCreateAppModeMenu.value = "Voxel edit mode\u25BE";
                    }
                }

                if (data.voxelEditMode !== undefined) {
                    if (data.voxelEditMode === "single") {
                        voxelEditModeValue = data.voxelEditMode;
                        elVoxelEditModeMenu.value = "Single voxels\u25BE";
                    }
                    if (data.voxelEditMode === "sphere") {
                        voxelEditModeValue = data.voxelEditMode;
                        elVoxelEditModeMenu.value = "Spheres\u25BE";
                    }
                    if (data.voxelEditMode === "cube") {
                        voxelEditModeValue = data.voxelEditMode;
                        elVoxelEditModeMenu.value = "Cubes\u25BE";
                    }
                }

                if (data.voxelSphereSize !== undefined) {
                    elVoxelSphereSize.value = data.voxelSphereSize;
                }

                if (data.voxelRemove !== undefined) {
                    elVoxelRemove.checked = data.voxelRemove == true;
                }

                if (data.origin) {
                    var origin = data.origin;
                    elPosY.value = origin.y;
                }

                if (data.minorGridEvery !== undefined) {
                    elMinorSpacing.value = data.minorGridEvery;
                }

                if (data.majorGridEvery !== undefined) {
                    elMajorSpacing.value = data.majorGridEvery;
                }

                if (data.gridColor) {
                    gridColor = data.gridColor;
                }

                if (data.snapToGrid !== undefined) {
                    elSnapToGrid.checked = data.snapToGrid == true;
                }

                if (data.visible !== undefined) {
                    elHorizontalGridVisible.checked = data.visible == true;
                }
            });

            function emitUpdate() {
                EventBridge.emitWebEvent(JSON.stringify({
                    type: "update",
                    origin: {
                        y: elPosY.value,
                    },
                    minorGridEvery: elMinorSpacing.value,
                    majorGridEvery: elMajorSpacing.value,
                    gridColor: gridColor,
                    snapToGrid: elSnapToGrid.checked,
                    visible: elHorizontalGridVisible.checked,
                }));
            }

            function emitUpdateEditTools() {
                EventBridge.emitWebEvent(JSON.stringify({
                    type: "update-edit-tools",
                    createAppMode: createAppModeValue,
                    voxelEditMode: voxelEditModeValue,
                    voxelSphereSize: elVoxelSphereSize.value,
                    voxelRemove: elVoxelRemove.checked,
                }));
            }
        }

        elVoxelSphereSize.addEventListener("change", emitUpdateEditTools);
        elVoxelRemove.addEventListener("change", emitUpdateEditTools);
        
        elPosY.addEventListener("change", emitUpdate);
        elMinorSpacing.addEventListener("change", emitUpdate);
        elMajorSpacing.addEventListener("change", emitUpdate);
        elSnapToGrid.addEventListener("change", emitUpdate);
        elHorizontalGridVisible.addEventListener("change", emitUpdate);

        elMoveToAvatar.addEventListener("click", function() {
            EventBridge.emitWebEvent(JSON.stringify({
                type: "action",
                action: "moveToAvatar",
            }));
        });
        elMoveToSelection.addEventListener("click", function() {
            EventBridge.emitWebEvent(JSON.stringify({
                type: "action",
                action: "moveToSelection",
            }));
        });

        var gridColor = { red: 255, green: 255, blue: 255 };
        var elColor = document.getElementById("grid-color");
        elColor.style.backgroundColor = "rgb(" + gridColor.red + "," + gridColor.green + "," + gridColor.blue + ")";

        var colorPickFunction = function (red, green, blue) {
            gridColor = { red: red, green: green, blue: blue };
            emitUpdate();
        };

        $('#grid-color').colpick({
            colorScheme: 'dark',
            layout: 'rgbhex',
            color: { r: gridColor.red, g: gridColor.green, b: gridColor.blue },
            submit: false,
            onShow: function (colpick) {
                $('#grid-color').attr('active', 'true');
            },
            onHide: function (colpick) {
                $('#grid-color').attr('active', 'false');
            },
            onChange: function (hsb, hex, rgb, el) {
                $(el).css('background-color', '#' + hex);
                colorPickFunction(rgb.r, rgb.g, rgb.b);
            }
        });

        augmentSpinButtons();
        disableDragDrop();

        EventBridge.emitWebEvent(JSON.stringify({ type: 'init' }));
    });

    const KEY_CODES = {
        BACKSPACE: 8,
        DELETE: 46
    };

    document.addEventListener("keyup", function (keyUpEvent) {
        const FILTERED_NODE_NAMES = ["INPUT", "TEXTAREA"];
        if (FILTERED_NODE_NAMES.includes(keyUpEvent.target.nodeName)) {
            return;
        }
        let {code, key, keyCode, altKey, ctrlKey, metaKey, shiftKey} = keyUpEvent;

        let controlKey = window.navigator.platform.startsWith("Mac") ? metaKey : ctrlKey;

        let keyCodeString;
        switch (keyCode) {
            case KEY_CODES.DELETE:
                keyCodeString = "Delete";
                break;
            case KEY_CODES.BACKSPACE:
                keyCodeString = "Backspace";
                break;
            default:
                keyCodeString = String.fromCharCode(keyUpEvent.keyCode);
                break;
        }

        EventBridge.emitWebEvent(JSON.stringify({
            type: 'keyUpEvent',
            keyUpEvent: {
                code,
                key,
                keyCode,
                keyCodeString,
                altKey,
                controlKey,
                shiftKey,
            }
        }));
    }, false);

    function closeAllEntityListMenu() {
        document.getElementById("menuBackgroundOverlay").style.display = "none";
        document.getElementById("edit-mode-menu").style.display = "none";
        document.getElementById("voxel-edit-mode-menu").style.display = "none";
        document.getElementById("voxel-help-popup").style.display = "none";
    }

    // Disable right-click context menu which is not visible in the HMD and makes it seem like the app has locked
    document.addEventListener("contextmenu", function (event) {
        event.preventDefault();
    }, false);

}
