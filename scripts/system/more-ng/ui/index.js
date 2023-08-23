// Helper functions
const qs = (target) => document.querySelector(target);
const qsa = (target) => document.querySelectorAll(target);

qs("#open-settings").addEventListener("click", () => openPage("settings"));
qs("#close-settings").addEventListener("click", () => openPage("app-listing"));
qs("#open-filters").addEventListener("click", () => openFilter());

EventBridge.scriptEventReceived.connect((message) =>
    newMessage(JSON.parse(message))
);
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
    if (message.action === "app_list") {
        buildAppList(message.data);
    }
}

function buildAppList(app_list) {
    // Remove existing listings
    qsa(".app-list .listing").forEach((item) => item.remove());

    app_list.forEach((app) => {
        let template = qs("#app-listing-template").content.cloneNode(true);
        if (app.local === true) return;
        console.log(JSON.stringify(app));

        if (app.installed) {
            template.querySelector(".listing").classList.add("installed");
            template
                .querySelector(".app-actions [data-intention='install']")
                .classList.add("hidden");
            template
                .querySelector(".app-actions [data-intention='uninstall']")
                .classList.remove("hidden");
            template
                .querySelector(".app-actions .bad")
                .addEventListener("click", () => {
                    uninstallApp(app.url);
                });
        }

        if (app.icon) template.querySelector(".icon img").src = app.icon;

        template.querySelector(".app-info .body .title").innerText = app.title;

        qs(".app-list").appendChild(template);
    });
}

function uninstallApp(url) {
    _sendMessage({ action: "uninstall_script", data: url });
}

_sendMessage({ action: "ready" });
