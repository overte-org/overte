//
//  editModes.js
//
//  Created by Karol Suprynowicz on 2022.05.15.
//  Copyright 2022 Overte e.V.
//
//  Partially based on gridTool.js
//  Created by Ryan Huffman on 6 Nov 2014
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors.
//  Copyright 2022 Overte e.V.
//
//  This script implements a class for managing different edit modes
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

EditTools = function(options) {
    var that = {};
    
    var createAppMode = "object";
    var voxelEditMode = "single";
    var voxelSphereSize = 0.3;
    var voxelEditDynamics = "click";
    var voxelRemove = false;
    var voxelPointerMode = "laser";
    var voxelBrushLength = 0.5;
    var listeners = [];
    
    var createToolsWindow = options.createToolsWindow;
    
    that.emitUpdate = function() {
        var dataString = JSON.stringify({
                createAppMode : createAppMode,
                voxelEditMode : voxelEditMode,
                voxelSphereSize : voxelSphereSize,
                voxelEditDynamics : voxelEditDynamics,
                voxelRemove : voxelRemove,
                voxelPointerMode : voxelPointerMode,
                voxelBrushLength : voxelBrushLength,
            });
        webView.emitScriptEvent(dataString);
        createToolsWindow.emitScriptEvent(dataString);
    };
    
    that.getCreateAppMode = function() {
        return createAppMode;
    }
    
    that.setCreateAppMode = function(value) {
        createAppMode = value;
        that.emitUpdate();
    }
    
    that.getVoxelEditMode = function() {
        return voxelEditMode;
    }
    
    that.setVoxelEditMode = function(value) {
        voxelEditMode = value;
        that.emitUpdate();
    }
    
    that.getVoxelSphereSize = function() {
        return voxelSphereSize;
    }
    
    that.setVoxelSphereSize = function(value) {
        voxelSphereSize = value;
        that.emitUpdate();
    }
    
    that.getVoxelEditDynamics = function() {
        return voxelEditDynamics;
    }
    
    that.setVoxelEditDynamics = function(value) {
        voxelEditDynamics = value;
        that.emitUpdate();
    }
    
    that.getVoxelRemove = function() {
        return voxelRemove;
    }
    
    that.setVoxelRemove = function(value) {
        voxelRemove = value;
        that.emitUpdate();
    }
    
    that.getVoxelPointerMode = function() {
        return voxelPointerMode;
    }
    
    that.setVoxelPointerMode = function(value) {
        voxelPointerMode = value;
        that.emitUpdate();
    }
    
    that.getVoxelBrushLength = function() {
        return voxelBrushLength;
    }
    
    that.setVoxelBrushLength = function(value) {
        voxelBrushLength = value;
        that.emitUpdate();
    }
    
    that.update = function(data) {
        if (data.type !== "update-edit-tools") {
            return;
        }
        
        var needsUpdate = false;
        
        if (data.createAppMode) {
            createAppMode = data.createAppMode;
            needsUpdate = true;
        }
        if (data.voxelEditMode) {
            voxelEditMode = data.voxelEditMode;
            needsUpdate = true;
        }
         if (data.voxelSphereSize) {
            voxelSphereSize = data.voxelSphereSize;
            needsUpdate = true;
        }
         if (data.voxelEditDynamics) {
            voxelEditDynamics = data.voxelEditDynamics;
            needsUpdate = true;
        }
         if (data.voxelRemove) {
            voxelRemove = data.voxelRemove;
            needsUpdate = true;
        }
         if (data.voxelPointerMode) {
            voxelPointerMode = data.voxelPointerMode;
            needsUpdate = true;
        }
         if (data.voxelBrushLength) {
            voxelBrushLength = data.voxelBrushLength;
            needsUpdate = true;
        }
     }
    
    var webEventReceived = function(data) {
        try {
            data = JSON.parse(data);
        } catch (e) {
            return;
        }

        if (data.type === "init") {
            that.emitUpdate();
        } else if (data.type === "update-edit-tools") {
            that.update(data);
            for (var i = 0; i < listeners.length; i++) {
                listeners[i] && listeners[i](data);
            }
        }
    };
    
    var webView = null;
    webView = Tablet.getTablet("com.highfidelity.interface.tablet.system");

    webView.webEventReceived.connect(webEventReceived);
    createToolsWindow.webEventReceived.addListener(webEventReceived);

    that.addListener = function(callback) {
        listeners.push(callback);
    };
    
    function cleanup() {
    }
    
    return that;
}
