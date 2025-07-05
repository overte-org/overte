import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3


Item {
	width: 150;
	height: parent.height;

	property var callback: null;
	property string buttonText: "";

	Rectangle {
		color: "#333";
		width: parent.width;
		height: parent.height;
		radius: 5;

		Text {
			anchors.centerIn: parent;
			text: buttonText;
			color: "white";
			font.pixelSize: 20;
		}

		MouseArea {
			anchors.fill: parent;
			hoverEnabled: true;

			onEntered: {
				parent.color = "#555";
			}

			onExited: {
				parent.color = "#333";
			}

			onClicked: {
				callback();
			}
		}
	}
}