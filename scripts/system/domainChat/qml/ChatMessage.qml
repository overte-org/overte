import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
	width: parent && parent.width - 10 || 0;
	height: children[0].height + 10;
	property int delegateIndex: 0;
	property string delegateMessage: "";
	property string delegateUsername: "";
	property string delegateDate: "";
	property bool isSystem: false;

	Column {
		width: parent.width - 20;
		x: 10;

		// Message head
		RowLayout {
			width: parent.width;

			Text {
				text: delegateUsername;
				font.pixelSize: 16;
				color: isSystem ? "#00aaff" : "gray";
			}

			Text {
				text: delegateDate;
				font.pixelSize: 14;
				color: "gray";
				Layout.fillWidth: true;
				horizontalAlignment: Text.AlignRight;
			}
		}

		// Message body
		TextEdit { 
			text: delegateMessage;
			color: "white";
			font.pixelSize: 18;
			wrapMode: Text.Wrap;
			textFormat: TextEdit.RichText;
			width: parent.width;
			readOnly: true
			selectByMouse: true
			selectByKeyboard: true

			onLinkActivated: {
				if (link.includes("?noOpen=true")) {
					// Don't open this in external browser
					link = link.replace("?noOpen=true", "");
					Window.openWebBrowser(link);
					return;
				} else {
					Qt.openUrlExternally(link);
				}
			}
		}
	}

	Rectangle {
		color: Qt.rgba(1,1,1,0.025);
		z: -1;
		anchors.centerIn: parent;
		anchors.fill: parent;
	}

	Rectangle {
		visible: isSystem;
		color: "#00aaff";
		z: -1;
		width: 5;
		height: parent.height;
	}
}