"use strict";
//
// audioMuteOverlay.js
//
// Created by Triplelexx on March 9th, 2017
// Reworked by Seth Alves on February 17th, 2019
// Copyright 2017 High Fidelity, Inc.
// Copyright 2023 Overte e.V.
//
// client script that creates an overlay to provide mute feedback
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

/* global Audio, Script, Entities, Quat, MyAvatar, HMD */

(function() { // BEGIN LOCAL_SCOPE

    var lastShortTermInputLoudness = 0.0;
    var lastLongTermInputLoudness = 0.0;
    var sampleRate = 8.0; // Hz

    var shortTermAttackTC =  Math.exp(-1.0 / (sampleRate * 0.500)); // 500 milliseconds attack
    var shortTermReleaseTC =  Math.exp(-1.0 / (sampleRate * 1.000)); // 1000 milliseconds release

    var longTermAttackTC =  Math.exp(-1.0 / (sampleRate * 5.0)); // 5 second attack
    var longTermReleaseTC =  Math.exp(-1.0 / (sampleRate * 10.0)); // 10 seconds release

    var activationThreshold = 0.05; // how much louder short-term needs to be than long-term to trigger warning

    var holdReset = 2.0 * sampleRate; // 2 seconds hold
    var holdCount = 0;
    var warningOverlayID = null;
    var pollInterval = null;
    var warningText = "Muted";

    function showWarning() {
        if (warningOverlayID) {
            return;
        }

        if (HMD.active) {
            warningOverlayID = Entities.addEntity({
                "type": "Text",
                "name": "Muted-Warning",
                "localPosition": { "x": 0.0, "y": -0.45, "z": -1.0 },
                "localRotation": Quat.fromVec3Degrees({ "x": 0.0, "y": 0.0, "z": 0.0, "w": 1.0 }),
                "text": warningText,
                "unlit": true,
                "textAlpha": 1.0,
                "textColor": { "red": 245, "green": 44, "blue": 74 },
                "backgroundAlpha": 0.0,
                "lineHeight": 0.042,
                "dimensions": {"x": 0.11, "y": 0.05, "z": 0.01 },
                "visible": true,
                "ignorePickIntersection": true,
                "renderLayer": "front",
                "grab": {
                    "grabbable": false
                },
                "parentID": MyAvatar.SELF_ID,
                "parentJointIndex": MyAvatar.getJointIndex("_CAMERA_MATRIX")
            }, "local");
        }
    }

    function hideWarning() {
        if (!warningOverlayID) {
            return;
        }
        Entities.deleteEntity(warningOverlayID);
        warningOverlayID = null;
    }

    function startPoll() {
        if (pollInterval) {
            return;
        }
        pollInterval = Script.setInterval(function() {
            var shortTermInputLoudness = Audio.inputLevel;
            var longTermInputLoudness = shortTermInputLoudness;

            var shortTc = (shortTermInputLoudness > lastShortTermInputLoudness) ? shortTermAttackTC : shortTermReleaseTC;
            var longTc = (longTermInputLoudness > lastLongTermInputLoudness) ? longTermAttackTC : longTermReleaseTC;

            shortTermInputLoudness += shortTc * (lastShortTermInputLoudness - shortTermInputLoudness);
            longTermInputLoudness += longTc * (lastLongTermInputLoudness - longTermInputLoudness);

            lastShortTermInputLoudness = shortTermInputLoudness;
            lastLongTermInputLoudness = longTermInputLoudness;

            if (shortTermInputLoudness > lastLongTermInputLoudness + activationThreshold) {
                holdCount = holdReset;
            } else {
                holdCount = Math.max(holdCount - 1, 0);
            }

            if (holdCount > 0) {
                showWarning();
            } else {
                hideWarning();
            }
        }, 1000.0 / sampleRate);
    }

    function stopPoll() {
        if (!pollInterval) {
            return;
        }
        Script.clearInterval(pollInterval);
        pollInterval = null;
        hideWarning();
    }

    function startOrStopPoll() {
        if (Audio.warnWhenMuted && Audio.muted) {
            startPoll();
        } else {
            stopPoll();
        }
    }

    function cleanup() {
        stopPoll();
    }
  
    Script.scriptEnding.connect(cleanup);

    startOrStopPoll();
    Audio.mutedChanged.connect(startOrStopPoll);
    Audio.warnWhenMutedChanged.connect(startOrStopPoll);

}()); // END LOCAL_SCOPE
