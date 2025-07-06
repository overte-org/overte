import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3

Rectangle {
	property string bubbleText: "";
	property string bubbleDetails: "";
	property int bubbleId: 0;
	property bool isClosing: false;
	property int notificationLifetimeMS: 5000;
	property int notificationFadeoutMS: 150;

	color: Qt.rgba(0,0,0,0.9);
	width: parent && parent.width || 0;
	height: 1;
	radius: 10;
	clip: true;
	id: bubble;
	
	Text {
		width: parent.width;
		anchors.centerIn: parent;
		color: "white";
		text: bubbleText;
		font.pixelSize: 16;
		wrapMode: Text.Wrap;
		horizontalAlignment: Text.AlignHCenter;
	}

	// Timeout progress bar
	Rectangle {
		y: 45;
		width: parent.width;
		height: 5;
		color: Qt.rgba(1,1,1,0.8);
		radius: 10;

		Behavior on width {
			NumberAnimation {
				duration: notificationLifetimeMS;
			}
		}

		Component.onCompleted: {
			width = 0;
		}
	}


	MouseArea {
		anchors.fill: parent;
		hoverEnabled: true;
		propagateComposedEvents: true;	

		onPressed: {
			toScript({type: "openNotificationFromOverlay", bubbleDetails});
		}

		onEntered: {
			parent.color = Qt.rgba(0,0,0,1);
		}

		onExited: {
			parent.color = Qt.rgba(0,0,0,0.9);
		}
	}

	Behavior on height {
		NumberAnimation {
			duration: notificationFadeoutMS;
			easing.type: Easing.InOutCubic;
		}
	}

	Component.onCompleted: {
		bubble.height = 50;
		fadeoutTimeout.running = true;
		deleteTimeout.running = true;
	}
	
	Timer {
		id: fadeoutTimeout;
		interval: notificationLifetimeMS - notificationFadeoutMS;
		repeat: false;
		onTriggered: {
			bubble.height = 0;
		}
	}

	Timer {
		id: deleteTimeout;
		interval: notificationLifetimeMS;
		repeat: false;
		onTriggered: {
			removeBubble();
		}
	}

	function removeBubble() {
		notifications.remove(bubbleId);
		if (typeof sendBubbleCountUpdate == 'function') sendBubbleCountUpdate();
	}
}