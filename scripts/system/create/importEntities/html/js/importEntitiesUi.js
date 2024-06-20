//  importEntitiesUi.js
//
//  Created by Alezia Kurdis on March 13th, 2024
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

let elJsonUrl;
let elBrowseBtn;
let elImportAtAvatar;
let elImportAtSpecificPosition;
let elImportAtSpecificPositionContainer;
let elPositionX;
let elPositionY;
let elPositionZ;
let elEntityHostTypeDomain;
let elEntityHostTypeAvatar;
let elMessageContainer;
let elImportBtn;
let elBackBtn;
let elTpTutorialBtn;
let elPastePositionBtn;

let lockUntil;

const LOCK_BTN_DELAY = 2000; //2 sec

function loaded() {
    lockUntil = 0;
    
    elJsonUrl = document.getElementById("jsonUrl");
    elBrowseBtn = document.getElementById("browseBtn");
    elImportAtAvatar = document.getElementById("importAtAvatar");
    elImportAtSpecificPosition = document.getElementById("importAtSpecificPosition");
    elImportAtSpecificPositionContainer = document.getElementById("importAtSpecificPositionContainer");
    elPositionX = document.getElementById("positionX");
    elPositionY = document.getElementById("positionY");
    elPositionZ = document.getElementById("positionZ");
    elEntityHostTypeDomain = document.getElementById("entityHostTypeDomain");
    elEntityHostTypeAvatar = document.getElementById("entityHostTypeAvatar");
    elMessageContainer = document.getElementById("messageContainer");
    elImportBtn = document.getElementById("importBtn");
    elBackBtn = document.getElementById("backBtn");
    elTpTutorialBtn = document.getElementById("tpTutorialBtn");
    elPastePositionBtn = document.getElementById("pastePositionBtn");
    
    elJsonUrl.oninput = function() {
        persistData();
    }
    
    elPositionX.oninput = function() {
        persistData();
    }
    
    elPositionY.oninput = function() {
        persistData();
    }
    
    elPositionZ.oninput = function() {
        persistData();
    }

    elEntityHostTypeDomain.onclick = function() {
        persistData();
    }
    
    elEntityHostTypeAvatar.onclick = function() {
        persistData();
    }
    
    elBrowseBtn.onclick = function() {
        const d = new Date();
        let time = d.getTime();
        if ((d.getTime() - lockUntil) > LOCK_BTN_DELAY) {
            EventBridge.emitWebEvent(JSON.stringify({ "type": "importUiBrowse" }));
            lockUntil = d.getTime() + LOCK_BTN_DELAY;
        }
    };
    
    elImportAtAvatar.onclick = function() {
        elImportAtSpecificPositionContainer.style.display = "None";
        persistData();
    };

    elImportAtSpecificPosition.onclick = function() {
        elImportAtSpecificPositionContainer.style.display = "Block";
        persistData();
    };
    
    elImportBtn.onclick = function() {
        const d = new Date();
        let time = d.getTime();
        if ((d.getTime() - lockUntil) > LOCK_BTN_DELAY) {
            importJsonToWorld();
            lockUntil = d.getTime() + LOCK_BTN_DELAY;
        }
    };
    
    elBackBtn.onclick = function() {
        const d = new Date();
        let time = d.getTime();
        if ((d.getTime() - lockUntil) > LOCK_BTN_DELAY) {
            EventBridge.emitWebEvent(JSON.stringify({ "type": "importUiGoBack" }));
            lockUntil = d.getTime() + LOCK_BTN_DELAY;
        }
    };
    
    elTpTutorialBtn.onclick = function() {
        const d = new Date();
        let time = d.getTime();
        if ((d.getTime() - lockUntil) > LOCK_BTN_DELAY) {
            EventBridge.emitWebEvent(JSON.stringify({ "type": "importUiGoTutorial" }));
            lockUntil = d.getTime() + LOCK_BTN_DELAY;
        }
    };
    
    elPastePositionBtn.onclick = function() {
        const d = new Date();
        let time = d.getTime();
        if ((d.getTime() - lockUntil) > LOCK_BTN_DELAY) {
            EventBridge.emitWebEvent(JSON.stringify({ "type": "importUiGetCopiedPosition" }));
            lockUntil = d.getTime() + LOCK_BTN_DELAY;
        }
    };
    
    EventBridge.emitWebEvent(JSON.stringify({ "type": "importUiGetPersistData" }));
}

function persistData() {
    let message = {
        "type": "importUiPersistData",
        "importUiPersistedData": {
            "elJsonUrl": elJsonUrl.value,
            "elImportAtAvatar": elImportAtAvatar.checked,
            "elImportAtSpecificPosition": elImportAtSpecificPosition.checked,
            "elPositionX": elPositionX.value,
            "elPositionY": elPositionY.value,
            "elPositionZ": elPositionZ.value,
            "elEntityHostTypeDomain": elEntityHostTypeDomain.checked,
            "elEntityHostTypeAvatar": elEntityHostTypeAvatar.checked
        }
    };
    EventBridge.emitWebEvent(JSON.stringify(message));
}

function loadDataInUi(importUiPersistedData) {
    elJsonUrl.value = importUiPersistedData.elJsonUrl;
    elImportAtAvatar.checked = importUiPersistedData.elImportAtAvatar;
    elImportAtSpecificPosition.checked = importUiPersistedData.elImportAtSpecificPosition;
    elPositionX.value = importUiPersistedData.elPositionX;
    elPositionY.value = importUiPersistedData.elPositionY;
    elPositionZ.value = importUiPersistedData.elPositionZ;
    elEntityHostTypeDomain.checked = importUiPersistedData.elEntityHostTypeDomain;
    elEntityHostTypeAvatar.checked = importUiPersistedData.elEntityHostTypeAvatar;
    if (elImportAtSpecificPosition.checked) {
        elImportAtSpecificPositionContainer.style.display = "Block";
    }
}

function importJsonToWorld() {
    elMessageContainer.innerHTML = "";

    if (elJsonUrl.value === "") {
        elMessageContainer.innerHTML = "<div style = 'padding: 10px; color: #000000; background-color: #ff7700;'>ERROR: 'URL/File (.json)' is required.</div>";
        return;
    }
    
    let positioningMode = getRadioValue("importAtPosition");
    let entityHostType = getRadioValue("entityHostType");

    if (positioningMode === "position" && (elPositionX.value === "" || elPositionY.value === "" || elPositionZ.value === "")) {
        elMessageContainer.innerHTML = "<div style = 'padding: 10px; color: #000000; background-color: #ff7700;'>ERROR: 'Position' is required.</div>";
        return;
    }
    let position = {"x": parseFloat(elPositionX.value), "y": parseFloat(elPositionY.value), "z": parseFloat(elPositionZ.value)};
    let message = {
        "type": "importUiImport",
        "jsonURL": elJsonUrl.value,
        "positioningMode": positioningMode,
        "position": position,
        "entityHostType": entityHostType
    };
    EventBridge.emitWebEvent(JSON.stringify(message));
}

function getRadioValue(objectName) {
    let radios = document.getElementsByName(objectName);
    let i; 
    let selectedValue = "";
    for (i = 0; i < radios.length; i++) {
        if (radios[i].checked) {
            selectedValue = radios[i].value;
            break;
        }
    }
    return selectedValue;
}

EventBridge.scriptEventReceived.connect(function(message){
    let messageObj = JSON.parse(message);
    if (messageObj.type === "importUi_IMPORT_CONFIRMATION") {
        elMessageContainer.innerHTML = "<div style = 'padding: 10px; color: #000000; background-color: #00ff00;'>IMPORT SUCCESSFUL.</div>";
    } else if (messageObj.type === "importUi_IMPORT_ERROR") {
        elMessageContainer.innerHTML = "<div style = 'padding: 10px; color: #FFFFFF; background-color: #ff0000;'>IMPORT ERROR: " + messageObj.reason + "</div>";
    } else if (messageObj.type === "importUi_SELECTED_FILE") {
        elJsonUrl.value = messageObj.file;
        persistData();
    } else if (messageObj.type === "importUi_POSITION_TO_PASTE") {
        elPositionX.value = messageObj.position.x;
        elPositionY.value = messageObj.position.y;
        elPositionZ.value = messageObj.position.z;
        persistData();
    } else if (messageObj.type === "importUi_LOAD_DATA") {
        loadDataInUi(messageObj.importUiPersistedData);
    }
});
