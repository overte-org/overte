import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import ".." as Overte
import "."

Item {
    property var appButtons: ({})

    function reloadThemeSettings() {
        Overte.Theme.darkMode = SettingsInterface.getValue("Theme/darkMode", true);
        Overte.Theme.highContrast = SettingsInterface.getValue("Theme/highContrast", false);
        Overte.Theme.reducedMotion = SettingsInterface.getValue("Theme/reducedMotion", false);
    }

    function fromScript(rawMsg) {
        let msg;
        try {
            msg = JSON.parse(rawMsg);
        } catch (_) {
            return;
        }

        if (msg?.dashboard?.event === "theme_change") {
            reloadThemeSettings();
        } else if (msg?.dash_window?.event === "hide") {
            dashBar.state = "HIDDEN";
        } else if (msg?.dash_window?.event === "unhide") {
            dashBar.state = "OPEN";
        } else if (msg?.dash_bar?.event === "set_app_button") {
            console.info(`set_app_button ${msg.dash_bar.data.text} ${msg.dash_bar.data.active}`);
            appButtons[msg.dash_bar.ipc_id] = msg.dash_bar.data;
            // qml doesn't automatically acknowledge property changes
            appButtonsChanged();
        } else if (msg?.dash_bar?.event === "delete_app_button") {
            delete appButtons[msg.dash_bar.ipc_id];
            // qml doesn't automatically acknowledge property changes
            appButtonsChanged();
        }
    }

    onAppButtonsChanged: {
        console.log(JSON.stringify(appButtons));
    }

    function toScript(msg) {
        eventBridge.emitWebEvent(JSON.stringify(msg));
    }

    function resourceURL(url) {
        return `${resourceDirectoryUrl}${url}`;
    }

    Component.onCompleted: {
        eventBridge.scriptEventReceived.connect(fromScript);
    }

    id: dashBar
    anchors.centerIn: parent
    state: "HIDDEN"

    states: [
        State {
            name: "HIDDEN"
            PropertyChanges { target: dashBar; opacity: 0 }
        },
        State {
            name: "OPEN"
            PropertyChanges { target: dashBar; opacity: 1 }
        },
    ]

    transitions: [
        Transition {
            from: "OPEN"
            to: "HIDDEN"

            PropertyAnimation {
                target: dashBar
                property: "opacity"
                duration: 250
            }

            onRunningChanged: {
                if (!running) {
                    dashBar.toScript({ dash_window: { event: "finished_hiding" } });
                }
            }
        },
        Transition {
            from: "HIDDEN"
            to: "OPEN"

            PropertyAnimation {
                target: dashBar
                property: "opacity"
                duration: 250
            }
        },
    ]

    Settings {
        category: "Dashboard"
        property alias appsBarOpen: appsBarToggle.checked
    }

    ColumnLayout {
        anchors.top: systemBar.top
        anchors.bottom: systemBar.bottom
        anchors.left: parent.left

        id: systemLeft
        spacing: 8

        Overte.RoundButton {
            icon.source: "../icons/settings_cog.svg"
            icon.color: undefined
            icon.width: 24
            icon.height: 24

            onClicked: {
                dashBar.toScript({
                    dash_window: {
                        event: "spawn_window",
                        title: qsTr("Settings"),
                        source_url: resourceURL("qml/overte/settings/Settings.qml"),
                    },
                });
            }

            Overte.ToolTip { text: qsTr("Settings") }
        }

        Overte.RoundButton {
            icon.source: "../icons/shutdown.svg"
            icon.color: Overte.Theme.paletteActive.buttonText
            icon.width: 24
            icon.height: 24

            backgroundColor: (
                hovered ?
                Overte.Theme.paletteActive.buttonDestructive :
                Overte.Theme.paletteActive.button
            )

            onClicked: WindowScriptingInterface.quit()

            Overte.ToolTip { text: qsTr("Quit") }
        }
    }

    Rectangle {
        anchors.left: systemLeft.right
        anchors.right: systemRight.left
        anchors.top: parent.top
        anchors.leftMargin: 8
        anchors.rightMargin: 8

        implicitHeight: 80 + border.width + 8

        id: systemBar

        color: {
            const c = Overte.Theme.paletteActive.base;
            return Qt.rgba(
                c.r,
                c.g,
                c.b,
                c.a * (Overte.Theme.highContrast ? 1 : 0.9)
            );
        }
        border.color: (
            Overte.Theme.highContrast ?
            Overte.Theme.paletteActive.text :
            Qt.darker(color, Overte.Theme.borderDarker)
        )
        border.width: Overte.Theme.borderWidth
        radius: Overte.Theme.borderRadius

        RowLayout {
            anchors.fill: parent
            anchors.margins: 4

            Repeater {
                model: [
                    {
                        buttonName: qsTr("Contacts"),
                        buttonIcon: "../icons/users.svg",
                        windowName: qsTr("Contacts"),
                        windowSource: resourceURL("qml/overte/contacts/ContactsList.qml"),
                        windowTag: "system contacts"
                    },
                    {
                        buttonName: qsTr("Avatar"),
                        buttonIcon: "../icons/avatars.png",
                        windowName: qsTr("Avatar"),
                        windowSource: resourceURL("qml/overte/avatar_picker/AvatarPicker.qml"),
                        windowTag: "system avatars"
                    },
                    {
                        buttonName: qsTr("Places"),
                        buttonIcon: "../icons/home.svg",
                        windowName: qsTr("Places"),
                        windowSource: resourceURL("qml/overte/place_picker/PlacePicker.qml"),
                        windowTag: "system places"
                    },
                ]

                DashBarButton {
                    required property string buttonName
                    required property string buttonIcon
                    required property string windowName
                    required property string windowSource
                    required property string windowTag

                    Layout.fillWidth: true

                    text: buttonName
                    icon.source: buttonIcon
                    icon.color: undefined

                    onClicked: {
                        dashBar.toScript({
                            dash_window: {
                                event: "spawn_window",
                                title: windowName,
                                source_url: windowSource,
                                tag: windowTag,
                            },
                        });
                    }
                }
            }
        }
    }

    ColumnLayout {
        anchors.top: systemBar.top
        anchors.bottom: systemBar.bottom
        anchors.right: parent.right

        id: systemRight

        Overte.RoundButton {
            checkable: true
            icon.source: checked ? "../icons/speaker_muted.svg" : "../icons/speaker_active.svg"
            icon.width: 24
            icon.height: 24
            icon.color: Overte.Theme.paletteActive.buttonText
            backgroundColor: (
                checked ?
                Overte.Theme.paletteActive.buttonDestructive :
                Overte.Theme.paletteActive.button
            )
            checked: AudioScriptingInterface.muted

            onToggled: AudioScriptingInterface.muted = checked

            Overte.ToolTip { text: parent.checked ? qsTr("Unmute") : qsTr("Mute") }
        }

        Overte.RoundButton {
            checkable: true
            visible: HMD.active
            icon.source: checked ? "../icons/triangle_down.svg" : "../icons/triangle_up.svg"
            icon.width: 24
            icon.height: 24
            backgroundColor: (
                checked ?
                Overte.Theme.paletteActive.highlight :
                Overte.Theme.paletteActive.button
            )
            checked: MyAvatar.standingMode !== 0

            onToggled: {
                MyAvatar.standingMode = (
                    checked ?
                    2 : // ForcedHeight
                    0   // Standing
                );
            }

            Overte.ToolTip { text: parent.checked ? qsTr("Seated") : qsTr("Standing") }
        }
    }

    Rectangle {
        anchors.left: systemBar.left
        anchors.right: systemBar.right
        y: -(height - systemBar.height)
        z: -1
        opacity: 0
        implicitHeight: 96 + border.width + 8

        id: appsBar

        states: State {
            name: "open"
            when: appsBarToggle.checked
            PropertyChanges {
                target: appsBar
                y: systemBar.height + 4
                opacity: 1
            }
        }

        transitions: Transition {
            NumberAnimation {
                properties: Overte.Theme.reducedMotion ? "" : "opacity,y"
                easing.type: Easing.OutExpo
                duration: Overte.Theme.reducedMotion ? 1 : 300
            }
        }

        color: {
            const c = Overte.Theme.paletteActive.base;
            return Qt.rgba(
                c.r,
                c.g,
                c.b,
                c.a * (Overte.Theme.highContrast ? 1 : 0.9)
            );
        }
        border.color: (
            Overte.Theme.highContrast ?
            Overte.Theme.paletteActive.text :
            Qt.darker(color, Overte.Theme.borderDarker)
        )
        border.width: Overte.Theme.borderWidth
        radius: Overte.Theme.borderRadius

        ListView {
            anchors.fill: parent
            anchors.margins: 4

            model: Object.values(dashBar.appButtons)

            delegate: Overte.Button {
                required property var modelData

                text: modelData.text
                checked: modelData.active
                enabled: appsBarToggle.checked

                implicitWidth: 96
                implicitHeight: 96

                backgroundColor: (
                    checked ?
                    Overte.Theme.paletteActive.highlight :
                    Overte.Theme.paletteActive.button
                )
                color: (
                    checked ?
                    Overte.Theme.paletteActive.highlightedText :
                    Overte.Theme.paletteActive.buttonText
                )

                font.pixelSize: Overte.Theme.fontPixelSizeSmall
                icon.source: {
                    let icon = modelData.icons;

                    if (Overte.Theme.darkTheme) {
                        icon = Overte.Theme.highContrast ? icon?.darkContrast : icon?.dark;
                    } else {
                        icon = Overte.Theme.highContrast ? icon?.lightContrast : icon?.light;
                    }

                    return checked ? icon?.active : icon?.idle;
                }
                icon.width: 64
                icon.height: 64
                icon.color: undefined //Overte.Theme.paletteActive.buttonText

                display: Button.TextUnderIcon
                focusPolicy: Qt.NoFocus

                onClicked: dashBar.toScript({
                    app_button: {
                        event: "clicked",
                        ipc_id: modelData.ipcID,
                    },
                });
            }
        }
    }

    Overte.RoundButton {
        anchors.top: appsBar.bottom
        anchors.horizontalCenter: appsBar.horizontalCenter
        anchors.topMargin: 4

        id: appsBarToggle
        focusPolicy: Qt.NoFocus
        icon.source: checked ? "../icons/triangle_up.svg" : "../icons/triangle_down.svg"
        icon.width: 20
        icon.height: 20
        checkable: true
    }
}
