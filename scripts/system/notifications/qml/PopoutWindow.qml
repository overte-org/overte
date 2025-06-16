import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

import "."

Rectangle {
    id: root;
	signal sendToScript(var message);
	width: parent.width;
	height: parent.height;
	color: colors.darkBackground1;
	
	property var notificationList: [];

	Colors {
		id: colors;
	}

	Flickable {
		Layout.margins: 10;
		Layout.topMargin: 0;
		width: parent.width;
		height: parent.height;
		contentHeight: notificationListColumn.height;
		clip: true;

		Column {
			width: parent.width - 10;
			spacing: 5;
			id: notificationListColumn;

			Repeater {
				model: notificationList.length;
				delegate: NotificationListing {
					title: notificationList[index].title;
					description: notificationList[index].description;
					type: notificationList[index].type;
					time: notificationList[index].timestamp;
				}
			}

		}

		ScrollBar.vertical: ScrollBar {
			policy: Qt.ScrollBarAlwaysOn;

			background: Rectangle {
				color: "transparent";
				radius: 5;
				visible: parent.visible;
			}
		}
	}

	function fromScript(message) {
		print(JSON.stringify(message));
		switch (message.type){
			case "notificationList":
				notificationList = message.messages;
				break;
		}
	}
}

