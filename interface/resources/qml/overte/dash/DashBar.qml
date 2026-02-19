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

    RowLayout {
        id: row
        anchors.fill: parent

        DashBarButton {
            text: qsTr("Quit")
            icon.source: "../icons/close.svg"
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
                        source_url: "qrc:///qml/overte/settings/Settings.qml",
                    },
                });
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: (Overte.Theme.borderWidth * 2) + 8 + 96 + Overte.Theme.scrollbarWidth

            border.width: Overte.Theme.borderWidth
            border.color: Qt.darker(color, Overte.Theme.borderDarker)
            radius: Overte.Theme.borderRadius
            color: Overte.Theme.paletteActive.base

            ListView {
                anchors.fill: parent
                anchors.margins: Overte.Theme.borderWidth * 2
                orientation: ListView.Horizontal
                spacing: 4
                clip: true

                 model: [
                     { name: "Places", windowTitle: "Places", windowSource: "qrc://scripts/system/places/places.html" },
                     { name: "Contacts", windowTitle: "Contacts", windowSource: "qrc:///qml/overte/contacts/ContactsList.qml" },
                     { name: "Avatar", windowTitle: "Avatar", windowSource: "qrc:///qml/overte/avatar_picker/AvatarPicker.qml" },
                     { name: "Widget Zoo", windowTitle: "Widget Zoo", windowSource: "qrc:///qml/overte/WidgetZoo.qml" },
                 ]

                 delegate: Overte.Button {
                     required property string name
                     required property string windowTitle
                     required property string windowSource

                     text: name
                     implicitWidth: 96
                     implicitHeight: 96
                     font.pixelSize: Overte.Theme.fontPixelSizeSmall

                     onClicked: {
                         dashBar.toScript({
                             dash_window: {
                                 event: "spawn_window",
                                 title: windowTitle,
                                 source_url: windowSource,
                             },
                         });
                     }
                 }

                 ScrollBar.horizontal: Overte.ScrollBar {
                     policy: ScrollBar.AlwaysOn
                 }
             }
         }

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
            text: checked ? qsTr("Seated") : qsTr("Standing")
            icon.source: checked ? "../icons/triangle_down.svg" : "../icons/triangle_up.svg"
            backgroundColor: (
                checked ?
                Overte.Theme.paletteActive.highlight :
                Overte.Theme.paletteActive.button
            )

            onToggled: console.error(`TODO: sitting mode ${checked}`)
        }
    }
}
