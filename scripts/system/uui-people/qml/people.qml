import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import "./widgets"


Rectangle {
    signal sendToScript(var message);
	color: Qt.rgba(0.1,0.1,0.1,1);
	width: parent.width;
	height: parent.height;
	anchors.centerIn: parent;
	anchors.horizontalCenter: parent.horizontalCenter;

	property var users: [];
	property var isAdmin: false;
	property var myData: {};
	property var focusedUserData: {};
	property var page: "Home";
	property var pages: ["Home", "User"];

	Column {
		// Home page
		width: parent.width - 20;
		height: parent.height;
		spacing: 15;
		anchors.horizontalCenter: parent.horizontalCenter;
		visible: page == "Home";

		Item {
			// Spacer
			height: 1;
			width: 1;
		}
		
		UserAbout {
			// Current user who is logged in
			sessionDisplayName: myData.displayName;
			icon: myData.icon;
			isSelf: true;
		}

		UserList {
			// List of connected users
		}
	}

	ColumnLayout {
		// Focused user page
		width: parent.width - 20;
		height: parent.height;
		spacing: 15;
		anchors.horizontalCenter: parent.horizontalCenter;
		visible: page == "User";

		Item {
			// Spacer
			height: 1;
			width: 1;
		}
		
		UserAbout {
			id: focusedUserAbout;
			sessionDisplayName: focusedUserData.displayName;
		}

		UserAudio {
		}

		UserOptions {
			Layout.fillHeight: true;
		}

		BackButton {
			
		}

		Item {
			// Spacer
			height: 1;
			width: 1;
		}
		
	}

	function toUserPage(sessionUUID){
		focusedUserData = {};
		toScript({type: "getUserData", user: sessionUUID});
	}

	function fromScript(message) {
		if (message.type == "myData"){
			myData = message.data;
			return;
		}

		if (message.type == "palList") {
			users = message.data;
			return;
		}

		if (message.type == "isAdmin") {
			isAdmin = message.isAdmin;
			return;
		}

		if (message.type == "focusedUserData"){
			focusedUserData = message.data;
			
			focusedUserAbout.sessionDisplayName = focusedUserData.sessionDisplayName;
			print(`Focused User Data:\n${JSON.stringify(focusedUserData, null, 4)}`);
			page = "User";
			return;
		}
	}

	// Send message to script
	function toScript(packet){
		sendToScript(packet)
	}
}

