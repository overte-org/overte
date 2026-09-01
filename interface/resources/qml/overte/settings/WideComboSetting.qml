import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../" as Overte

ColumnLayout {
    property alias text: labelItem.text
    property alias model: comboItem.model
    property alias textRole: comboItem.textRole
    property alias valueRole: comboItem.valueRole
    property alias currentIndex: comboItem.currentIndex
    property alias enabled: comboItem.enabled

    id: item
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.margins: 16
    spacing: 4

    Overte.Label {
        Layout.fillWidth: true

        id: labelItem
    }

    Overte.ComboBox {
        Layout.fillWidth: true

        id: comboItem
        font.pixelSize: Overte.Theme.fontPixelSizeSmall
    }
}
