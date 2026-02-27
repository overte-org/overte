import QtQuick
import QtQuick.Controls

import ".." as Overte

Rectangle {
    anchors.fill: parent
    color: Overte.Theme.paletteActive.base

    component ButtonRow: Column {
        required property string text
        required property string buttonText
        property string description

        width: scrollView.contentWidth - itemColumn.anchors.leftMargin - itemColumn.anchors.rightMargin
        spacing: 4

        signal clicked()

        Overte.Label {
            width: parent.width
            text: parent.text
            wrapMode: Text.Wrap
        }

        Overte.Label {
            width: parent.width
            text: parent.description
            wrapMode: Text.Wrap
            font.pixelSize: Overte.Theme.fontPixelSizeSmall
            visible: text !== ""
        }

        Overte.Button {
            width: parent.width
            text: parent.buttonText
            onClicked: parent.clicked()
        }
    }

    ScrollView {
        id: scrollView
        anchors.fill: parent
        ScrollBar.vertical: Overte.ScrollBar {
            interactive: true
            policy: ScrollBar.AlwaysOn
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
        }
        contentWidth: width - ScrollBar.vertical.width

        Column {
            id: itemColumn
            anchors.fill: parent
            anchors.margins: 8
            spacing: 16

            ButtonRow {
                text: qsTr("Script Log")
                description: qsTr("Lets you view messages logged by scripts, in a dash window. You can also check Developer > Log in the desktop menubar for a separate native window version.")
                buttonText: qsTr("Open script log window")
                onClicked: {
                    ScriptDiscoveryService.loadScript("/~//system/dashboard/scriptLog.js", false);
                }
            }

            ButtonRow {
                text: qsTr("Legacy Tablet")
                description: qsTr("Older content may not work with the dashboard UI. This lets you open the old tablet to interact with that older content. Content relying on the tablet may stop working entirely some day.")
                buttonText: qsTr("Open tablet")
                onClicked: HMD.openTablet()
            }
        }
    }
}
