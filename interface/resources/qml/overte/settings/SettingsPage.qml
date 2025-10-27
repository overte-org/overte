import QtQuick
import QtQuick.Controls

import "../" as Overte

ScrollView {
    default property alias children: column.children

    ScrollBar.vertical: Overte.ScrollBar {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        policy: ScrollBar.AsNeeded
    }
    contentWidth: width - ScrollBar.vertical.width

    Column {
        id: column
        anchors.fill: parent
        spacing: 8
        padding: 16
    }
}
