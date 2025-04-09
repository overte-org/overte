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
			source: icon || "../../img/default_profile_avatar.svg";
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

	Column {
		width: 300;
		Text {
			text: sessionDisplayName || "";
			font.pointSize: 16;
			color: "white";
			clip: true;
			width: parent.width;
			wrapMode: Text.WrapAnywhere;
			elide: Text.ElideRight;
			maximumLineCount: 2;
		}

		RowLayout {
			visible: isSelf;

			Rectangle {
				// Squarcle
				width: 15;
				height: 15;
				radius: 100;
				color: statusColors[myData.findableBy] || "magenta";
				Layout.alignment: Qt.AlignVCenter;
			}

			Text {
				x: parent.children[0].width + 10;
				text: statusLiteral[myData.findableBy] || "...";
				font.pointSize: 16;
				color: statusColors[myData.findableBy] || "magenta";
				Layout.alignment: Qt.AlignVCenter;
			}
		}

	}
}