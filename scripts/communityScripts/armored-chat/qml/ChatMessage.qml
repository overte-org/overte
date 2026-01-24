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
			id: messageText;
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

			Component.onCompleted: {
				// Images do not appear when done loading until
				// the component is redrawn, which it does not do
				// automatically; so we need to create a hidden
				// Image object and wait for that to load
				// before we cause it to redraw by altering the content.
				var re = /<img[^>]+src="([^"]+)"[^>]+>/gi
				var m;
				while ((m = re.exec(text)) !== null) {
					var loader = Qt.createQmlObject('import QtQuick 2.0; Image {}', messageText);
					loader.source = m[1];
					loader.visible = false;
					loader.onStatusChanged.connect(function () {
						if (loader.status === Image.Ready) {
							// Alter the content of messageText
							// to prompt it to redraw
							var originalText = messageText.text;
							messageText.text = "";          // force a change
							messageText.text = originalText;
						}
					});
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
