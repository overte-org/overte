import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
	id: root;
	property string settingText: "";
	property bool settingEnabled: false;

	height: 50;
	width: parent.width;

	RowLayout {
		width: parent.width - 10;
		height: parent.height;
		anchors.horizontalCenter: parent.horizontalCenter;


		Text {
			id: settingTextElem
			height: parent.height;
			text: settingText;
			color: "white";
			font.pointSize: 14;
		}


		RowLayout {
			Layout.alignment: Qt.AlignRight;
			anchors.left: settingTextElem.right + 5;

			Text {
				text: "<";
				font.pointSize: 16;
				color: "white";
			}

			Rectangle {
				color: "transparent";
				height: parent.parent.height - 15;
				width: 200;
				radius: 10;
				border.color: settingEnabled ? "white" : "#333";
				border.width: 1;

				Text {
					width: parent.width;
					height: parent.height;
					text: settingEnabled ? "Enabled" : "Disabled";
					color: settingEnabled ? "white" : "gray";
					horizontalAlignment: Text.AlignHCenter;
					verticalAlignment: Text.AlignVCenter;
					font.pointSize: 14;
				}

				MouseArea {
					anchors.fill: parent

					onClicked: {
						settingEnabled = !settingEnabled
					}
				}

				Behavior on color {
					ColorAnimation {
						duration: 70
					}
				}
			}

			Text {
				text: ">";
				font.pointSize: 16;
				color: "white";
			}
		}

	}
}