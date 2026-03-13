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
                buttonText: qsTr("Open")
                onClicked: {
                    ScriptDiscoveryService.loadScript("/~//system/dashboard/scriptLog.js", false);
                }
            }

            ButtonRow {
                text: qsTr("Running Scripts")
                description: qsTr("Opens a window that shows the currently running scripts, as well as ways of starting new ones.")
                buttonText: qsTr("Open")
                onClicked: {
                    Messages.sendLocalMessage("Dash DevTools", JSON.stringify({
                        open_window: "running scripts",
                    }));
                }
            }

            ButtonRow {
                text: qsTr("Server Assets")
                description: qsTr("Domain servers can host assets themselves, without the need for a web server. This lets you explore, upload, and get URLs for assets stored on the domain server.")
                buttonText: qsTr("Open")
                onClicked: {
                    Messages.sendLocalMessage("Dash DevTools", JSON.stringify({
                        open_window: "asset server",
                    }));
                }
            }
        }
    }
}
