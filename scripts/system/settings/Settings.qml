import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import "./qml_widgets"
import "./qml_widgets/pages"

Rectangle {
    signal sendToScript(var message);
	color: Qt.rgba(0.1,0.1,0.1,1);
	width: parent.width;
	height: parent.height;
	anchors.centerIn: parent;
	anchors.horizontalCenter: parent.horizontalCenter
	property var pages: [
		{name: "General", icon: "../img/overte.svg", target_page: "hifi/dialogs/GeneralPreferencesDialog.qml" }, 
		{name: "Graphics", icon: "../img/computer.svg", target_page: "" }, 
		{name: "Audio", icon: "../img/volume.svg", target_page: "hifi/audio/Audio.qml" }, 
		{name: "Controls", icon: "../img/dpad.svg", target_page: "hifi/tablet/ControllerSettings.qml" }, 
		{name: "Security", icon: "../img/badge.svg", target_page: "hifi/dialogs/security/Security.qml" }, 
		{name: "QML Allowlist", icon: "../img/lock.svg", target_page: "hifi/dialogs/security/EntityScriptQMLAllowlist.qml" }, 
		{name: "Script Security", icon: "../img/shield.svg", target_page: "hifi/dialogs/security/ScriptSecurity.qml" }, 
	];
	property string current_page: "Settings"

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
			id: home_page
			visible: current_page == "Settings"
			Layout.fillHeight: true

			Repeater {
				model: pages.length;
				delegate: SettingSubviewListElement {
					property string page_name: pages[index].name;
					property string page_icon: pages[index].icon;
					property string target_page: pages[index].target_page;
				}
			}
		}

		// Graphics 
		GraphicsSettings {}

		// Audio
		SettingCenterContainer {
			id: audio_page
			visible: current_page == "Audio"
		}

		// Templates
	}

	// Messages from script
	function fromScript(message) {
		switch (message.type){
			case "loadPage":
				current_page = message.page;
				break;
		}
	}

	// Send message to script
	function toScript(packet){
		sendToScript(packet)
	}
}
