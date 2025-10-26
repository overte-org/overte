import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../../" as Overte

Item {
	required property string mode
	required property string username
	required property string password
	property url accountServer: "https://mv.overte.org/server"

	function loginError(desc) {
		let topLine = (
			mode === "register" ?
			qsTr("Failed to register!") :
			qsTr("Failed to log in!")
		);
		statusText.text = `${topLine}\n\n${qsTr(desc)}`;
		backButton.visible = true;
	}

	// DEBUG
	Timer {
		interval: 1000
		running: true
		onTriggered: {
			loginError("Debug mode! Not connected to anything.");
		}
	}

	ColumnLayout {
		anchors.fill: parent
		anchors.margins: 32
		spacing: Overte.Theme.fontPixelSize * 2

		Overte.Label {
			Layout.fillWidth: true
			Layout.fillHeight: true

			id: statusText
			text: mode === "register" ? qsTr("Registering…") : qsTr("Logging in…")
			horizontalAlignment: Text.AlignHCenter
			verticalAlignment: Text.AlignVCenter
		}

		RowLayout {
			spacing: 16
			Layout.minimumHeight: Overte.Theme.fontPixelSize * 3
			Layout.maximumHeight: Overte.Theme.fontPixelSize * 3

			Overte.Button {
				id: backButton
				visible: false

				// make the buttons equal size
				Layout.preferredWidth: 1
				Layout.preferredHeight: 1
				Layout.fillWidth: true
				Layout.fillHeight: true

				text: qsTr("Back", "Return to previous page")
				icon.source: "../../icons/triangle_left.svg"
				icon.width: 24
				icon.height: 24
				icon.color: Overte.Theme.paletteActive.buttonText

				onClicked: {
					stack.pop();
				}
			}

			Item {
				// make the buttons equal size
				Layout.preferredWidth: 1
				Layout.preferredHeight: 1
				Layout.fillWidth: true
				Layout.fillHeight: true
			}
		}
	}
}

