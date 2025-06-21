import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

import "."

Item {
    id: root;
	signal sendToScript(var message);
    property var window;
	property int notificationId: 0; // Incremental value used to reference each individual notification 
	property bool isHovered: false;

	ListModel {
        id: notifications;
    }

    Binding { target: root; property:'window'; value: parent.parent; when: Boolean(parent.parent) }
    Binding { target: window; property: 'shown'; value: false; when: Boolean(window) }

    Rectangle {
        parent: desktop
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


	function addSystemNotification(text, details) {
		var targetNotification = notificationId;

		// Insert notification to the stack
		notifications.append({bubbleText: text, bubbleDetails: details, id: targetNotification});

		print(JSON.stringify(notifications))

		notificationId = notificationId + 1;
	}

    // Messages from script
    function fromScript(message) {
		print(JSON.stringify(message));
        switch (message.type){
            case "addSystemNotification":
				addSystemNotification(message.title, message.description)
                break;
        }
    }

    // Send message to script
    function toScript(packet){
        sendToScript(packet)
    }
}