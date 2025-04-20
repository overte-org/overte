import QtQuick 2.15
import QtQuick.Layouts 1.3
import TabletScriptingInterface 1.0

Item {
	property color bgColor: index % 2 === 0 ? Qt.rgba(0,0,0,0) : Qt.rgba(0.15,0.15,0.15,1);
	property int initialTextXPosition;

	width: parent.width;
	height: 60;

	Rectangle {
		id: backgroundElement;
		width: parent.width;
		height: parent.height;
		color: bgColor;
		anchors.fill: parent;

		Behavior on color {
			ColorAnimation {
				duration: 50
				easing.type: Easing.InOutCubic
			}
		}
	}

	Row {
		width: parent.width - 20;
		height: parent.height;
		anchors.centerIn: parent;

		// Image/Icon container
		Item {
			width: 45;
			height: parent.height;

			Image {
				sourceSize.height: 25;
				source: pageIcon;
				anchors.centerIn: parent;
			}
		}

		// Page name
		Text {
			id: pageNameElement
			text: pageName;
			color: "white";
			font.pixelSize: 24;
			anchors.verticalCenter: parent.verticalCenter;

			// Set a variable to the initial X position, used for animating it on hover.
			Component.onCompleted: {
				initialTextXPosition = x;
			}

			Behavior on x {
				NumberAnimation {
					duration: 50
					easing.type: Easing.InOutCubic
				}
			}
		}
	}



	MouseArea {
		anchors.fill: parent;
		hoverEnabled: true;

		onClicked: {
			Tablet.playSound(TabletEnums.ButtonClicked);
			if (targetPage !== "") {
				toScript({type:"switchApp", appUrl: targetPage});
				return;
			}
			currentPage = pageName;
		}

		onEntered: {
			backgroundElement.color = "#333";
			pageNameElement.x = initialTextXPosition + 20;
			Tablet.playSound(TabletEnums.ButtonHover);
		}

		onExited: {
			backgroundElement.color = bgColor;
			pageNameElement.x = initialTextXPosition;
		}
	}
}