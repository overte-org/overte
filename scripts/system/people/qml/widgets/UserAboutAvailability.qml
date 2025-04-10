import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0


Rectangle {
	property var status: "";
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

	width: parent.width;
	height: 60;
	color: "#333";
	Layout.alignment: Qt.AlignHCenter;

	Column {
		width: parent.width - 10;
		spacing: 0;
		anchors.verticalCenter: parent.verticalCenter;

		Text {
			x: 10;
			width: parent.width;
			text: statusLiteral[status];
			color: statusColors[status];
			font.pointSize: 16;
			horizontalAlignment: Text.AlignHCenter

			// Animation for the font size of the element.
			Behavior on font.pointSize {
				NumberAnimation {
					duration: 100
					easing.type: Easing.InOutCubic
				}
			}
		}

	}

	MouseArea {
		anchors.fill: parent;
		hoverEnabled: true;

		onEntered: {
			parent.color = "#555";
			parent.children[0].children[0].font.pointSize = 20;
		}

		onExited: {
			parent.color = "#333";
			parent.children[0].children[0].font.pointSize = 16;
		}

		onClicked: {
			AccountServices.findableBy = status;
			sendToScript({type: "updateMyData"});
		}
	}
}