'use strict';
//
//  activator-doppleganger.js
//
//  Created by Alezia Kurdis on February 20th, 2022.
//  Copyright 2022 Overte e.V.
//
//  This script is display a doppleganger of the user by entering an entity.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

(function() {
    var isActive = false;
    var thisEntityID;
    var versioncall = Math.floor(Math.random()*50000);
    var DopplegangerClass = Script.require('https://more.overte.org/tutorial/doppleganger.js?version=' + versioncall);

    var doppleganger = new DopplegangerClass({
        avatar: MyAvatar,
        mirrored: false,
        autoUpdate: true
    });

    this.preload = function(entityID) {
       thisEntityID = entityID;
    }

    function onDomainChanged() {
        if (doppleganger.active) {
            doppleganger.stop('domain_changed');
        }
    }

    Window.domainChanged.connect(onDomainChanged);

    Window.domainConnectionRefused.connect(onDomainChanged);

    Script.scriptEnding.connect(function() {
        if (isActive) {
            doppleganger.stop();
            isActive = false;
        }
        Window.domainChanged.disconnect(onDomainChanged);
        Window.domainConnectionRefused.disconnect(onDomainChanged);
        
    });

    this.enterEntity = function(entityID) {
        print("ENTERING");
        startDopplegangerShow(entityID);
        isActive = true;
    }

    this.leaveEntity = function(entityID) {
        print("LEAVING");
        doppleganger.stop();
        isActive = false;
    }    

    function startDopplegangerShow(entityID) {
        var properties = Entities.getEntityProperties(entityID, ["position", "rotation"]);
        var avatarPosition = MyAvatar.position;
        var drawPosition = {
            "x": properties.position.x,
            "y": avatarPosition.y,
            "z": properties.position.z
        };
        var param = {
            "position": drawPosition,
            "orientation": properties.rotation,
            "autoUpdate": true
        };
        doppleganger.start(param);
    }


    MyAvatar.skeletonModelURLChanged.connect(function () {
        if (isActive) {
            print("CHANGED WHILE ACTIVE");
            doppleganger.stop();
            isActive = false;
            var timer = Script.setTimeout(function () {
                startDopplegangerShow(thisEntityID);
                isActive = true;
            }, 4000);
            
        }
    });

    // alert the user if there was an error applying their skeletonModelURL
    doppleganger.addingEntity.connect(function(error, result) {
        if (doppleganger.active && error) {
            Window.alert('doppleganger | ' + error + '\n' + doppleganger.skeletonModelURL);
        }
    });

})
