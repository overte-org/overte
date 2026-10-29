import QtQuick
import QtQuick.Layouts

import "../" as Overte
import "../dialogs" as OverteDialogs

ColumnLayout {
    property alias text: labelItem.text
    property alias value: textFieldItem.text
    property bool enabled: true

    id: item
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.margins: 16
    spacing: 4

    Overte.Label {
        Layout.alignment: Qt.AlignBottom
        id: labelItem
        wrapMode: Text.Wrap
    }

    RowLayout {
        Layout.fillWidth: true

        Overte.TextField {
            Layout.fillWidth: true

            id: textFieldItem
            enabled: item.enabled
        }

        Overte.RoundButton {
            enabled: item.enabled

            icon.source: "../icons/folder.svg"
            icon.width: 24
            icon.height: 24

            onClicked: settingsRoot.openFolderPicker(
                folder => {
                    value = folder;
                },
                value
            );
        }
    }
}
