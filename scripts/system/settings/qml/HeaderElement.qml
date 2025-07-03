import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
	height: 60;
	width: parent.width;
	id: root;

	Rectangle {
		anchors.fill: parent;
		color: "black";
	}

	Image {
		source: "../img/back_arrow.png";
		anchors.verticalCenter: parent.verticalCenter;
		height: 40;
		width: 40;
		x: currentPage == "Settings" ? -40 : 10;

		Behavior on x {
			NumberAnimation {
				duration: 200;
				easing.type: Easing.InOutCubic;
			}
		}

		MouseArea {
			anchors.fill: parent;
			onClicked: {
				currentPage = "Settings";
			}
		}
	}

	Text {
		text: currentPage;
		color: "white";
		font.pixelSize: 26;
		anchors.horizontalCenter: parent.horizontalCenter;
		anchors.verticalCenter: parent.verticalCenter;
		horizontalAlignment: Text.AlignHCenter;
	}
}
