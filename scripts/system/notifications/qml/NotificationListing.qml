import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3

Rectangle {
	property string bubbleId: "";
	property string title: "";
	property string description: "";
	property string type: "";
	property string time: "";
	property bool isOpen: false;
	property int expandHeight: description !== "" ? 250 : 110;

	color: "transparent";
	width: parent && parent.width - 10;
	x: 5;
	height: 75;
	clip: true;
	id: root;

	Behavior on height {
		NumberAnimation {
			duration: 50;
			easing.type: Easing.InOutCubic;
		}
	}

	Column {
		width: parent.width;
		height: parent.height;

		Text {
			height: 75;
			width: parent.width - 15;
			color: "white";
			text: title;
			font.pixelSize: 18;
			wrapMode: Text.Wrap;
			x: 5;
			verticalAlignment: Text.AlignVCenter;
		}

		// Extra information
		Rectangle {
			id: notificationDetailsElement;
			color: Qt.rgba(1,1,1,0.05);
			width: parent.width;
			height: 200;

			Column {
				width: parent.width - 10;
				height: parent.height - 10;
				spacing: 5;

				Text {
					text: time;
					font.pixelSize: 18;
					color: colors.lightText1;
					width: parent.width;
					wrapMode: Text.Wrap;
					horizontalAlignment: Text.AlignRight;
				}

				Text {
					text: description;
					font.pixelSize: 18;
					color: colors.lightText1;
					width: parent.width;
					wrapMode: Text.Wrap;
				}
			}


			Behavior on height {
				NumberAnimation {
					duration: 50;
					easing.type: Easing.InOutCubic;
				}
			}
		}
	}

	// Bottom border
	Rectangle {
		color: Qt.rgba(1,1,1,0.1);
		height: 2;
		width: parent.width;
		y: parent.height - height;
	}

	MouseArea {
		anchors.fill: parent;
		hoverEnabled: true;
		propagateComposedEvents: true;	

		onPressed: {
			isOpen = !isOpen;
			notificationDetailsElement.height = isOpen ? expandHeight : 0;
			root.height = isOpen ? expandHeight : 75;
		}

		onEntered: {
			parent.color = colors.darkBackground2;
		}

		onExited: {
			parent.color = "transparent";
		}
	}
}