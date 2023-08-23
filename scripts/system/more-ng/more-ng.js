//
// Copyright 2023 Overte e.V.

const html_url = Script.resolvePath("./ui/index.html");
let tablet = Tablet.getTablet("com.highfidelity.interface.tablet.system");
let shown = false;

tablet.screenChanged.connect(onScreenChanged);
Script.scriptEnding.connect(shutdownTabletApp);

let tabletButton = tablet.addButton({
    text: "MORE",
    icon: Script.resolvePath("./img/icon.png"),
    activeIcon: Script.resolvePath("./img/icon-a.png"),
});

// REVIEW: How can we optimize this further?
tabletButton.clicked.connect(clicked);

function onScreenChanged(type, url) {
    if (type !== "Web" || url !== html_url) {
        tablet.webEventReceived.disconnect(onWebEventReceived);
        tabletButton.editProperties({ isActive: false });
        shown = false;
    } else {
        tabletButton.editProperties({ isActive: true });
        tablet.webEventReceived.connect(onWebEventReceived);
        shown = true;
    }
}

function clicked() {
    if (shown) {
        tablet.webEventReceived.disconnect(onWebEventReceived);
        tablet.gotoHomeScreen();
        shown = false;
    } else {
        tablet.gotoWebScreen(html_url);
        tablet.webEventReceived.connect(onWebEventReceived);
        shown = true;
    }
    tabletButton.editProperties({
        isActive: shown,
    });
}

function shutdownTabletApp() {
    tablet.removeButton(tabletButton); // Remove the app
    tablet.screenChanged.disconnect(onScreenChanged);
    tablet.webEventReceived.disconnect(onWebEventReceived);
}

const _sendMessage = (message) =>
    tablet.emitScriptEvent(JSON.stringify(message));

function onWebEventReceived(message) {
    message = JSON.parse(message);

    if (message.action === "ready") {
        // Get apps from repos
        // Form custom Object and send to ui

        // Get installed apps
        const installed_apps = ScriptDiscoveryService.getRunning();
        const repo_apps = _get("");

        const app_list = _createAppList(installed_apps, repo_apps);

        _sendMessage({ action: "app_list", data: app_list });
    }

    if (message.action === "new_repo") {
    }

    if (message.action === "refresh_repo") {
    }

    if (message.action === "delete_repo") {
    }

    if (message.action === "install_script") {
        // On installed, create "setting" to save app metadata
    }

    if (message.action === "uninstall_script") {
        ScriptDiscoveryService.stopScript(message.data);
        // TODO: Reload application window
    }
}

function _get(url) {
    // HTTPS Request here
    return [];
}

function _createAppList(installed_apps, repo_apps) {
    let return_array = [];

    installed_apps.forEach((app) => {
        // REVIEW: Multiplatform support functional?
        if (app.url.includes("file://")) return;

        return_array.push({
            title: app.name,
            url: app.url,
            icon: app.icon || "",
            installed: true,
        });
    });

    // REVIEW: Is this legit?
    repo_apps.forEach((app) => {
        for (let i = 0; installed_apps.length > i; i++) {
            if (installed_apps[Object.keys(installed_apps)[i]].url === app.url)
                return;
        }

        return_array.push({
            title: app.name,
            url: app.url,
            icon: app.icon || "",
            installed: false,
        });
    });

    return return_array;
}
