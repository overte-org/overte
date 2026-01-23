import QtQuick 2.12
import QtQuick.Controls 2.12

// NOTE: This is temporary to tie us over until the new dashboard UI.
Button {
    id: control
    text: checked ? "Seated" : "Standing"
    checked: MyAvatar.standingMode !== 0
    checkable: true
    hoverEnabled: true

    onHoveredChanged: {
        if (hovered) {
            Tablet.playSound(1 /* ButtonHover */);
        }
    }

    onToggled: {
        // seated without forced height isn't implemented yet
        const standing = 0;
        const forcedHeight = 2;

        MyAvatar.standingMode = checked ? forcedHeight : standing;
        Tablet.playSound(0 /* ButtonClick */);
    }

    contentItem: Row {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        spacing: 6

        Image {
            anchors.verticalCenter: parent.verticalCenter
            source: control.checked ? "../../../icons/sitToggle-sitting.svg" : "../../../icons/sitToggle-standing.svg"
            sourceSize.width: 24
            sourceSize.height: 24
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: control.text
            font.family: "Rawline"
            font.pixelSize: 16
            color: "white"
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }
    }

    background: Rectangle {
        implicitHeight: 44
        implicitWidth: 120

        color: "transparent"
        border.color: (
            control.hovered ?
            "white" :
            (control.checked ? "#60d0b0" : "#404040")
        )
        border.width: 2
        radius: 4
    }
}
