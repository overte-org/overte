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
	property var canKick: false;	// The only way to tell if a user is an admin of a domain is if they have the kick permissions
	property var myData: {icon: "../img/default_profile_avatar.svg"; displayName: ""; sessionDisplayName: ""};
	property var focusedUserData: {sessionDisplayName: ""; audioLoudness: 0.01};
	property var focusedUser: "";
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
			sessionDisplayName: focusedUserData.sessionDisplayName;
		}

		UserAudio {
		}

		UserOptions {
			Layout.fillHeight: true;

			UserOptionButton {
				buttonText: "Teleport";
				action: () => {
					var avatar = AvatarList.getAvatar(focusedUser);
					MyAvatar.goToLocation(avatar.position, true, Quat.cancelOutRollAndPitch(avatar.orientation), true);
				};
			}
			UserOptionButton {
				buttonText: "Mute";
				action: () => {Users.personalMute(focusedUser, !Users.getPersonalMuteStatus(focusedUser))};
			}
			UserOptionButton {
				buttonText: "Ignore";
				action: () => {Users.ignore(focusedUser, !Users.getIgnoreStatus(focusedUser))};
			}

			UserOptionButton {
				buttonText: "Kick";
				visible: canKick;
				isDangerButton: true;
				action: () => {Users.kick(focusedUser, Users.NO_BAN)};
			}
			UserOptionButton {
				buttonText: "Ban";
				visible: canKick;
				isDangerButton: true;
				action: () => {Users.kick(focusedUser, Users.BAN_BY_USERNAME | Users.BAN_BY_FINGERPRINT | Users.BAN_BY_IP)};
			}
			UserOptionButton {
				buttonText: "Silence";
				visible: canKick;
				isDangerButton: true;
				action: () => {Users.mute(focusedUser)};
			}
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
		focusedUser = sessionUUID;
		focusedUserData = users.filter((user) => user.sessionUUID === focusedUser)[0];
		page = "User";
		toScript({type: "focusedUser", user: focusedUser});
	}

	function fromScript(message) {
		if (message.type == "myData"){
			myData = message.data;
			canKick = message.data.canKick;
			return;
		}

		if (message.type == "palList") {
			users = message.data;
			if (focusedUser) focusedUserData = users.filter((user) => user.sessionUUID === focusedUser)[0];
			// print(JSON.stringify(users, null, 4));
			return;
		}
	}

	// Send message to script
	function toScript(packet){
		sendToScript(packet)
	}
}

