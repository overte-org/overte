import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import controlsUit 1.0 as HifiControlsUit

Rectangle {
    color: Qt.rgba(0.1,0.1,0.1,1)
    signal sendToScript(var message);
    width: 200
    height: 700
	anchors.horizontalCenter: parent.horizontalCenter
    id: root

    property string current_page: "Graphics"

	// Navigation Header
	Item {
		height: 60
		width: parent.width

		Rectangle {
			anchors.fill: parent;
			color: "black"
		}

		Text {
			y: 10
			text: current_page
			color: "white"
			font.pointSize: 18
			anchors.horizontalCenter: parent.horizontalCenter
			anchors.verticalCenter: parent.verticalCenter
			horizontalAlignment: Text.AlignHCenter
			width: parent.width - 10
		}
	}


	// Graphics 
    ColumnLayout {
        width: parent.width - 10
        visible: current_page == "Graphics"
		anchors.horizontalCenter: parent.horizontalCenter
		y: parent.children[0].height + 10
		spacing: 10

		// Graphics Presets
		RowLayout {
			width: parent.width

			Text {
				text: "Graphics Preset"
				color: "white"
				height: parent.height
				width: parent.width - 150
				font.pointSize: 14
				Layout.fillWidth: true
			}

			ComboBox {
				currentIndex: 2
				Layout.fillWidth: true
				model: ListModel {
					id: cbItems
					ListElement { text: "Low Power" }
					ListElement { text: "Low" }
					ListElement { text: "Medium" }
					ListElement { text: "High" }
					ListElement { text: "Custom" }
				}
				onCurrentIndexChanged: console.debug(cbItems.get(currentIndex).text + ", " + cbItems.get(currentIndex).color)
			}
		}

		// Rendering Effects
		RowLayout {
			width: parent.width

			Text {
				text: "Rendering effects"
				color: "white"
				height: parent.height
				width: parent.width - 150
				font.pointSize: 14
				Layout.fillWidth: true
			}

			CheckBox {
				id: rendering_effects_state
            }
		}
		// Rendering Effects sub options
		Item {
			Layout.fillWidth: true
			visible: rendering_effects_state.checked == true

			Rectangle {
				color: "#333333"
				height: parent.children[1].height
				width: parent.width
			}
			
			GridLayout {
				columns: 2
				height: implicitHeight + 10
				width: parent.width - 10
				anchors.horizontalCenter: parent.horizontalCenter

				CheckBox {
					text: "Shadows"
					Layout.fillWidth: true
					palette.windowText: "gray" // TODO: Is this good?
				}
				
				CheckBox {
					text: "Local Lights"
					Layout.fillWidth: true
					palette.windowText: "gray" // TODO: Is this good?
				}
				
				CheckBox {
					text: "Fog"
					Layout.fillWidth: true
					palette.windowText: "gray" // TODO: Is this good?
				}
				
				CheckBox {
					text: "Bloom"
					Layout.fillWidth: true
					palette.windowText: "gray" // TODO: Is this good?
				}
			}
		}

		// FPS
		RowLayout {
			width: parent.width

			Text {
				text: "Max FPS"
				color: "white"
				height: parent.height
				width: parent.width - 150
				font.pointSize: 14
				Layout.fillWidth: true
			}

			TextField {
				Layout.maximumWidth: 50
				id: max_fps
				inputMethodHints: Qt.ImhFormattedNumbersOnly
				validator: RegExpValidator { regExp: /^-?[0-9]*(\[0-9]*[1-9])?$/ }
			}
		}
	}

    // Templates

    // Messages from script
    function fromScript(message) {
        switch (message.type){
            // TODO:
            case "active_polls":
                break;
        }
    }

    // Send message to script
    function toScript(packet){
        sendToScript(packet)
    }
}
