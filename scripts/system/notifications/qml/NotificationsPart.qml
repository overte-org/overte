import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Rectangle {
	width: HMD.active ? parent.width : 300;
	height: HMD.active ? 520 : 260;
	visible: true;
	color: "transparent";

	Column {
		id: columnContainer;
		width: parent.width;
		height: parent.height;
		spacing: 5;

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