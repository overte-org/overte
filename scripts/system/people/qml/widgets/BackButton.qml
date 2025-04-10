import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Rectangle {
	width: parent.width;
	height: 40;
	color: "#333";
	Layout.alignment: Qt.AlignHCenter;

	Column {
		width: parent.width - 10;
		spacing: 0;
		anchors.verticalCenter: parent.verticalCenter;

		Text {
			x: 10;
			width: parent.width;
			text: "Back";
			color: "white";
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
			page = "Home";
			toScript({type: "focusedUser", user: null});
			focusedUser = null;
		}
	}
}