//
//  audioFeedback.js
//
//  Created by Alezia Kurdis on September 30, 2020.
//  Copyright 2020 Vircadia contributors.
//
//  This script add audio feedback (confirmation and rejection) for user interactions that require one.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
"use strict";

audioFeedback = (function() {
    const that = {};

    const confirmationSound = SoundCache.getSound(Script.resolvePath("./sounds/confirmation.mp3"));
    const rejectionSound = SoundCache.getSound(Script.resolvePath("./sounds/rejection.mp3"));
    const actionSound = SoundCache.getSound(Script.resolvePath("./sounds/action.mp3"));
    
    that.confirmation = function() { //Play a confirmation sound
        Audio.playSound(confirmationSound, {
            "volume": 0.3,
            "localOnly": true
        });
    }

    that.rejection = function() { //Play a rejection sound
        Audio.playSound(rejectionSound, {
            "volume": 0.3,
            "localOnly": true
        });
    }

    that.action = function() { //Play an action sound
        Audio.playSound(actionSound, {
            "volume": 0.3,
            "localOnly": true
        });
    }

    return that;
})();
