import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import "./qml"
import "./qml/pages"

Rectangle {
    signal sendToScript(var message);
	color: Qt.rgba(0.1,0.1,0.1,1);
	width: parent.width;
	height: parent.height;
	anchors.centerIn: parent;
	anchors.horizontalCenter: parent.horizontalCenter
	property var pages: [
		{name: "General", icon: "../img/overte.svg", targetPage: "hifi/tablet/TabletGeneralPreferences.qml" },
		{name: "Graphics", icon: "../img/computer.svg", targetPage: "" }, 
		{name: "Audio", icon: "../img/volume.svg", targetPage: "hifi/audio/Audio.qml" }, 
		{name: "Controls", icon: "../img/dpad.svg", targetPage: "hifi/tablet/ControllerSettings.qml" }, 
		{name: "Security", icon: "../img/badge.svg", targetPage: "hifi/dialogs/security/Security.qml" }, 
		{name: "QML Allowlist", icon: "../img/lock.svg", targetPage: "hifi/dialogs/security/EntityScriptQMLAllowlist.qml" }, 
		{name: "Script Security", icon: "../img/shield.svg", targetPage: "hifi/dialogs/security/ScriptSecurity.qml" }, 
	];
	property string currentPage: "Settings"

	ColumnLayout {
		width: parent.width
		height: parent.height
		anchors.horizontalCenter: parent.horizontalCenter
		id: root

		// Navigation Header
		HeaderElement {
			id: header
		}

		// Home page
		SettingCenterContainer {
			id: homePage
			visible: currentPage == "Settings"
			Layout.fillHeight: true

			Repeater {
				model: pages.length;
				delegate: SettingSubviewListElement {
					property string pageName: pages[index].name;
					property string pageIcon: pages[index].icon;
					property string targetPage: pages[index].targetPage;
				}
			}
		}

		// Graphics 
		GraphicsSettings {}

		// Templates
	}

	// Messages from script
	function fromScript(message) {
		switch (message.type){
			case "loadPage":
				currentPage = message.page;
				break;
		}
	}

	// Send message to script
	function toScript(packet){
		sendToScript(packet)
	}
}
