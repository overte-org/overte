import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

ColumnLayout {
	width: parent.width;
	height: parent.height;
	id: userOptionsRoot;

	Text {
		id: label;
		text: "Options";
		color: "white";
		font.pointSize: 18;
	}

	ScrollView {
		width: parent.width;
		contentHeight: gridLay.height;
		Layout.fillHeight: true;
		Layout.fillWidth: true;
		clip: true;

		Grid {
			id: gridLay;
			columns: 2
			spacing: 10
			width: parent.width;
			anchors.horizontalCenter: parent.horizontalCenter;

			Rectangle {
				color: "red";
				width: parent.width;
			}
		}

	}

	Component.onCompleted: {
		while (userOptionsRoot.children.length > 2){
			userOptionsRoot.children[2].parent = gridLay;
		}
	}
}