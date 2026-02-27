import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import ".." as Overte
import "."

Item {
    function fromScript(rawMsg) {
        let msg;
        try {
            msg = JSON.parse(rawMsg);
        } catch (_) {
            return;
        }

        if (msg?.dash_window?.event === "hide") {
            dashBar.state = "HIDDEN";
        } else if (msg?.dash_window?.event === "unhide") {
            dashBar.state = "OPEN";
        }
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

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        implicitHeight: childrenRect.height + border.width + 8

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

        Row {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.margins: 4

            DashBarButton {
                text: qsTr("Quit")
                icon.source: "../icons/shutdown.svg"
                icon.color: Overte.Theme.paletteActive.buttonText
                backgroundColor: (
                    hovered ?
                    Overte.Theme.paletteActive.buttonDestructive :
                    Overte.Theme.paletteActive.button
                )

                onClicked: WindowScriptingInterface.quit()
            }

            DashBarButton {
                text: qsTr("Settings")
                icon.source: "../icons/settings_cog.svg"
                icon.color: Overte.Theme.paletteActive.buttonText

                onClicked: {
                    dashBar.toScript({
                        dash_window: {
                            event: "spawn_window",
                            title: qsTr("Settings"),
                            source_url: resourceURL("qml/overte/settings/Settings.qml"),
                        },
                    });
                }
            }

            DashBarButton {
                visible: SettingsInterface.getValue("Settings/Developer Menu", false)
                text: qsTr("Dev Tools")
                icon.source: "../icons/dev_tools.png"
                icon.color: undefined

                onClicked: {
                    dashBar.toScript({
                        dash_window: {
                            event: "spawn_window",
                            title: qsTr("Developer Tools"),
                            source_url: resourceURL("qml/overte/dash/DevTools.qml"),
                        },
                    });
                }
            }
        }

        Row {
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.margins: 4

            Repeater {
                model: [
                    {
                        buttonName: qsTr("Places"),
                        buttonIcon: "../icons/home.svg",
                        windowName: qsTr("Places"),
                        windowSource: resourceURL("qml/overte/place_picker/PlacePicker.qml"),
                        windowTag: "system places"
                    },
                    {
                        buttonName: qsTr("Contacts"),
                        buttonIcon: "../icons/users.svg",
                        windowName: qsTr("Contacts"),
                        windowSource: resourceURL("qml/overte/contacts/ContactsList.qml"),
                        windowTag: "system contacts"
                    },
                    {
                        buttonName: qsTr("Avatar"),
                        buttonIcon: "../icons/add_friend.svg",
                        windowName: qsTr("Avatar"),
                        windowSource: resourceURL("qml/overte/avatar_picker/AvatarPicker.qml"),
                        windowTag: "system avatars"
                    },
                ]

                DashBarButton {
                    required property string buttonName
                    required property string buttonIcon
                    required property string windowName
                    required property string windowSource
                    required property string windowTag

                    text: buttonName
                    icon.source: buttonIcon
                    icon.color: Overte.Theme.paletteActive.buttonText

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

        Row {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 4

            DashBarButton {
                checkable: true
                text: checked ? qsTr("Unmute") : qsTr("Mute")
                icon.source: checked ? "../icons/speaker_muted.svg" : "../icons/speaker_active.svg"
                backgroundColor: (
                    checked ?
                    Overte.Theme.paletteActive.buttonDestructive :
                    Overte.Theme.paletteActive.button
                )
                checked: AudioScriptingInterface.muted

                onToggled: AudioScriptingInterface.muted = checked
            }

            DashBarButton {
                checkable: true
                visible: HMD.active
                text: checked ? qsTr("Seated") : qsTr("Standing")
                icon.source: checked ? "../icons/triangle_down.svg" : "../icons/triangle_up.svg"
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
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
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

            model: [
                {
                    buttonName: qsTr("More…"),
                    buttonIcon: "../icons/plus.svg",
                    windowName: qsTr("More Apps"),
                    windowSource: resourceURL("qml/overte/more_apps/MoreApps.qml"),
                    windowTag: "system more apps"
                },
            ]

            delegate: Overte.Button {
                required property string buttonName
                required property string buttonIcon
                required property string windowName
                required property string windowSource
                required property string windowTag

                implicitWidth: 96
                implicitHeight: 96

                text: buttonName
                font.pixelSize: Overte.Theme.fontPixelSizeSmall
                icon.source: buttonIcon
                icon.width: 64
                icon.height: 64
                icon.color: Overte.Theme.paletteActive.buttonText

                display: Button.TextUnderIcon
                focusPolicy: Qt.NoFocus

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
