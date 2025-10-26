import QtQuick
import QtQuick.Controls
import "."

TabButton {
	id: button
	property color backgroundColor: Theme.paletteActive.base

	readonly property int borderWidth: checked ? Theme.borderWidth * 2 : Theme.borderWidth

	font.family: Theme.fontFamily
	font.pixelSize: Theme.fontPixelSize
	horizontalPadding: 12
	verticalPadding: 8

	anchors.top: parent.top
	anchors.bottom: parent.bottom
	anchors.topMargin: checked ? 2 : 6
	anchors.bottomMargin: -borderWidth
	implicitHeight: Theme.fontPixelSize + 16

	contentItem: Text {
		text: button.text
		font: button.font
		color: Theme.paletteActive.text
		opacity: enabled ? 1.0 : 0.3
		horizontalAlignment: Text.AlignHCenter
		verticalAlignment: Text.AlignVCenter
		elide: Text.ElideRight
	}

	background: Rectangle {
		id: buttonBg
		border.width: parent.borderWidth
		border.color: {
			if (parent.activeFocus) {
				return Theme.paletteActive.focusRing;
			} else if (parent.checked) {
				return Theme.paletteActive.highlight;
			} else if (Theme.highContrast) {
				return Theme.paletteActive.buttonText;
			} else {
				return Qt.darker(parent.backgroundColor, Theme.borderDarker);
			}
		}
		color: parent.backgroundColor;
		gradient: Gradient {
			GradientStop {
				position: 0.0; color: Qt.lighter(buttonBg.color, 1.2)
			}
			GradientStop {
				position: 1.0; color: buttonBg.color
			}
		}
	}
}
