import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import ".." as Overte
import "."

Rectangle {
	id: root
	anchors.fill: parent
	implicitWidth: 480
	implicitHeight: 720
	color: Overte.Theme.paletteActive.base

	property string localSearchExpression: ".*"
	property string accountSearchExpression: ".*"

	property list<var> localContactsModel: [
		{user: "ada.tv", name: "ada.tv", volume: 1.0, badgeIconSource: "../icons/gold_star.svg"},
		{user: "admin", name: "Admin", volume: 1.0, badgeIconSource: "../icons/admin_shield.svg"},
		{user: "{81cb9b67-d034-40f6-9ab7-a4ecb62f8961}", name: "Anonymous", volume: 1.0, badgeIconSource: ""},
		{user: "", name: "Not logged in", volume: 1.0, badgeIconSource: ""},

		{user: "ScrollingTest", name: "Scroll test", volume: 1.0, badgeIconSource: ""},
		{user: "ScrollingTest", name: "Scroll test", volume: 1.0, badgeIconSource: ""},
		{user: "ScrollingTest", name: "Scroll test", volume: 1.0, badgeIconSource: ""},
		{user: "ScrollingTest", name: "Scroll test", volume: 1.0, badgeIconSource: ""},
		{user: "ScrollingTest", name: "Scroll test", volume: 1.0, badgeIconSource: ""},
		{user: "ScrollingTest", name: "Scroll test", volume: 1.0, badgeIconSource: ""},
		{user: "ScrollingTest", name: "Scroll test", volume: 1.0, badgeIconSource: ""},
		{user: "ScrollingTest", name: "Scroll test", volume: 1.0, badgeIconSource: ""},
		{user: "ScrollingTest", name: "Scroll test", volume: 1.0, badgeIconSource: ""},
		{user: "ScrollingTest", name: "Scroll test", volume: 1.0, badgeIconSource: ""},
		{user: "ScrollingTest", name: "Scroll test", volume: 1.0, badgeIconSource: ""},
		{user: "ScrollingTest", name: "Scroll test", volume: 1.0, badgeIconSource: ""},
		{user: "ScrollingTest", name: "Scroll test", volume: 1.0, badgeIconSource: ""},
	]

	property list<var> accountContactsModel: [
		/*{user: "ada.tv", avatarUrl: "file:///home/ada/art/doodles/bevy blep avi.png", status: 0, friend: true, currentPlaceName: "overte_hub"},
		{user: "x74hc595", avatarUrl: "https://cdn.discordapp.com/avatars/761127759367634965/915ea6b0e6b380458bce46616fa1fe35.webp?size=96", status: 2, friend: true, currentPlaceName: "overte_hub"},
		{user: "juliangro", avatarUrl: "https://cdn.discordapp.com/avatars/181488002831351808/7c17a81a149da388d078d8ac795f64fe.webp?size=96", status: 1, friend: true, currentPlaceName: "overte_hub"},*/
		{user: "AccountContact", avatarUrl: "file:///home/ada/art/doodles/bevy blep avi.png", status: 0, friend: false, currentPlaceName: ""},
		{user: "AccountContact", avatarUrl: "file:///home/ada/art/doodles/bevy blep avi.png", status: 1, friend: false, currentPlaceName: ""},
		{user: "AccountContact", avatarUrl: "file:///home/ada/art/doodles/bevy blep avi.png", status: 2, friend: false, currentPlaceName: "overte_hub"},
		{user: "AccountContact", avatarUrl: "file:///home/ada/art/doodles/bevy blep avi.png", status: 3, friend: false, currentPlaceName: "overte_hub"},
		{user: "Friend", avatarUrl: "file:///home/ada/art/doodles/bevy blep avi.png", status: 2, friend: true, currentPlaceName: "overte_hub"},
	]

	ColumnLayout {
		anchors.fill: root

		MyAccountInfo {
			Layout.fillWidth: true
			status: MyAccountInfo.Status.LoggedOut
			id: myAccountInfo
		}

		Overte.TabBar {
			Layout.fillWidth: true
			id: tabBar

			Overte.TabButton { text: qsTr("Local") }
			Overte.TabButton { text: qsTr("Account") }
		}

		StackLayout {
			currentIndex: tabBar.currentIndex

			// Local
			ColumnLayout {
				ListView {
					Layout.fillWidth: true
					Layout.fillHeight: true
					clip: true

					ScrollBar.vertical: Overte.ScrollBar {
						policy: ScrollBar.AsNeeded
					}

					contentWidth: root.width - Overte.Theme.scrollbarWidth

					model: {
						const regex = new RegExp(localSearchExpression, "i");
						let tmp = [];

						for (const item of localContactsModel) {
							if (item.name.match(regex) || item.user.match(regex)) {
								tmp.push(item);
							}
						}

						return tmp;
					}
					delegate: SessionContact {}
				}

				RowLayout {
					Layout.fillWidth: true
					Layout.leftMargin: 4
					Layout.rightMargin: 4

					Overte.TextField {
						Layout.fillWidth: true

						Keys.onEnterPressed: {
							localSearchButton.clicked();
							forceActiveFocus();
						}

						Keys.onReturnPressed: {
							localSearchButton.clicked();
							forceActiveFocus();
						}

						placeholderText: qsTr("Search…")
						id: localSearchField
					}

					Overte.RoundButton {
						icon.source: "../icons/search.svg"
						icon.width: 24
						icon.height: 24
						icon.color: Overte.Theme.paletteActive.buttonText
						id: localSearchButton

						onClicked: localSearchExpression = localSearchField.text
					}
				}
			}

			// Account
			ColumnLayout {
				ListView {
					Layout.fillWidth: true
					Layout.fillHeight: true
					clip: true
					visible: myAccountInfo.status !== MyAccountInfo.Status.LoggedOut

					ScrollBar.vertical: Overte.ScrollBar {
						policy: ScrollBar.AsNeeded
					}

					contentWidth: root.width - Overte.Theme.scrollbarWidth

					model: {
						const regex = new RegExp(accountSearchExpression, "i");
						let tmp = [];

						for (const item of accountContactsModel) {
							if (item.user.match(regex)) {
								tmp.push(item);
							}
						}

						return tmp;
					}
					delegate: AccountContact {}
				}

				RowLayout {
					Layout.fillWidth: true
					Layout.leftMargin: 4
					Layout.rightMargin: 4
					visible: myAccountInfo.status !== MyAccountInfo.Status.LoggedOut

					Overte.TextField {
						Layout.fillWidth: true

						Keys.onEnterPressed: {
							accountSearchButton.clicked();
							forceActiveFocus();
						}

						Keys.onReturnPressed: {
							accountSearchButton.clicked();
							forceActiveFocus();
						}

						placeholderText: qsTr("Search…")
						id: accountSearchField
					}

					Overte.RoundButton {
						icon.source: "../icons/search.svg"
						icon.width: 24
						icon.height: 24
						icon.color: Overte.Theme.paletteActive.buttonText
						id: accountSearchButton

						onClicked: accountSearchExpression = accountSearchField.text
					}
				}

				Overte.Label {
					Layout.fillWidth: true
					Layout.fillHeight: true
					visible: myAccountInfo.status === MyAccountInfo.Status.LoggedOut
					wrapMode: Text.Wrap
					horizontalAlignment: Text.AlignHCenter
					verticalAlignment: Text.AlignVCenter
					text: qsTr("Log in to track contacts")
				}
			}
		}
	}
}
