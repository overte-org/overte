"use strict";
//======================================
// Version 1.1
// Addaption to "Local" Entities (since Overlays get deprecated for "Model" type.)
// by Alezia Kurdis on February 2020
//======================================
// Version 1.0
//  doppleganger.js
//
//  Created by Timothy Dedischew on 04/21/2017.
//  Copyright 2017 High Fidelity, Inc.
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
//
/* global module */
// @module doppleganger
//
// This module contains the `Doppleganger` class implementation for creating an inspectable replica of
// an Avatar (as a model directly in front of and facing them).  Joint positions and rotations are copied
// over in an update thread, so that the model automatically mirrors the Avatar's joint movements.
// An Avatar can then for example walk around "themselves" and examine from the back, etc.
//
// This should be helpful for inspecting your own look and debugging avatars, etc.
//
// The doppleganger is created as an overlay so that others do not see it -- and this also allows for the
// highest possible update rate when keeping joint data in sync.
//
// NOTE: THIS IS A MODIFIED VERSION SPECIFICALLY FOR THE TUTORIAL.

module.exports = Doppleganger;

// @property {bool} - when set true, Script.update will be used instead of setInterval for syncing joint data
Doppleganger.USE_SCRIPT_UPDATE = false;

// @property {int} - the frame rate to target when using setInterval for joint updates
Doppleganger.TARGET_FPS = 60;

// @property {int} - the maximum time in seconds to wait for the model overlay to finish loading
Doppleganger.MAX_WAIT_SECS = 10;

// @function - derive mirrored joint names from a list of regular joint names
// @param {Array} - list of joint names to mirror
// @return {Array} - list of mirrored joint names (note: entries for non-mirrored joints will be `undefined`)

var Setarry; 

Doppleganger.getMirroredJointNames = function(jointNames) {
    return jointNames.map(function(name, i) {
        if (/Left/.test(name)) {
            return name.replace('Left', 'Right');
        }
        if (/Right/.test(name)) {
            return name.replace('Right', 'Left');
        }
        return undefined;
    });
};

// @class Doppleganger - Creates a new instance of a Doppleganger.
// @param {Avatar} [options.avatar=MyAvatar] - Avatar used to retrieve position and joint data.
// @param {bool}   [options.mirrored=true]   - Apply "symmetric mirroring" of Left/Right joints.
// @param {bool}   [options.autoUpdate=true] - Automatically sync joint data.
function Doppleganger(options) {
    options = options || {};
    this.avatar = options.avatar || MyAvatar;
    this.mirrored = 'mirrored' in options ? options.mirrored : false;
    this.autoUpdate = 'autoUpdate' in options ? options.autoUpdate : true;

    // @public
    this.active = false;   // whether doppleganger is currently being displayed/updated
    this.entityID = null; // current doppleganger's Entity id
    this.frame = 0;        // current joint update frame

    // @signal - emitted when .active state changes
    this.activeChanged = signal(function(active, reason) {});
    // @signal - emitted once model overlay is either loaded or errors out
    this.addingEntity = signal(function(error, result){});
    // @signal - emitted each time the model overlay's joint data has been synchronized
    this.jointsUpdated = signal(function(entityID){});
}

Doppleganger.prototype = {
    // @public @method - toggles doppleganger on/off
    toggle: function() {
        if (this.active) {
            log('toggling off');
            this.stop();
        }else{
            log('toggling on');
            this.start();
        }
        return this.active;
    },

    // @public @method - synchronize the joint data between Avatar / doppleganger
    update: function() {
        this.frame++;
        try {
            if (!this.entityID) {
                throw new Error('!this.entityID');
            }

            if (this.avatar.skeletonModelURL !== this.skeletonModelURL) {
                //return this.stop('avatar_changed');
            }

            var rotations = this.avatar.getJointRotations();
            var translations = this.avatar.getJointTranslations();
            var size = rotations.length;


            // note: this mismatch can happen when the avatar's model is actively changing
            if (size !== translations.length ||
                (this.jointStateCount && size !== this.jointStateCount)) {
                log('mismatched joint counts (avatar model likely changed)', size, translations.length, this.jointStateCount);
                this.stop('avatar_changed_joints');
                return;
            }
            this.jointStateCount = size;


            if (this.mirrored) {
                var mirroredIndexes = this.mirroredIndexes;
                var outRotations = new Array(size);
                var outTranslations = new Array(size);
                for (var i=0; i < size; i++) {
                    var index = mirroredIndexes[i];
                    if (index < 0 || index === false) {
                        index = i;
                    }
                    var rot = rotations[index];
                    var trans = translations[index];
                    trans.x *= -1;
                    rot.y *= -1;
                    rot.z *= -1;
                    outRotations[i] = rot;
                    outTranslations[i] = trans;
                }
                rotations = outRotations;
                translations = outTranslations;
            }
			
			
            Entities.editEntity(this.entityID, {
                jointRotations: rotations,
                jointTranslations: translations,
				jointRotationsSet: Setarry,
				jointTranslationsSet: Setarry
            });
			
            this.jointsUpdated(this.entityID);
        } catch (e) {
            //log('.update error: '+ e, index);
            this.stop('update_error');
        }
    },

    // @public @method - show the doppleganger (and start the update thread, if options.autoUpdate was specified).
    // @param {vec3} [options.position=(in front of avatar)] - starting position
    // @param {quat} [options.orientation=avatar.orientation] - starting orientation
    start: function(options) {
		
		
        options = options || {};
        if (this.entityID) {
            //log('start() called but entity model already exists', this.entityID);
            return;
        }
        var avatar = this.avatar;
        if (!avatar.jointNames.length) {
            return this.stop('joints_unavailable');
        }

        this.frame = 0;
        this.position = options.position || Vec3.sum(avatar.position, Quat.getForward(avatar.orientation));
        this.orientation = options.orientation || avatar.orientation;
        this.skeletonModelURL = avatar.skeletonModelURL;
        this.jointStateCount = 0;
        this.jointNames = avatar.jointNames;
        this.mirroredNames = Doppleganger.getMirroredJointNames(this.jointNames);
		//log(this.mirroredNames);
        this.mirroredIndexes = this.mirroredNames.map(function(name) {
            return name ? avatar.getJointIndex(name) : false;
        });
		//log(this.mirroredIndexes);
		var prop = {
			type: "Model",
			name: 'Doppelganger', //added
            visible: false, // normally false
            modelURL: this.skeletonModelURL, //was field: url
            position: this.position,
            rotation: this.orientation
        };
		
        this.entityID = Entities.addEntity(prop, "local");

        var allJoints = avatar.getJointRotations();
        var nbrJoints = allJoints.length;
		Setarry = Array(nbrJoints);
		for (var i=0; i < nbrJoints; i++) {
					Setarry[i] = true;
		}
		
        this.onAddingEntity = function(error, result) {

            if (error) {
                return this.stop(error);
            }
            log('ModelEntity is ready; # joints == ' + result.jointNames.length);
            Entities.editEntity(this.entityID, { visible: true });

            this.syncVerticalPosition('LeftFoot');

            if (this.autoUpdate) {
                this._createUpdateThread();
            }
        };
        this.addingEntity.connect(this, 'onAddingEntity');

        log('doppleganger created; entityID =', this.entityID);

        // trigger clean up (and stop updates) if the overlay gets deleted
        this.onDeletedEntity = function(uuid) {
            if (uuid === this.entityID) {
                log('onDeletedEntity', uuid);
                this.stop('entity_deleted');
            }
        };
        Entities.deletingEntity.connect(this, 'onDeletedEntity');

        if ('onLoadComplete' in avatar) {
            // stop the current doppleganger if Avatar loads a different model URL
            this.onLoadComplete = function() {
                if (avatar.skeletonModelURL !== this.skeletonModelURL) {
                    //this.stop('avatar_changed_load');
                }
            };
            avatar.onLoadComplete.connect(this, 'onLoadComplete');
        }

        this.activeChanged(this.active = true, 'start');
        this._waitForModel(ModelCache.prefetch(this.skeletonModelURL));
    },

    // @public @method - hide the doppleganger
    // @param {String} [reason=stop] - the reason stop was called
    stop: function(reason) {
        reason = reason || 'stop';
        if (this.onUpdate) {
            Script.update.disconnect(this, 'onUpdate');
            delete this.onUpdate;
        }
        if (this._interval) {
            Script.clearInterval(this._interval);
            this._interval = undefined;
        }
        if (this.onDeletedEntity) {
            Entities.deletingEntity.disconnect(this, 'onDeletedEntity');
            delete this.onDeletedEntity;
        }
        if (this.onLoadComplete) {
            this.avatar.onLoadComplete.disconnect(this, 'onLoadComplete');
            delete this.onLoadComplete;
        }
        if (this.onAddingEntity) {
            this.addingEntity.disconnect(this, 'onAddingEntity');
        }
        if (this.entityID) {
            Entities.deleteEntity(this.entityID);
            this.entityID = undefined;
        }
        if (this.active) {
            this.activeChanged(this.active = false, reason);
        } else if (reason) {
            log('already stopped so not triggering another activeChanged; latest reason was:', reason);
        }
    },

    // @public @method - Reposition the doppleganger so it sees "eye to eye" with the Avatar.
    // @param {String} [byJointName=Hips] - the reference joint used to align the Doppleganger and Avatar
    syncVerticalPosition: function(byJointName) {
        byJointName = byJointName || 'Hips';

		var dopplePosition = Entities.getEntityProperties(this.entityID, ["position"]);    
		var doppleJointIndex = Entities.getJointIndex( this.entityID, byJointName );
		var doppleJointPosition = Vec3.sum(Entities.getAbsoluteJointTranslationInObjectFrame( this.entityID, doppleJointIndex ), dopplePosition);
		
		//log("Joint Pos = " + JSON.stringify(doppleJointPosition));
		
        var avatarPosition = this.avatar.position;
        var avatarJointIndex = this.avatar.getJointIndex(byJointName);
		var avatarJointPosition = Vec3.sum(this.avatar.getAbsoluteJointTranslationInObjectFrame(avatarJointIndex), avatarPosition);

		//log("AV Joint Pos = " + JSON.stringify(avatarJointPosition));

        dopplePosition.position.y = avatarJointPosition.y - doppleJointPosition.y;
        this.position = dopplePosition.position;
        Entities.editEntity(this.entityID, { position: this.position });
    },

    // @private @method - creates the update thread to synchronize joint data
    _createUpdateThread: function() {
        if (Doppleganger.USE_SCRIPT_UPDATE) {
            log('creating Script.update thread');
            this.onUpdate = this.update;
            Script.update.connect(this, 'onUpdate');
        } else {
            log('creating Script.setInterval thread @ ~', Doppleganger.TARGET_FPS +'fps');
            var timeout = 1000 / Doppleganger.TARGET_FPS;
            this._interval = Script.setInterval(bind(this, 'update'), timeout);
        }
    },

    // @private @method - waits for model to load and handles timeouts
    // @param {ModelResource} resource - a prefetched resource to monitor loading state against
    _waitForModel: function(resource) {
        var RECHECK_MS = 50;
        var id = this.entityID,
            watchdogTimer = null;

        function waitForJointNames() {
            var error = null, result = null;
            if (!watchdogTimer) {
                error = 'joints_unavailable';
            } else if (resource.state === Resource.State.FAILED) {
                error = 'prefetch_failed';
            } else if (resource.state === Resource.State.FINISHED) {
                var names = Entities.getJointNames(id);
                if (Array.isArray(names) && names.length) {
                    result = { entityID: id, jointNames: names };
                }
            }
            if (error || result !== null) {
                Script.clearInterval(this._interval);
                this._interval = null;
                if (watchdogTimer) {
                    Script.clearTimeout(watchdogTimer);
                }
                this.addingEntity(error, result);
            }
        }
        watchdogTimer = Script.setTimeout(function() {
            watchdogTimer = null;
        }, Doppleganger.MAX_WAIT_SECS * 1000);
        this._interval = Script.setInterval(bind(this, waitForJointNames), RECHECK_MS);
    }
};

// @function - bind a function to a `this` context
// @param {Object} - the `this` context
// @param {Function|String} - function or method name
function bind(thiz, method) {
    method = thiz[method] || method;
    return function() {
        return method.apply(thiz, arguments);
    };
}

// @function - Qt signal polyfill
function signal(template) {
    var callbacks = [];
    return Object.defineProperties(function() {
        var args = [].slice.call(arguments);
        callbacks.forEach(function(obj) {
            obj.handler.apply(obj.scope, args);
        });
    }, {
        connect: { value: function(scope, handler) {
            callbacks.push({scope: scope, handler: scope[handler] || handler || scope});
        }},
        disconnect: { value: function(scope, handler) {
            var match = {scope: scope, handler: scope[handler] || handler || scope};
            callbacks = callbacks.filter(function(obj) {
                return !(obj.scope === match.scope && obj.handler === match.handler);
            });
        }}
    });
}

// @function - debug logging
function log() {
    //print('doppleganger | ' + [].slice.call(arguments).join(' '));
}

// -- ADVANCED DEBUGGING --
// @function - Add debug joint indicators / extra debugging info.
// @param {Doppleganger} - existing Doppleganger instance to add controls to
//
// @note:
//   * rightclick toggles mirror mode on/off
//   * shift-rightclick toggles the debug indicators on/off
//   * clicking on an indicator displays the joint name and mirrored joint name in the debug log.
//
// Example use:
//    var doppleganger = new Doppleganger();
//    Doppleganger.addDebugControls(doppleganger);
Doppleganger.addDebugControls = function(doppleganger) {
    DebugControls.COLOR_DEFAULT = { red: 255, blue: 255, green: 255 };
    DebugControls.COLOR_SELECTED = { red: 0, blue: 255, green: 0 };

    function DebugControls() {
        this.enableIndicators = true;
        this.selectedJointName = null;
        this.debugEntityIDs = undefined;
        this.jointSelected = signal(function(result) {});
    }
    DebugControls.prototype = {
        start: function() {
            if (!this.onMousePressEvent) {
                this.onMousePressEvent = this._onMousePressEvent;
                Controller.mousePressEvent.connect(this, 'onMousePressEvent');
            }
        },

        stop: function() {
            this.removeIndicators();
            if (this.onMousePressEvent) {
                Controller.mousePressEvent.disconnect(this, 'onMousePressEvent');
                delete this.onMousePressEvent;
            }
        },

        createIndicators: function(jointNames) {
            this.jointNames = jointNames;
            return jointNames.map(function(name, i) {
                return Entities.addEntity({
                    type: "Shape",
					shape: 'Icosahedron',
                    scale: 0.1,
                    solid: false,
                    alpha: 0.5
                });
            });
        },

        removeIndicators: function() {
            if (this.debugEntityIDs) {
                this.debugEntityIDs.forEach(Entities.deleteEntity);
                this.debugEntityIDs = undefined;
            }
        },

        onJointsUpdated: function(entityID) {
            if (!this.enableIndicators) {
                return;
            }
            var jointNames = Entities.getJointNames(entityID),
                jointOrientations = Entities.getEntityProperties(entityID, ['jointRotations']), //was jointOrientations
                jointPositions = Entities.getEntityProperties(entityID, ['jointTranslations']), //was jointPositions
                //selectedIndex = jointNames.indexOf(this.selectedJointName); 
				selectedIndex = Entities.getJointIndex( entityID, this.selectedJointName );

            if (!this.debugEntityIDs) {
                this.debugEntityIDs = this.createIndicators(jointNames);
            }

            // batch all updates into a single call (using the editOverlays({ id: {props...}, ... }) API)
            var updatedOverlays = this.debugEntityIDs.reduce(function(updates, id, i) {
                updates[id] = {
                    position: jointPositions.jointTranslations[i],
                    rotation: jointOrientations.jointRotations[i],
                    color: i === selectedIndex ? DebugControls.COLOR_SELECTED : DebugControls.COLOR_DEFAULT,
                    solid: i === selectedIndex
                };
                return updates;
            }, {});
            //Entities.editOverlays(updatedOverlays);
        },

        _onMousePressEvent: function(evt) {
            if (!evt.isLeftButton || !this.enableIndicators || !this.debugEntityIDs) {
                return;
            }
            var ray = Camera.computePickRay(evt.x, evt.y),
                hit = Entities.findRayIntersection(ray, true, this.debugEntityIDs);

            hit.jointIndex = this.debugEntityIDs.indexOf(hit.entityID);
            hit.jointName = this.jointNames[hit.jointIndex];
            this.jointSelected(hit);
        }
    };

    if ('$debugControls' in doppleganger) {
        throw new Error('only one set of debug controls can be added per doppleganger');
    }
    var debugControls = new DebugControls();
    doppleganger.$debugControls = debugControls;

    function onMousePressEvent(evt) {
        if (evt.isRightButton) {
            if (evt.isShifted) {
                debugControls.enableIndicators = !debugControls.enableIndicators;
                if (!debugControls.enableIndicators) {
                    debugControls.removeIndicators();
                }
            } else {
                doppleganger.mirrored = !doppleganger.mirrored;
            }
        }
    }

    doppleganger.activeChanged.connect(function(active) {
        if (active) {
            debugControls.start();
            doppleganger.jointsUpdated.connect(debugControls, 'onJointsUpdated');
            Controller.mousePressEvent.connect(onMousePressEvent);
        } else {
            Controller.mousePressEvent.disconnect(onMousePressEvent);
            doppleganger.jointsUpdated.disconnect(debugControls, 'onJointsUpdated');
            debugControls.stop();
        }
    });

    debugControls.jointSelected.connect(function(hit) {
        debugControls.selectedJointName = hit.jointName;
        if (hit.jointIndex < 0) {
            return;
        }
        hit.mirroredJointName = Doppleganger.getMirroredJointNames([hit.jointName])[0];
        log('selected joint:', JSON.stringify(hit, 0, 2));
    });

    Script.scriptEnding.connect(debugControls, 'removeIndicators');

    return doppleganger;
};
