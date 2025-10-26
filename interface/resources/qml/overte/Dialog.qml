import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs

import "."

Rectangle {
	id: dialog
	visible: false

	default property alias content: dialogWindow.children

	property int maxWidth: 480
	property int maxHeight: -1

	property bool shadedBackground: true

	function open() {
		visible = true;
		opacity = Theme.reducedMotion ? 1 : 0;
	}

	function close() {
		visible = false;
		opacity = 0;
	}

	color: !shadedBackground ? "transparent" : Theme.paletteActive.dialogShade

	opacity: Theme.reducedMotion ? 1 : 0
	OpacityAnimator on opacity {
		from: Theme.reducedMotion ? 1 : 0
		to: 1
		duration: 150
		easing.type: Easing.InQuad
		running: dialog.visible
	}

	// block any inputs from underneath
	MouseArea {
		enabled: parent.visible
		anchors.fill: parent
		hoverEnabled: true
	}

	Rectangle {
		id: dialogWindow
		width: Math.min(
			(maxWidth == -1 ? Infinity : maxWidth),
			children[0].implicitWidth + (children[0].anchors.margins * 2),
			parent.width - 8
		)
		height: Math.min(
			(maxHeight == -1 ? Infinity : maxHeight),
			children[0].implicitHeight + (children[0].anchors.margins * 2),
			parent.height - 8
		)
		anchors.centerIn: parent

		color: Theme.paletteActive.base
		radius: 8
		border.width: 2
		border.color: Theme.highContrast ? Theme.paletteActive.text : Qt.darker(Theme.paletteActive.base, Theme.borderDarker)

		OpacityAnimator on opacity {
			from: Theme.reducedMotion ? 1 : 0
			to: 1
			easing.type: Easing.OutQuad
			duration: 200
			running: dialog.visible
		}

		ScaleAnimator on scale {
			from: Theme.reducedMotion ? 1 : 0.9
			to: 1
			easing.type: Easing.OutQuad
			duration: 200
			running: dialog.visible
		}
	}
}
