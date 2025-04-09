import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Rectangle {
	property var buttonText: "";
	property var isDangerButton: false;
	property var action: null;

	color: "#333";
	width: 220;
	height: 100;

	Text {
		text: buttonText;
		anchors.centerIn: parent;
		color: isDangerButton ? "#ff3030" : "white"
		font.pointSize: 18;
	}

	MouseArea {
		anchors.fill: parent;
		hoverEnabled: true;

		onEntered: {
			parent.color = "#555";
			parent.children[0].font.pointSize = 22;
		}

		onExited: {
			parent.color = "#333";
			parent.children[0].font.pointSize = 18;
		}

		onClicked: {
			action();
		}
	}
}