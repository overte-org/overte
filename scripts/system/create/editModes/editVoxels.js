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
//  This script implements voxel edit mode
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

Script.include([
    "./libraries/utils.js",
]);

EditVoxels = function() {
    var that = {};

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

    var editSphereRadius = 0.15;
    var brushLength = 0.5;
    
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

        var properties = Entities.getEntityProperties(entityID);
        if (properties.type != "PolyVox") {
            return false;
        }
        
        if (!editEnabled) {
            return false;
        }

        if (addingVoxels == false && deletingVoxels == false && addingSpheres == false && deletingSpheres == false) {
            return false;
        }

        var voxelOrigin = Entities.worldCoordsToVoxelCoords(entityID, Vec3.subtract(intersectionLocation, pickRayDir));
        var voxelPosition = Entities.worldCoordsToVoxelCoords(entityID, intersectionLocation);
        var pickRayDirInVoxelSpace = Vec3.subtract(voxelPosition, voxelOrigin);
        pickRayDirInVoxelSpace = Vec3.normalize(pickRayDirInVoxelSpace);

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
            return Entities.setVoxel(entityID, floorVector(toErasePosition), 0);
        }
        if (doAdd) {
            var toDrawPosition = Vec3.subtract(voxelPosition, Vec3.multiply(pickRayDirInVoxelSpace, 0.1));
            return Entities.setVoxel(entityID, floorVector(toDrawPosition), 255);
        }
        if (doDeleteSphere) {
            var toErasePosition = intersectionLocation;
            return Entities.setVoxelSphere(entityID, floorVector(toErasePosition), editSphereRadius, 0);
        }
        if (doAddSphere) {
            var toDrawPosition = intersectionLocation;
            return Entities.setVoxelSphere(entityID, floorVector(toDrawPosition), editSphereRadius, 255);
        }
    }

    function attemptVoxelChange(pickRayDir, intersection) {

        var ids;

        ids = Entities.findEntities(intersection.intersection, editSphereRadius + 1.0);
        if (ids.indexOf(intersection.entityID) < 0) {
            ids.push(intersection.entityID);
        }

        var success = false;
        for (var i = 0; i < ids.length; i++) {
            var entityID = ids[i];
            success |= attemptVoxelChangeForEntity(entityID, pickRayDir, intersection.intersection)
        }
        return success;
    }

    function mousePressEvent(event) {
        if (!event.isLeftButton) {
            return;
        }

        var pickRay = Camera.computePickRay(event.x, event.y);
        var intersection = Entities.findRayIntersection(pickRay, true); // accurate picking

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

    function cleanup() {
        toolBar.cleanup();
    }

    Controller.mousePressEvent.connect(mousePressEvent);
    Controller.keyPressEvent.connect(keyPressEvent);
    Controller.keyReleaseEvent.connect(keyReleaseEvent);
    Script.scriptEnding.connect(cleanup);

    return that;
}
