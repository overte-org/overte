import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Rectangle {
	property var backgroundColor: index % 2 == 0 ? "#333" : "#2a2a2a";
	property var isFriendCheckBoxInitialized: false;

	width: parent.width;
	height: 40;
	color: backgroundColor;
	anchors.horizontalCenter: parent.horizontalCenter;

	RowLayout {
		width: parent.width - 10;
		spacing: 0;
		anchors.verticalCenter: parent.verticalCenter;

		Image {
			id: avatarImageElement;
			source: icon || "../../img/default_profile_avatar.svg";
			sourceSize.width: 30;
       		sourceSize.height: 30;
			width: 40;
		}

		Item {
			// Spacer
			width: 20;
			height: 1;
		}

		Text {
			Layout.fillWidth: true;
			text: displayName;
			color: "white";
			font.pointSize: 16;
			elide: Text.ElideRight;
			maximumLineCount: 1;
		}

		CheckBox {
			checked: isFriend;
			width: 50;
			height: parent.height;

			onCheckedChanged: {
				if (!isFriendCheckBoxInitialized) return;
				if (checked) return toScript({type: "addFriend", username: displayName});
				else return toScript({type: "removeFriend", username: displayName}); 
			}

			Component.onCompleted: {
				isFriendCheckBoxInitialized = true;
			}
		}

		// Animation for the x of the element.
		Behavior on x {
			NumberAnimation {
				duration: 100
				easing.type: Easing.InOutCubic
			}
		}
	}


}