import QtQuick
import QtQuick.Controls
import "."

ComboBox {
	id: control

	property color backgroundColor: flat ? "#00000000" : Theme.paletteActive.window
	property color color: Theme.paletteActive.windowText

	implicitHeight: Theme.fontPixelSize * 2

	font.pixelSize: Theme.fontPixelSize
	font.family: Theme.fontFamily

	indicator: Button {
		anchors.right: control.right
		width: control.height
		height: control.height

		horizontalPadding: 2
		verticalPadding: 2
		focusPolicy: Qt.NoFocus

		icon.source: "./icons/triangle_down.svg"
		icon.width: 24
		icon.height: 24
		icon.color: Theme.paletteActive.buttonText

		onClicked: {
			control.forceActiveFocus();

			if (control.popup.opened) {
				control.popup.close();
			} else {
				control.popup.open();
			}
		}
	}

	contentItem: Text {
		leftPadding: 6
		rightPadding: control.indicator.width + control.spacing
		text: control.displayText
		font: control.font
		color: control.color
		verticalAlignment: Text.AlignVCenter
		elide: Text.ElideRight
		opacity: enabled ? 1.0 : 0.5
	}

	background: Rectangle {
		opacity: enabled ? 1.0 : 0.5
		width: (control.width - control.indicator.width) + Theme.borderWidth
		radius: Theme.borderRadius
		border.width: control.activeFocus ? Theme.borderWidthFocused : Theme.borderWidth
		border.color: {
			if (control.activeFocus) {
				return Theme.paletteActive.focusRing;
			} else if (control.flat) {
				return "#00000000";
			} else if (Theme.highContrast) {
				return parent.color;
			} else {
				return Qt.darker(control.backgroundColor, Theme.borderDarker);
			}
		}
		gradient: Gradient {
			GradientStop { position: 0.0; color: Qt.darker(control.backgroundColor, control.flat ? 1.0 : 1.02) }
			GradientStop { position: 0.5; color: control.backgroundColor }
			GradientStop { position: 1.0; color: Qt.lighter(control.backgroundColor, control.flat ? 1.0 : 1.02) }
		}
	}

	delegate: ItemDelegate {
		id: delegate

		required property var model
		required property int index

		width: control.width
		contentItem: Text {
			text: delegate.model[control.textRole]
			color: highlighted ? Theme.paletteActive.highlightedText : Theme.paletteActive.tooltipText
			font: control.font
			elide: Text.ElideRight
			verticalAlignment: Text.AlignVCenter
		}

		background: Rectangle {
			color: highlighted ? Theme.paletteActive.highlight : Theme.paletteActive.tooltip
		}

		highlighted: control.highlightedIndex === index
	}

	popup: Popup {
		y: control.height - Theme.borderWidth
		width: control.width + (contentItem.ScrollBar.vertical.opacity > 0.0 ? contentItem.ScrollBar.vertical.width : 0)
		height: Math.min(contentItem.implicitHeight, control.Window.height - topMargin - bottomMargin)
		padding: 3

		contentItem: ListView {
			clip: true
			implicitHeight: contentHeight + (parent.padding * 2)
			model: control.popup.visible ? control.delegateModel : null
			currentIndex: control.highlightedIndex

			ScrollBar.vertical: ScrollBar {
				interactive: false
			}
		}

		background: Item {
			// drop shadow
			Rectangle {
				x: 3
				y: 3
				width: parent.width
				height: parent.height

				radius: Theme.borderRadius
				color: "#a0000000"
			}

			Rectangle {
				x: 0
				y: 0
				width: parent.width
				height: parent.height

				radius: Theme.borderRadius
				border.width: Theme.borderWidth
				border.color: Qt.darker(Theme.paletteActive.tooltip, 2.5)
				color: Theme.paletteActive.tooltip
			}
		}
	}
}

