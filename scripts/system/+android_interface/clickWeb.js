"use strict";
//
//  clickWeb.js
//  scripts/system/+android
//
//  Created by Gabriel Calero & Cristian Duarte on Jun 22, 2018
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

(function() { // BEGIN LOCAL_SCOPE

var logEnabled = false;
var touchEntityID;

function printd(str) {
    if (logEnabled)
        print("[clickWeb.js] " + str);
}

function intersectsWebEntity(intersection) {
    if (intersection && intersection.intersects && intersection.entityID) {
        var properties = Entities.getEntityProperties(intersection.entityID, ["type", "sourceUrl"]);
        return properties.type && properties.type == "Web" && properties.sourceUrl;
    }
    return false;
}

function findRayIntersection(pickRay) {
    // Check all entities. Argument is an object with origin and direction.
    var entityRayIntersection = Entities.findRayIntersection(pickRay, Picks.PICK_DOMAIN_ENTITIES | Picks.PICK_AVATAR_ENTITIES | Picks.PICK_LOCAL_ENTITIES);
    var isEntityInters = intersectsWebEntity(entityRayIntersection);

    if (isEntityInters) {
        return { type: 'entity', obj: entityRayIntersection };
    }
    return false;
}

function touchBegin(event) {
    var intersection = findRayIntersection(Camera.computePickRay(event.x, event.y));
    if (intersection) {
        touchEntityID = intersection.obj.entityID;
    }
}

function touchEnd(event) {
    var intersection = findRayIntersection(Camera.computePickRay(event.x, event.y));
    if (intersection && touchEntityID == intersection.obj.entityID) {
        var properties = Entities.getEntityProperties(touchEntityID, ["sourceUrl"]);
        if (properties.sourceUrl && !properties.sourceUrl.match(/\.qml$/)) {
            Window.openUrl(properties.sourceUrl);
        }
    }

    touchEntityID = null;
}

function ending() {
    Controller.touchBeginEvent.disconnect(touchBegin);
    Controller.touchEndEvent.disconnect(touchEnd);
}

function init() {
    Controller.touchBeginEvent.connect(touchBegin);
    Controller.touchEndEvent.connect(touchEnd);

    Script.scriptEnding.connect(function () {
        ending();
    });

}

module.exports = {
    init: init,
    ending: ending
}

init();

}()); // END LOCAL_SCOPE
