import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

ColumnLayout {
	width: parent.width;
	anchors.horizontalCenter: parent.horizontalCenter;
	height: parent.height;
	
	Text {
		id: label;
		text: "Connections";
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
			width: parent.width;
			spacing: 0;

			Repeater {
				model: connections.connections.length;

				delegate: UserConnectionElement {
					property string displayName: connections.connections[index].username;
					property string icon: connections.connections[index].images.thumbnail;
					property bool isFriend: connections.connections[index].connection == "friend";
				}
			}
		}
	}
}