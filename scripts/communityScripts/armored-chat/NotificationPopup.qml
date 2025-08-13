import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3

Rectangle {
	property string chatMessage: "";
	property string chatAuthor: "";
	property int notificationLifetimeMS: 10000;
	property int notificationFadeoutMS: 150;
	property int notificationId: 0;

	color: Qt.rgba(0,0,0,0.95);
	width: parent && parent.width || 0;
	height: children[0].height;
	radius: 5;
	clip: true;
	id: root;

	Column {
		height: children[0].contentHeight + children[1].contentHeight + 10;
		width: parent.width - 10;
		anchors.centerIn: parent;
		clip: true;

		Text {
			width: parent.width - 10;
			color: "white";
			text: chatAuthor;
			font.pixelSize: 16;
			wrapMode: Text.Wrap;
			horizontalAlignment: Text.AlignLeft;
		}

		Text {
			width: parent.width - 20;
			color: "white";
			text: chatMessage;
			font.pixelSize: 16;
			wrapMode: Text.Wrap;
			horizontalAlignment: Text.AlignLeft;
			anchors.horizontalCenter: parent.horizontalCenter;
		}
	}

	Behavior on height {
		NumberAnimation {
			duration: notificationFadeoutMS;
			easing.type: Easing.InOutCubic;
		}
	}

	Component.onCompleted: {
		root.height = Math.max(children[0].contentHeight + 10, 50);
		fadeoutTimeout.running = true;
		deleteTimeout.running = true;
	}
	
	Timer {
		id: fadeoutTimeout;
		interval: notificationLifetimeMS - notificationFadeoutMS;
		repeat: false;
		onTriggered: {
			root.height = 0;
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
		notifications.remove(notificationId);
	}
}