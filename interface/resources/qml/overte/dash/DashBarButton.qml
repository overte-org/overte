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
    implicitWidth: 72
    Layout.maximumWidth: implicitWidth
    Layout.fillWidth: false
    Layout.fillHeight: true

    signal clicked()
    signal toggled()

    ColumnLayout {
        anchors.centerIn: parent

        Overte.RoundButton {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom

            id: button

            implicitWidth: 48
            implicitHeight: 48
            backgroundColor: (
                checked ?
                Overte.Theme.paletteActive.highlight :
                Overte.Theme.paletteActive.button
            )

            checkable: false
            checked: false

            icon.source: "../icons/delete.svg"
            icon.width: 24
            icon.height: 24

            onClicked: barButton.clicked()
            onToggled: barButton.toggled()
        }

        Overte.Label {
            Layout.fillWidth: true

            id: label
            text: "NO LABEL!"

            font.pixelSize: Overte.Theme.fontPixelSizeSmall
            wrapMode: Text.Wrap

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignHCenter

            Rectangle {
                z: -1

                // FIXME: weird magic constants
                x: -4
                y: -4
                width: parent.width + 8
                height: parent.height + 8
                radius: Overte.Theme.borderRadius

                color: {
                    if (Overte.Theme.highContrast) {
                        return Overte.Theme.darkMode ? "black" : "white";
                    } else {
                        return Overte.Theme.darkMode ? "#d0000000" : "#e0ffffff"
                    }
                }
            }
        }
    }
}
