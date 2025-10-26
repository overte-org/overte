import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import ".." as Overte
import "."

Rectangle {
	id: control
	color: index % 2 === 0 ? Overte.Theme.paletteActive.base : Overte.Theme.paletteActive.alternateBase
	implicitWidth: ListView.view.contentWidth
	implicitHeight: 96

	required property int index

	// directory username
	required property string user
	required property url avatarUrl
	required property int status
	required property bool friend
	required property string currentPlaceName

	ColumnLayout {
		anchors.fill: parent
		anchors.margins: 4
		anchors.leftMargin: 8
		anchors.rightMargin: 8

		RowLayout {
			Layout.fillWidth: true
			spacing: 16

			AccountAvatar {
				source: avatarUrl
				status: control.status
			}

			ColumnLayout {
				Layout.fillWidth: true
				Layout.fillHeight: true

				Overte.Label {
					Layout.fillWidth: true
					Layout.fillHeight: true
					elide: Text.ElideRight
					verticalAlignment: Text.AlignVCenter
					text: user
				}

				Overte.Label {
					Layout.fillWidth: true
					Layout.fillHeight: true
					verticalAlignment: Text.AlignVCenter

					color: {
						switch (control.status) {
							case 0: return Overte.Theme.paletteActive.text;
							case 1: return Overte.Theme.paletteActive.statusFriendsOnly;
							case 2: return Overte.Theme.paletteActive.statusContacts;
							case 3: return Overte.Theme.paletteActive.statusEveryone;
						}
					}

					text: {
						switch (control.status) {
							case 0: return qsTr("Offline");
							case 1: return qsTr("Friends Only");
							case 2: return qsTr("Contacts")
							case 3: return qsTr("Everyone")
						}
					}
				}

				Overte.Label {
					Layout.fillWidth: true
					Layout.fillHeight: true
					font.pixelSize: Overte.Theme.fontPixelSizeSmall
					opacity: Overte.Theme.highContrast ? 1.0 : 0.8
					verticalAlignment: Text.AlignVCenter
					text: currentPlaceName
					visible: currentPlaceName !== ""
				}
			}

			Overte.RoundButton {
				backgroundColor: (
					control.friend ?
					Overte.Theme.paletteActive.buttonDestructive :
					Overte.Theme.paletteActive.buttonAdd
				)
				icon.source: (
					control.friend ?
					"../icons/remove_friend.svg" :
					"../icons/add_friend.svg"
				)
				icon.width: 24
				icon.height: 24
				icon.color: Overte.Theme.paletteActive.buttonText

				onClicked: {
					// TODO
					control.friend = !control.friend;
				}
			}
		}
	}
}
