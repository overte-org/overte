import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import ".." as Overte
import "."

Item {
    function fromScript(rawMsg) {
        console.log(`QML: ${rawMsg}`);
    }

    function toScript(msg) {
        eventBridge.emitWebEvent(JSON.stringify(msg));
    }

    Component.onCompleted: {
        eventBridge.scriptEventReceived.connect(fromScript);
    }

    id: dashBar
    anchors.centerIn: parent

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
                     { name: "Settings", windowTitle: "Settings", windowSource: "qrc:///qml/overte/settings/Settings.qml" },
                     { name: "Avatar", windowTitle: "Avatar", windowSource: "qrc:///qml/overte/avatar_picker/AvatarPicker.qml" },
                     "Banana",
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
                             event: "spawn_window",
                             title: windowTitle,
                             qmlSource: windowSource,
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
