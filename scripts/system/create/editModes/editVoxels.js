//
//  editModes.js
//
//  Created by Karol Suprynowicz on 2022.05.17.
//  Copyright 2022 Overte e.V.
//
//  Based on voxels.js
//  Created by Seth Alves on 2015-08-25
//  Copyright 2015 High Fidelity, Inc.
//
//  Based on entitySelectionTool.js
//  Created by Brad hefta-Gaub on 10/1/14.
//    Modified by Daniela Fontes * @DanielaFifo and Tiago Andrade @TagoWill on 4/7/2017
//    Modified by David Back on 1/9/2018
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors
//
//  This script implements voxel edit mode
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

Script.include([
    "./libraries/utils.js",
]);

EditVoxels = function() {
    var self = this;
    var that = {};

    const NO_HAND = -1;

    var controlHeld = false;
    var shiftHeld = false;

    var editEnabled = false;
    var addingVoxels = false;
    var deletingVoxels = false;
    var addingSpheres = false;
    var deletingSpheres = false;
    var addingCubes = false;
    var deletingCubes = false;
    var continuousPaint = false;
    var brushPointer = false;
    var isActive = true;

    var editSphereRadius = 0.15;
    var brushLength = 0.5;

    that.triggerClickMapping = Controller.newMapping(Script.resolvePath('') + '-click-voxels');
    that.triggerPressMapping = Controller.newMapping(Script.resolvePath('') + '-press-voxels');
    that.triggeredHand = NO_HAND;
    that.pressedHand = NO_HAND;
    
    that.setActive = function(active) {
        isActive = (active === true);
    }
    
    that.updateEditSettings = function(data) {
        
        if (data.createAppMode) {
            if (data.createAppMode === "voxel"){
                editEnabled = true;
            } else {
                editEnabled = false;
            }
        }
        
        if (data.voxelEditMode) {
            addingVoxels = false;
            deletingVoxels = false;
            addingSpheres = false;
            deletingSpheres = false;
            addingCubes = false;
            deletingCubes = false;
            if (data.voxelRemove) {
                if (data.voxelEditMode === "single") {
                    deletingVoxels = true;
                } else if (data.voxelEditMode === "sphere") {
                    deletingSpheres = true;
                } else if (data.voxelEditMode === "cube") {
                    deletingCubes = true;
                }
            } else {
                if (data.voxelEditMode === "single") {
                    addingVoxels = true;
                } else if (data.voxelEditMode === "sphere") {
                    addingSpheres = true;
                } else if (data.voxelEditMode === "cube") {
                    addingCubes = true;
                }
            }
        }
        
        if (data.voxelSphereSize) {
            editSphereRadius = parseFloat(data.voxelSphereSize) / 2.0;
        }
        
        if (data.voxelEditDynamics) {
            if (data.voxelEditDynamics === "continuous"){
                continuousPaint = true;
            } else {
                continuousPaint = false;
            }
        }
        
        if (data.voxelPointerMode) {
            if (data.voxelPointerMode === "brush") {
                brushPointer = true;
            } else {
                brushPointer = false;
            }
        }
        
        if (data.voxelBrushLength) {
            voxelBrushLength = parseFloat(data.voxelBrushLength);
        }
        
    }

    function floorVector(v) {
        return {
            x: Math.floor(v.x),
            y: Math.floor(v.y),
            z: Math.floor(v.z)
        };
    }

    function attemptVoxelChangeForEntity(entityID, pickRayDir, intersectionLocation) {
        var wantDebug = false;
        if (wantDebug) {
            print("=============== eV::attemptVoxelChangeForEntity BEG =======================");
        }

        var properties = Entities.getEntityProperties(entityID);
        if (properties.type != "PolyVox") {
            return false;
        }
        
        if (!editEnabled || !isActive) {
            return false;
        }

        if (addingVoxels == false && deletingVoxels == false && addingSpheres == false && deletingSpheres == false) {
            return false;
        }

        var voxelOrigin = Entities.worldCoordsToVoxelCoords(entityID, Vec3.subtract(intersectionLocation, pickRayDir));
        var voxelPosition = Entities.worldCoordsToVoxelCoords(entityID, intersectionLocation);
        var pickRayDirInVoxelSpace = Vec3.subtract(voxelPosition, voxelOrigin);
        pickRayDirInVoxelSpace = Vec3.normalize(pickRayDirInVoxelSpace);

        if (wantDebug) {
            print("voxelOrigin: " + JSON.stringify(voxelOrigin));
            print("voxelPosition: " + JSON.stringify(voxelPosition));
            print("pickRayDirInVoxelSpace: " + JSON.stringify(pickRayDirInVoxelSpace));
        }

        var doAdd = addingVoxels;
        var doDelete = deletingVoxels;
        var doAddSphere = addingSpheres;
        var doDeleteSphere = deletingSpheres;

        if (controlHeld) {
            if (doAdd) {
                doAdd = false;
                doDelete = true;
            } else if (doDelete) {
                doDelete = false;
                doAdd = true;
            } else if (doAddSphere) {
                doAddSphere = false;
                doDeleteSphere = true;
            } else if (doDeleteSphere) {
                doDeleteSphere = false;
                doAddSphere = true;
            }
        }

        if (doDelete) {
            var toErasePosition = Vec3.sum(voxelPosition, Vec3.multiply(pickRayDirInVoxelSpace, 0.1));
            if (wantDebug) {
                print("Calling setVoxel to delete");
                print("entityID: " + JSON.stringify(entityID));
                print("floorVector(toErasePosition): " + JSON.stringify(floorVector(toErasePosition)));
            }
            return Entities.setVoxel(entityID, floorVector(toErasePosition), 0);
        }
        if (doAdd) {
            var toDrawPosition = Vec3.subtract(voxelPosition, Vec3.multiply(pickRayDirInVoxelSpace, 0.1));
            if (wantDebug) {
                print("Calling setVoxel to add");
                print("entityID: " + JSON.stringify(entityID));
                print("floorVector(toDrawPosition): " + JSON.stringify(floorVector(toDrawPosition)));
            }
            return Entities.setVoxel(entityID, floorVector(toDrawPosition), 255);
        }
        if (doDeleteSphere) {
            var toErasePosition = intersectionLocation;
            if (wantDebug) {
                print("Calling setVoxelSphere to delete");
                print("entityID: " + JSON.stringify(entityID));
                print("editSphereRadius: " + JSON.stringify(editSphereRadius));
                print("floorVector(toErasePosition): " + JSON.stringify(floorVector(toErasePosition)));
            }
            return Entities.setVoxelSphere(entityID, floorVector(toErasePosition), editSphereRadius, 0);
        }
        if (doAddSphere) {
            var toDrawPosition = intersectionLocation;
            if (wantDebug) {
                print("Calling setVoxelSphere to add");
                print("entityID: " + JSON.stringify(entityID));
                print("editSphereRadius: " + JSON.stringify(editSphereRadius));
                print("floorVector(toDrawPosition): " + JSON.stringify(floorVector(toDrawPosition)));
            }
            return Entities.setVoxelSphere(entityID, floorVector(toDrawPosition), editSphereRadius, 255);
        }
    }

    function attemptVoxelChange(pickRayDir, intersection) {
        var wantDebug = false;
        if (wantDebug) {
            print("=============== eV::attemptVoxelChange BEG =======================");
        }

        var ids;

        ids = Entities.findEntities(intersection.intersection, editSphereRadius + 1.0);
        if (ids.indexOf(intersection.entityID) < 0) {
            ids.push(intersection.entityID);
        }

        if (wantDebug) {
            print("Entities: " + JSON.stringify(ids));
        }

        var success = false;
        for (var i = 0; i < ids.length; i++) {
            var entityID = ids[i];
            success |= attemptVoxelChangeForEntity(entityID, pickRayDir, intersection.intersection)
        }
        return success;
    }

    function controllerComputePickRay() {
        var hand = triggered() ? that.triggeredHand : that.pressedHand;
        var controllerPose = getControllerWorldLocation(hand, true);
        if (controllerPose.valid) {
            var controllerPosition = controllerPose.translation;
            // This gets point direction right, but if you want general quaternion it would be more complicated:
            var controllerDirection = Quat.getUp(controllerPose.rotation);
            return {origin: controllerPosition, direction: controllerDirection};
        }
    }

    function generalComputePickRay(x, y) {
        return controllerComputePickRay() || Camera.computePickRay(x, y);
    }

    function mousePressEvent(event) {
        var wantDebug = false;
        if (!editEnabled || !isActive) {
            return false;
        }

        if (wantDebug) {
            print("=============== eV::mousePressEvent BEG =======================");
        }

        if (!event.isLeftButton && !triggered()) {
            return;
        }

        var pickRay = generalComputePickRay(event.x, event.y);
        var intersection = Entities.findRayIntersection(pickRay, true); // accurate picking

        if (wantDebug) {
            print("Pick ray: " + JSON.stringify(pickRay));
            print("Intersection: " + JSON.stringify(intersection));
        }

        if (intersection.intersects) {
            if (attemptVoxelChange(pickRay.direction, intersection)) {
                return;
            }
        }

        // if the PolyVox entity is empty, we can't pick against its "on" voxels.  try picking against its
        // bounding box, instead.
        intersection = Entities.findRayIntersection(pickRay, false); // bounding box picking
        if (intersection.intersects) {
            attemptVoxelChange(pickRay.direction, intersection);
        }
    }

    function mouseReleaseEvent(event) {
        return;
    }

    function keyPressEvent(event) {
        if (event.text == "CONTROL") {
            controlHeld = true;
        }
        if (event.text == "SHIFT") {
            shiftHeld = true;
        }
    }

    function keyReleaseEvent(event) {
        if (event.text == "CONTROL") {
            controlHeld = false;
        }
        if (event.text == "SHIFT") {
            shiftHeld = false;
        }
    }

    function triggered() {
        return that.triggeredHand !== NO_HAND;
    };
    
    function pointingAtDesktopWindowOrTablet(hand) {
        var pointingAtDesktopWindow = (hand === Controller.Standard.RightHand && 
                                       SelectionManager.pointingAtDesktopWindowRight) ||
                                      (hand === Controller.Standard.LeftHand && 
                                       SelectionManager.pointingAtDesktopWindowLeft);
        var pointingAtTablet = (hand === Controller.Standard.RightHand && SelectionManager.pointingAtTabletRight) ||
                               (hand === Controller.Standard.LeftHand && SelectionManager.pointingAtTabletLeft);
        return pointingAtDesktopWindow || pointingAtTablet;
    }

    function makeClickHandler(hand) {
        return function (clicked) {
            if (!editEnabled) {
                return;
            }
            // Don't allow both hands to trigger at the same time
            if (triggered() && hand !== that.triggeredHand) {
                return;
            }
            if (!triggered() && clicked && !pointingAtDesktopWindowOrTablet(hand)) {
                that.triggeredHand = hand;
                mousePressEvent({});
            } else if (triggered() && !clicked) {
                that.triggeredHand = NO_HAND;
                mouseReleaseEvent({});
            }
        };
    }

    function makePressHandler(hand) {
        return function (value) {
            if (!editEnabled) {
                return;
            }
            if (value >= TRIGGER_ON_VALUE && !triggered() && !pointingAtDesktopWindowOrTablet(hand)) {
                that.pressedHand = hand;
            } else {
                that.pressedHand = NO_HAND;
            }
        }
    }

    function cleanup() {
        Controller.mousePressEvent.disconnect(self.mousePressEvent);
        Controller.mouseReleaseEvent.disconnect(self.mouseReleaseEvent);
        Controller.keyPressEvent.disconnect(self.keyPressEvent);
        Controller.keyReleaseEvent.disconnect(self.keyReleaseEvent);
    }

    Controller.mousePressEvent.connect(mousePressEvent);
    Controller.mouseReleaseEvent.connect(mouseReleaseEvent);
    Controller.keyPressEvent.connect(keyPressEvent);
    Controller.keyReleaseEvent.connect(keyReleaseEvent);
    that.triggerClickMapping.from(Controller.Standard.RTClick).peek().to(makeClickHandler(Controller.Standard.RightHand));
    that.triggerClickMapping.from(Controller.Standard.LTClick).peek().to(makeClickHandler(Controller.Standard.LeftHand));
    that.triggerPressMapping.from(Controller.Standard.RT).peek().to(makePressHandler(Controller.Standard.RightHand));
    that.triggerPressMapping.from(Controller.Standard.LT).peek().to(makePressHandler(Controller.Standard.LeftHand));
    that.enableTriggerMapping = function() {
        that.triggerClickMapping.enable();
        that.triggerPressMapping.enable();
    };
    that.disableTriggerMapping = function() {
        that.triggerClickMapping.disable();
        that.triggerPressMapping.disable();
    };
    that.enableTriggerMapping();
    Script.scriptEnding.connect(cleanup);
    Script.scriptEnding.connect(that.disableTriggerMapping);

    return that;
}
