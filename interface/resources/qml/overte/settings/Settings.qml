import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../" 1.0 as Overte
import "." as OverteSettings
import "./pages" as SettingsPages

Rectangle {
	id: root
	width: 480
	height: 720
	visible: true
	anchors.fill: parent
	color: Overte.Theme.paletteActive.base

	Overte.TabBar {
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.top: parent.top
		id: tabBar

		Overte.TabButton { text: qsTr("General") }
		Overte.TabButton { text: qsTr("Graphics") }
		Overte.TabButton { text: qsTr("Controls") }
		Overte.TabButton { text: qsTr("Audio") }
	}

	StackLayout {
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.top: tabBar.bottom
		anchors.bottom: parent.bottom
		anchors.topMargin: Overte.Theme.fontPixelSize
		currentIndex: tabBar.currentIndex

		SettingsPages.General {}

		SettingsPages.Graphics {}

		SettingsPages.Controls {}

		SettingsPages.Audio {}
	}
}
