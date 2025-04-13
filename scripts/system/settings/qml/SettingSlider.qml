import QtQuick 2.7
import QtQuick.Controls 2.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3

Item {
	id: root;
	property string settingText: "";
	property var settingValue: 0;

	property real minValue: 0;
	property real maxValue: 9;
	property real sliderStepSize: 0.1;
	property int roundDisplay: 1;

	signal sliderValueChanged(real value);

	height: 50;
	width: parent.width;

	RowLayout {
		width: parent.width - 10;
		height: parent.height;
		anchors.horizontalCenter: parent.horizontalCenter;
		Layout.alignment: Qt.AlignTop;

		TextEdit {
			id: settingTextElem
			height: parent.height;
			text: settingText;
			color: "white";
			font.pointSize: 14;
			Layout.fillWidth: true;
			selectByMouse: true;
			readOnly: true;
		}

		RowLayout {
			Layout.alignment: Qt.AlignRight;
			width: 225;
			implicitWidth: 225;
			height: parent.height;
			Layout.fillWidth: false;

			Text {
				id: sliderValueDisplay
				text: slider.value.toFixed(roundDisplay);
				color: "white";
				height: parent.height;
				verticalAlignment: Qt.AlignVCenter
				width: 25;
				font.pointSize: 14;
			}

			Slider {
				Layout.fillWidth: true;
				height: parent.height;
				id: slider;
				from: minValue;
				to: maxValue;
				stepSize: sliderStepSize;
				snapMode: Slider.SnapOnRelease;
				value: settingValue;

				handle: Rectangle {
					x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
					y: slider.topPadding + slider.availableHeight / 2 - height / 2
					implicitWidth: 20
					implicitHeight: 40
                    color: "black"

                    Rectangle {
                        width: 16
                        height: 36
                        color: "gray"
                        anchors.horizontalCenter: parent.horizontalCenter;
                        anchors.verticalCenter: parent.verticalCenter;
                    }
				}

				background: Rectangle {
					x: slider.leftPadding
					y: slider.topPadding + slider.availableHeight / 2 - height / 2
					implicitWidth: 200
					implicitHeight: 20
					width: slider.availableWidth
					height: implicitHeight
					radius: 2
					color: "white"
					clip: true;

					Rectangle {
						width: slider.visualPosition * parent.width
						height: parent.height
						color: "#5153bd"
						radius: 2
					}
				}

				onValueChanged: {
					sliderValueChanged(value)
				}
			}


		}
	}
}