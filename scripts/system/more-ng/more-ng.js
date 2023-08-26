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
        installScript(message.data);
    }

    if (message.action === "uninstall_script") {
        uninstallScript(message.data);
    }
}

function uninstallScript(url) {
    logs(`Uninstalling script ${url}`);
    ScriptDiscoveryService.stopScript(url);
    let installed_apps = Settings.getValue("more_installed_list", []);

    for (let i = 0; installed_apps.length > i; i++) {
        if (installed_apps[i].url === url) {
            installed_apps.splice(i, 1);
            Settings.setValue("more_installed_list", installed_apps);
            break;
        }
    }
}

function installScript(url) {
    logs(`Installing script ${url}`);
    ScriptDiscoveryService.loadScript(url);
    let installed_apps = Settings.getValue("more_installed_list", []);

    app_info.thirdparty_apps.forEach((app) => {
        if (app.url === url) {
            // Update installed list to include newly installed app
            logs("Updating installed apps list");
            Settings.setValue("more_installed_list", [...installed_apps, app]);
        }
    });
}

function getInstalledApps() {
    const installed_apps = Settings.getValue("more_installed_list", []);
    const running_apps = ScriptDiscoveryService.getRunning();
    let list = [];

    list = installed_apps;

    // logs(list);

    running_apps.forEach((app) => {
        let data_exists = false; // Do we have a formatted object saved in our settings?
        //  Do not include local apps
        if (app.url.includes("file://")) return;

        // Check app in list against apps we have formatted entries for in Settings
        list.forEach((existing_setting) => {
            if (existing_setting.url === app.url) data_exists = true;
        });

        // We have saved data, no need to process it
        if (data_exists) return;

        let formatted_entry = {
            title: app.name,
            url: app.url,
            icon: app.icon,
            description: app.description,
            directory: app.directory,
            installed: true,
        };

        // Insert our entry for the running script
        list.push(formatted_entry);
    });

    // Save our updated list
    Settings.setValue("more_installed_list", list);
    _sendMessage({ action: "installed_apps", data: list });
}

async function findThirdParty() {
    // Clear third party app list
    app_info.thirdparty_apps = [];

    let requests = [];

    repo_list.forEach((url) => {
        requests.push(_getRequest(url));
    });

    const request_finished = await Promise.all(requests);

    request_finished.forEach((repo_data) => {
        // REVIEW: Is this the best way to do it?

        repo_data.data.forEach((app) => {
            let found = false;
            let repo_valid_apps = [];

            logs(app);
            const app_dir =
                repo_data.repo.split("/metadata.json")[0] +
                "/" +
                app.directory +
                "/";
            const icon_url = app_dir + app.icon;
            const script_url = app_dir + app.script;

            // Check if app is installed by URL
            app_info.installed_apps.forEach((app) => {
                if (app.url === script_url) found = true;
            });

            // Already installed and in the list.
            if (found) return;

            let app_form = {
                title: app.name,
                url: script_url,
                icon: icon_url,
                description: app.description,
                directory: app.directory,
            };

            repo_valid_apps.push(app_form);
            // app_info.thirdparty_apps.push(app_form);

            return _sendMessage({
                action: "repo_loaded",
                data: repo_valid_apps,
            });

            logs(icon_url);
            logs(script_url);
        });
    });
}

// TODO: Check for 404s or other errors
async function _getRequest(url) {
    return new Promise((resolve, reject) => {
        let xhr = new XMLHttpRequest();
        xhr.open("GET", url);
        xhr.onreadystatechange = function () {
            if (xhr.readyState == 4) {
                resolve({ data: JSON.parse(xhr.response), repo: url });
            }
        };
        xhr.send();
    });
}
Settings.setValue("more_installed_list", []);

repo_list = Settings.getValue("more_repo_list", repo_list);
