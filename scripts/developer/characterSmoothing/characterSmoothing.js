//
// Copyright 2023 Overte e.V.

let smoothing_settings = {
    enabled: true,
    targets: {
        LeftHand: { transform: 1, rotation: 1 },
        RightHand: { transform: 1, rotation: 1 },
        LeftFoot: { transform: 1, rotation: 1 },
        RightFoot: { transform: 1, rotation: 1 },
        Hips: { transform: 1, rotation: 1 },
        Spine2: { transform: 1, rotation: 1 },
    },
};

let mapping;
let mapping_settings = {
    name: "org.overte.controllers.smoothing",
    channels: [],
};

const html_url = Script.resolvePath("./ui/index.html");
let tablet = Tablet.getTablet("com.highfidelity.interface.tablet.system");
let shown = false;

tablet.screenChanged.connect(onScreenChanged);
Script.scriptEnding.connect(shutdownTabletApp);

let tabletButton = tablet.addButton({
    text: "AviSmooth",
    icon: Script.resolvePath("./img/icon.png"),
    activeIcon: Script.resolvePath("./img/icon-a.png"),
});

tabletButton.clicked.connect(() => {
    tablet.gotoWebScreen(html_url);
});

function onScreenChanged(type, url) {
    if (type !== "Web" || url !== html_url) {
        tablet.webEventReceived.disconnect(onWebEventReceived);
        tabletButton.editProperties({ isActive: false });
        shown = false;
    } else {
        tabletButton.editProperties({ isActive: true });
        tablet.webEventReceived.connect(onWebEventReceived);
        shown = true;
        smoothing_settings = Settings.getValue(
            "smoothing_settings",
            smoothing_settings
        );
    }
}

function shutdownTabletApp() {
    if (mapping) mapping.disable(); // Disable custom mapping
    tablet.removeButton(tabletButton); // Remove the app
    tablet.screenChanged.disconnect(onScreenChanged);
    tablet.webEventReceived.disconnect(onWebEventReceived);
}

const _sendMessage = (message) =>
    tablet.emitScriptEvent(JSON.stringify(message));

function onWebEventReceived(message) {
    message = JSON.parse(message);

    if (message.action === "ready") {
        _sendMessage({
            action: "initialize",
            data: smoothing_settings,
        });
    }

    if (message.action === "new_settings") {
        smoothing_settings = message.data;
        mappingChanged();
    }

    if (message.action === "set_state") {
        smoothing_settings.enabled = message.data;
        mappingChanged();
    }
}

function mappingChanged() {
    Settings.setValue("smoothing_settings", smoothing_settings);
    if (mapping) mapping.disable();

    if (smoothing_settings.enabled) {
        // Build mapping_settings
        mapping_settings.channels = [];

        Object.keys(smoothing_settings.targets).forEach((target) =>
            mapping_settings.channels.push(_generateChannel(target))
        );

        function _generateChannel(name) {
            return {
                from: `Standard.${name}`,
                to: `Actions.${name}`,
                filters: [
                    {
                        type: "exponentialSmoothing",
                        translation: smoothing_settings.targets[name].transform,
                        rotation: smoothing_settings.targets[name].rotation,
                    },
                ],
            };
        }

        mapping = Controller.parseMapping(JSON.stringify(mapping_settings));
        mapping.enable();
    }
}
