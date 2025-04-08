import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Column {
	width: parent.width;
	anchors.horizontalCenter: parent.horizontalCenter;
	height: parent.height;
	
	Text {
		id: label;
		text: "Present Users";
		color: "white";
		font.pointSize: 18;
	}

	Flickable {
		visible: true;
		width: parent.width;
		height: parent.height;
		Layout.fillHeight: true;
		y: label.height + 10;
		contentWidth: parent.width;
		contentHeight: flickableContent.height;
		clip: true;

		Column {
			id: flickableContent;
			height: parent.height;
			width: parent.width;
			spacing: 5;

			Repeater {
				model: users.length;

				delegate: PresentUserListElement {
					property string sessionDisplayName: users[index].sessionDisplayName;
					property string sessionUUID: users[index].sessionUUID;
				}
			}
		}
	}
}