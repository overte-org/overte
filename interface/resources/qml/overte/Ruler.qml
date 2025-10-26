import QtQuick

import "."

Item {
	property color color: (
		Theme.highContrast ?
		Theme.paletteActive.text :
		Theme.paletteActive.base
	)

	implicitHeight: 4

	Rectangle {
		x: 0
		y: 0
		height: Math.floor(parent.height / 2)
		width: parent.width
		color: Qt.darker(parent.color, Theme.depthDarker)
	}

	Rectangle {
		x: 0
		y: Math.floor(parent.height / 2)
		height: Math.floor(parent.height / 2)
		width: parent.width
		color: Qt.lighter(parent.color, Theme.depthLighter)
	}
}
