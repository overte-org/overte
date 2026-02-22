import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../" as Overte

RowLayout {
    property alias text: labelItem.text
    property alias value: spinboxItem.value
    property alias from: spinboxItem.from
    property alias to: spinboxItem.to
    property alias enabled: spinboxItem.enabled

    id: item
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.margins: 16
    spacing: 16

    Overte.Label {
        // equally sized items
        Layout.preferredWidth: 1
        Layout.fillWidth: true

        id: labelItem
        wrapMode: Text.Wrap
    }

    Overte.SpinBox {
        Layout.alignment: Qt.AlignRight

        id: spinboxItem
        editable: true
    }
}
