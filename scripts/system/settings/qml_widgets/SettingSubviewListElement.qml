import QtQuick 2.15

Item {
	width: parent.width
	height: 60

	Rectangle {
		width: parent.width
		height: parent.height
		color: index % 2 === 0 ? Qt.rgba(0,0,0,0) : Qt.rgba(0.15,0.15,0.15,1)
		anchors.fill: parent
	}

	Text {
		text: page_name
		color: "white"
		font.pointSize: 16
		anchors.verticalCenter: parent.verticalCenter
		x: 10
	}

	MouseArea {
		anchors.fill: parent
		onClicked: {
			current_page = page_name;
		}
	}
}