import QtQuick 2.7
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Component {
	id: template_notification

	Rectangle {
		property int index: delegateIndex
		property string username: delegateUsername

		color: "#171717"
		width: parent.width
		height: 40

		Item {
			width: 10
			height: parent.height

			Rectangle {
				height: parent.height
				width: 5
				color: "#505186"
			}
		}

		Item {
			width: parent.width - parent.children[0].width - 5
			height: parent.height
			anchors.left: parent.children[0].right

			TextEdit {
				text: delegateText
				color: "white"
				font.pointSize: 12
				readOnly: true
				width: parent.width * 0.8
				selectByMouse: true
				selectByKeyboard: true
				height: parent.height
				wrapMode: Text.Wrap
				verticalAlignment: Text.AlignVCenter
				font.italic: true
			}

			Text {
				text: delegateDate
				color: "white"
				font.pointSize: 12
				anchors.right: parent.right
				height: parent.height
				wrapMode: Text.Wrap
				horizontalAlignment: Text.AlignRight
				verticalAlignment: Text.AlignVCenter
				font.italic: true
			}
		}
	}
}