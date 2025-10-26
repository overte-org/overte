import QtQuick
import QtQuick.Layouts

import "../" as Overte

ColumnLayout {
	property alias text: labelItem.text
	property alias value: textFieldItem.text
    property bool enabled: true

	id: item
	anchors.left: parent.left
	anchors.right: parent.right
	anchors.margins: 16
	spacing: 4

    // prevent binding loops
    Component.onCompleted: value = value

	Overte.Label {
		Layout.alignment: Qt.AlignBottom
		id: labelItem
		wrapMode: Text.Wrap
	}

	RowLayout {
		Layout.fillWidth: true

		Overte.TextField {
			Layout.fillWidth: true

			id: textFieldItem
            enabled: item.enabled
		}

        Overte.RoundButton {
            enabled: item.enabled

			icon.source: "../icons/folder.svg"
			icon.width: 24
            icon.height: 24

            // TODO
		}
	}
}
