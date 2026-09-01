import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../" as Overte

Rectangle {
    id: root
    implicitWidth: 480
    implicitHeight: 360
    color: Overte.Theme.paletteActive.base

    ScrollView {
        anchors.fill: parent

        ScrollBar.vertical: Overte.ScrollBar {
            policy: ScrollBar.AsNeeded
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
        }

        contentWidth: width - ScrollBar.vertical.width

        Column {
            spacing: 16
            padding: 8

            Image {
                width: root.width
                height: sourceSize.height
                fillMode: Image.PreserveAspectFit
                source: Overte.Theme.darkMode ? "./assets/logo_dark.png" : "./assets/logo_light.png"
            }

            Overte.Label {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: root.width / 8
                anchors.rightMargin: root.width / 8
                text: qsTr("Welcome to Overte!\nThis is an offline tutorial world.\n\nTODO: More intro text")
                wrapMode: Text.Wrap
            }
        }
    }
}
