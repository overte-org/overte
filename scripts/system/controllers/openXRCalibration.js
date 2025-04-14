function handOverrideFunc(srcProps) {
    return {
        leftHandType: 0,
        rightHandType: 0,
        leftHandPosition: {x: 0.2, y: 0.5, z: 0.2},
        rightHandPosition: {x: -0.2, y: 0.5, z: 0.2},
        leftHandRotation: Quat.fromPitchYawRollDegrees(90, 0, -90),
        rightHandRotation: Quat.fromPitchYawRollDegrees(90, 0, 90),
    };
}

let handsOverride = MyAvatar.addAnimationStateHandler(handOverrideFunc, [
    "leftHandType", "leftHandPosition", "leftHandRotation",
    "rightHandType", "rightHandPosition", "rightHandRotation",
]);

Script.setTimeout(() => {
    MyAvatar.removeAnimationStateHandler(handsOverride);
    Script.stop();
}, 5200);
