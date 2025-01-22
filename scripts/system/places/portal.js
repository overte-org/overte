//
//  portal.js
//
//  Created by Alezia Kurdis, January 14th, 2025.
//  Copyright 2025, Overte e.V.
//
//  3D portal for Places app. portal spawner.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
(function(){

    var ROOT = Script.resolvePath('').split("portal.js")[0];
    var portalURL = "";
    var portalName = "";
    var TP_SOUND = SoundCache.getSound(ROOT + "sounds/teleportSound.mp3");
    
    this.preload = function(entityID) {

        var properties = Entities.getEntityProperties(entityID, ["userData", "dimensions"]);
        var userDataObj = JSON.parse(properties.userData);
        portalURL = userDataObj.url;
        portalName = userDataObj.name;
        var portalColor = getColorFromPlaceID(userDataObj.placeID);

        var textLocalPosition = {"x": 0.0, "y": (properties.dimensions.y / 2) * 1.2, "z": 0.0};
        var scale = textLocalPosition.y/1.2;
        var textID = Entities.addEntity({
            "type": "Text",
            "parentID": entityID,
            "localPosition": textLocalPosition,
            "dimensions": {
                "x": 1 * scale, 
                "y": 0.15 * scale, 
                "z": 0.01
            },
            "name": portalName,
            "text": portalName,
            "textColor": portalColor.light,
            "lineHeight": 0.10 * scale,
            "backgroundAlpha": 0.0,
            "unlit": true,
            "alignment": "center",
            "verticalAlignment": "center",
            "canCastShadow": false,
            "billboardMode": "yaw",
            "grab": {
                "grabbable": false
            }
        },"local");
        
        var fxID = Entities.addEntity({
            "type": "ParticleEffect",
            "parentID": entityID,
            "localPosition": {
                "x": 0.0,
                "y": 0.0,
                "z": 0.0
            },
            "name": "PORTAL_FX",
            "dimensions": {
                "x": 5.2 * scale,
                "y": 5.2 * scale,
                "z": 5.2 * scale
            },
            "grab": {
                "grabbable": false
            },
            "shapeType": "ellipsoid",
            "color": portalColor.light,
            "alpha": 0.1,
            "textures": ROOT + "icons/portalFX.png",
            "maxParticles": 600,
            "lifespan": 0.6,
            "emitRate": 1000,
            "emitSpeed": -1 * scale,
            "speedSpread": 0 * scale,
            "emitOrientation": {
                "x": 0,
                "y": 0,
                "z": 0,
                "w": 1
            },
            "emitDimensions": {
                "x": 1.28 * scale,
                "y": 2 * scale,
                "z": 1.28 * scale
            },
            "polarFinish": 3.1415927410125732,
            "emitAcceleration": {
                "x": 0,
                "y": 0,
                "z": 0
            },
            "particleRadius": 0.4000000059604645 * scale,
            "radiusSpread": 0.30000001192092896 * scale,
            "radiusStart": 1 * scale,
            "radiusFinish": 0 * scale,
            "colorStart": portalColor.saturated,
            "colorFinish": {
                "red": 255,
                "green": 255,
                "blue": 255
            },
            "alphaSpread": 0.019999999552965164,
            "alphaStart": 0,
            "alphaFinish": 0.20000000298023224,
            "emitterShouldTrail": true,
            "particleSpin": 1.5700000524520874,
            "spinSpread": 2.9700000286102295,
            "spinStart": 0,
            "spinFinish": 0
        },"local");

        var loopSoundID = Entities.addEntity({
            "type": "Sound",
            "parentID":  entityID,
            "localPosition": {"x": 0.0, "y": 0.0, "z": 0.0},
            "name": "PORTAL SOUND",
            "soundURL": ROOT + "sounds/portalSound.mp3",
            "volume": 0.15,
            "loop": true,
            "positional": true,
            "localOnly": true
        },"local");

    }

    this.enterEntity = function(entityID) {
        var injectorOptions = {
            "position": MyAvatar.position,
            "volume": 0.3,
            "loop": false,
            "localOnly": true
        };
        var injector = Audio.playSound(TP_SOUND, injectorOptions);
        
        var timer = Script.setTimeout(function () {
            Window.location = portalURL;
            Entities.deleteEntity(entityID);
        }, 1000);

    };

    function getColorFromPlaceID(placeID) {
        var idIntegerConstant = getStringScore(placeID);
        var hue = (idIntegerConstant%360)/360;
        var color = hslToRgb(hue, 1, 0.5);
        var colorLight = hslToRgb(hue, 1, 0.75);
        return {
            "saturated": {"red": color[0], "green": color[1], "blue": color[2]},
            "light": {"red": colorLight[0], "green": colorLight[1], "blue": colorLight[2]},
        };
    }

    function getStringScore(str) {
        var score = 0;
        for (var j = 0; j < str.length; j++){
            score += str.charCodeAt(j);
        }
        return score;
    }

    /*
     * Converts an HSL color value to RGB. Conversion formula
     * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
     * Assumes h, s, and l are contained in the set [0, 1] and
     * returns r, g, and b in the set [0, 255].
     *
     * @param   {number}  h       The hue
     * @param   {number}  s       The saturation
     * @param   {number}  l       The lightness
     * @return  {Array}           The RGB representation
     */
    function hslToRgb(h, s, l){
        var r, g, b;

        if(s == 0){
            r = g = b = l; // achromatic
        }else{
            var hue2rgb = function hue2rgb(p, q, t){
                if(t < 0) t += 1;
                if(t > 1) t -= 1;
                if(t < 1/6) return p + (q - p) * 6 * t;
                if(t < 1/2) return q;
                if(t < 2/3) return p + (q - p) * (2/3 - t) * 6;
                return p;
            }

            var q = l < 0.5 ? l * (1 + s) : l + s - l * s;
            var p = 2 * l - q;
            r = hue2rgb(p, q, h + 1/3);
            g = hue2rgb(p, q, h);
            b = hue2rgb(p, q, h - 1/3);
        }

        return [Math.round(r * 255), Math.round(g * 255), Math.round(b * 255)];
    }

})
