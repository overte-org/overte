import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import TabletScriptingInterface 1.0

Item {
	id: root;
	property string settingText: "";
	property int optionIndex: 0;
    property var _optionText: "";
    readonly property string optionText: _optionText;
	property var options: [""];
    property bool disabled: false;

	signal valueChanged(int index);

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
				selectByMouse: true;
				readOnly: true;
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
					_optionText = options[currentIndex];
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

				contentItem: RowLayout {
					width: parent.width - 20;
					height: parent.height;

					Item {
						width: parent.width - 10;
						height: parent.height;

						Text {
							anchors.centerIn: parent;
							width: parent.width;
							text: control.displayText;
							horizontalAlignment: Text.AlignHCenter;
							verticalAlignment: Text.AlignVCenter;
							elide: Text.ElideRight;
							font.pixelSize: 22;
							color: "white";
						}
					}
				}

				background: Rectangle {
					id: comboBoxBackground;
					color: disabled ? "gray" : "#333";
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

					onVisibleChanged: {
						Tablet.playSound(TabletEnums.ButtonClicked);
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

        MouseArea {
            anchors.fill: parent;
            hoverEnabled: true;
            propagateComposedEvents: true;

            onPressed: mouse => {
                if (disabled) return;
                mouse.accepted = false;
            }

            onEntered: {
                if (disabled) return;
                backgroundElement.color = "#333";
				comboBoxBackground.color = "#444";
            }

            onExited: {
                if (disabled) return;
                backgroundElement.color = "transparent";
				comboBoxBackground.color = "#333";
            }
        }

        Behavior on color {
			ColorAnimation {
				duration: 50
				easing.type: Easing.InOutCubic
			}
		}
    }

    onDisabledChanged: {
        if (disabled) comboBoxBackground.color = "gray";
        else comboBoxBackground.color = "#333";
    }

	// Updates the contents of a combobox.
	// This is only required if the desired contents needs to be gathered from a javascript function and then set after the fact.
	// Ideally, this would not be used, but sometimes you gotta do what you gotta do.
	function setOptions(newOptions) {
		// Clear the model
		options = [];

		// Add the new options to the model
		for (var opt of newOptions){
			options.push(opt);
		}

		// Whack it with a hammer
		control.model = options;
	}

	function setOptionIndex(index) {
		control.currentIndex = index;
	}
}
