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
        parent: desktop;
        x: 10;
        y: parent.height - height - 10;
        width: 600;
        height: Math.min(300, children[0].contentHeight);
        z: -1;
        visible: true;
		color: "transparent";

		ListView {
			id: messageListView
			width: parent.width;
			height: parent.height;
			clip: true;
			model: notifications;
			spacing: 10;
			orientation: ListView.Vertical;

			onCountChanged: {
				if (count > 0) {
					// Scroll to the bottom when a new message is added
					currentIndex = count - 1;
				}
			}

			delegate: NotificationPopup {
				chatMessage: model.message;
				chatAuthor: model.author;
			}
		}
    }

	function addMessage(author, message) {
		// Insert notification to the stack
		notifications.append({author, message});

		// If there are more than the max messages, remove the first one.
		if (notifications.length > 4) {
			notifications.remove(0);
		}
	}

    // Messages from script
    function fromScript(message) {
		print(JSON.stringify(message));
        switch (message.type){
            case "message":
				addMessage(message.author, message.message);
                break;
        }
    }

    // Send message to script
    function toScript(packet){
        sendToScript(packet);
    }
}