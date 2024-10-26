import Hifi 1.0 as Hifi
import QtQuick 2.5
import stylesUit 1.0

ShadowRectangle {
    id: header
    anchors.left: parent.left
    anchors.right: parent.right
    height: 60

    property alias pageTitle: title.text
    property alias settingsButtonVisible: settingsButton.visible

    signal settingsClicked;

    AvatarAppStyle {
        id: style
    }

    color: style.colors.lightGrayBackground

    // TextStyle6
    RalewaySemiBold {
        id: title
        size: 22;
        x: 20
        anchors.verticalCenter: parent.verticalCenter
        text: 'Avatar'
    }

    HiFiGlyphs {
        id: settingsButton
        anchors.right: parent.right
        anchors.rightMargin: 30
        anchors.verticalCenter: parent.verticalCenter
        text: "&"

        MouseArea {
            id: settingsMouseArea
            anchors.fill: parent
            onClicked: {
                settingsClicked();
            }
        }
    }
}
