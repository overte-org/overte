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
		width: parent.width;
		height: parent.height;

		Rectangle {
			color: settingEnabled ? "#077e30" : "darkgray";
			height: parent.parent.height - 15;
			width: 90;
			radius: 10;

			Text {
				width: parent.width;
				height: parent.height;
				text: settingEnabled ? "On" : "Off";
				color: "white";
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
			anchors.left: parent.children[0].right + 5;
			height: parent.height;
			text: settingText;
			color: "white";
			font.pointSize: 14;
		}
	}
}