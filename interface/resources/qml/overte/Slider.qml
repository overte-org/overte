import QtQuick
import QtQuick.Controls

import "."

Slider {
	id: control

	background: Rectangle {
		y: control.topPadding + control.availableHeight / 2 - height / 2
		implicitWidth: 200
		height: 8
		radius: height / 2
		color: Theme.paletteActive.base

		border.width: Theme.borderWidth
		border.color: Qt.darker(color, Theme.borderDarker)

		Rectangle {
			width: control.visualPosition * parent.width
			height: parent.height
			radius: parent.radius
			color: Theme.paletteActive.highlight

			border.color: Qt.darker(color, Theme.borderDarker)
			border.width: Theme.borderWidth
		}
	}

	handle: Rectangle {
		implicitWidth: 26
		implicitHeight: 26
		radius: height / 2

		x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
		y: control.topPadding + control.availableHeight / 2 - height / 2

		id: handle
		border.width: control.activeFocus ? Theme.borderWidthFocused : Theme.borderWidth
		border.color: (
			control.activeFocus ?
			Theme.paletteActive.focusRing :
			(
				Theme.highContrast ?
				Theme.paletteActive.buttonText :
				Qt.darker(Theme.paletteActive.button, Theme.borderDarker)
			)
		)
		color: {
			if (control.hovered && control.enabled) {
				return Qt.lighter(Theme.paletteActive.button, Theme.hoverLighter);
			} else if (!control.enabled) {
				return Theme.paletteActive.base;
			} else {
				return Theme.paletteActive.button;
			}
		}
		gradient: Gradient {
			GradientStop {
				position: 0.0; color: Qt.lighter(handle.color, control.enabled ? 1.1 : 1.0)
			}
			GradientStop {
				position: 0.5; color: handle.color
			}
			GradientStop {
				position: 1.0; color: Qt.darker(handle.color, control.enabled ? 1.1 : 1.0)
			}
		}
	}
}
