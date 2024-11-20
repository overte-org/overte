import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import "./qml_widgets"

Rectangle {
    signal sendToScript(var message);
	color: Qt.rgba(0.1,0.1,0.1,1);
	width: parent.width;
	height: parent.height;
	anchors.centerIn: parent;
	anchors.horizontalCenter: parent.horizontalCenter
	property var pages: ["Graphics", "Audio", "Controls", "Privacy and Security"]
	property string current_page: "Settings"

	ColumnLayout {
		width: parent.width
		height: parent.height
		anchors.horizontalCenter: parent.horizontalCenter
		id: root

		// Navigation Header
		HeaderElement {
			id: header
		}

		// Home page
		SettingCenterContainer {
			id: home_page
			visible: current_page == "Settings"
			Layout.fillHeight: true

			Repeater {
				model: pages.length
				delegate: SettingSubviewListElement {
					property string page_name: pages[index];
				}
			}
		}

		// Graphics 
		Flickable {
			id: graphics_page
			visible: current_page == "Graphics"
			width: parent.width
			Layout.fillHeight: true
			y: header.height + 10
			contentWidth: parent.width
			contentHeight: graphics_page_column.height
			clip: true

			ColumnLayout {
				id: graphics_page_column
				width: parent.width - 10
				anchors.horizontalCenter: parent.horizontalCenter
				spacing: 10

				// Graphics Presets
				SettingComboBox {
					settingText: "Graphics preset";
					optionIndex: Performance.getPerformancePreset() - 1;
					options: ["Low Power", "Low", "Medium", "High", "Custom"];

					onValueChanged: {
						Performance.setPerformancePreset(index + 1)
					}
				}

				// Rendering Effects
				SettingBoolean {
					settingText: "Rendering effects";
					settingEnabled: Render.renderMethod == 0

					onSettingEnabledChanged: {
						Render.renderMethod = settingEnabled ? 0 : 1;
					}
				}

				// Rendering Effects sub options
				Item {
					Layout.fillWidth: true;
					visible: Render.renderMethod == 0;
					height: children[0].height;
					width: parent.width;

					Rectangle {
						color: "#222222";
						width: parent.width;
						height: children[0].height;
						radius: 10;	

						ColumnLayout {
							width: parent.width - 10;
							id: renderSettingsContainer;

							SettingBoolean {
								settingText: "Shadows";
								settingEnabled: Render.shadowsEnabled;

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
				SettingComboBox {
					settingText: "Refresh rate";
					optionIndex: Performance.getPerformancePreset() - 1;
					options: ["Economical", "Interactive", "Real-Time", "Custom"];

					Component.onCompleted: {
						optionIndex = Performance.getRefreshRateProfile()
					}

					onValueChanged: {
						Performance.setRefreshRateProfile(index)
					}
				}


				Item {
					Layout.fillWidth: true;
					visible: refresh_rate_cb.currentIndex == 3;
					height: children[0].height;
					width: parent.width;

					Rectangle {
						color: "#222222";
						width: parent.width;
						height: children[0].height;
						radius: 10;	

						ColumnLayout {
							width: parent.width - 10;

							SettingNumber {
								settingText: "Focus Active";
								minValue: 1;
								maxValue: 9999;
								suffixText: "fps";
								settingValue: Performance.getCustomRefreshRate(0)

								onValueChanged: {
									Performance.setCustomRefreshRate(0, value);
								}
							}

							SettingNumber {
								settingText: "Focus Inactive";
								minValue: 1;
								maxValue: 9999;
								suffixText: "fps";
								settingValue: Performance.getCustomRefreshRate(1)

								onValueChanged: {
									Performance.setCustomRefreshRate(1, value);
								}
							}

							SettingNumber {
								settingText: "Minimized";
								minValue: 1;
								maxValue: 9999;
								suffixText: "fps";
								settingValue: Performance.getCustomRefreshRate(3)

								onValueChanged: {
									Performance.setCustomRefreshRate(3, value);
								}
							}

							SettingNumber {
								settingText: "Startup";
								minValue: 1;
								maxValue: 9999;
								suffixText: "fps";
								settingValue: Performance.getCustomRefreshRate(4)

								onValueChanged: {
									Performance.setCustomRefreshRate(4, value);
								}
							}

							SettingNumber {
								settingText: "Shutdown";
								minValue: 1;
								maxValue: 9999;
								suffixText: "fps";
								settingValue: Performance.getCustomRefreshRate(5)

								onValueChanged: {
									Performance.setCustomRefreshRate(5, value);
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
				SettingComboBox {
					settingText: "Anti-aliasing";
					optionIndex: Render.antialiasingMode;
					options: ["None", "TAA", "FXAA"];

					onValueChanged: {
						Render.antialiasingMode = index;
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
}
