// Helper functions
const qs = (target) => document.querySelector(target);
const qsa = (target) => document.querySelectorAll(target);

// Message listeners
const _sendMessage = (message) =>
    EventBridge.emitWebEvent(JSON.stringify(message));
EventBridge.scriptEventReceived.connect((message) =>
    newMessage(JSON.parse(message))
);

// Settings & data
let smoothing_settings = {};

function newMessage(message) {
    if (message.action === "initialize") {
        initialize(message.data);
    }
}

function initialize(data) {
    smoothing_settings = data;

    // Clear all existing listings (if any)
    qsa("body .target-list").forEach((item) => item.remove());

    // Set state
    if (smoothing_settings.enabled === false) _toggleEnabledFalse();
    if (smoothing_settings.enabled === true) _toggleEnabledTrue();

    // For each target point
    Object.keys(smoothing_settings.targets).forEach((target) => {
        // Use the target data to build a listing
        let template = qs("#target-template").content.cloneNode(true);

        template.querySelector(".container").dataset.name = target;
        template.querySelector(".container-header").innerText = target;

        const rotation_area = template.querySelector('[data-value="rotation"]');
        const transform_area = template.querySelector(
            '[data-value="transform"]'
        );

        rotation_area.querySelector("input").value = _fromDecimal(
            smoothing_settings.targets[target].rotation
        );
        transform_area.querySelector("input").value = _fromDecimal(
            smoothing_settings.targets[target].transform
        );

        rotation_area.querySelector(".type-value").innerText = _formatPercent(
            _fromDecimal(smoothing_settings.targets[target].rotation)
        );
        transform_area.querySelector(".type-value").innerText = _formatPercent(
            _fromDecimal(smoothing_settings.targets[target].transform)
        );

        rotation_area.querySelector("input").addEventListener("change", () => {
            rotation_area.querySelector(".type-value").innerText =
                _formatPercent(rotation_area.querySelector("input").value);
            smoothing_settings.targets[target].rotation = _toDecimal(
                rotation_area.querySelector("input").value
            );
        });
        transform_area.querySelector("input").addEventListener("change", () => {
            transform_area.querySelector(".type-value").innerText =
                _formatPercent(transform_area.querySelector("input").value);
            smoothing_settings.targets[target].transform = _toDecimal(
                transform_area.querySelector("input").value
            );
        });

        // // Append our newly created child
        qs("#target-list").appendChild(template);
    });
}

qsa("input").forEach((button) =>
    button.addEventListener("click", (event) => event.target.blur())
);

function toggleSmoothing() {
    if (smoothing_settings.enabled) _toggleEnabledFalse();
    else _toggleEnabledTrue();
}

function _toggleEnabledFalse() {
    _sendMessage({ action: "set_state", data: false });
    qs("#toggle-button").classList.remove("bad");
    qs("#toggle-button").classList.add("good");
    qs("#toggle-button").innerText = "Enable";
    smoothing_settings.enabled = false;
}
function _toggleEnabledTrue() {
    _sendMessage({ action: "set_state", data: true });
    qs("#toggle-button").classList.remove("good");
    qs("#toggle-button").classList.add("bad");
    qs("#toggle-button").innerText = "Disable";
    smoothing_settings.enabled = true;
}

_sendMessage({ action: "ready" });
const applySettings = () =>
    _sendMessage({ action: "new_settings", data: smoothing_settings });
const _formatPercent = (value) => parseInt(value).toString() + " %";
const _toDecimal = (value) => value / 100;
const _fromDecimal = (value) => value * 100;
