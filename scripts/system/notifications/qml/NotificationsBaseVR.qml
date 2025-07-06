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

	NotificationsPart { }

	function addSystemNotification(message, details) {
		var targetNotification = notificationId;

		// Insert notification to the stack
		notifications.append({bubbleText: message, bubbleDetails: details, id: targetNotification});

		notificationId = notificationId + 1;
	}

    // Messages from script
    function fromScript(message) {
		print(JSON.stringify(message));
        switch (message.type){
            case "addSystemNotification":
				addSystemNotification(message.message, message.details)
                break;
        }
    }

    // Send message to script
    function toScript(packet){
        sendToScript(packet)
    }
}