import QtQuick
import QtQuick.Controls

import "../" as Overte
import "." as OverteChat

Rectangle {
    id: root
    anchors.fill: parent
    color: Overte.Theme.paletteActive.base

    // stop the StackView from leaking out of overlay windows
    clip: true

    property bool settingJoinNotifications: true
    property bool settingBroadcast: false
    property bool settingChatBubbles: true
    property bool settingDesktopWindow: false

    property var typingIndicatorNames: ({})

    // When the window gets closed, the chat history is forgotten.
    // Keep a log of events we've received so we can replay them
    // when recreating the window.
    property list<var> eventsLog: []

    Component.onCompleted: {
        // fullPrivate so it's never accessible from other scripts,
        // we don't want entity client scripts that are able to scrape chat history
        const savedEvents = SettingsInterface.getValue("fullPrivate/chat/eventsLog") ?? [];
        for (let event of savedEvents) {
            fromScript(event);
        }
    }

    Component.onDestruction: {
        SettingsInterface.setValue("fullPrivate/chat/eventsLog", eventsLog);
    }

    onMessagesCleared: {
        eventsLog = [];
        SettingsInterface.setValue("fullPrivate/chat/eventsLog", eventsLog);
    }

    // NOTE: "int" makes sense here as the timestamps are whole milliseconds,
    // but it's 32 bits and overflows, so we need real's ~53 bits to work properly
    signal messagePushed(name: string, body: string, timestamp: real)
    signal notificationPushed(text: string, timestamp: real)
    signal messagesCleared()

    function toScript(obj) {
        sendToScript(JSON.stringify(obj));
    }

    function fromScript(rawObj) {
        let obj = (typeof(rawObj) === "string") ? JSON.parse(rawObj) : rawObj;
        const timestamp = obj.timestamp ?? Date.now();

        // keep chat events in the log
        if (
            obj.event !== "start_typing" &&
            obj.event !== "end_typing" &&
            obj.event !== "change_setting"
        ) {
            if (!obj.timestamp) {
                obj.timestamp = Date.now();
            }
            eventsLog.push(obj);

            // TODO: is this a performance problem? i'm not sure how else we could handle this robustly
            SettingsInterface.setValue("fullPrivate/chat/eventsLog", eventsLog);
        }

        switch (obj.event) {
            case "recv_message":
                messagePushed(obj.name ? obj.name : "<Unnamed>", obj.body, timestamp);
                break;

            case "user_joined": if (settingJoinNotifications) {
                notificationPushed(qsTr("%1 joined").arg(obj.name), timestamp);
            } break;

            case "user_left": if (settingJoinNotifications) {
                notificationPushed(qsTr("%1 left").arg(obj.name), timestamp);
            } break;

            case "user_name_changed":
                notificationPushed(
                    qsTr("%1 changed their name to %2").arg(obj.old_name).arg(obj.new_name),
                    timestamp
                );
                break;

            case "start_typing":
                typingIndicatorNames[obj.uuid] = obj.name;
                // propChanged is only fired by Qt on variable assignments, not property changes
                typingIndicatorNamesChanged();
                break;

            case "end_typing":
                delete typingIndicatorNames[obj.uuid];
                // propChanged is only fired by Qt on variable assignments, not property changes
                typingIndicatorNamesChanged();
                break;

            case "change_setting":
                updateSetting(obj.name, obj.value);
                break;

            default:
                console.error(`fromScript: Unknown event type "${obj.event}"`);
                console.error(JSON.stringify(obj));
                break;
        }
    }

    function updateSetting(name, value) {
        switch (name) {
            case "join_notify":
                settingJoinNotifications = value;
                break;

            case "broadcast":
                settingBroadcast = value;
                break;

            case "chat_bubbles":
                settingChatBubbles = value;
                break;

            case "desktop_window":
                settingDesktopWindow = value;
                break;
        }
    }

    function sendSettingsUpdate() {
        toScript({event: "change_setting", setting: "join_notify", value: settingJoinNotifications});
        toScript({event: "change_setting", setting: "broadcast", value: settingBroadcast});
        toScript({event: "change_setting", setting: "chat_bubbles", value: settingChatBubbles});
        toScript({event: "change_setting", setting: "desktop_window", value: settingDesktopWindow});
    }

    Overte.StackView {
        anchors.fill: parent

        id: stack
        initialItem: OverteChat.ChatPage {}
    }
}
