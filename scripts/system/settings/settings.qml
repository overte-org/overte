import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import "./qml_widgets"


// TODO: Some default values wait until component is completed. Is this necessary? 

Rectangle {
    color: Qt.rgba(0.1,0.1,0.1,1)
    signal sendToScript(var message);
    width: 200
    height: 700
	anchors.horizontalCenter: parent.horizontalCenter
    id: root

	property var pages: ["Graphics", "Audio", "Controls", "Privacy and Security"]
    property string current_page: "Settings"

	// Navigation Header
	HeaderElement {
		id: header
	}

	// Home page
	ScrollView {
		width: parent.width
		height:parent.height
		y: header.height
		id: home_page

		ColumnLayout {
			width: parent.width
			visible: current_page == "Settings"
			anchors.horizontalCenter: parent.horizontalCenter
			spacing: 0
			
			Repeater {
				model: pages.length
				delegate: SettingSubviewListElement {
					property string page_name: pages[index];
				}
			}
		}
	}

	// Graphics 
    ColumnLayout {
		id: graphics_page
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
				currentIndex: Performance.getPerformancePreset() - 1 // One off error FTW!
				Layout.fillWidth: true
				model: ListModel {
					id: cbItems
					ListElement { text: "Low Power" }
					ListElement { text: "Low" }
					ListElement { text: "Medium" }
					ListElement { text: "High" }
					ListElement { text: "Custom" }
				}

				onCurrentIndexChanged: {
					Performance.setPerformancePreset(currentIndex + 1)
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
				checked: Render.renderMethod === 0
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
			

			// TODO: Some things were hard corded to be enabled / disabled depending on renderer.
			// This is fixed currently in the ProtocolChanges branch, but not yet merged into master
			// Remove "checked: XXXXXX" and replace with commented out "checked: XXXXX" this when merged
			// Also please remove "enabled:false". That was just to make sure users don't diddle with settings they can't change :)
			GridLayout {
				columns: 2
				height: 100
				width: parent.width - 10
				anchors.horizontalCenter: parent.horizontalCenter

				CheckBox {
					text: "Shadows"
					Layout.fillWidth: true
					palette.windowText: "gray"
					checked: Render.shadowsEnabled
					onCheckedChanged: {
					   Render.shadowsEnabled = checked;
					}
				}
				
				CheckBox {
					enabled: false
					text: "Local Lights"
					Layout.fillWidth: true
					palette.windowText: "gray"
					//checked: Render.localLightsEnabled
					checked: rendering_effects_state.checked
					//onCheckedChanged: {
					//    Render.localLightsEnabled = checked;
					//}
				}
				
				CheckBox {
					enabled: false
					text: "Fog"
					Layout.fillWidth: true
					palette.windowText: "gray" 
					//checked: Render.fogEnabled
					checked: rendering_effects_state.checked 
					//onCheckedChanged: {
					//    Render.fogEnabled = checked;
					//}
				}
				
				CheckBox {
					enabled: false
					text: "Bloom"
					Layout.fillWidth: true
					palette.windowText: "gray"
					//checked: Render.bloomEnabled
					checked: rendering_effects_state.checked 
					//onCheckedChanged: {
					//    Render.bloomEnabled = checked;
					//}
				}
			}
		}


		// FPS
		RowLayout {
			width: parent.width

			Text {
				text: "Refresh Rate"
				color: "white"
				height: parent.height
				width: parent.width - 150
				font.pointSize: 14
				Layout.fillWidth: true
			}

			ComboBox {
				id: refresh_rate_cb
				currentIndex: 3
				Layout.fillWidth: true
				model: ListModel {
					ListElement { text: "Economical" }
					ListElement { text: "Interactive" }
					ListElement { text: "Real-Time" }
					ListElement { text: "Custom" }
				}

				Component.onCompleted: {
					refresh_rate_cb.currentIndex = Performance.getRefreshRateProfile()
				}

				onCurrentIndexChanged: {
					Performance.setRefreshRateProfile(currentIndex)
				}
			}
		}


		// FIXME: Height is hardcoded
		// FPS sub options
		Item {
			Layout.fillWidth: true
			visible: refresh_rate_cb.currentIndex == 3
			height: 75 * 3


			Rectangle {
				color: "#333333"
				height: parent.children[1].height
				width: parent.width
			}
			
			GridLayout {
				columns: 2
				width: parent.width - 10
				anchors.horizontalCenter: parent.horizontalCenter
				height: 75 * 3

				Column {
					Text {
						text: "Focus Active"
						Layout.fillWidth: true
						color: "white"
					}
					TextField {
						width: 100
						Layout.maximumWidth: 50
						inputMethodHints: Qt.ImhFormattedNumbersOnly
						validator: RegExpValidator { regExp: /[0-9]*/ }

						Component.onCompleted: {
							text = Performance.getCustomRefreshRate(0)
						}
						
						onTextChanged: {
							Performance.setCustomRefreshRate(0, text)
						}
					}
				}

				Column {
					Text {
						text: "Focus Inactive"
						Layout.fillWidth: true
						color: "white"
					}
					TextField {
						width: 100
						Layout.maximumWidth: 50
						inputMethodHints: Qt.ImhFormattedNumbersOnly
						validator: RegExpValidator { regExp: /[0-9]*/ }

						Component.onCompleted: {
							text = Performance.getCustomRefreshRate(1)
						}
						
						onTextChanged: {
							Performance.setCustomRefreshRate(1, text)
						}
					}
				}

				Column {
					Text {
						text: "Unfocus"
						Layout.fillWidth: true
						color: "white"
					}
					TextField {
						width: 100
						Layout.maximumWidth: 50
						inputMethodHints: Qt.ImhFormattedNumbersOnly
						validator: RegExpValidator { regExp: /[0-9]*/ }

						Component.onCompleted: {
							text = Performance.getCustomRefreshRate(2)
						}
						
						onTextChanged: {
							Performance.setCustomRefreshRate(2, text)
						}
					}
				}

				Column {
					Text {
						text: "Minimized"
						Layout.fillWidth: true
						color: "white"
					}
					TextField {
						width: 100
						Layout.maximumWidth: 50
						inputMethodHints: Qt.ImhFormattedNumbersOnly
						validator: RegExpValidator { regExp: /[0-9]*/ }

						Component.onCompleted: {
							text = Performance.getCustomRefreshRate(3)
						}

						
						onTextChanged: {
							Performance.setCustomRefreshRate(3, text)
						}
					}
				}

				Column {
					Text {
						text: "Startup"
						Layout.fillWidth: true
						color: "white"
					}
					TextField {
						width: 100
						Layout.maximumWidth: 50
						inputMethodHints: Qt.ImhFormattedNumbersOnly
						validator: RegExpValidator { regExp: /[0-9]*/ }

						Component.onCompleted: {
							text = Performance.getCustomRefreshRate(4)
						}
						
						onTextChanged: {
							Performance.setCustomRefreshRate(4, text)
						}
					}
				}

				Column {
					Text {
						text: "Shutdown"
						Layout.fillWidth: true
						color: "white"
					}
					TextField {
						width: 100
						Layout.maximumWidth: 50
						inputMethodHints: Qt.ImhFormattedNumbersOnly
						validator: RegExpValidator { regExp: /[0-9]*/ }

						Component.onCompleted: {
							text = Performance.getCustomRefreshRate(5)
						}

						onTextChanged: {
							Performance.setCustomRefreshRate(5, text)
						}
					}
				}

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
				value: Render.viewportResolutionScale.toFixed(1)
				stepSize: 0.1

				onValueChanged: {
					Render.viewportResolutionScale = value
				}
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
				text: parent.children[2].value.toFixed(0)
				color: "white"
			}

			// FIXME: QML Slider binding loop
			Slider {
				id: fov_slider
				from: 20
				to: 130
				value: Render.verticalFieldOfView.toFixed(1) // TODO: Need to set to Overte default
				stepSize: 1

				onValueChanged: {
					Render.verticalFieldOfView = value
				}
			}
		}

		// Anti Aliasing
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
				currentIndex: Render.antialiasingMode
				Layout.fillWidth: true
				model: ListModel {
					ListElement { text: "None" }
					ListElement { text: "TAA" }
					ListElement { text: "FXAA" }
				}

				onCurrentIndexChanged: {
					Render.antialiasingMode = currentIndex;
				}
			}
		}
	}

    // Templates

    // Messages from script
    function fromScript(message) {
        switch (message.type){
            case "":
                break;
        }
    }

    // Send message to script
    function toScript(packet){
        sendToScript(packet)
    }
}
