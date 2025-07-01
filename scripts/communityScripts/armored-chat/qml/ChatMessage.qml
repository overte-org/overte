import QtQuick 2.7
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
	width: parent && parent.width - 10 || 0;
	height: children[0].height + 10;
	property int delegateIndex: 0;
	property string delegateText: "";
	property string delegateUsername: "";
	property string delegateDate: "";

	ColumnLayout {
		width: parent.width - 20;
		x: 10;

		// Message head
		RowLayout {
			width: parent.width;
			height: 50;

			Text {
				text: delegateUsername;
				font.pixelSize: 16;
				color: "gray";
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
		Item {
			width: parent.width;
			height: children[0].contentHeight;

			Text {
				text: delegateText;
				width: parent.width;
				color: "white";
				font.pixelSize: 18;
				wrapMode: Text.Wrap;
				height: contentHeight;
			}
		}

		// Embed area 
		// ColumnLayout {
		// 	width: parent.width;
			

		// }

	}

	Rectangle {
		color: Qt.rgba(1,1,1,0.025);
		z: -1;
		anchors.centerIn: parent;
		anchors.fill: parent;
	}
}