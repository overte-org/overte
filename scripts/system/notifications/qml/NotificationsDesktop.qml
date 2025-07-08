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
    property bool announcementHistoryVisible: false;
    property bool previewNotificationActive: false;
    property bool hasUnread: false;

	ListModel { id: notifications }         // Main storage of notifications

    Binding { target: root; property:'window'; value: parent.parent; when: Boolean(parent.parent) }
    Binding { target: window; property: 'shown'; value: false; when: Boolean(window) }

    Row {
        parent: desktop;
	    x: announcementHistoryVisible || previewNotificationActive ? parent.width - width + 5 : parent.width - 45;
    	y: 5;
	    z: 99;

        width: 355;
        spacing: 5;

        // Notification history
        Rectangle {
            width: 300;
            height: 300;
            color: Qt.rgba(0,0,0,0.5);
            radius: 5;
            visible: announcementHistoryVisible;

            Flickable {
                x: 5;
                y: 5
                width: parent.width - 5;
                height: parent.height - 10;
                clip: true;
                contentHeight: notificationHistoryListView.contentHeight;

                Column {
                    width: parent.width - 10;
                    height: parent.height - 10;

                    ListView {
                        id: notificationHistoryListView;
                        width: parent.width;
                        height: parent.height;
                        model: notifications;
                        spacing: 2;

                        delegate: Rectangle {
                            width: parent && parent.width || 0;
                            height: 50;
                            color: Qt.rgba(0,0,0,0.8);
                            opacity: 0.9;
                            radius: 5;

                            RowLayout {
                                width: parent.width - 10;
                                height: parent.height;
                                anchors.centerIn: parent;

                                // TODO: Announcement icons?
                                // Rectangle {
                                //     width: 40;
                                //     height: 40;
                                //     color: "orange";
                                // }

                                Text {
                                    Layout.fillWidth: true;
                                    Layout.fillHeight: true;
                                    text: notifications.get(index) && notifications.get(index).bubbleText || "";
                                    color: "white";
                                    font.pixelSize: 18;
		                            wrapMode: Text.Wrap;
                                    clip: true;
                                }
                            }
                            MouseArea {
                                anchors.fill: parent;
                                hoverEnabled: true;
                                propagateComposedEvents: true;	

                                onEntered: {
                                    parent.color = Qt.rgba(0,0,0,0.9);
                                    parent.opacity = 1;
                                }

                                onExited: {
                                    parent.color = Qt.rgba(0,0,0,0.8);
                                    parent.opacity = 0.9;
                                }
                            }

                            Behavior on color {
                                ColorAnimation {
                                    duration: 100;
                                    easing.type: Easing.InOutCubic;
                                }
                            }
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

                    contentItem: Rectangle {
                        implicitWidth: 6;
                        implicitHeight: 100;
                        radius: width / 2;
                        color: Qt.rgba(1,1,1,1);
                    }
                }
            }
        }

        // Preview New Notification
        Rectangle {
            width: 300;
            height: 50;
            color: Qt.rgba(0,0,0,0.8);
            opacity: 0.9;
            radius: 5;
            visible: !announcementHistoryVisible && previewNotificationActive;

            RowLayout {
                width: parent.width - 10;
                height: parent.height;
                anchors.centerIn: parent;

                // TODO: Announcement icons?
                // Rectangle {
                //     width: 40;
                //     height: 40;
                //     color: "orange";
                // }

                Text {
                    Layout.fillWidth: true;
                    Layout.fillHeight: true;
                    id: previewNotificationText;
                    text: "";
                    color: "white";
                    font.pixelSize: 18;
                    wrapMode: Text.Wrap;
                    clip: true;
                }
            }
            MouseArea {
                anchors.fill: parent;
                hoverEnabled: true;
                propagateComposedEvents: true;	

                onClicked: {
                    announcementHistoryVisible = true;
                }

                onEntered: {
                    parent.color = Qt.rgba(0,0,0,0.9);
                    parent.opacity = 1;
                }

                onExited: {
                    parent.color = Qt.rgba(0,0,0,0.8);
                    parent.opacity = 0.9;
                }
            }

            Behavior on color {
                ColorAnimation {
                    duration: 100;
                    easing.type: Easing.InOutCubic;
                }
            }

            onVisibleChanged: {
                previewNotificationTimer.running = true;
            }

            Timer {
                id: previewNotificationTimer;
                running: false;
                repeat: false;
                interval: 5000;
                onTriggered: {
                    previewNotificationActive = false;
                }
            }
        }

        // Bell Icon and container
        Rectangle {
            color: Qt.rgba(0,0,0,0.5);
            width: 40;
            height: 40;
            radius: 5;

            Image {
                source: hasUnread ? "../img/notification-bell-unread.svg" : "../img/notification-bell.svg";
                height: 30;
                width: 30;
                sourceSize.width: 128;
                sourceSize.height: 128;
                fillMode: Image.PreserveAspectFit;
                anchors.centerIn: parent;
                opacity: 0.9;
                id: notificationIcon;

                Behavior on opacity {
                    NumberAnimation {
                        duration: 100;
                        easing.type: Easing.InOutCubic;
                    }
                }
                SequentialAnimation {
                    id: shakeAnimation
                    NumberAnimation { target: notificationIcon; property: "rotation"; to: 20; duration: 200}
                    NumberAnimation { target: notificationIcon; property: "rotation"; to: -20; duration: 200}
                    NumberAnimation { target: notificationIcon; property: "rotation"; to: 20; duration: 200}
                    NumberAnimation { target: notificationIcon; property: "rotation"; to: 0; duration: 200}
                }
            }

            MouseArea {
                anchors.fill: parent;
                hoverEnabled: true;
                propagateComposedEvents: true;	

                onPressed: {
                    announcementHistoryVisible = !announcementHistoryVisible;
                    if (announcementHistoryVisible) hasUnread = false;
                }

                onEntered: {
                    parent.color = Qt.rgba(0,0,0,0.75);
                    parent.children[0].opacity = 1;
                }

                onExited: {
                    parent.color = Qt.rgba(0,0,0,0.5);
                    parent.children[0].opacity = 0.9;
                }
            }

            Behavior on color {
                ColorAnimation {
                    duration: 100;
                    easing.type: Easing.InOutCubic;
                }
            }
        }
    }

    Timer {
        id: shakeAnimationTimer;
        running: false;
        repeat: true;
        interval: 5000;
        onTriggered: {
            shakeAnimation.start();
        }
    }

    onAnnouncementHistoryVisibleChanged: {
        if (announcementHistoryVisible === true) {
            shakeAnimationTimer.running = false;
            hasUnread = false;
            previewNotificationTimer.running = false;
            previewNotificationActive = false;
        }
    }

	function addSystemNotification(message, details, skipVisualEffects = false) {
		var targetNotification = notificationId;

		// Insert notification to the stack
		notifications.insert(0, {bubbleText: message, bubbleDetails: details, id: targetNotification});

		notificationId = notificationId + 1;

        if (skipVisualEffects === false) {
            // Display a preview
            if (announcementHistoryVisible === false) {
                if (previewNotificationTimer.running) {
                    previewNotificationTimer.restart();
                }
                previewNotificationText.text = message;
                previewNotificationActive = true;
            }

            // Visual effects.
            shakeAnimation.start();
            if (announcementHistoryVisible === false) {
                hasUnread = true;
                shakeAnimationTimer.running = true;
            }
        }
	}

    // Messages from script
    function fromScript(message) {
		print(JSON.stringify(message));
        switch (message.type){
            case "addSystemNotification":
				addSystemNotification(message.message, message.details);
                break;
            case "closeAllNotifications":
                notifications.clear();
                break;
            case "notificationList":
                message.messages.forEach((message) => {
                    addSystemNotification(message.message, message.details, true);
                })
                break;
        }
    }

    // Send message to script
    function toScript(packet){
        sendToScript(packet)
    }
}