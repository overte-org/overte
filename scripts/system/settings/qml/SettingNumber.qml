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
	property string suffixText: "";
	property real stepSize: 1;
	property color buttonColor: "#333";

	signal valueChanged(int value);

	height: 50;
	width: parent.width;

	Rectangle {
        id: backgroundElement;
        width: parent.width;
        height: parent.height;
        color: "transparent";
        radius: 15;

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
				font.pixelSize: 22;
				width: parent.width - 200;			
				selectByMouse: true;
				readOnly: true;
			}

			Item {
				Layout.alignment: Qt.AlignRight;
				width: 225;
				height: parent.height;

				SpinBox {
					id: spinbox;
					value: settingValue;
					from: minValue;
					to: maxValue;
					stepSize: stepSize;
					Layout.alignment: Qt.AlignRight;
					implicitWidth: 200;
					implicitHeight: parent.height;

					contentItem: TextField {
						id: spinboxText;
						color: "white";
						text: parent.value;
						verticalAlignment: Qt.AlignVCenter
						horizontalAlignment: TextInput.AlignHCenter
						width: parent.width;
						clip: true;
						font.pixelSize: 22
						validator: RegExpValidator { regExp: /[0-9]*/ }

						background: Rectangle {
							color: "#111";
							border.width: 0;
							radius: 10;
						}

						onTextChanged: {
							valueChanged(spinboxText.text);
							settingValue = spinboxText.text;
						}

						Keys.onPressed: {
							if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
								valueChanged(spinboxText.text);
								settingValue = spinboxText.text;
							}
						}
					}

					up.indicator: Button {
						height: parent.height - 15;
						width: parent.height;
						x: parent.width - width;
						anchors.verticalCenter: parent.verticalCenter;

						background: Rectangle {
							color: buttonColor;
							border.width: 0;
							radius: 10;
						}

						Text {
							text: "+";
							color: "white";
							font.pixelSize: 28;
							verticalAlignment: Qt.AlignVCenter	
							anchors.horizontalCenter: parent.horizontalCenter;
							anchors.verticalCenter: parent.verticalCenter;
						}

						onClicked: {
							spinbox.value += stepSize;
							valueChanged(spinbox.value);
						}
					}

					down.indicator: Button {
						height: parent.height - 15;
						width: parent.height;
						anchors.verticalCenter: parent.verticalCenter;

						background: Rectangle {
							color: buttonColor;
							border.width: 0;
							radius: 10;
						}

						Text {
							text: "-";
							color: "white";
							font.pixelSize: 28;
							anchors.horizontalCenter: parent.horizontalCenter;
							anchors.verticalCenter: parent.verticalCenter;
						}

						onClicked: {
							spinbox.value -= stepSize;
							valueChanged(spinbox.value);
						}
					}

					background: Rectangle {
						color: "transparent";
					}

				}

				Text {
					visible: suffixText != "";
					text: suffixText;
					color: "white";
					height: parent.height;
					verticalAlignment: Qt.AlignVCenter
					x: spinbox.width + 10;
				}
			}
		}

        MouseArea {
            anchors.fill: parent;
            hoverEnabled: true;
            propagateComposedEvents: true;

            onPressed: {
                mouse.accepted = false
            }

            onEntered: {
                backgroundElement.color = "#333";
				buttonColor = "#444";
            }

            onExited: {
                backgroundElement.color = "transparent";
				buttonColor = "#333";
            }
        }

        Behavior on color {
			ColorAnimation {
				duration: 50
				easing.type: Easing.InOutCubic
			}
		}
    }
}