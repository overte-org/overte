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
	property var myData: {icon: "../img/default_profile_avatar.svg"; displayName: ""; username: ""};
	property var focusedUserData: {sessionDisplayName: ""; audioLoudness: 0.0};
	property var focusedUser: null;
	property var page: "Home";
	property var pages: ["Home", "User", "EditSelf"];
	property var adminUserData: {};

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

			MouseArea {
				anchors.fill: parent;

				onClicked: {
					page = "EditSelf";
				}
			}
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
			sessionDisplayName: focusedUserData && focusedUserData.sessionDisplayName || "";
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
				action: () => {toScript({type: "ignoreUser", sessionUUID: focusedUser, user: focusedUserData})};
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

	ColumnLayout {
		// Edit self information
		width: parent.width - 20;
		height: parent.height;
		spacing: 15;
		anchors.horizontalCenter: parent.horizontalCenter;
		visible: page == "EditSelf";


		Item {
			// Spacer
			height: 1;
			width: 1;
		}
		
		UserAboutEdit {
			sessionDisplayName: myData.displayName;
			icon: myData.icon;
			isSelf: true;
		}

		ColumnLayout {
			width: parent.width;
			Layout.fillHeight: true;

			Text {
				text: "Availability";
				color: "white";
				font.pointSize: 18; // TODO: Sync with the other label
			}
			
			UserAboutAvailability {
				status: "all";
			}
			UserAboutAvailability {
				status: "connections";
			}
			UserAboutAvailability {
				status: "friends";
			}
			UserAboutAvailability {
				status: "none";
			}
		}



		Item {
			// Spacer
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
			else focusedUserData = {sessionDisplayName: "", audioLoudness: 0.0};
			return;
		}

		if (message.type == "adminUserData") {
			adminUserData = message.data;
			return;
		}
	}

	// Send message to script
	function toScript(packet){
		sendToScript(packet)
	}
}

