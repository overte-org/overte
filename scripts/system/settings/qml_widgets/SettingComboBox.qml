import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Item {
	id: root;
	property string settingText: "";
	property int optionIndex: 0;
	property var options: [];

	signal valueChanged(int index);

	height: 50;
	width: parent.width;

	RowLayout {
		width: parent.width - 10;
		height: parent.height;
		anchors.horizontalCenter: parent.horizontalCenter;
		Layout.alignment: Qt.AlignTop;

		Text {
			id: settingTextElem
			height: parent.height;
			text: settingText;
			color: "white";
			font.pointSize: 14;
		}

		ComboBox {
			id: control
			Layout.alignment: Qt.AlignRight;
			implicitWidth: 225;
			implicitHeight: parent.height - 15;
			model: options;
			currentIndex: optionIndex;

			onCurrentIndexChanged: {
				valueChanged(currentIndex);
			}

			delegate: ItemDelegate {
				width: control.width
				contentItem: Text {
					text: options[index]
					color: "white"
					font: control.font
					elide: Text.ElideRight
					verticalAlignment: Text.AlignVCenter
					horizontalAlignment: Text.AlignHCenter
				}
				background: Rectangle {
					color: highlighted ? "gray" : "transparent";
					radius: 10
				}
				highlighted: control.highlightedIndex === index
			}

			contentItem: Text {
				text: control.displayText
				color: "white";
				verticalAlignment: Text.AlignVCenter
				horizontalAlignment: Text.AlignHCenter
				font.pointSize: 14;
				elide: Text.ElideRight
				width: parent.width;
			}

			background: Rectangle {
				border.color: "white";
				border.width: 1;
				color: "transparent";
				radius: 10;
				width: parent.width;
			}

			popup: Popup {
				width: control.width
				padding: 1

				contentItem: ListView {
					clip: true
					implicitHeight: contentHeight
					model: control.popup.visible ? control.delegateModel : null
					currentIndex: control.highlightedIndex

					ScrollIndicator.vertical: ScrollIndicator { }
				}

				background: Rectangle {
					color: Qt.rgba(0,0,0,0.9)
					radius: 10
				}
			}

			indicator: Canvas {
				id: canvas
				x: control.width - width - control.rightPadding
				y: control.topPadding + (control.availableHeight - height) / 2
				width: 12
				height: 8
				contextType: "2d"

				Connections {
					target: control
					function onPressedChanged() { canvas.requestPaint(); }
				}

				onPaint: {
					context.reset();
					context.moveTo(0, 0);
					context.lineTo(width, 0);
					context.lineTo(width / 2, height);
					context.closePath();
					context.fillStyle = "white";
					context.fill();
				}
			}
		}
	}
}