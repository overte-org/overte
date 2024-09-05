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

		// FIXME: Height is hardcoded
		// Rendering Effects sub options
		Item {
			Layout.fillWidth: true
			visible: rendering_effects_state.checked == true
			height: 100

			Rectangle {
				color: "#333333"
				height: parent.children[1].height
				width: parent.width
			}
			
			GridLayout {
				columns: 2
				height: 100
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
				validator: RegExpValidator { regExp: /[0-9]*/ }
			}
		}

		// Resolution Scale
		RowLayout {
			width: parent.width

			Text {
				text: "Resolution Scale"
				color: "white"
				height: parent.height
				width: parent.width - 150
				font.pointSize: 14
				Layout.fillWidth: true
			}

			Text {
				text: parent.children[2].value.toFixed(1)
				color: "white"
			}

			Slider {
				id: resolution_slider
				from: 0.1
				to: 2
				value: 1
				stepSize: 0.1
			}
		}

		// FOV
		RowLayout {
			width: parent.width

			Text {
				text: "FOV"
				color: "white"
				height: parent.height
				width: parent.width - 150
				font.pointSize: 14
				Layout.fillWidth: true
			}

			Text {
				text: parent.children[2].value.toFixed(1)
				color: "white"
			}

			Slider {
				id: fov_slider
				from: 20
				to: 130
				value: 90 // TODO: Need to set to Overte default
				stepSize: 1
			}
		}

		// Graphics Presets
		RowLayout {
			width: parent.width

			Text {
				text: "Anti-Aliasing"
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
					ListElement { text: "None" }
					ListElement { text: "TAA" }
					ListElement { text: "FXAA" }
				}
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
