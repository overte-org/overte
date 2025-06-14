import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11


Item {
	property int imageSize: 65;
	property string icon: "";
	width: imageSize;
	height: imageSize;

	Rectangle {
		color: colors.darkBackground3;
		radius: 10;
		border.color: "white";
		width: imageSize;
		height: imageSize;

		Image {
			anchors.centerIn: parent;
			source: icon;
			width: imageSize - 10;
			height: imageSize - 10;
			sourceSize.width: imageSize - 100;
			sourceSize.height: imageSize - 100;
		}
	}
}
