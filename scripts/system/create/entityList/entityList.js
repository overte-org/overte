"use strict";

//  entityList.js
//
//  Created by Ryan Huffman on November 19th, 2014
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors.
//  Copyright 2023-2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

/* global EntityListTool, Tablet, Entities, Camera, MyAvatar, Vec3, Menu, Messages,
   MENU_EASE_ON_FOCUS,
   Script, Clipboard */

var PROFILING_ENABLED = false;
var profileIndent = '';

const PROFILE_NOOP = function(_name, fn, args) {
    fn.apply(this, args);
};
const PROFILE = !PROFILING_ENABLED ? PROFILE_NOOP : function(name, fn, args) {
    console.log("PROFILE-Script " + profileIndent + "(" + name + ") Begin");
    var previousIndent = profileIndent;
    profileIndent += '  ';
    var before = Date.now();
    fn.apply(this, args);
    var delta = Date.now() - before;
    profileIndent = previousIndent;
    console.log("PROFILE-Script " + profileIndent + "(" + name + ") End " + delta + "ms");
};

var EntityListTool = function(shouldUseEditTabletApp, selectionManager) {
    var that = {};
    that.selectionManager = selectionManager;

    var CreateWindow = Script.require('../modules/createWindow.js');

    var TITLE_OFFSET = 60;
    var ENTITY_LIST_WIDTH = 495;
    var MAX_DEFAULT_CREATE_TOOLS_HEIGHT = 778;
    var entityListWindow = new CreateWindow(
        Script.resolvePath("./qml/EditEntityList.qml"),
        'Entity List',
        'com.highfidelity.create.entityListWindow',
        function () {
            var windowHeight = Window.innerHeight - TITLE_OFFSET;
            if (windowHeight > MAX_DEFAULT_CREATE_TOOLS_HEIGHT) {
                windowHeight = MAX_DEFAULT_CREATE_TOOLS_HEIGHT;
            }
            return {
                size: {
                    x: ENTITY_LIST_WIDTH,
                    y: windowHeight
                },
                position: {
                    x: Window.x,
                    y: Window.y + TITLE_OFFSET
                }
            };
        },
        false
    );

    var webView = null;
    webView = Tablet.getTablet("com.highfidelity.interface.tablet.system");
    webView.setVisible = function(value){ };

    var filterInView = false;
    var searchRadius = 100;
    var localEntityFilter = false;

    var visible = false;

    that.webView = webView;

    that.setVisible = function(newVisible) {
        visible = newVisible;
        webView.setVisible(shouldUseEditTabletApp() && visible);
        entityListWindow.setVisible(!shouldUseEditTabletApp() && visible);
    };

    that.isVisible = function() {
        return entityListWindow.isVisible();
    };

    that.setVisible(false);

    function emitJSONScriptEvent(data) {
        var dataString;
        PROFILE("Script-JSON.stringify", function() {
            dataString = JSON.stringify(data);
        });
        PROFILE("Script-emitScriptEvent", function() {
            webView.emitScriptEvent(dataString);
            if (entityListWindow.window) {
                entityListWindow.window.emitScriptEvent(dataString);
            }
        });
    }

    that.toggleVisible = function() {
        that.setVisible(!visible);
    };

    selectionManager.addEventListener(function(isSelectionUpdate, caller) {
        if (caller === that) {
            // ignore events that we emitted from the entity list itself
            return;
        }
        // Otherwise this will emit tens of events every second when objects are moved.
        if (!isSelectionUpdate) {
            return;
        }
        var selectedIDs = [];

        for (var i = 0; i < that.selectionManager.selections.length; i++) {
            selectedIDs.push(that.selectionManager.selections[i]);
        }

        emitJSONScriptEvent({
            type: 'selectionUpdate',
            selectedIDs: selectedIDs
        });
    });

    that.setSpaceMode = function(spaceMode) {
        emitJSONScriptEvent({
            type: 'setSpaceMode',
            spaceMode: spaceMode
        });
    };

    that.clearEntityList = function() {
        emitJSONScriptEvent({
            type: 'clearEntityList'
        });
    };

    that.removeEntities = function (deletedIDs, selectedIDs) {
        emitJSONScriptEvent({
            type: 'removeEntities',
            deletedIDs: deletedIDs,
            selectedIDs: selectedIDs
        });
    };

    that.deleteEntities = function (deletedIDs) {
        emitJSONScriptEvent({
            type: "deleted",
            ids: deletedIDs
        });
    };

    that.setListMenuSnapToGrid = function (isSnapToGrid) {
        emitJSONScriptEvent({ "type": "setSnapToGrid", "snap": isSnapToGrid });
    };

    that.toggleSnapToGrid = function () {
        if (!grid.getSnapToGrid()) {
            grid.setSnapToGrid(true);
            emitJSONScriptEvent({ "type": "setSnapToGrid", "snap": true });
        } else {
            grid.setSnapToGrid(false);
            emitJSONScriptEvent({ "type": "setSnapToGrid", "snap": false });
        }
    };

    function valueIfDefined(value) {
        return value !== undefined ? value : "";
    }

    function entityIsBaked(properties) {
        if (properties.type === "Model") {
            var lowerModelURL = properties.modelURL.toLowerCase();
            return lowerModelURL.endsWith(".baked.fbx") || lowerModelURL.endsWith(".baked.fst");
        } else if (properties.type === "Zone") {
            var lowerSkyboxURL = properties.skybox ? properties.skybox.url.toLowerCase() : "";
            var lowerAmbientURL = properties.ambientLight ? properties.ambientLight.ambientURL.toLowerCase() : "";
            return (lowerSkyboxURL === "" || lowerSkyboxURL.endsWith(".texmeta.json")) &&
                (lowerAmbientURL === "" || lowerAmbientURL.endsWith(".texmeta.json"));
        } else {
            return false;
        }
    }

    that.sendUpdate = function() {
        var tablet = Tablet.getTablet("com.highfidelity.interface.tablet.system");
        if (HMD.active) {
            tablet.setLandscape(true);
        }
        emitJSONScriptEvent({
            "type": "confirmHMDstate",
            "isHmd": HMD.active
        });
        
        PROFILE('Script-sendUpdate', function() {
            var entities = [];

            var ids;
            PROFILE("findEntities", function() {
                var domainAndAvatarIds;
                if (filterInView) {
                    domainAndAvatarIds = Entities.findEntitiesInFrustum(Camera.frustum);
                } else {
                    domainAndAvatarIds = Entities.findEntities(MyAvatar.position, searchRadius);
                }
                var localIds = [];
                if (localEntityFilter) {
                    localIds = Overlays.findOverlays(MyAvatar.position, searchRadius);
                    var tabletLocalEntityToExclude = [
                        HMD.tabletID,
                        HMD.tabletScreenID,
                        HMD.homeButtonID,
                        HMD.homeButtonHighlightID,
                        HMD.miniTabletID,
                        HMD.miniTabletScreenID,
                        that.grid.getGridEntityToolID()
                    ];
                    var seltoolsIds = SelectionDisplay.toolEntityMaterial.concat(
                        SelectionDisplay.allToolEntities, 
                        allOverlays,
                        that.createApp.entityShapeVisualizerLocalEntityToExclude,
                        tabletLocalEntityToExclude
                    );
                    for (var i = localIds.length - 1; i >= 0; i--) {
                        if (seltoolsIds.includes(localIds[i]) || Keyboard.containsID(localIds[i])) {
                            localIds.splice(i, 1);
                        }
                    }
                }
                ids = domainAndAvatarIds.concat(localIds);
            });

            var cameraPosition = Camera.position;
            PROFILE("getMultipleProperties", function () {
                var multipleProperties = Entities.getMultipleEntityProperties(ids, ['position', 'name', 'type', 'locked',
                    'visible', 'renderInfo', 'modelURL', 'materialURL', 'imageURL', 'script', 'serverScripts',
                    'skybox.url', 'ambientLight.url', 'soundURL', 'created', 'lastEdited', 'entityHostType']);
                for (var i = 0; i < multipleProperties.length; i++) {
                    var properties = multipleProperties[i];

                    if (!filterInView || Vec3.distance(properties.position, cameraPosition) <= searchRadius) {
                        var url = "";
                        if (properties.type === "Model") {
                            url = properties.modelURL;
                        } else if (properties.type === "Material") {
                            url = properties.materialURL;
                        } else if (properties.type === "Image") {
                            url = properties.imageURL;
                        } else if (properties.type === "Sound") {
                            url = properties.soundURL;
                        }
                        //print("Global object before getParentState call: " + JSON.stringify(globalThis));
                        var parentStatus = that.createApp.getParentState(ids[i]);
                        var parentState = "";
                        if (parentStatus === "PARENT") {
                            parentState = "A";
                        } else if (parentStatus === "CHILDREN") {
                            parentState = "C";
                        } else if (parentStatus === "PARENT_CHILDREN") {
                            parentState = "B";
                        }

                        entities.push({
                            id: ids[i],
                            name: properties.name,
                            type: properties.type,
                            url: url,
                            locked: properties.locked,
                            visible: properties.visible,
                            verticesCount: (properties.renderInfo !== undefined ?
                                valueIfDefined(properties.renderInfo.verticesCount) : ""),
                            texturesCount: (properties.renderInfo !== undefined ?
                                valueIfDefined(properties.renderInfo.texturesCount) : ""),
                            texturesSize: (properties.renderInfo !== undefined ?
                                valueIfDefined(properties.renderInfo.texturesSize) : ""),
                            hasTransparent: (properties.renderInfo !== undefined ?
                                valueIfDefined(properties.renderInfo.hasTransparent) : ""),
                            isBaked: entityIsBaked(properties),
                            drawCalls: (properties.renderInfo !== undefined ?
                                valueIfDefined(properties.renderInfo.drawCalls) : ""),
                            hasScript: (properties.script !== "" || properties.serverScripts !== ""),
                            parentState: parentState,
                            created: formatToStringDateTime(properties.created),
                            lastEdited: formatToStringDateTime(properties.lastEdited),
                            entityHostType: properties.entityHostType
                        });
                    }
                }
            });

            var selectedIDs = [];
            for (var j = 0; j < that.selectionManager.selections.length; j++) {
                selectedIDs.push(that.selectionManager.selections[j]);
            }

            emitJSONScriptEvent({
                type: "update",
                entities: entities,
                selectedIDs: selectedIDs,
                spaceMode: SelectionDisplay.getSpaceMode(),
            });
        });
    };

    function formatToStringDateTime(timestamp) {
        var d = new Date(Math.floor(timestamp/1000));
        var dateTime = d.getUTCFullYear() + "-" + zeroPad((d.getUTCMonth() + 1), 2) + "-" + zeroPad(d.getUTCDate(), 2);
        dateTime = dateTime + " " + zeroPad(d.getUTCHours(), 2) + ":" + zeroPad(d.getUTCMinutes(), 2) + ":" + zeroPad(d.getUTCSeconds(), 2); 
        dateTime = dateTime + "." + zeroPad(d.getUTCMilliseconds(), 3);
        return dateTime;
    }

    function zeroPad(num, size) {
        num = num.toString();
        while (num.length < size) {
            num = "0" + num;
        }
        return num;
    }

    function onFileSaveChanged(filename) {
        Window.saveFileChanged.disconnect(onFileSaveChanged);
        if (filename !== "") {
            var success = Clipboard.exportEntities(filename, that.selectionManager.selections);
            if (!success) {
                Window.notifyEditError("Export failed.");
            }
        }
    }

    var onWebEventReceived = function(data) {
        //print("entityList.js onWebEventReceived: " + data);
        try {
            data = JSON.parse(data);
        } catch(e) {
            print("entityList.js: Error parsing JSON");
            return;
        }

        if (data.type === "selectionUpdate") {
            var ids = data.entityIds;
            var entityIDs = [];
            for (var i = 0; i < ids.length; i++) {
                entityIDs.push(ids[i]);
            }
            that.selectionManager.setSelections(entityIDs, that);
            if (data.focus) {
                that.cameraManager.enable();
                that.cameraManager.focus(that.selectionManager.worldPosition,
                                    that.selectionManager.worldDimensions,
                                    Menu.isOptionChecked(MENU_EASE_ON_FOCUS));
            }
        } else if (data.type === "refresh") {
            that.sendUpdate();
        } else if (data.type === "teleport") {
            if (that.selectionManager.hasSelection()) {
                MyAvatar.position = that.selectionManager.worldPosition;
            }
        } else if (data.type === "export") {
            if (!that.selectionManager.hasSelection()) {
                Window.notifyEditError("No entities have been selected.");
            } else {
                Window.saveFileChanged.connect(onFileSaveChanged);
                Window.saveAsync("Select Where to Save", "", "*.json");
            }
        } else if (data.type === "delete") {
            that.createApp.deleteSelectedEntities();
        } else if (data.type === "toggleLocked") {
            that.createApp.toggleSelectedEntitiesLocked();
        } else if (data.type === "toggleVisible") {
            that.createApp.toggleSelectedEntitiesVisible();
        } else if (data.type === "filterInView") {
            filterInView = data.filterInView === true;
        } else if (data.type === "radius") {
            searchRadius = data.radius;
        } else if (data.type === "cut") {
            that.selectionManager.cutSelectedEntities();
        } else if (data.type === "copy") {
            that.selectionManager.copySelectedEntities();
        } else if (data.type === "copyID") {
            that.selectionManager.copyIdsFromSelectedEntities();
        } else if (data.type === "paste") {
            that.selectionManager.pasteEntities();
        } else if (data.type === "duplicate") {
            that.selectionManager.duplicateSelection();
            that.sendUpdate();
        } else if (data.type === "rename") {
            Entities.editEntity(data.entityID, {name: data.name});
            // make sure that the name also gets updated in the properties window
            that.selectionManager._update();
        } else if (data.type === "toggleSpaceMode") {
            SelectionDisplay.toggleSpaceMode();
        } else if (data.type === 'keyUpEvent') {
            that.createApp.keyUpEventFromUIWindow(data.keyUpEvent);
        } else if (data.type === 'undo') {
            that.createApp.undoHistory.undo();
        } else if (data.type === 'redo') {
            that.createApp.undoHistory.redo();
        } else if (data.type === 'parent') {
            that.createApp.parentSelectedEntities();
        } else if (data.type === 'unparent') {
            that.createApp.unparentSelectedEntities();
        } else if (data.type === 'hmdMultiSelectMode') {
            that.createApp.hmdMultiSelectMode = data.value;
        } else if (data.type === 'selectAllInBox') {
            that.createApp.selectAllEntitiesInCurrentSelectionBox(false);
        } else if (data.type === 'selectAllTouchingBox') {
            that.createApp.selectAllEntitiesInCurrentSelectionBox(true);
        } else if (data.type === 'selectParent') {
            that.selectionManager.selectParent();
        } else if (data.type === 'selectTopParent') {
            that.selectionManager.selectTopParent();
        } else if (data.type === 'addChildrenToSelection') {
            that.selectionManager.addChildrenToSelection();
        } else if (data.type === 'selectFamily') {
            that.selectionManager.selectFamily();
        } else if (data.type === 'selectTopFamily') {
            that.selectionManager.selectTopFamily();
        } else if (data.type === 'teleportToEntity') {
            that.selectionManager.teleportToEntity();
        } else if (data.type === 'rotateAsTheNextClickedSurface') {
            that.createApp.rotateAsNextClickedSurface();
        } else if (data.type === 'quickRotate90x') {
            that.selectionDisplay.rotate90degreeSelection("X");
        } else if (data.type === 'quickRotate90y') {
            that.selectionDisplay.rotate90degreeSelection("Y");
        } else if (data.type === 'quickRotate90z') {
            that.selectionDisplay.rotate90degreeSelection("Z");
        } else if (data.type === 'moveEntitySelectionToAvatar') {
            that.selectionManager.moveEntitiesSelectionToAvatar();
        } else if (data.type === 'loadConfigSetting') {
            var columnsData = Settings.getValue(that.createApp.SETTING_EDITOR_COLUMNS_SETUP, "NO_DATA");
            var defaultRadius = Settings.getValue(that.createApp.SETTING_ENTITY_LIST_DEFAULT_RADIUS, 100);
            emitJSONScriptEvent({
                "type": "loadedConfigSetting",
                "columnsData": columnsData,
                "defaultRadius": defaultRadius,
                "localEntityFilter": localEntityFilter
            });
        } else if (data.type === 'saveColumnsConfigSetting') {
            Settings.setValue(that.createApp.SETTING_EDITOR_COLUMNS_SETUP, data.columnsData);
        } else if (data.type === 'importFromFile') {
            that.createApp.importEntitiesFromFile();
        } else if (data.type === 'importFromUrl') {
            that.createApp.importEntitiesFromUrl();
        } else if (data.type === 'setCameraFocusToSelection') {
            that.createApp.setCameraFocusToSelection();
        } else if (data.type === 'alignGridToSelection') {
            that.createApp.alignGridToSelection();
        } else if (data.type === 'alignGridToAvatar') {
            that.createApp.alignGridToAvatar();
        } else if (data.type === 'brokenURLReport') {
            brokenURLReport(that.selectionManager.selections);
        } else if (data.type === 'renderWithZonesManager') {
            renderWithZonesManager(that.selectionManager.selections);
        } else if (data.type === 'toggleGridVisibility') {
            that.createApp.toggleGridVisibility();
        } else if (data.type === 'toggleSnapToGrid') {
            that.toggleSnapToGrid();
        } else if (data.type === 'localEntityFilter') {
            localEntityFilter = data.localEntityFilter;
            that.sendUpdate();
        }

    };

    webView.webEventReceived.connect(onWebEventReceived);
    entityListWindow.webEventReceived.addListener(onWebEventReceived);
    that.interactiveWindowHidden = entityListWindow.interactiveWindowHidden;

    return that;
};
