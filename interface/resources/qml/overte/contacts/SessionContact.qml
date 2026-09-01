import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import ".." as Overte

Rectangle {
    id: control
    color: index % 2 === 0 ? Overte.Theme.paletteActive.base : Overte.Theme.paletteActive.alternateBase
    implicitWidth: ListView.view.contentWidth
    implicitHeight: 96

    required property int index

    required property string uuid
    required property string name
    required property real volume

    readonly property string username: contactsList.adminData[uuid]?.username ?? ""
    readonly property url badgeIconSource: contactsList.adminData[uuid]?.badge ?? ""

    // TODO: does this work?
    function dbToReal(x) {
        return 1.0 - (x / 100);
    }

    function realToDb(x) {
        return (1.0 - x) * 100;
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 4
        anchors.leftMargin: 8
        anchors.rightMargin: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            Image {
                Layout.leftMargin: 6
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                source: badgeIconSource
            }

            ColumnLayout {
                Layout.fillWidth: true

                Overte.Label {
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    text: name
                }

                Overte.BodyText {
                    font.pixelSize: Overte.Theme.fontPixelSizeSmall
                    visible: username !== ""
                    //elide: Text.ElideRight
                    text: username
                    palette.buttonText: Overte.Theme.paletteActive.link
                    opacity: Overte.Theme.highContrast ? 1.0 : 0.7
                }
            }

            Item { Layout.fillWidth: true }

            // TODO: find a way of disabling this if the target is already in the account
            // contacts, might be tricky with how usernames are hidden from non-admins
            /*Overte.RoundButton {
                icon.source: "../icons/add_friend.svg"
                icon.width: 24
                icon.height: 24

                backgroundColor: hovered ? Overte.Theme.paletteActive.buttonAdd : Overte.Theme.paletteActive.button

                Overte.ToolTip { text: qsTr("Send contact request") }

                onClicked: console.warn("TODO")
            }*/
        }

        RowLayout {
            Overte.RoundButton {
                icon.source: volumeSlider.value === 0.0 ? "../icons/speaker_muted.svg" : "../icons/speaker_active.svg"
                icon.width: 24
                icon.height: 24
                icon.color: Overte.Theme.paletteActive.buttonText

                onClicked: volumeSlider.value = 0.0
            }

            Overte.Slider {
                Layout.fillWidth: true
                id: volumeSlider
                from: 0.0
                to: 1.2
                stepSize: 0.1
                snapMode: Slider.SnapAlways
                value: dbToReal(volume)

                onMoved: Users.setAvatarGain(control.uuid, realToDb(value))
            }

            Overte.Label {
                Layout.preferredWidth: Overte.Theme.fontPixelSize * 3
                //text: `${Math.round(volumeSlider.value * 100)}%`
                // why did hifi have to use decibels for avatar volume??
                text: `${Math.round(volumeSlider.value * 100)}%\n${volume} db`
            }
        }
    }
}
