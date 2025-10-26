import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../" as Overte
import "../settings" as OverteSettings

Column {
	id: settingsPage
	spacing: 16

	Item {
		anchors.left: parent.left
		anchors.right: parent.right
		implicitHeight: Math.max(settingsBackBtn.implicitHeight, settingsTitleLabel.implicitHeight)

		Overte.Label {
			anchors.fill: parent

			id: settingsTitleLabel

			horizontalAlignment: Text.AlignHCenter
			verticalAlignment: Text.AlignVCenter
			text: qsTr("Chat Settings")
		}

		Overte.Button {
			id: settingsBackBtn
			icon.source: "../icons/triangle_left.svg"
			icon.width: 24
			icon.height: 24
			icon.color: Overte.Theme.paletteActive.buttonText

			text: qsTr("Back", "Return to previous page")

			onClicked: {
				stack.pop();
			}
		}
	}

	OverteSettings.SwitchSetting {
		text: qsTr("Join/Leave Notifications")

		value: root.settingJoinNotifications
        onValueChanged: {
            root.settingJoinNotifications = value;
            root.sendSettingsUpdate();
        }
	}

	OverteSettings.SwitchSetting {
		text: qsTr("Chat Bubbles")

		value: root.settingChatBubbles
        onValueChanged: {
            root.settingChatBubbles = value;
            root.sendSettingsUpdate();
        }
	}

	OverteSettings.SwitchSetting {
		text: qsTr("Desktop Window")

		value: root.settingDesktopWindow
        onValueChanged: {
            root.settingDesktopWindow = value;
            root.sendSettingsUpdate();
        }
	}

	RowLayout {
		anchors.left: parent.left
		anchors.right: parent.right

		Overte.Button {
			Layout.alignment: Qt.AlignHCenter
			text: qsTr("Clear History")
			backgroundColor: Overte.Theme.paletteActive.buttonDestructive

			onClicked: { root.messagesCleared(); }
		}
	}
}
