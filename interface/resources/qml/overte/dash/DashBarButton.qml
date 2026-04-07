import QtQuick
import QtQuick.Layouts

import ".." as Overte

Item {
    property alias text: label.text
    property alias icon: button.icon
    property alias hovered: button.hovered
    property alias checked: button.checked
    property alias checkable: button.checkable
    property alias backgroundColor: button.backgroundColor

    id: barButton
    implicitWidth: 80
    implicitHeight: 80

    signal clicked()
    signal toggled()

    ColumnLayout {
        anchors.centerIn: parent

        Overte.RoundButton {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom

            id: button

            implicitWidth: 56
            implicitHeight: 56
            backgroundColor: (
                checked ?
                Overte.Theme.paletteActive.highlight :
                Overte.Theme.paletteActive.button
            )

            focusPolicy: Qt.NoFocus
            checkable: false
            checked: false

            icon.source: "../icons/delete.svg"
            icon.color: "transparent"
            icon.width: 40
            icon.height: 40

            onClicked: barButton.clicked()
            onToggled: barButton.toggled()
        }

        Overte.Label {
            Layout.fillWidth: true

            id: label
            visible: text !== ""

            font.pixelSize: Overte.Theme.fontPixelSizeSmall
            wrapMode: Text.Wrap

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignHCenter
        }
    }
}
