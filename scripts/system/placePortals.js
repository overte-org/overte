//
//  placePortals.js
//
//  Created by Ada <ada@thingvellir.net> on 2025-11-07
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
//  SPDX-License-Identifier: Apache-2.0
"use strict";

const PORTAL_LIFETIME_SECS = 20;
const MESSAGE_CHANNEL = "org.overte.PlacePortal";

// convenience function because QML can't use the vec3 or quat we normally use
const CREATE_MESSAGE_CHANNEL = "org.overte.PlacePortal.Create";

const ROOT_DEFAULT_PROPS = {
    type: "Shape",
    shape: "Cylinder",
    grab: { grabbable: false },
    ignorePickIntersection: true,
    dimensions: [1, 2, 1],
    alpha: 0,
};

const VISUAL_DEFAULT_PROPS = {
    type: "ParticleEffect",
    // FIXME: why is this oriented weird?
    emitDimensions: [0.5, 0.5, 1.5],
    textures: Script.resolvePath("./places/icons/portalFX.png"),
    emitRate: 100,
    lifespan: 3,
    maxParticles: 500,
    polarStart: 0,
    polarFinish: Math.PI,
    emitAcceleration: [0, 0, 0],
    radiusStart: 1.0,
    particleRadius: 0.5,
    radiusFinish: 0.3,
    alphaStart: 0.1,
    alpha: 0.1,
    alphaFinish: 0.1,
    emitSpeed: -0.1,
    speedSpread: 0,
    colorStart: [255, 0, 0],
    color: [255, 0, 0],
    colorFinish: [255, 255, 255],
};

const TITLE_DEFAULT_PROPS = {
    type: "Text",
    grab: { grabbable: false },
    ignorePickIntersection: true,
    dimensions: [1, 0.5, 0.1],
    localPosition: [0, 1, 0],
    billboardMode: "yaw",
    backgroundAlpha: 0.0,
    lineHeight: 0.1,
    unlit: true,
    textColor: "white",
    textEffect: "outline fill",
    textEffectColor: "black",
    textEffectThickness: 0.3,
    alignment: "center",
};

// key is the UUID of the collider,
// value is {
//   titleEntity: UUID,
//   visualEntity: UUID,
//   placeUrl: string,
//   onTick: function,
//   lifetime: int,
//   tickInterval: setInterval,
// }
let portalInfo = new Map();

function deleteAllPortals() {
    for (const [id, props] of portalInfo) {
        Entities.deleteEntity(id);
        Entities.deleteEntity(props.titleEntity);
    }

    portalInfo.clear();
}

function colorHash(x) {
    const FNV_PRIME = 0x01000193;
    const FNV_OFFSET = 0x811c9dc5;

    let value = FNV_OFFSET;
    for (const c of x) {
        value ^= c;
        value = (value * FNV_PRIME) & 0xffffffff;
    }
    value /= 0x7fffffff;

    const TAU = 2 * Math.PI;

    const r = 127 * Math.sin((value - (1 / 3)) * TAU) + 128;
    const g = 127 * Math.sin((value + 0) * TAU) + 128;
    const b = 127 * Math.sin((value + (1 / 3)) * TAU) + 128;

    return [r, g, b];
}

function createPortal(placeName, placeUrl, position) {
    const color = colorHash(placeName);

    // TODO: translation support
    const goingToText = `Going to ${placeName}`;

    const root = Entities.addEntity({
        position: Vec3.sum(position, [0, 1, 0]),
        // enterEntity can't be connected to through Entities, is that a bug?
        script: `(function(){
    this.enterEntity = _id => {
        Window.displayAnnouncement(${JSON.stringify(goingToText)});
        location.handleLookupString(${JSON.stringify(placeUrl)});
    };
})`,
        ...ROOT_DEFAULT_PROPS,
    }, "local");

    const title = Entities.addEntity({
        parentID: root,
        text: `${placeName}\n${PORTAL_LIFETIME_SECS}`,
        ...TITLE_DEFAULT_PROPS,
    }, "local");

    const visual = Entities.addEntity({
        parentID: root,
        ...VISUAL_DEFAULT_PROPS,
        colorStart: color,
        color: color,
    }, "local");

    let props = {
        rootEntity: root,
        titleEntity: title,
        visualEntity: visual,
        placeUrl: placeUrl,

        lifetime: PORTAL_LIFETIME_SECS,

        onTick() {
            this.lifetime -= 1;
            Entities.editEntity(this.titleEntity, { text: `${placeName}\n${this.lifetime}` });

            if (this.lifetime <= 0) {
                portalInfo.delete(this.rootEntity);
                Script.clearInterval(this.tickInterval);

                Entities.deleteEntity(this.titleEntity);
                Entities.deleteEntity(this.visualEntity);
                Entities.deleteEntity(this.rootEntity);
            }
        },
    };

    props.tickInterval = Script.setInterval(() => props.onTick(), 1000);

    portalInfo.set(root, props);
}

Messages.messageReceived.connect((channel, rawMsg, _senderID, _localOnly) => {
    if (channel === MESSAGE_CHANNEL) {
        try {
            const data = JSON.parse(rawMsg);

            if (data.place_name && data.place_url && data.position) {
                createPortal(data.place_name, data.place_url, data.position);
            } else {
                console.warn(MESSAGE_CHANNEL, "Necessary data missing, can't create portal");
            }
        } catch (e) { console.error(e); }
    } else if (channel === CREATE_MESSAGE_CHANNEL) {
        try {
            const data = JSON.parse(rawMsg);

            if (data.place_name && data.place_url) {
                createPortal(
                    data.place_name,
                    data.place_url,
                    Vec3.sum(
                        MyAvatar.feetPosition,
                        Vec3.multiply(
                            1.5 * MyAvatar.sensorToWorldScale,
                            Quat.getForward(MyAvatar.orientation)
                        )
                    )
                );
            } else {
                console.warn(MESSAGE_CHANNEL, "Necessary data missing, can't create portal");
            }
        } catch (e) { console.error(e); }
    }
});

Messages.subscribe(MESSAGE_CHANNEL);
Script.scriptEnding.connect(() => deleteAllPortals());
Window.domainChanged.connect(_url => deleteAllPortals());
