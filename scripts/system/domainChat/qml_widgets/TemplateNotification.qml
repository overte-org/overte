import QtQuick 2.7
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Component {
	id: template_notification

	Rectangle {
		color: "#171717"
		width: parent.width
		height: 40

		RowLayout {
			width: parent.width
			height: parent.height

			Rectangle {
				height: parent.height
				width: 5
				color: "#505186"
			}

			Repeater {
				model: delegateText

				TextEdit {
					visible: model.value != undefined;
					text: model.value || ""
					color: "white"
					font.pointSize: 12
					readOnly: true
					selectByMouse: true
					selectByKeyboard: true
					height: root.height
					wrapMode: Text.Wrap
					font.italic: true
				}
			}
		}
	}
}
