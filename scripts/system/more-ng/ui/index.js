// Helper functions
const qs = (target) => document.querySelector(target);
const qsa = (target) => document.querySelectorAll(target);

// Page selection
qs("#open-settings").addEventListener("click", () => openPage("settings"));
qs("#close-settings").addEventListener("click", () => openPage("app-listing"));
qs("#open-filters").addEventListener("click", () => openFilter());
const logs = (info) => console.log("[NEW_MORE] " + JSON.stringify(info));

EventBridge.scriptEventReceived.connect((message) =>
    newMessage(JSON.parse(message))
);

let app_info = {
    installed_apps: [],
    thirdparty_apps: [],
};

let search_filters = {
    only_installed: false,
};

const _sendMessage = (message) =>
    EventBridge.emitWebEvent(JSON.stringify(message));

function openPage(page_name) {
    qsa(".page").forEach((page) => page.classList.add("hidden"));
    qs(`#${page_name}`).classList.remove("hidden");
}

function openFilter() {
    const state = qs("#app-filters").classList.contains("hidden");

    if (state) {
        qs("#app-filters").classList.remove("hidden");
    } else {
        qs("#app-filters").classList.add("hidden");
    }
}

function newMessage(message) {
    if (message.action === "installed_apps") {
        buildAppList(message.data, true);
    }

    if (message.action === "repo_loaded") {
        buildAppList(message.data);
    }
}

function buildAppList(app_list, clear) {
    // Remove existing listings
    if (clear) qsa(".app-list .listing").forEach((item) => item.remove());

    app_list.forEach((app) => {
        let found = false;

        app_info.installed_apps.forEach((check_app) => {
            if (check_app.url === app.url) found = true;
        });
        if (found) return;

        let template = qs("#app-listing-template").content.cloneNode(true);

        template.querySelector(".icon img").src = app.icon;
        template.querySelector(".app-info .body .title").innerText = app.title;
        template.querySelector(".app-info .body .description").innerText =
            app.description;

        if (app.installed) {
            // Hide "Install" button
            template
                .querySelector(".app-actions [data-intention='install']")
                .classList.add("hidden");

            // Unhide "uninstall" button
            template
                .querySelector(".app-actions [data-intention='uninstall']")
                .classList.remove("hidden");

            // Uninstall Script button
            template
                .querySelector(".app-actions .bad")
                .addEventListener("click", () => {
                    uninstallApp(app.url);
                });

            template.querySelector(".listing").classList.add("installed");

            app_info.installed_apps.push(app);
        }
        if (!app.installed) {
            // Unhide "Install" button
            template
                .querySelector(".app-actions [data-intention='install']")
                .classList.remove("hidden");
            // Hide "uninstall" button
            template
                .querySelector(".app-actions [data-intention='uninstall']")
                .classList.add("hidden");

            // Install script button
            template
                .querySelector(".app-actions [data-intention='install']")
                .addEventListener("click", () => {
                    installApp(app.url);
                });
        }
        qs(".app-list").appendChild(template);
    });
}

function uninstallApp(url) {
    _sendMessage({ action: "uninstall_script", data: url });
}

function installApp(url) {
    _sendMessage({ action: "install_script", data: url });
}

function toggleFilter(filter_name) {
    if (search_filters[filter_name] === true) {
        search_filters[filter_name] = false;
        qs('#app-filters [data-filter_name="only_installed"').classList.remove(
            "generic"
        );
    } else {
        search_filters[filter_name] = true;
        qs('#app-filters [data-filter_name="only_installed"').classList.add(
            "generic"
        );
    }
}

_sendMessage({ action: "ready" });
