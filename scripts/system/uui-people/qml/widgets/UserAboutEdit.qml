import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

RowLayout {
	property var sessionDisplayName: "";
	property var icon: "../../img/default_profile_avatar.svg";
	property var isSelf: false;

	property var statusColors: {
		"all": "#3babe1",
		"connections": "#3edf44",
		"friends": "#f7ff3a",
		"none": "#969696"
	}

	property var statusLiteral: {
		"all": "Everyone",
		"connections": "Contacts",
		"friends": "Friends",
		"none": "Offline"
	}

	width: parent.width - 20;
	height: 80;
	spacing: 10;

	Item {
		height: 80;
		width: 80;

		Rectangle {
			color: "#333";
			radius: 100;
			height: 80;
			width: 80;
			id: avatarImageBackground;
			anchors.centerIn: parent;
		}

		Image {
			id: avatarImageElement;
			source: icon;
			sourceSize.width: 80;
       		sourceSize.height: 80;
			z: 1;
			anchors.centerIn: parent;
        	visible: false;
		}

		OpacityMask {
			anchors.fill: avatarImageElement;
			source: avatarImageElement;
			maskSource: avatarImageBackground;
		}
	}

	ColumnLayout {
		width: 300;
		height: 40;

		Item {
			width: parent.width;
			height: 40;

			Rectangle {
				color: "#333";
				anchors.fill: parent;
				width: parent.width;
				height: parent.height;
			}

			TextInput {
				id: displayNameEntry
				text: myData.displayName;
				font.pointSize: 16;
				color: "white";
				width: parent.width - 4;
				height: parent.height - 4;
				anchors.centerIn: parent;
				clip: true;

				onTextEdited: {
					MyAvatar.displayName = text;
				}
			}
		}

	}


}