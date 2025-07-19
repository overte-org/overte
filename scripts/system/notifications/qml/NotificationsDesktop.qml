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
    property bool notificationDetailsActive: false;
    property bool hasUnread: false;
    property bool doNotDisturb: false;

	ListModel { id: notifications }         // Main storage of notifications

    Binding { target: root; property: 'window'; value: parent.parent; when: Boolean(parent.parent) }
    Binding { target: window; property: 'shown'; value: false; when: Boolean(window) }

    Row {
        parent: desktop;
	    x: announcementHistoryVisible || previewNotificationActive || notificationDetailsActive ? parent.width - width + 5 : parent.width - 45;
    	y: 5;
	    z: 99;

        width: 355;
        spacing: 5;

        // Notification history
        Rectangle {
            width: 300;
            height: 330;
            color: Qt.rgba(0,0,0,0.95);
            radius: 5;
            visible: announcementHistoryVisible;

            Column {
                width: parent.width;
                height: parent.height;
                spacing: 2;

                Item {
                    height: 35;
                    width: parent.width;

                    RowLayout {
                        width: parent.width;
                        height: parent.height;

                        Row {
                            height: 20;

                            Switch { 
                                height: 20;
                                id: doNotDisturbSwitch;

                                indicator: Rectangle {
                                    implicitWidth: 48;
                                    implicitHeight: 20;
                                    x: doNotDisturbSwitch.leftPadding;
                                    y: parent.height / 2 - height / 2 + 5;
                                    radius: 13;
                                    color: doNotDisturbSwitch.checked ? "#17a81a" : "#ffffff";
                                    border.color: doNotDisturbSwitch.checked ? "#17a81a" : "#cccccc";

                                    Rectangle {
                                        x: doNotDisturbSwitch.checked ? parent.width - width : 0;
                                        width: 20;
                                        height: 20;
                                        radius: 13;
                                        color: doNotDisturbSwitch.down ? "#cccccc" : "#ffffff";
                                        border.color: doNotDisturbSwitch.checked ? (doNotDisturbSwitch.down ? "#17a81a" : "#21be2b") : "#999999";
                                    }
                                }

                                onClicked: {
                                    doNotDisturb = checked;
                                    toScript({type: "doNotDisturbState", state: doNotDisturb});
                                }
                            }

                            Text {
                                color: "White";
                                font.pixelSize: 18;
                                text: "Do not disturb";
                                font.weight: Font.Medium;
                                anchors.verticalCenter: parent.verticalCenter;
                            }
                        }

                        Row {
                            Layout.fillHeight: true;
                            width: 20;

                            Image {
                                source: "../img/delete.svg";
                                height: 20;
                                width: 20;
                                sourceSize.width: 128;
                                sourceSize.height: 128;
                                fillMode: Image.PreserveAspectFit;
                                anchors.centerIn: parent;

                                MouseArea {
                                    anchors.fill: parent;
                                    hoverEnabled: true;
                                    propagateComposedEvents: true;	

                                    onClicked: {
                                        notifications.clear();
                                    }
                                }
                            }
                        }
                    }

                }

                Flickable {
                    x: 5;
                    y: 5
                    width: parent.width - 5;
                    height: parent.height - 40;
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
                                color: Qt.rgba(0,0,0,0.95);
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

                                    onClicked: {
                                        showNotificationDetails(notifications.get(index).bubbleText, notifications.get(index).bubbleDetails)
                                    }

                                    onEntered: {
                                        parent.color = Qt.rgba(0,0,0,1);
                                    }

                                    onExited: {
                                        parent.color = Qt.rgba(0,0,0,0.95);
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

        }

        // Preview New Notification
        Rectangle {
            width: 300;
            height: 50;
            color: Qt.rgba(0,0,0,0.95);
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
                    parent.color = Qt.rgba(0,0,0,1);
                }

                onExited: {
                    parent.color = Qt.rgba(0,0,0,0.95);
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

        // Notification Details
        Rectangle {
            width: 300;
            height: children[0].height + 10;
            color: Qt.rgba(0,0,0,0.95);
            visible: notificationDetailsActive;
            radius: 5;

            ColumnLayout {
                width: parent.width - 15;
                height: notificationDetailsDetails.text == "" ? children[0].contentHeight + 10 : children[0].contentHeight + children[1].height + 10;
                anchors.centerIn: parent;
                spacing: 5;
                
                Text {
                    id: notificationDetailsTitle;
                    color: "white";
                    font.pixelSize: 20;
                    text: "";
                    wrapMode: Text.Wrap;
                    Layout.fillWidth: true;
                }

                Rectangle {
                    color: Qt.rgba(0,0,0,0.5);
                    width: parent.width;
                    Layout.fillHeight: true;
                    Layout.alignment: Qt.AlignHCenter;
                    x: 5;
                    radius: 5;
                    height: children[0].contentHeight;
                    visible: children[0].text != "";

                    Text {
                        id: notificationDetailsDetails;
                        width: parent.width - 10;
                        anchors.centerIn: parent;
                        wrapMode: Text.Wrap;
                        text: "";
                        color: "white";
                        font.pixelSize: 18;
                    }
                }
            }

            MouseArea {
                anchors.fill: parent;

                onPressed: {
                    notificationDetailsActive = false;
                }
            }

        }

        // Bell Icon and container
        Rectangle {
            color: Qt.rgba(0,0,0,0.95);
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
                id: notificationIcon;

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
                    parent.color = Qt.rgba(0,0,0,1);
                }

                onExited: {
                    parent.color = Qt.rgba(0,0,0,0.95);
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
            notificationDetailsActive = false;
        }
    }

	function addSystemNotification(message, details, skipVisualEffects = false) {
        var maximumNotifications = 20;
		var targetNotification = notificationId;

		// Insert notification to the stack
		notifications.insert(0, {bubbleText: message, bubbleDetails: details, id: targetNotification});

		notificationId = notificationId + 1;

        // Limit maximum displayed notifications
        if (notifications.count > maximumNotifications){
            notifications.remove(notifications.count - 1, 1);
        }

        if (skipVisualEffects === false && doNotDisturb === false) {
            // Display a preview
            if (announcementHistoryVisible === false && notificationDetailsActive === false) {
                if (previewNotificationTimer.running) {
                    previewNotificationTimer.restart();
                }
                previewNotificationText.text = message;
                previewNotificationActive = true;
            }

            // Visual effects.
            // shakeAnimation.start();
            if (announcementHistoryVisible === false) {
                hasUnread = true;
                // shakeAnimationTimer.running = true;
            }
        }
	}

    function showNotificationDetails(title, details) {
        notificationDetailsTitle.text = title;
        notificationDetailsDetails.text = details;

        announcementHistoryVisible = false;
        previewNotificationActive = false;
        notificationDetailsActive = true;
    }

    // Messages from script
    function fromScript(message) {
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