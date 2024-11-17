import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import "./qml_widgets"

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
	SettingCenterContainer {
		id: home_page
        visible: current_page == "Settings"

		Repeater {
			model: pages.length
			delegate: SettingSubviewListElement {
				property string page_name: pages[index];
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

				Component.onCompleted: {				
					checked: Render.renderMethod == 1
				}

				onCheckedChanged: {
					if (checked){
						Render.renderMethod = 0;
					} else {
						Render.renderMethod = 1;
					}
				}
            }
		}

		// Rendering Effects sub options
		Item {
			Layout.fillWidth: true;
			visible: rendering_effects_state.checked == true;
			height: children[0].height;

			Rectangle {
				color: "#222222";
				width: parent.width;
				height: children[0].height;
				radius: 10;	

				GridLayout {
					columns: 2;
					width: parent.width - 10;
					anchors.horizontalCenter: parent.horizontalCenter;
					id: renderSettingsContainer;

					SettingBoolean {
						settingText: "Shadows";
						settingEnabled: Render.shadowsEnabled

						onSettingEnabledChanged: {
							Render.shadowsEnabled = settingEnabled;
						}
					}

					SettingBoolean {
						settingText: "Local Lights";
						settingEnabled: Render.localLightsEnabled

						onSettingEnabledChanged: {
							Render.localLightsEnabled = settingEnabled;
						}
					}
					
					SettingBoolean {
						settingText: "Fog";
						settingEnabled: Render.fogEnabled

						onSettingEnabledChanged: {
							Render.fogEnabled = settingEnabled;
						}
					}

					SettingBoolean {
						settingText: "Haze";
						settingEnabled: Render.hazeEnabled

						onSettingEnabledChanged: {
							Render.hazeEnabled = settingEnabled;
						}
					}

					SettingBoolean {
						settingText: "Bloom";
						settingEnabled: Render.bloomEnabled 

						onSettingEnabledChanged: {
							Render.bloomEnabled = settingEnabled;
						}
					}
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

	// Audio
	SettingCenterContainer {
		id: audio_page
        visible: current_page == "Audio"
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
