import QtQuick
import QtQuick.Controls
import "."

TabBar {
	id: tabBar
	spacing: 2
	clip: true

	background: Item {
		Rectangle { anchors.fill: parent; color: Theme.paletteActive.base }
		Rectangle {
			anchors.left: parent.left
			anchors.right: parent.right
			anchors.bottom: parent.bottom
			height: Theme.borderWidth
			color: Qt.darker(Theme.paletteActive.base)
		}
	}

	implicitHeight: Theme.fontPixelSize + 16
}
