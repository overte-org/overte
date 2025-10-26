import QtQuick
import QtQuick.Layouts

import "../" as Overte

RowLayout {
	property alias text: labelItem.text
	property alias value: switchItem.checked
	property bool enabled: true

	id: item
	anchors.left: parent.left
	anchors.right: parent.right
	anchors.margins: 16
	spacing: 16

	Overte.Label {
		Layout.fillWidth: true

		id: labelItem
		wrapMode: Text.Wrap
	}

	Overte.Switch {
		id: switchItem
		enabled: item.enabled
	}
}
