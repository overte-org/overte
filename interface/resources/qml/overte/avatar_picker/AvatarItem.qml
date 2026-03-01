import QtQuick
import QtQuick.Controls
import QtQuick.VectorImage

import "../" as Overte

Item {
    id: item
    required property int index
    required property string name
    required property url avatarUrl
    required property url iconUrl
    required property list<string> tags

    implicitWidth: GridView.view.cellWidth
    implicitHeight: GridView.view.cellHeight

    Overte.Button {
        anchors.fill: parent
        anchors.margins: 4

        id: avatarButton

        Image {
            id: buttonIcon
            source: item.iconUrl
            fillMode: Image.PreserveAspectFit

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: buttonLabel.top
            anchors.margins: 4

            // placeholder icon
            Image {
                anchors.centerIn: parent
                width: 64
                height: 64

                fillMode: Image.PreserveAspectFit
                source: "../icons/avatars.png"
                sourceSize.width: width
                sourceSize.height: height
                visible: buttonIcon.status === Image.Error || buttonIcon.status === Image.Null
            }
        }

        Overte.Label {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: buttonLabel.top
            anchors.margins: 4
            visible: buttonIcon.status === Image.Loading
            opacity: Overte.Theme.highContrast ? 1.0 : 0.6
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: qsTr("Loading…")
        }

        Overte.Label {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 4

            text: item.name
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignBottom
            wrapMode: Text.Wrap

            id: buttonLabel
        }

        onClicked: root.loadBookmark(item.name)
    }

    Overte.RoundButton {
        id: deleteButton
        anchors.top: parent.top
        anchors.left: parent.left

        visible: root.editable && (hovered || editButton.hovered || avatarButton.hovered)
        opacity: hovered || Overte.Theme.highContrast ? 1.0 : 0.75
        backgroundColor: Overte.Theme.paletteActive.buttonDestructive

        implicitWidth: 44
        implicitHeight: 44

        icon.source: "../icons/delete.svg"
        icon.width: 32
        icon.height: 32
        icon.color: Overte.Theme.paletteActive.buttonText

        Overte.ToolTip { text: qsTr("Delete") }

        onClicked: root.requestDelete(item.index, item.name)
    }

    Overte.RoundButton {
        id: editButton
        anchors.top: parent.top
        anchors.right: parent.right

        visible: root.editable && (hovered || deleteButton.hovered || avatarButton.hovered)
        opacity: hovered || Overte.Theme.highContrast ? 1.0 : 0.75
        backgroundColor: Overte.Theme.paletteActive.buttonInfo

        implicitWidth: 44
        implicitHeight: 44

        icon.source: "../icons/pencil.svg"
        icon.width: 32
        icon.height: 32
        icon.color: Overte.Theme.paletteActive.buttonText
    
        Overte.ToolTip { text: qsTr("Edit") }

        onClicked: root.requestEdit(item.index)
    }
}
