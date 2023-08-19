// TODO: Good UI
// TODO: Script optimization
// TODO: AccelerationLimiter choice?
// TODO: Force limit values on smoothing_settings.targets to 0 though 1

//
// Copyright 2023 Overte e.V.

// Start everything at no smoothing.
// Ideally the miniscule about of smoothing that is actually still there should be sufficient.
let smoothing_settings = {
    enabled: false,
    targets: {
        left_hand: {
            transform: 1,
            rotation: 1,
        },
        right_hand: {
            transform: 1,
            rotation: 1,
        },
        left_foot: {
            transform: 1,
            rotation: 1,
        },
        right_foot: {
            transform: 1,
            rotation: 1,
        },
        hips: {
            transform: 1,
            rotation: 1,
        },
        spine2: {
            transform: 1,
            rotation: 1,
        },
    },
};
let mappingJson = {
    name: "org.overte.controllers.smoothing",
    channels: [
        {
            from: "Standard.LeftHand",
            to: "Actions.LeftHand",
            filters: [
                {
                    type: "exponentialSmoothing",
                    translation: smoothing_settings.targets.left_hand.transform,
                    rotation: smoothing_settings.targets.left_hand.rotation,
                },
            ],
        },
        {
            from: "Standard.RightHand",
            to: "Actions.RightHand",
            filters: [
                {
                    type: "exponentialSmoothing",
                    translation:
                        smoothing_settings.targets.right_hand.transform,
                    rotation: smoothing_settings.targets.right_hand.rotation,
                },
            ],
        },
        {
            from: "Standard.LeftFoot",
            to: "Actions.LeftFoot",
            filters: [
                {
                    type: "exponentialSmoothing",
                    translation: smoothing_settings.targets.left_foot.transform,
                    rotation: smoothing_settings.targets.left_foot.rotation,
                },
            ],
        },
        {
            from: "Standard.RightFoot",
            to: "Actions.RightFoot",
            filters: [
                {
                    type: "exponentialSmoothing",
                    translation:
                        smoothing_settings.targets.right_foot.transform,
                    rotation: smoothing_settings.targets.right_foot.rotation,
                },
            ],
        },
        {
            from: "Standard.Hips",
            to: "Actions.Hips",
            filters: [
                {
                    type: "exponentialSmoothing",
                    translation: smoothing_settings.targets.hips.transform,
                    rotation: smoothing_settings.targets.hips.rotation,
                },
            ],
        },
        {
            from: "Standard.Spine2",
            to: "Actions.Spine2",
            filters: [
                {
                    type: "exponentialSmoothing",
                    translation: smoothing_settings.targets.spine2.transform,
                    rotation: smoothing_settings.targets.spine2.rotation,
                },
            ],
        },
    ],
};

let mapping;

// Build tablet
const HTML_URL = Script.resolvePath("./index.html");
let shown = false;
let tablet = Tablet.getTablet("com.highfidelity.interface.tablet.system");
let tabletButton = tablet.addButton({
    text: "CHR",
    // TODO: Icon
    icon: Script.resolvePath("./img/icon.png"),
});
tabletButton.clicked.connect(() => {
    if (shown) tablet.gotoHomeScreen();
    else tablet.gotoWebScreen(HTML_URL);
});

function onScreenChanged(type, url) {
    if (type === "Web" && url === HTML_URL) {
        tabletButton.editProperties({ isActive: true });

        if (!shown) {
            // hook up to event bridge
            tablet.webEventReceived.connect(onWebEventReceived);
            shownChanged(true);
        }
        // FIXME: Works, just need to wait for response before we send data
        Script.setTimeout(() => {
            _sendMessage({
                action: "load_listings",
                data: smoothing_settings,
            });
        }, 1000);

        shown = true;
    } else {
        tabletButton.editProperties({ isActive: false });

        if (shown) {
            // disconnect from event bridge
            tablet.webEventReceived.disconnect(onWebEventReceived);
            shownChanged(false);
        }
        shown = false;
    }
}
tablet.screenChanged.connect(onScreenChanged);

function shutdownTabletApp() {
    tablet.removeButton(tabletButton);
    if (shown) {
        tablet.webEventReceived.disconnect(onWebEventReceived);
        tablet.gotoHomeScreen();
    }
    tablet.screenChanged.disconnect(onScreenChanged);
}

function onWebEventReceived(msg) {
    msg = JSON.parse(msg);

    // TODO
    // Toggle smoothing
    // if (msg.action === "set_state") {
    //     smoothing_settings.enabled = msg.value ? true : false;
    //     mappingChanged();
    // }

    // Adjust a target's rotation and transform values
    if (msg.action === "new_settings") {

        smoothing_settings = msg.data
        mappingChanged();
    }
}

function mappingChanged() {
    Settings.setValue("smoothing_settings", smoothing_settings);

    if (mapping) mapping.disable();

    if (smoothing_settings.enabled) {
        mapping = Controller.parseMapping(JSON.stringify(mappingJson));
        mapping.enable();
    }
}

function shownChanged(newShown) {
    if (newShown) mappingChanged();
    else if (mapping) mapping.disable();
}

Script.scriptEnding.connect(function () {
    if (mapping) mapping.disable();
    tablet.removeButton(tabletButton);
});

function _sendMessage(message) {
    message = JSON.stringify(message);
    tablet.emitScriptEvent(message);
}

// Load settings
smoothing_settings = Settings.getValue(
    "smoothing_settings",
    smoothing_settings
);

// TODO: Does script init work?
// Settings.setValue(
//     "smoothing_settings",
//     {
//     enabled: false,
//     targets: {
//         left_hand: {
//             transform: 1,
//             rotation: 1,
//         },
//         right_hand: {
//             transform: 1,
//             rotation: 1,
//         },
//         left_foot: {
//             transform: 1,
//             rotation: 1,
//         },
//         right_foot: {
//             transform: 1,
//             rotation: 1,
//         },
//         hips: {
//             transform: 1,
//             rotation: 1,
//         },
//         spine2: {
//             transform: 1,
//             rotation: 1,
//         },
//     },
// }
// );

mappingChanged();
