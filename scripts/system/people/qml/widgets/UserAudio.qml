import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Column {
	width: parent.width;
	height: 50;

	Text {
		text: "Audio";
		color: "white";
		font.pointSize: 18;
	}

	Rectangle {
		width: parent.width;
		height: 20;
		color: "white";

		Rectangle {
			width: (focusedUserData && focusedUserData.audioLoudness || 0) * parent.width - 4;
			height: parent.height - 4;
			color: "#505186";
			anchors.verticalCenter: parent.verticalCenter;
			x: 2
		}

		Slider {
			from: -60;
			to: 20;
			value: Users.getAvatarGain(focusedUser);
			snapMode: Slider.SnapAlways;
			stepSize: 1;
			width: parent.width;
			anchors.horizontalCenter: parent.horizontalCenter;
			anchors.verticalCenter: parent.verticalCenter;

			background: Rectangle {
				color: "transparent"
			}

			handle: Rectangle {
				x: parent.leftPadding + parent.visualPosition * (parent.availableWidth - width)
				y: -14
				width: 20
				height: 40
				color: "black"

				Rectangle {
					width: 16
					height: 36
					color: "gray"
					anchors.horizontalCenter: parent.horizontalCenter;
					anchors.verticalCenter: parent.verticalCenter;
				}
			}

			onValueChanged: {
				Users.setAvatarGain(focusedUser, value)
			}
		}
	}
}