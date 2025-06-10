import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import "."
import "./widgets"
import "./widgets/CustomListView"
import "./pages"

Rectangle {
	signal sendToScript(var message);
	property var appList: [];
	property var repositoryList: [];
	property int focusedAppIndex: -1;

	Colors {
		id: colors;
	}

	width: parent.width;
	height: parent.height;
	color: colors.darkBackground1;

	ColumnLayout {
		width: parent.width;
		height: parent.height;

		// App list page
		ApplicationListPage {
			id: applicationListPage;
			visible: true;
		}

		// App detail page
		ApplicationDetailPage {
			id: applicationDetailPage;
			visible: false;
			Layout.fillHeight: true;
		}

		// App settings page
		SettingsPage {
			id: settingsPage;
			visible: false;
			entryList: repositoryList;
			Layout.fillHeight: true;
		}

	}


	function showAppDetailPage() {
		hideAllPages();
		applicationDetailPage.visible = true;
	}

	function showAppListPage() {
		hideAllPages();
		applicationListPage.visible = true;
	}

	function showSettingsPage() {
		hideAllPages();
		settingsPage.visible = true;
	}

	function hideAllPages() {
		applicationListPage.visible = false;
		applicationDetailPage.visible = false;
		settingsPage.visible = false;
	}

	function installApp(appUrl, baseUrl) {
		toScript({type: "installApp", appUrl, baseUrl});
	}

	function uninstallApp(appUrl, baseUrl) {
		toScript({type: "uninstallApp", appUrl, baseUrl});
	}

	function openAppRepository(repositoryUrl) {
		toScript({type: "openAppRepository", repositoryUrl});
	}

	function onRemoveEntryButton(entryUrl) {
		toScript({type: "removeRepository", entryUrl});
	}

	function onSearchChanged(searchQuery) {
		for (let i = 0; appList.length > i; i++){
			// For each of the appList entries...

			var app = appList[i];
			var appEntry = applicationListPage.children[1].children[0].children[0].children[i];
			var appNameLowercase = app.appName.toLowerCase();
			var searchQueryLowercase = searchQuery.toLowerCase();

			if (appNameLowercase.includes(searchQueryLowercase)) {
				appEntry.visible = true;
			}
			else {
				appEntry.visible = false;
			}
		}
	}

	// Messages from script
	function fromScript(message) {
		switch (message.type) {
			case "appList": 
				appList = message.appList;
				break;
			case "repositoryList":
				repositoryList = message.repositoryList;
				break;
		}
	}

	// Send message to script
	function toScript(packet){
		sendToScript(packet)
	}
}

