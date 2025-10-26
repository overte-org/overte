import QtQuick
import QtQuick.Layouts

import "../" as Overte

Overte.Label {
	id: item
	anchors.left: parent.left
	anchors.right: parent.right
	anchors.margins: 16
	wrapMode: Text.Wrap
	font.pixelSize: Overte.Theme.fontPixelSizeSmall
}
