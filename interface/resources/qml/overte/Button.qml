import QtQuick
import QtQuick.Controls
import "."

Button {
	id: button
	property color backgroundColor: Theme.paletteActive.button
	property color color: Theme.paletteActive.buttonText

	palette.buttonText: color
	font.family: Theme.fontFamily
	font.pixelSize: Theme.fontPixelSize
	horizontalPadding: 12
	verticalPadding: 8
	hoverEnabled: true

	opacity: enabled ? 1.0 : 0.5

	background: Rectangle {
		opacity: flat ? 0.0 : 1.0

		id: buttonBg
		radius: Theme.borderRadius
		border.width: button.activeFocus ? Theme.borderWidthFocused : Theme.borderWidth
		border.color: (
			button.activeFocus ?
			Theme.paletteActive.focusRing :
			(
				Theme.highContrast ?
				Theme.paletteActive.buttonText :
				Qt.darker(button.backgroundColor, Theme.borderDarker)
			)
		)
		color: {
			if (button.down || button.checked) {
				return Qt.darker(button.backgroundColor, Theme.checkedDarker);
			} else if (button.hovered && button.enabled) {
				return Qt.lighter(button.backgroundColor, Theme.hoverLighter);
			} else {
				return button.backgroundColor;
			}
		}
		gradient: Gradient {
			GradientStop {
				position: 0.0; color: Qt.lighter(buttonBg.color, (button.down || button.checked) ? 0.9 : 1.1)
			}
			GradientStop {
				position: 0.5; color: buttonBg.color
			}
			GradientStop {
				position: 1.0; color: Qt.darker(buttonBg.color, (button.down || button.checked) ? 0.9 : 1.1)
			}
		}
	}
}
