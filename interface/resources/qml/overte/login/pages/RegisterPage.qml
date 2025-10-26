import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../../" as Overte

Item {
	// TODO: once the account server can accept
	// usernames starting with numbers or underscores, modify this
	readonly property var usernameRegex: /[a-zA-Z][0-9a-zA-Z_.-]{2,}/

	// passwords must be at least 6 characters
	readonly property var passwordRegex: /.{6,}/

	ColumnLayout {
		anchors.fill: parent
		anchors.margins: 32
		spacing: Overte.Theme.fontPixelSize * 2

		Overte.Label {
			Layout.fillWidth: true

			text: qsTr("Register")
			horizontalAlignment: Text.AlignHCenter
			verticalAlignment: Text.AlignVCenter
		}

		Overte.TextField {
			Layout.fillWidth: true

			id: usernameField
			placeholderText: qsTr("Username")

			validator: RegularExpressionValidator {
				regularExpression: usernameRegex
			}

			Overte.ToolTip {
				visible: parent.activeFocus && !parent.text.match(usernameRegex)
				text: qsTr("Usernames must start with a letter and be at least 3 characters long.")
			}
		}

		RowLayout {
			Layout.fillWidth: true

			Overte.TextField {
				id: passwordField
				Layout.fillWidth: true
				placeholderText: qsTr("Password")
				echoMode: showPassword.checked ? TextInput.Normal : TextInput.Password
				passwordMaskDelay: 500

				validator: RegularExpressionValidator {
					regularExpression: passwordRegex
				}

				Overte.ToolTip {
					visible: parent.activeFocus && !parent.text.match(usernameRegex)
					text: qsTr("Passwords must be at least 6 characters.")
				}
			}

			Overte.RoundButton {
				id: showPassword
				checkable: true
				icon.source: checked ? "../../icons/eye_closed.svg" : "../../icons/eye_open.svg"
				icon.color: Overte.Theme.paletteActive.buttonText
				icon.width: 24
				icon.height: 24
			}
		}

		Overte.TextField {
			Layout.fillWidth: true

			id: accountServerField
			placeholderText: qsTr("Account server (Optional)")
		}

		RowLayout {
			spacing: 16
			Layout.maximumHeight: Overte.Theme.fontPixelSize * 3

			Overte.Button {
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

			Overte.Button {
				// make the buttons equal size
				Layout.preferredWidth: 1
				Layout.preferredHeight: 1
				Layout.fillWidth: true
				Layout.fillHeight: true

				text: qsTr("Register", "Register button")
				backgroundColor: Overte.Theme.paletteActive.buttonAdd

				enabled: usernameField.text.match(usernameRegex) && passwordField.text.match(passwordRegex)

				onClicked: {
					let props = {
						mode: "register",
						username: usernameField.text,
						// don't let stray newlines through
						// or stuff can break
						password: (
							passwordField.text
							.replace("\n", "")
							.replace("\r", "")
						),
					};

					if (accountServerField.text !== "") {
						props.accountServer = accountServerField.text;
					}

					stack.push("./ProgressPage.qml", props);
				}
			}
		}
	}
}
