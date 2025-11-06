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
    required property string description

    implicitWidth: GridView.view.cellWidth
    implicitHeight: GridView.view.cellHeight

    Overte.Button {
        anchors.fill: parent
        anchors.margins: 4

        id: avatarButton

        Overte.ToolTip {
            visible: description !== "" && parent.hovered
            text: description
            delay: 500
        }

        Image {
            id: buttonIcon
            source: item.iconUrl
            fillMode: Image.PreserveAspectFit

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: buttonLabel.top
            anchors.margins: 4
        }

        Overte.Label {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: buttonLabel.top
            anchors.margins: 4
            visible: buttonIcon.status !== Image.Ready
            opacity: Overte.Theme.highContrast ? 1.0 : 0.6
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: buttonIcon.status === Image.Loading ? qsTr("Loading…") : qsTr("No icon")
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

        onClicked: AvatarBookmarks.loadBookmark(item.name)
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

        onClicked: {
            root.requestEdit(item.index);
        }
    }
}
