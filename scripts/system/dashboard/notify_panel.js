// dashboard.js
// Created by Ada <ada@thingvellir.net> on 2026-02-25
// SPDX-License-Identifier: Apache-2.0
"use strict";

// TODO: use the system utilMath once it's been merged
const { Vector3, Quaternion, vec3, quat, euler } = require("./utilMath.js");
const Defs = require("./consts.js");

class NotifyPanel {
    #scaleCallback;

    #setupCallbacks() {
        this.#scaleCallback = () => Entities.editEntity(this.entityID, {
            dpi: Defs.scaleHackInv(Defs.notifyPanelDPI),
        });
        MyAvatar.sensorToWorldScaleChanged.connect(this.#scaleCallback);
    }

    /** @type {Uuid} */
    entityID;

    /**
     * If the notify panel smoothly floats to follow the HMD.
     * @type {boolean}
     */
    #floating;

    /** @type {Vector3} */
    #position = Vector3.ZERO;

    /** @type {Quaternion} */
    #rotation = Quaternion.IDENTITY;

    constructor() {
        this.#floating = HMD.active;
        this.#setupCallbacks();

        this.entityID = Entities.addEntity({
            type: "Web",
            name: "Notification Panel",
            // not attached to the dash root
            // so it can follow the player's view
            parentID: MyAvatar.SELF_ID,
            ignorePickIntersection: true,
            grab: { grabbable: false },
            localDimensions: vec3(0.3, 0.4, 0),
            dpi: Defs.scaleHackInv(Defs.notifyPanelDPI),
            sourceUrl: Defs.notifyPanelQmlURL,
            maxFPS: 90,
            wantsKeyboardFocus: false,
            showKeyboardHighlight: false,
            useBackground: false,
            renderLayer: "front",
        }, "local");

        this.#reparent();
    }

    #reparent() {
        if (this.#floating) {
            // vr mode
            Entities.editEntity(this.entityID, {
                parentJointIndex: Defs.sensorToWorldJoint,
            });
        } else {
            // desktop mode
            this.#position = vec3(0.0, -0.2, -0.6);
            this.#rotation = Quaternion.IDENTITY;
            Entities.editEntity(this.entityID, {
                localPosition: this.#position,
                localRotation: this.#rotation,
                parentJointIndex: Defs.cameraJoint,
            });
        }
    }

    /**
     * @param {number} dt - Delta time
     */
    update(dt) {
        // display mode has changed, so reconfigure
        // and reparent the web entity
        if (HMD.active !== this.#floating) {
            this.#floating = HMD.active;
            this.#reparent();
        }

        // the web entity is directly attached
        // to the camera on desktop, so don't touch it
        if (!this.#floating) { return; }

        const cameraPos = vec3(Entities.worldToLocalPosition(
            Camera.position,
            MyAvatar.SELF_ID,
            Defs.sensorToWorldJoint,
            true
        ));
        const cameraRot = quat(Entities.worldToLocalRotation(
            Camera.orientation,
            MyAvatar.SELF_ID,
            Defs.sensorToWorldJoint,
            true
        ));

        const targetPos = (
            cameraRot
            .multiply(vec3(0, -0.32, -0.85))
            .add(cameraPos)
        );

        // TODO: cancel roll or use look-at instead
        let targetRot = euler(-20, 0, 0).multiply(cameraRot);

        // keep the panel still within a threshold
        // TODO: tune this to make it feel good
        // if (targetPos.distance(this.#position) < 0.1) { return; }

        this.#position = this.#position.lerpTo(targetPos, dt * Defs.notifyPanelFloatSpeed);
        this.#rotation = this.#rotation.lerpTo(targetRot, dt * Defs.notifyPanelFloatSpeed).normalized();

        Entities.editEntity(this.entityID, {
            // FIXME: setting the position and rotation of a web entity
            // triggers a lot of resizes, even when the dimensions don't change
            localPosition: this.#position,
            localRotation: this.#rotation,
        });
    }

    /**
     * @param {object} args
     * @param {string} args.text - Notification text
     * @param {string} args.icon - URL to notification icon image
     * @param {string} args.image - URL to notification body image
     * @param {number} [args.lifetime=5]
     */
    postNotification({ text, icon, image, lifetime = 5 }) {
        Entities.emitScriptEvent(this.entityID, JSON.stringify({
            notify: {
                text,
                icon,
                image,
                lifetime,
            }
        }));
    }

    dispose() {
        MyAvatar.sensorToWorldScaleChanged.disconnect(this.#scaleCallback);
        Entities.deleteEntity(this.entityID);
    }
}

module.exports = { NotifyPanel };
