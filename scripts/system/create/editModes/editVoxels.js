//
//  editModes.js
//
//  Created by dr Karol Suprynowicz on 2022.05.17.
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
"use strict";

Script.include([
    "../../libraries/utils.js",
    "../entitySelectionTool/entitySelectionTool.js"
]);

const selectionManager = SelectionManager; // TODO: unused


const EditVoxels = function() {
    const self = this; // TODO: unused
    const that = {};

    const NO_HAND = -1;
    const controllerStandard = Controller.Standard;

    let controlHeld = false;
    let shiftHeld = false;
    let isLeftGripPressed = false;
    let isRightGripPressed = false;

    let editEnabled = false;
    let editSingleVoxels = false;
    let editSpheres = false;
    let editCubes = false;
    let editAdd = true; // Remove voxels if false
    let inverseOperation = false; // True when middle mouse button or grip is pressed
    let brushPointer = false;
    let isActive = true;
    
    let editSphereRadius = 0.15;
    const brushLength = 0.5; // TODO: Unused
    // Vector calculated from editSphereRadius for adding/remiving cubes
    let cubeSize = null;
    
    // Local plane for continuous voxel editing
    // 0 - plane parallel to YZ plane
    // 1 - plane parallel to XZ plane
    // 2 - plane parallel to YZ plane
    let editPlane = 0;
    // Is true when mouse button is pressed
    let isEditing = false;
    let editedVoxelEntity = null;
    // Position of last edit in voxel space
    let oldEditPosition = null;
    // True when original operation added voxels, false otherwise
    let lastEditValue = 255;
    let isOnUpdateConnected = false;
    let isSphereResizingStarted = true;
    let sphereResizingInitialHandDistance = 0.1;
    let sphereInitialRadius = editSphereRadius;
    let sphereEntityID = null;

    that.triggerClickMapping = Controller.newMapping(Script.resolvePath('') + '-click-voxels');
    that.triggerPressMapping = Controller.newMapping(Script.resolvePath('') + '-press-voxels');
    that.gripPressMapping = Controller.newMapping(Script.resolvePath('') + '-grip-voxels');
    that.triggeredHand = NO_HAND;
    that.pressedHand = NO_HAND;
    
    const soundAdd = SoundCache.getSound(Script.resourcesPath() + "sounds/Button05.wav");
    const soundDelete = SoundCache.getSound(Script.resourcesPath() + "sounds/Tab03.wav");
    
    // Continuous start timer prevents activating continuous mode on short button presses
    // and adding multiple voxels when only one was intended
    
    const continuousStartTimerMax = 0.200;
    let continuousStartTimer = 0.0;
    
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
            editAdd = true;
            if (data.voxelRemove) {
                editAdd = false;
            }
            if (data.voxelEditMode === "single") {
                editSpheres = false;
                editSingleVoxels = true;
                editCubes = false;
            } else if (data.voxelEditMode === "sphere") {
                editSpheres = true;
                editSingleVoxels = false;
                editCubes = false;
            } else if (data.voxelEditMode === "cube") {
                editSpheres = false;
                editSingleVoxels = false;
                editCubes = true;
            }
        }
        
        if (data.voxelSphereSize) {
            editSphereRadius = parseFloat(data.voxelSphereSize) / 2.0;
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
    
    function ceilVector(v) {
        return {
            x: Math.ceil(v.x),
            y: Math.ceil(v.y),
            z: Math.ceil(v.z)
        };
    }

    function attemptVoxelChangeForEntity(entityID, pickRayDir, intersectionLocation) {
        const wantDebug = false;
        if (wantDebug) {
            print("=============== eV::attemptVoxelChangeForEntity BEG =======================");
        }

        const properties = Entities.getEntityProperties(entityID);
        if (properties.type != "PolyVox") {
            return false;
        }
        
        if (!editEnabled || !isActive) {
            return false;
        }

        if (editSingleVoxels === false && editSpheres === false && editCubes === false) {
            return false;
        }

        const voxelOrigin = Entities.worldCoordsToVoxelCoords(entityID, Vec3.subtract(intersectionLocation, pickRayDir));
        const voxelPosition = Entities.worldCoordsToVoxelCoords(entityID, intersectionLocation);
        let pickRayDirInVoxelSpace = Vec3.subtract(voxelPosition, voxelOrigin);
        pickRayDirInVoxelSpace = Vec3.normalize(pickRayDirInVoxelSpace);
        
        const absX = Math.abs(pickRayDirInVoxelSpace.x);
        const absY = Math.abs(pickRayDirInVoxelSpace.y);
        const absZ = Math.abs(pickRayDirInVoxelSpace.z);
        if(absX >= absY && absX >= absZ){
            editPlane = 0;
        }else if(absY >= absX && absY >= absZ){
            editPlane = 1;
        }else if(absZ >= absX && absZ >= absY){
            editPlane = 2;
        }

        if (wantDebug) {
            print("voxelOrigin: " + JSON.stringify(voxelOrigin));
            print("voxelPosition: " + JSON.stringify(voxelPosition));
            print("pickRayDirInVoxelSpace: " + JSON.stringify(pickRayDirInVoxelSpace));
        }
        
        lastEditValue = 0;
        if((editAdd && !inverseOperation) || (!editAdd && inverseOperation)){
            lastEditValue = 255;
        }

        let toDrawPosition = null;

        if(lastEditValue === 255){
            toDrawPosition = Vec3.subtract(voxelPosition, Vec3.multiply(pickRayDirInVoxelSpace, 0.1));
        }else{
            toDrawPosition = Vec3.subtract(voxelPosition, Vec3.multiply(pickRayDirInVoxelSpace, -0.1));
        }

        if (editSingleVoxels) {
            if (wantDebug) {
                print("Calling setVoxel");
                print("entityID: " + JSON.stringify(entityID));
                print("floorVector(toDrawPosition): " + JSON.stringify(floorVector(toDrawPosition)));
            }
            oldEditPosition = floorVector(toDrawPosition);
            if (Entities.setVoxel(entityID, oldEditPosition, lastEditValue)){
                Audio.playSystemSound((lastEditValue === 255) ? soundAdd : soundDelete);
                return true;
            }else{
                return false;
            }
        }
        if (editSpheres) {
            if (wantDebug) {
                print("Calling setVoxelSphere");
                print("entityID: " + JSON.stringify(entityID));
                print("editSphereRadius: " + JSON.stringify(editSphereRadius));
                print("floorVector(toDrawPosition): " + JSON.stringify(floorVector(toDrawPosition)));
            }
            oldEditPosition = floorVector(toDrawPosition);
            const toDrawPositionWorld = Entities.voxelCoordsToWorldCoords(entityID, oldEditPosition);
            if (Entities.setVoxelSphere(entityID, toDrawPositionWorld, editSphereRadius, lastEditValue)){
                Audio.playSystemSound((lastEditValue === 255) ? soundAdd : soundDelete);
                return true;
            }else{
                return false;
            }
        }
        if (editCubes) {
            if (wantDebug) {
                print("Calling setVoxelsInCuboid");
                print("entityID: " + JSON.stringify(entityID));
                print("editSphereRadius: " + JSON.stringify(editSphereRadius));
                print("floorVector(toDrawPosition): " + JSON.stringify(floorVector(toDrawPosition)));
            }
            oldEditPosition = floorVector(toDrawPosition);
            const cubeSizeWorld = {x : editSphereRadius * 2, y : editSphereRadius * 2, z : editSphereRadius * 2};
            const zeroVecWorld = {x : 0, y: 0, z: 0};
            const zeroVecLocal = Entities.worldCoordsToVoxelCoords(entityID, zeroVecWorld);
            const cubeSizeVecLocal = Entities.worldCoordsToVoxelCoords(entityID, cubeSizeWorld);
            cubeSize = ceilVector(Vec3.subtract(cubeSizeVecLocal, zeroVecLocal));
            const lowPosition = Vec3.subtract(oldEditPosition, Vec3.multiply(cubeSize, 0.5));
            if (Entities.setVoxelsInCuboid(entityID, lowPosition, cubeSize, lastEditValue)){
                Audio.playSystemSound((lastEditValue === 255) ? soundAdd : soundDelete);
                return true;
            }else{
                return false;
            }
        }
    }

    function attemptVoxelChange(pickRayDir, intersection) {
        const wantDebug = false;
        if (wantDebug) {
            print("=============== eV::attemptVoxelChange BEG =======================");
        }

        const ids = Entities.findEntities(intersection.intersection, editSphereRadius + 1.0);
        if (ids.indexOf(intersection.entityID) < 0) {
            ids.push(intersection.entityID);
        }

        if (wantDebug) {
            print("Entities: " + JSON.stringify(ids));
        }

        let success = false;
        for (let i = 0; i < ids.length; i++) {
            const entityID = ids[i];
            success |= attemptVoxelChangeForEntity(entityID, pickRayDir, intersection.intersection)
        }
        return success;
    }
    
    function controllerComputePickRay() {
        const hand = triggered() ? that.triggeredHand : that.pressedHand;
        const controllerPose = getControllerWorldLocation(hand, true);
        if (controllerPose.valid) {
            const controllerPosition = controllerPose.translation;
            // This gets point direction right, but if you want general quaternion it would be more complicated:
            const controllerDirection = Quat.getUp(controllerPose.rotation);
            return {origin: controllerPosition, direction: controllerDirection};
        }
    }

    function generalComputePickRay(x, y) {
        return controllerComputePickRay() || Camera.computePickRay(x, y);
    }

    function mousePressEvent(event) {
        const wantDebug = false;
        const attemptChangeOnEmpty = false;
        if (!editEnabled || !isActive) {
            return false;
        }

        if (wantDebug) {
            print("=============== eV::mousePressEvent BEG =======================");
        }

        if (!(event.isLeftButton || event.isMiddleButton) && !triggered()) {
            return;
        }
        
        if (event.isLeftButton || event.isMiddleButton){
            if (event.isMiddleButton){
                inverseOperation = true;
            }else{
                inverseOperation = false;
            }
        }else{
            inverseOperation = false;
            if(that.triggeredHand === controllerStandard.RightHand && Controller.getValue(controllerStandard.RightGrip) > 0.5){
                inverseOperation = true;
            }
            if(that.triggeredHand === controllerStandard.LeftHand && Controller.getValue(controllerStandard.LeftGrip) > 0.5){
                inverseOperation = true;
            }
        }

        continuousStartTimer = 0;

        const pickRay = generalComputePickRay(event.x, event.y);
        const intersection = Entities.findRayIntersection(pickRay, true); // accurate picking
        const overlaysIntersection = Overlays.findRayIntersection(pickRay, true); // accurate picking

        if (wantDebug) {
            print("Pick ray: " + JSON.stringify(pickRay));
            print("Intersection: " + JSON.stringify(intersection));
        }

        if (intersection.intersects) {
            if (overlaysIntersection.intersects) {
                const overlaysIntersectionDistance = Vec3.distance(overlaysIntersection.intersection, pickRay.origin);
                const intersectionDistance = Vec3.distance(intersection.intersection, pickRay.origin);
                if (wantDebug) {
                    print("overlaysIntersectionDistance: " + overlaysIntersectionDistance);
                    print("intersectionDistance: " + intersectionDistance);
                }
                if (overlaysIntersectionDistance < intersectionDistance) {
                    return;
                }
            }
            if (attemptVoxelChangeForEntity(intersection.entityID, pickRay.direction, intersection.intersection)) {
                Script.update.connect(onUpdateHandler);
                isOnUpdateConnected = true;
                isEditing = true;
                editedVoxelEntity = intersection.entityID;
                if (wantDebug) {
                    print("onUpdateHandler connected");
                }
                return;
            }
        }

        // if the PolyVox entity is empty, we can't pick against its "on" voxels.  try picking against its
        // bounding box, instead.
        if (attemptChangeOnEmpty) {
            intersection = Entities.findRayIntersection(pickRay, false); // bounding box picking
            if (intersection.intersects) {
                if(attemptVoxelChange(pickRay.direction, intersection)){
                    Script.update.connect(onUpdateHandler);
                    isOnUpdateConnected = true;
                    if (wantDebug) {
                        print("onUpdateHandler connected");
                    }
                }
            }
        }
    }

    function mouseReleaseEvent(event) {
        const wantDebug = false;

        if (wantDebug) {
            print("=============== eV::mouseReleaseEvent BEG =======================");
        }
        if(isOnUpdateConnected){
            Script.update.disconnect(onUpdateHandler);
            isOnUpdateConnected = false;
            isEditing = false;
            editedVoxelEntity = null;
        }
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
    
    function makeClickHandler(hand) {
        return function (clicked) {
            if (!editEnabled) {
                return;
            }
            // Don't allow both hands to trigger at the same time
            if (triggered() && hand !== that.triggeredHand) {
                return;
            }
            if (!triggered() && clicked) {
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
            if (value >= TRIGGER_ON_VALUE && !triggered()) {
                that.pressedHand = hand;
            } else {
                that.pressedHand = NO_HAND;
                if(isOnUpdateConnected){
                    Script.update.disconnect(onUpdateHandler);
                    isOnUpdateConnected = false;
                }
            }
        }
    }

    function getDistanceBetweenControllers(){
        const poseLeft = getControllerWorldLocation(controllerStandard.LeftHand, true);
        const poseRight = getControllerWorldLocation(controllerStandard.RightHand, true);
        return Vec3.distance(poseLeft.translation, poseRight.translation);
    }
    function getEditSpherePosition( radius ){
        const poseLeft = getControllerWorldLocation(controllerStandard.LeftHand, true);
        const poseRight = getControllerWorldLocation(controllerStandard.RightHand, true);
        const handsPosition = Vec3.multiply(Vec3.sum(poseLeft.translation, poseRight.translation), 0.5);
        return Vec3.sum(handsPosition, Vec3.multiplyQbyV(MyAvatar.orientation, { x: 0, y: 0, z: radius * -2.0 }));
    }

    function updateSphereResizing(delta) {
        const wantDebug = false;
        const newDistance = getDistanceBetweenControllers();
        const newRadius = (sphereInitialRadius / sphereResizingInitialHandDistance) * newDistance;
        const newPosition = getEditSpherePosition(newRadius);
        const newDimensions = Vec3.multiply({ x: 1.0, y: 1.0, z: 1.0 }, newRadius * 2.0);
        if (wantDebug) {
            print("newDistance: " + JSON.stringify(newDistance));
            print("newRadius: " + JSON.stringify(newRadius));
            print("newPosition: " + JSON.stringify(newPosition));
            print("newDimensions: " + JSON.stringify(newDimensions));
        }
        Entities.editEntity(sphereEntityID, {
            position: newPosition,
            dimensions: newDimensions
        });
        if( that.editTools ) {
            editTools.setVoxelSphereSize(newRadius * 2);
        }
        editSphereRadius = newRadius;
    }

    function startSphereResizing() {
        const wantDebug = false;
        if (wantDebug) {
            print("=============== eV::startSphereResizing BEG =======================");
        }
        Script.update.connect(updateSphereResizing);
        sphereResizingInitialHandDistance = getDistanceBetweenControllers();
        sphereInitialRadius = editSphereRadius;
        const spherePosition = getEditSpherePosition(sphereInitialRadius);
        const sphereDimensions = Vec3.multiply({ x: 1.0, y: 1.0, z: 1.0 }, sphereInitialRadius * 2.0);
        sphereEntityID = Entities.addEntity({
            type: "Shape",
            shape: "Sphere",
            name: "voxelEditSphere",
            position: spherePosition,
            color: { r: 60, g: 100, b: 60 },
            alpha: 0.5,
            dimensions: sphereDimensions,
            collisionless: true,
            ignorePickIntersection: true,
            grab: { grabbable: false },
        }, "domain");
    }

    function stopSphereResizing() {
        const wantDebug = false;
        if (wantDebug) {
            print("=============== eV::stopSphereResizing BEG =======================");
        }
        Script.update.disconnect(updateSphereResizing);
        if (sphereEntityID !== null) {
            Entities.deleteEntity(sphereEntityID);
        }
        sphereEntityID = null;
    }

    function makeGripPressHandler(hand) {
        return function (value) {
            if (!editEnabled) {
                return;
            }
            if (value > 0.5) {
                if (hand === controllerStandard.LeftHand) {
                    isLeftGripPressed = true;
                } else if (hand === controllerStandard.RightHand) {
                    isRightGripPressed = true;
                }
            } else if (value < 0.4){
                if (hand === controllerStandard.LeftHand) {
                    isLeftGripPressed = false;
                } else if (hand === controllerStandard.RightHand) {
                    isRightGripPressed = false;
                }
            }
            if ( isLeftGripPressed && isRightGripPressed) {
                if( !isSphereResizingStarted ) {
                    isSphereResizingStarted = true;
                    startSphereResizing();
                }
            } else {
                if( isSphereResizingStarted ) {
                    isSphereResizingStarted = false;
                    stopSphereResizing();
                }
            }
        }
    }
    
    function onUpdateHandler(delta){
        const wantDebug = false;

        if (isEditing === false || editedVoxelEntity === null){
            return;
        }

        continuousStartTimer += delta;
        
        if (continuousStartTimer < continuousStartTimerMax) {
            return;
        }

        // Get pick ray origin and direction

        const pickRay = null;
        const hand = triggered() ? that.triggeredHand : that.pressedHand;
        
        if (hand === NO_HAND) {
            pickRay = Camera.computePickRay(Controller.getValue(Controller.Hardware.Keyboard.MouseX), Controller.getValue(Controller.Hardware.Keyboard.MouseY));
        }else{
            pickRay = controllerComputePickRay();
        }

        if (pickRay === null) {
            return;
        }


        // Compute intersection of pick ray with given plane in local coordinates

        const globalOriginInVoxelSpace = Entities.worldCoordsToVoxelCoords(editedVoxelEntity, { x: 0, y: 0, z: 0 });
        let pickRayDirInVoxelSpace = Vec3.subtract(Entities.worldCoordsToVoxelCoords(editedVoxelEntity, pickRay.direction), globalOriginInVoxelSpace);
        const voxelPickRayOrigin = Entities.worldCoordsToVoxelCoords(editedVoxelEntity, pickRay.origin);
        pickRayDirInVoxelSpace = Vec3.normalize(pickRayDirInVoxelSpace);
        let directionMultiplier = 1.0;
        const offsetVector = { x: 0, y: 0, z: 0 };
        switch (editPlane) {
            // 0 - plane parallel to YZ plane
            case 0:
                offsetVector.x = 0.5;
                offsetVector.y = (offsetVector.x / pickRayDirInVoxelSpace.x) * pickRayDirInVoxelSpace.y;
                offsetVector.z = (offsetVector.x / pickRayDirInVoxelSpace.x) * pickRayDirInVoxelSpace.z;
                directionMultiplier = (oldEditPosition.x - voxelPickRayOrigin.x) / pickRayDirInVoxelSpace.x;
                break;
            // 1 - plane parallel to XZ plane
            case 1:
                offsetVector.y = 0.5;
                offsetVector.x = (offsetVector.y / pickRayDirInVoxelSpace.y) * pickRayDirInVoxelSpace.x;
                offsetVector.z = (offsetVector.y / pickRayDirInVoxelSpace.y) * pickRayDirInVoxelSpace.z;
                directionMultiplier = (oldEditPosition.y - voxelPickRayOrigin.y) / pickRayDirInVoxelSpace.y;
                break;
            // 2 - plane parallel to XY plane
            case 2:
                offsetVector.z = 0.5;
                offsetVector.x = (offsetVector.z / pickRayDirInVoxelSpace.z) * pickRayDirInVoxelSpace.x;
                offsetVector.y = (offsetVector.z / pickRayDirInVoxelSpace.z) * pickRayDirInVoxelSpace.y;
                directionMultiplier = (oldEditPosition.z - voxelPickRayOrigin.z) / pickRayDirInVoxelSpace.z;
                break;
            default:
                return;
        }
        intersectionPoint = Vec3.sum(Vec3.multiply(pickRayDirInVoxelSpace, directionMultiplier), voxelPickRayOrigin);
        newEditPosition = floorVector(Vec3.sum(intersectionPoint, offsetVector));

        if (newEditPosition === oldEditPosition) {
            return;
        }

        if (wantDebug) {
            print("Old edit position: " + JSON.stringify(oldEditPosition));
            print("New edit position: " + JSON.stringify(newEditPosition));
            print("directionMultiplier: " + JSON.stringify(directionMultiplier) + " pickRay.direction: " + JSON.stringify(pickRay.direction) + " pickRayDirInVoxelSpace: " + JSON.stringify(pickRayDirInVoxelSpace) + " voxelPickRayOrigin: " + JSON.stringify(voxelPickRayOrigin) + " editPlane: " + JSON.stringify(editPlane));
        }

        if (editSingleVoxels) {
            if (Entities.setVoxel(editedVoxelEntity, newEditPosition, lastEditValue)){
                oldEditPosition = newEditPosition;
                Audio.playSystemSound((lastEditValue === 255) ? soundAdd : soundDelete);
            }
        } else if (editSpheres) {
            const toDrawPositionWorld = Entities.voxelCoordsToWorldCoords(editedVoxelEntity, newEditPosition);
            if (Entities.setVoxelSphere(editedVoxelEntity, toDrawPositionWorld, editSphereRadius, lastEditValue)){
                oldEditPosition = newEditPosition;
                Audio.playSystemSound((lastEditValue === 255) ? soundAdd : soundDelete);
            }
        } else if (editCubes) {
            const lowPosition = Vec3.subtract(newEditPosition, Vec3.multiply(cubeSize, 0.5));
            if (Entities.setVoxelsInCuboid(editedVoxelEntity, lowPosition, cubeSize, lastEditValue)){
                oldEditPosition = newEditPosition;
                Audio.playSystemSound((lastEditValue === 255) ? soundAdd : soundDelete);
            }
        }

    }

    function cleanup() {
        Controller.mousePressEvent.disconnect(mousePressEvent);
        Controller.mouseReleaseEvent.disconnect(mouseReleaseEvent);
        Controller.keyPressEvent.disconnect(keyPressEvent);
        Controller.keyReleaseEvent.disconnect(keyReleaseEvent);
    }

    Controller.mousePressEvent.connect(mousePressEvent);
    Controller.mouseReleaseEvent.connect(mouseReleaseEvent);
    Controller.keyPressEvent.connect(keyPressEvent);
    Controller.keyReleaseEvent.connect(keyReleaseEvent);
    that.triggerClickMapping.from(controllerStandard.RTClick).peek().to(makeClickHandler(controllerStandard.RightHand));
    that.triggerClickMapping.from(controllerStandard.LTClick).peek().to(makeClickHandler(controllerStandard.LeftHand));
    that.triggerPressMapping.from(controllerStandard.RT).peek().to(makePressHandler(controllerStandard.RightHand));
    that.triggerPressMapping.from(controllerStandard.LT).peek().to(makePressHandler(controllerStandard.LeftHand));
    that.gripPressMapping.from(controllerStandard.LeftGrip).peek().to(makeGripPressHandler(controllerStandard.LeftHand));
    that.gripPressMapping.from(controllerStandard.RightGrip).peek().to(makeGripPressHandler(controllerStandard.RightHand));
    that.enableTriggerMapping = function() {
        that.triggerClickMapping.enable();
        that.triggerPressMapping.enable();
        that.gripPressMapping.enable();
    };
    that.disableTriggerMapping = function() {
        that.triggerClickMapping.disable();
        that.triggerPressMapping.disable();
        that.gripPressMapping.disable();
    };
    that.enableTriggerMapping();
    
    Script.scriptEnding.connect(cleanup);
    Script.scriptEnding.connect(that.disableTriggerMapping);

    return that;
}
