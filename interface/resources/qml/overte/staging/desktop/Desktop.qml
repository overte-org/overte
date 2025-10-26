import QtQuick

import ".." as Overte
import "."

//Item {
Rectangle {
	color: "#303030"

	anchors.fill: parent
	implicitWidth: 800
	implicitHeight: 600

	id: desktopRoot

	Item {
		id: toolbarContainer
		anchors.horizontalCenter: desktopRoot.horizontalCenter
		anchors.bottom: toolbarToggleButton.top
		anchors.margins: 4
		height: appToolbar.height

		AppToolbar {
			id: appToolbar
			x: -width / 2
			y: Overte.Theme.reducedMotion ? 0 : height * 2
			opacity: 0.0

			states: State {
				name: "open"
				when: toolbarToggleButton.checked
				PropertyChanges {
					target: appToolbar
					y: 0
					opacity: 1.0
				}
			}

			transitions: Transition {
				reversible: true

				NumberAnimation {
					properties: "y"
					easing.type: Easing.OutExpo
					duration: 500
				}

				NumberAnimation {
					properties: "opacity"
					easing.type: Easing.OutExpo
					duration: 500
				}
			}
		}
	}

	Overte.RoundButton {
		anchors.horizontalCenter: desktopRoot.horizontalCenter
		anchors.bottom: desktopRoot.bottom
		anchors.margins: 16
		focusPolicy: Qt.NoFocus

		id: toolbarToggleButton
		checkable: true
		icon.source: checked ? "../icons/triangle_down.svg" : "../icons/triangle_up.svg"
		icon.width: 24
		icon.height: 24
		icon.color: Overte.Theme.paletteActive.buttonText
	}
}
