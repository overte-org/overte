//
//  renderWithZonesManager.js
//
//  Created by Alezia Kurdis on January 28th, 2024.
//  Copyright 2024 Overte e.V.
//
//  This script is to manage the zone in the property renderWithZones more efficiently in the Create Application.
//  It allows a global view over a specific selection with possibility to 
//  REPLACE, REMOVE or ADD zones on those properties more efficiently.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
let rwzmSelectedId = [];
let rwzmAllZonesList = [];
let rwzmUsedZonesList = [];
let rwzmSelectedEntitiesData = [];
let rwzmUndo = [];

let enforceLocked = false;

const RWZ_ZONE_SCAN_RADIUS = 27713; //maximal radius to cover the entire domain.

let rwzmOverlayWebWindow = null;

function renderWithZonesManager(entityIDs, highlightedID = "") {
    if (rwzmGetCheckSum(entityIDs) !== rwzmGetCheckSum(rwzmSelectedId)) {
        rwzmUndo = [];
    }
    rwzmSelectedId = entityIDs;
    if (entityIDs.length === 0) {
        audioFeedback.rejection();
        Window.alert("You have nothing selected.");
        return;
    } else {
        rwzmAllZonesList = [];
        rwzmUsedZonesList = [];
        rwzmSelectedEntitiesData = [];
        rwzmAllZonesList = rwzmGetExistingZoneList();
        
        let properties;
        let i = 0;
        let j = 0;
        let rwzmData = {};
        for (i = 0; i < entityIDs.length; i++ ){
            properties = Entities.getEntityProperties(entityIDs[i], ["renderWithZones", "locked", "name", "type"]);
            //Identify the unique zone used in renderWithZones properties of the entities and make the list of this in rwzmUsedZonesList
            if (properties.renderWithZones.length > 0) {
                for (j = 0; j < properties.renderWithZones.length; j++ ){
                    if (rwzmUsedZonesList.indexOf(properties.renderWithZones[j]) === -1) {
                        rwzmUsedZonesList.push(properties.renderWithZones[j]);
                    }
                }
            }
            //Make the list of entities, with their id, renderWithZones, locked, in rwzmSelectedEntitiesData
            rwzmData = {
                "id": entityIDs[i],
                "name": properties.name,
                "type": properties.type,
                "renderWithZones": properties.renderWithZones,
                "locked": properties.locked
            };
            rwzmSelectedEntitiesData.push(rwzmData);
        }
    }

    if (rwzmOverlayWebWindow === null) {
        rwzmOverlayWebWindow = new OverlayWebWindow({
            title: "RenderWithZones Manager",
            source: Script.resolvePath("renderWithZonesManager.html"),
            width: 1100,
            height: 600
        });
    
        rwzmOverlayWebWindow.closed.connect(uiHasClosed);
        rwzmOverlayWebWindow.webEventReceived.connect(webEventReceiver);
    }
    
    rwzmGenerateUI(highlightedID);
}

function rwzmGetCheckSum(array) {
    let i = 0;
    let sum = 0;
    let strForm = JSON.stringify(array);
    for (i = 0; i < strForm.length; i++) {
        sum = sum + strForm.charCodeAt(i);
    }
    return sum;
}
function uiHasClosed() {
    rwzmOverlayWebWindow.closed.disconnect(uiHasClosed);
    rwzmOverlayWebWindow.webEventReceived.disconnect(webEventReceiver);
    rwzmOverlayWebWindow = null;
}

function rwzmGenerateUI(highlightedID) {
    let canUnlock = Entities.canAdjustLocks();
    let uiContent = "";
    let i = 0;
    let k = 0;
    let zones = "";
    let name = "";
    let elementClass = "";
    let firstClass = "";
    let isLocked = "";
    let selectionBox = "";
    let setCheck = "";
    let addAction = "";
    let toHighlight = "";
    let viewBtnCaption = "";
    let warning = false;
    uiContent = uiContent + '    <h1>RenderWithZones Manager</h1><hr>\n';
    if (canUnlock) {
        if (enforceLocked) {
            setCheck = " checked";
        } else {
            setCheck = "";
        }
    }
    uiContent = uiContent + '    <table>\n';
    uiContent = uiContent + '        <tr valign = "top">\n';
    uiContent = uiContent + '            <td style="width: 40%">\n';
    if (rwzmUndo.length === 0) {
        undoBtnCode = "";
    } else {
        undoBtnCode = '<button class="greyBtn small" onClick="undo();">Undo</button>';
    }
    uiContent = uiContent + '                <table><tr><td style="width: 40%;"><h2>Visibility Zones:</h2></td><td style="width: 60%; text-align: right;">' + undoBtnCode + '</td></tr></table>\n';
    uiContent = uiContent + '                <div class="listContainer"><table>\n';
    uiContent = uiContent + '                    <tr style="width: 100%"><td class="cells header" style="width: 60%;">';
    uiContent = uiContent + '<b>ZONES</b></td><td class="cells header" style="width: 40%;"><b>ACTIONS (On listed entities)</b></td></tr>\n';
    for (i = 0; i < rwzmUsedZonesList.length; i++ ) {
        name = rwzmGetZoneName(rwzmUsedZonesList[i]);
        elementClass = "line";
        firstClass = "cells";
        if (name === "") {
            name = rwzmGenerateUnidentifiedZoneName(rwzmUsedZonesList[i]);
            elementClass = "errorline";
            warning = true;
        }
        toHighlight = rwzmUsedZonesList[i];
        viewBtnCaption = "View";
        if ( rwzmUsedZonesList[i] === highlightedID) {
            toHighlight = "";
            viewBtnCaption = "Hide";
            firstClass = "highlightedCells";
            if (elementClass === "line") {
                elementClass = "lineInverted";
            }
        }
        uiContent = uiContent + '                    <tr style="width: 100%"><td class="' + firstClass + ' ' + elementClass;
        uiContent = uiContent + '"><div style = "width:100%; height:100%; padding: 3px;" onClick = "highlight(' + "'";
        uiContent = uiContent + toHighlight + "'" +');">' + rwzmGetTruncatedString(name, 30) + '</div></td><td class="' + firstClass + ' ' + elementClass + '">';
        uiContent = uiContent + '<button class="greyBtn small" onclick="highlight(' + "'" + toHighlight + "'" +');">' + viewBtnCaption + '</button>';
        uiContent = uiContent + '<button class="redBtn" onclick="removeZoneOnAllEntities(' + "'" + rwzmUsedZonesList[i] + "'" +');">Remove</button>';
        uiContent = uiContent + '<button class="blueBtn" onclick="replaceZoneOnAllEntities(' + "'" + rwzmUsedZonesList[i] + "'" +');">Replace</button></td></tr>\n';
    }
    uiContent = uiContent + '                </table></div>\n';
    uiContent = uiContent + '            </td>\n';
    uiContent = uiContent + '            <td style="width: 3%">&nbsp;</td>\n';
    uiContent = uiContent + '            <td style="width: 57%">\n';
    uiContent = uiContent + '                <table><tr><td style="width: 15%;"><h2>Entities:</h2></td><td style="width: 30%;"><button class="addbtn" onClick="addZoneToEntity(getIdsFromScope());">Add to Selected</button></td>';
    uiContent = uiContent + '<td style="width: 55%; text-align: right;"><input type="checkbox" id = "enforceLocked"'+ setCheck + ' onClick="refresh()";> <font style="color: #3bc7ff;">Modify locked entities for me.</font></td></tr></table>\n';
    uiContent = uiContent + '                <div class="listContainer"><table>\n';
    uiContent = uiContent + '                    <tr style="width: 100%;"><td class="cells header" style="width: 5%;">';
    uiContent = uiContent + '<input type="checkbox" id="fullScope" onClick="selectAllOrNone();"></td><td class="cells header" style="width: 3%;">';
    uiContent = uiContent + '<font class="hifiGlyphs">&#xe006;</font></td><td class="cells header" style = "width: 45%;"><b>ENTITIES</b></td><td class="cells header" style = "width: 40%;">';
    uiContent = uiContent + '<b>RENDER WITH ZONES</b></td><td style = "width: 7%;"class="cells header">&nbsp;</td></tr>\n';
    for (i = 0; i < rwzmSelectedEntitiesData.length; i++ ) {
        elementClass = "line";
        firstClass = "cells";
        if (rwzmSelectedEntitiesData[i].renderWithZones.indexOf(highlightedID) !== -1) {
            firstClass = "highlightedCells";
            elementClass = "lineInverted";
        }
        zones = "&nbsp;";
        if (rwzmSelectedEntitiesData[i].renderWithZones.length > 0) {
            for (k = 0; k < rwzmSelectedEntitiesData[i].renderWithZones.length; k++ ) {
                name = rwzmGetTruncatedString(rwzmGetZoneName(rwzmSelectedEntitiesData[i].renderWithZones[k]),30);
                if (name === "") {
                    name = rwzmGetTruncatedString(rwzmGenerateUnidentifiedZoneName(rwzmSelectedEntitiesData[i].renderWithZones[k]),30);
                }
                if ((canUnlock && enforceLocked && rwzmSelectedEntitiesData[i].locked) || !rwzmSelectedEntitiesData[i].locked) {
                    name = name + " <span class='delBtn' onClick='removeZoneFromRWZ(" + '"' + rwzmSelectedEntitiesData[i].id + '", "' + rwzmSelectedEntitiesData[i].renderWithZones[k] + '"' + ");'>&#11198;</span>";
                }
                
                if (k === 0) {
                    zones = zones + name;
                } else {
                    zones = zones + "<br>" + name;
                }
            }
        }
        isLocked = "&nbsp;";
        selectionBox = "&nbsp;";
        addAction = "&nbsp;";
        if ((canUnlock && enforceLocked && rwzmSelectedEntitiesData[i].locked) || !rwzmSelectedEntitiesData[i].locked) {
            addAction = "<button class='addbtn' onClick='addZoneToEntity([" + '"' + rwzmSelectedEntitiesData[i].id + '"' + "]);'>Add</button>";
            selectionBox = '<input type="checkbox" name="entitiesScope" value = "' + rwzmSelectedEntitiesData[i].id + '">';
        }
        if (rwzmSelectedEntitiesData[i].locked) {
            if (canUnlock) {
                isLocked = "&#xe006;"; //Locked
            } else {
                isLocked = "&#128711;"; //Forbidden
            }
        }
        uiContent = uiContent + '                    <tr style="width: 100%"><td class="' + firstClass + ' ' + elementClass + '">' + selectionBox;
        uiContent = uiContent + '</td><td class="' + firstClass + ' ' + elementClass + '"><font class="hifiGlyphs">' + isLocked + '</font></td><td class="';
        uiContent = uiContent + firstClass + ' ' + elementClass + '">' + rwzmSelectedEntitiesData[i].type + ' - ' + rwzmGetTruncatedString(rwzmSelectedEntitiesData[i].name, 30) + '</b></td><td class="' + firstClass;
        uiContent = uiContent + ' ' + elementClass + '">' + zones + '</td><td class="' + firstClass + ' ' + elementClass + '">' + addAction + '</td></tr>\n';
    }
    uiContent = uiContent + '                </table>\n';
    uiContent = uiContent + '            </td>\n';
    uiContent = uiContent + '        </tr>\n';
    uiContent = uiContent + '    </table></div>\n';
    uiContent = uiContent + '    <input type = "hidden" id = "highlightedID" value = "' + highlightedID + '">\n';
    if (warning) {
        uiContent = uiContent + '    <div class="warning"><b>WARNING</b>: The "<b>ZONE NOT FOUND</b>" visibility zones might simply not be loaded if too far and small. Please, verify before.</div>\n';
    }
    //Zone selector Add
    uiContent = uiContent + '    <div id="rwzmAddZoneSelector">\n';
    uiContent = uiContent + '    <h2>Select the zone to add:</h2><div class="zoneSelectorContainer">\n';
    for (i = 0; i < rwzmAllZonesList.length; i++ ) {
        uiContent = uiContent + "        <button class = 'zoneSelectorButton' onClick='addThisZone(" + '"' + rwzmAllZonesList[i].id + '"' + ");'>" + rwzmAllZonesList[i].name + "</button><br>\n";
    }
    uiContent = uiContent + '        </div><div style="width: 98%; text-align: right;"><button class = "greyBtn" onclick="cancelZoneSelector();">Cancel</button></div>\n';
    uiContent = uiContent + '    </div>\n';
    //Zone selector Replace
    uiContent = uiContent + '    <div id="rwzmReplaceZoneSelector">\n';
    uiContent = uiContent + '    <h2>Select the replacement zone:</h2><div class="zoneSelectorContainer">\n';
    for (i = 0; i < rwzmAllZonesList.length; i++ ) {
        uiContent = uiContent + "        <button class = 'zoneSelectorButton' onClick='replaceByThisZone(" + '"' + rwzmAllZonesList[i].id + '"' + ");'>" + rwzmAllZonesList[i].name + "</button><br>\n";
    }
    uiContent = uiContent + '        </div><div style="width: 98%; text-align: right;"><button class = "greyBtn" onclick="cancelZoneSelector();">Cancel</button></div>\n';
    uiContent = uiContent + '    </div>\n';
    
    Script.setTimeout(function () {
        rwzmOverlayWebWindow.emitScriptEvent(uiContent);
    }, 300);
}

function rwzmGetZoneName(id) {
    let k = 0;
    let name = "";
    for (k = 0; k < rwzmAllZonesList.length; k++) {
        if (rwzmAllZonesList[k].id === id) {
            name = rwzmAllZonesList[k].name;
            break;
        }
    }
    return name;
}

function rwzmGenerateUnidentifiedZoneName(id) {
    let partialID = id.substr(1,8);
    return "ZONE NOT FOUND (" + partialID +")";
}

function rwzmGetExistingZoneList() {
    var center = { "x": 0, "y": 0, "z": 0 };
    var existingZoneIDs = Entities.findEntitiesByType("Zone", center, RWZ_ZONE_SCAN_RADIUS);
    var listExistingZones = [];
    var thisZone = {};
    var properties;
    for (var k = 0; k < existingZoneIDs.length; k++) {
        properties = Entities.getEntityProperties(existingZoneIDs[k], ["name"]);
        thisZone = {
            "id": existingZoneIDs[k],
            "name": properties.name
        };
        listExistingZones.push(thisZone);
    }
    listExistingZones.sort(rwzmZoneSortOrder);
    return listExistingZones;
}

function rwzmZoneSortOrder(a, b) {
    var nameA = a.name.toUpperCase();
    var nameB = b.name.toUpperCase();
    if (nameA > nameB) {
        return 1;
    } else if (nameA < nameB) {
        return -1;
    }
    if (a.name > b.name) {
        return 1;
    } else if (a.name < b.name) {
        return -1;
    }
    return 0;
}

function rwzmRemoveZoneFromEntity(id, zoneID, forceLocked, highlightedID) {
    rwzmUndo = [];
    let properties = Entities.getEntityProperties(id, ["renderWithZones", "locked"]);
    
    let newRenderWithZones = [];
    let i = 0;
    for (i = 0; i < properties.renderWithZones.length; i++) {
        if (properties.renderWithZones[i] !== zoneID) {
            newRenderWithZones.push(properties.renderWithZones[i]);
        }
    }
    
    if (forceLocked && properties.locked) {
        Entities.editEntity(id, {"locked": false});
        Entities.editEntity(id, {"renderWithZones": newRenderWithZones, "locked": properties.locked});
    } else {
        Entities.editEntity(id, {"renderWithZones": newRenderWithZones});
    }
    rwzmUndo.push({"id": id, "renderWithZones": properties.renderWithZones});
    renderWithZonesManager(rwzmSelectedId, highlightedID);
}

function rwzmAddZonesToEntities(ids, zoneID, forceLocked, highlightedID) {
    rwzmUndo = [];
    let k = 0;
    let j = 0;
    let properties;
    let newRenderWithZones = [];
    for (k = 0; k < ids.length; k++) {
        properties = Entities.getEntityProperties(ids[k], ["renderWithZones", "locked"]);
        newRenderWithZones = [];
        
        for (j = 0; j < properties.renderWithZones.length; j++) {
            if (properties.renderWithZones[j] !== zoneID) {
                newRenderWithZones.push(properties.renderWithZones[j]);
            }
        }
        newRenderWithZones.push(zoneID);
        if (forceLocked && properties.locked) {
            Entities.editEntity(ids[k], {"locked": false});
            Entities.editEntity(ids[k], {"renderWithZones": newRenderWithZones, "locked": properties.locked});
        } else {
            Entities.editEntity(ids[k], {"renderWithZones": newRenderWithZones});
        }
        rwzmUndo.push({"id": ids[k], "renderWithZones": properties.renderWithZones});
    }
    renderWithZonesManager(rwzmSelectedId, highlightedID);
}

function rwzmRemoveZoneFromAllEntities(zoneID, forceLocked, highlightedID) {
    rwzmUndo = [];
    let k = 0;
    let j = 0;
    let properties;
    let newRenderWithZones = [];
    for (k = 0; k < rwzmSelectedId.length; k++) {
        properties = Entities.getEntityProperties(rwzmSelectedId[k], ["renderWithZones", "locked"]);
        newRenderWithZones = [];
        
        for (j = 0; j < properties.renderWithZones.length; j++) {
            if (properties.renderWithZones[j] !== zoneID) {
                newRenderWithZones.push(properties.renderWithZones[j]);
            }
        }
        if (forceLocked && properties.locked) {
            Entities.editEntity(rwzmSelectedId[k], {"locked": false});
            Entities.editEntity(rwzmSelectedId[k], {"renderWithZones": newRenderWithZones, "locked": properties.locked});
        } else {
            Entities.editEntity(rwzmSelectedId[k], {"renderWithZones": newRenderWithZones});
        }
        rwzmUndo.push({"id": rwzmSelectedId[k], "renderWithZones": properties.renderWithZones});
    }
    renderWithZonesManager(rwzmSelectedId, highlightedID);
}

function rwzmReplaceZoneOnAllEntities(targetZoneID, replacementZoneID, forceLocked, highlightedID) {
    rwzmUndo = [];
    let k = 0;
    let j = 0;
    let properties;
    let newRenderWithZones = [];
    for (k = 0; k < rwzmSelectedId.length; k++) {
        properties = Entities.getEntityProperties(rwzmSelectedId[k], ["renderWithZones", "locked"]);
        newRenderWithZones = [];
        
        for (j = 0; j < properties.renderWithZones.length; j++) {
            if (properties.renderWithZones[j] !== targetZoneID) {
                newRenderWithZones.push(properties.renderWithZones[j]);
            } else {
                newRenderWithZones.push(replacementZoneID);
            }
        }
        if (forceLocked && properties.locked) {
            Entities.editEntity(rwzmSelectedId[k], {"locked": false});
            Entities.editEntity(rwzmSelectedId[k], {"renderWithZones": newRenderWithZones, "locked": properties.locked});
        } else {
            Entities.editEntity(rwzmSelectedId[k], {"renderWithZones": newRenderWithZones});
        }
        rwzmUndo.push({"id": rwzmSelectedId[k], "renderWithZones": properties.renderWithZones});
    }
    renderWithZonesManager(rwzmSelectedId, highlightedID);
}

function rwzmGetTruncatedString(str, max) {
    if (str.length > max) {
        return str.substr(0, max-1) + "&#8230;";
    } else {
        return str;
    }
}

function rwzmUndoLastAction(highlightedID) {
    let k = 0;
    let properties;
    let locked;
    for (k = 0; k < rwzmUndo.length; k++) {
        locked = Entities.getEntityProperties(rwzmUndo[k].id, ["locked"]).locked;
        if (locked) {
            Entities.editEntity(rwzmUndo[k].id, {"locked": false});
            Entities.editEntity(rwzmUndo[k].id, {"renderWithZones": rwzmUndo[k].renderWithZones, "locked": locked});
        } else {
            Entities.editEntity(rwzmUndo[k].id, {"renderWithZones": rwzmUndo[k].renderWithZones});
        }
    }
    rwzmUndo = [];
    renderWithZonesManager(rwzmSelectedId, highlightedID);
}

function webEventReceiver (message) {
    try {
        var data = JSON.parse(message);
    } catch(e) {
        print("renderWithZonesManager.js: Error parsing JSON");
        return;
    }
    if (data.action === "highlight") {
        enforceLocked = data.enforceLocked;
        renderWithZonesManager(rwzmSelectedId, data.id);
    } else if (data.action === "removeZoneFromEntity") {
        enforceLocked = data.enforceLocked;
        rwzmRemoveZoneFromEntity(data.id, data.zoneID, data.enforceLocked, data.highlightedID);
    } else if (data.action === "addZoneToEntities") {
        enforceLocked = data.enforceLocked;
        rwzmAddZonesToEntities(data.ids, data.zoneID, data.enforceLocked, data.highlightedID);
    } else if (data.action === "refresh") {
        enforceLocked = data.enforceLocked;
        renderWithZonesManager(rwzmSelectedId, data.highlightedID);
    } else if (data.action === "removeZoneOnAllEntities") {
        enforceLocked = data.enforceLocked;
        rwzmRemoveZoneFromAllEntities(data.zoneID, data.enforceLocked, data.highlightedID);
    } else if (data.action === "replaceZoneOnAllEntities") {
        enforceLocked = data.enforceLocked;
        rwzmReplaceZoneOnAllEntities(data.targetZoneID, data.replacementZoneID, data.enforceLocked, data.highlightedID);
    } else if (data.action === "undo") {
        enforceLocked = data.enforceLocked;
        rwzmUndoLastAction(data.highlightedID);
    }
}

