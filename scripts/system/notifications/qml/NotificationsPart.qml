import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Rectangle {
	x: parent.width - width - 10;
	y: 10;
	width: 300;
	height: 260;
	z: 99;
	visible: true;
	color: "transparent";

	Column {
		id: columnContainer;
		width: parent.width;
		height: parent.height;
		spacing: 5;

		Repeater {
			model: notifications.length;
			delegate: Bubble {
				bubbleText: notifications[index].bubbleText;
				bubbleDetails: notifications[index].bubbleDetails;
			}
		}

		ListView {
			id: bubbleInstance;
			width: parent.width;
			height: parent.height;
			model: notifications;
			spacing: 5;

			delegate: Bubble {
				bubbleText: notifications.get(index) && notifications.get(index).bubbleText || "";
				bubbleDetails: notifications.get(index) && notifications.get(index).bubbleDetails || "";
			}
		}
	}
}