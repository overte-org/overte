import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../" as Overte

RowLayout {
	property alias text: labelItem.text
	property alias model: comboItem.model
	property alias textRole: comboItem.textRole
	property alias valueRole: comboItem.valueRole
	property alias currentIndex: comboItem.currentIndex
    property alias enabled: comboItem.enabled

	id: item
	anchors.left: parent.left
	anchors.right: parent.right
	anchors.margins: 16
	spacing: 16

	Overte.Label {
		// equally sized items
		Layout.preferredWidth: 1
		Layout.fillWidth: true

		id: labelItem
		wrapMode: Text.Wrap
	}

	Overte.ComboBox {
		// equally sized items
		Layout.preferredWidth: 1
		Layout.fillWidth: true

		id: comboItem
	}
}
