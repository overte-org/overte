import QtQuick
import QtQuick.Layouts

import "../" as Overte

ColumnLayout {
    property alias text: labelItem.text

    id: item
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.margins: 16
    spacing: 2
    height: Overte.Theme.fontPixelSize * 3

    Overte.Label {
        id: labelItem
        Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
        opacity: Overte.Theme.highContrast ? 1.0 : 0.6
        wrapMode: Text.Wrap
    }

    Overte.Ruler {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignBottom
    }
}
