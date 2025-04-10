import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Rectangle {
	property var backgroundColor: index % 2 == 0 ? "#333" : "#2a2a2a";

	width: parent.width;
	height: canKick ? 60 : 40;
	color: backgroundColor;
	anchors.horizontalCenter: parent.horizontalCenter;

	Column {
		width: parent.width - 10;
		spacing: 0;
		anchors.verticalCenter: parent.verticalCenter;

		Text {
			x: 10;
			width: parent.width;
			text: sessionDisplayName;
			color: "white";
			font.pointSize: 16;
			elide: Text.ElideRight;
			maximumLineCount: 1;
		}

		Text {
			// Users real account name.
			visible: canKick;
			x: 30;
			width: parent.width;
			text: adminUserData[users[index].sessionUUID].username || ""
			color: "#3babe1";
			font.pointSize: 12;
			elide: Text.ElideRight;
			maximumLineCount: 1;
		}

		// Animation for the x of the element.
		Behavior on x {
			NumberAnimation {
				duration: 100
				easing.type: Easing.InOutCubic
			}
		}
	}

	MouseArea {
		anchors.fill: parent;
		hoverEnabled: true;

		onEntered: {
			parent.color = "#555";
			parent.children[0].x = 10;
		}

		onExited: {
			parent.color = backgroundColor
			parent.children[0].x = 0;
		}

		onClicked: {
			toUserPage(sessionUUID);
		}
	}
}