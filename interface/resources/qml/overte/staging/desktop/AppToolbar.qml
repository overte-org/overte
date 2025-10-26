import QtQuick
import QtQuick.Controls

import ".." as Overte

Item {
	readonly property int buttonSize: 80

	id: root
	implicitHeight: buttonSize + (buttonList.anchors.margins * 2) + Overte.Theme.scrollbarWidth + 4
	implicitWidth: 8 * (buttonSize + buttonList.spacing) + buttonList.anchors.margins

	Rectangle {
		anchors.fill: root
		radius: Overte.Theme.borderRadius
		color: Overte.Theme.paletteActive.dialogShade
	}

	ListView {
		anchors.fill: parent
		anchors.margins: 4

		id: buttonList
		orientation: Qt.Horizontal
		spacing: 4
		clip: true

		ScrollBar.horizontal: Overte.ScrollBar {
			policy: ScrollBar.AsNeeded
		}

		model: [
			{name: "Mute", iconSource: "../icons/speaker_muted.svg", checkable: true},
			{name: "Settings", iconSource: "../icons/settings_cog.svg", checkable: true},
			{name: "Contacts", iconSource: "../icons/add_friend.svg", checkable: true},
			{name: "Body Paint", iconSource: "../icons/pencil.svg", checkable: true},
			{name: "Eyes", iconSource: "../icons/eye_open.svg", checkable: false},

			{name: "Star", iconSource: "../icons/gold_star.svg", checkable: false},
			{name: "Star", iconSource: "../icons/gold_star.svg", checkable: false},
			{name: "Star", iconSource: "../icons/gold_star.svg", checkable: false},
			{name: "Star", iconSource: "../icons/gold_star.svg", checkable: false},
			{name: "Star", iconSource: "../icons/gold_star.svg", checkable: false},
		]
		delegate: Overte.Button {
			required property url iconSource
			required property string name
			required checkable

			implicitWidth: buttonSize
			implicitHeight: buttonSize

			Image {
				anchors.top: parent.top
				anchors.left: parent.left
				anchors.right: parent.right
				anchors.bottom: buttonLabel.top
				anchors.margins: 2

				source: iconSource
				sourceSize.width: width
				sourceSize.height: height

				fillMode: Image.PreserveAspectFit
			}

			Overte.Label {
				id: buttonLabel
				anchors.left: parent.left
				anchors.right: parent.right
				anchors.bottom: parent.bottom
				anchors.margins: 2
				wrapMode: Text.Wrap
				horizontalAlignment: Text.AlignHCenter
				verticalAlignment: Text.AlignBottom
				text: name

				font.pixelSize: Overte.Theme.fontPixelSizeSmall
			}
		}
	}
}
