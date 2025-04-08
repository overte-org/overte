import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

Row {
	property var sessionDisplayName: "";
	property var icon: "../../img/default_profile_avatar.svg";
	property var isSelf: false;

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

	Column {
		Text {
			text: sessionDisplayName;
			font.pointSize: 16;
			color: "white";
		}

		Text {
			text: "Edit >"
			font.pointSize: 14;
			color: "gray";
		}
	}
}