//
// Copyright 2023 Overte e.V.
// "https://more.overte.org/applications/metadata.js",

const html_url = Script.resolvePath("./ui/index.html");
let tablet = Tablet.getTablet("com.highfidelity.interface.tablet.system");
let shown = false;
const logs = (info) => console.log("[NEW_MORE] " + JSON.stringify(info));

tablet.screenChanged.connect(onScreenChanged);
Script.scriptEnding.connect(shutdownTabletApp);

let tabletButton = tablet.addButton({
    text: "MORE",
    icon: Script.resolvePath("./img/icon.png"),
    activeIcon: Script.resolvePath("./img/icon-a.png"),
});

let repo_list = [
    "https://raw.githubusercontent.com/Armored-Dragon/overte-app-examples/master/metadata.json",
];
let app_info = {
    installed_apps: [],
    thirdparty_apps: [],
};
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
        getInstalledApps();
        findThirdParty();
    }

    if (message.action === "new_repo") {
    }

    if (message.action === "refresh_repo") {
    }

    if (message.action === "delete_repo") {
    }

    if (message.action === "install_script") {
        logs(`Installing script ${message.data}`);
        ScriptDiscoveryService.loadScript(message.data);
    }

    if (message.action === "uninstall_script") {
        logs(`Uninstalling script ${message.data}`);
        ScriptDiscoveryService.stopScript(message.data);
    }
}

function getInstalledApps() {
    const running_apps = ScriptDiscoveryService.getRunning();
    const installed_apps = Settings.getValue("more_installed_list", []);

    let return_array = [];
    running_apps.forEach((app) => {
        // REVIEW: Multiplatform support functional?
        if (app.url.includes("file://")) return;
        let found = false;

        logs(`Processing ${app.name || app.title}`);
        logs(app.url);

        for (let i = 0; installed_apps.length > i; i++) {
            if (installed_apps[i].url === app.url) {
                return_array.push(installed_apps[i]);
                found = true;
                break;
            }

            // Our installed app is not saved in settings, add it
            if (installed_apps.length === i) {
                logs("Not in settings");
                let template = {
                    title: app.name,
                    url: app.url,
                    icon: app.icon || "",
                    description: app.description,
                    directory: app.directory,
                    installed: true,
                };
                return_array.push(template);
                found = true;
                return;
            }
        }
        if (!found) {
            logs("Settings was blank; just pushing it in.");
            let template = {
                title: app.name,
                url: app.url,
                icon: app.icon || "",
                description: app.description,
                directory: app.directory,
                installed: true,
            };
            return_array.push(template);
            return;
        }
    });

    Settings.setValue("more_installed_list", return_array);
    _sendMessage({ action: "installed_apps", data: return_array });
}

function findThirdParty() {
    repo_list.forEach((url) => {
        var req = new XMLHttpRequest();

        req.requestComplete.connect(() => {
            if (req.status !== 200) {
                _sendMessage({
                    action: "repo_loaded",
                    data: { error: true, status: req.status },
                });
            }

            const response = JSON.parse(req.responseText);
            let app_list_formatted = [];

            response.forEach((app) => {
                const app_dir =
                    url.split("/metadata.json")[0] + "/" + app.directory + "/";

                const icon_url = app_dir + app.icon;
                const script_url = app_dir + app.script;

                app_list_formatted.push({
                    title: app.name,
                    url: script_url,
                    icon: icon_url,
                    description: app.description,
                    directory: app.directory,
                    installed: false,
                });
            });

            return _sendMessage({
                action: "repo_loaded",
                data: app_list_formatted,
            });
        });

        req.open("GET", url);
        req.send();
    });
}

repo_list = Settings.getValue("more_repo_list", repo_list);
