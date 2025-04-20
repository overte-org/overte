import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import "../"

Flickable {
    property var verticalScrollBarWidth: 20;

    id: graphics_page;
    visible: current_page == "Graphics";
    width: parent.width;
    Layout.fillHeight: true;
    y: header.height + 10;
    contentWidth: parent.width;
    contentHeight: graphics_page_column.height;
    clip: true;
    flickDeceleration: 4000;

    Timer {
        id: verticalScrollBarInitialVisibilityTimer;
        interval: 200;
        running: false;
        repeat: false;

        onTriggered: {
            verticalScrollBarWidth = 15;
        }
    }

    onVisibleChanged: {
        // Set the initial values for the variables.
        verticalScrollBarWidth = 20;

        // We are leaving the page, don't animate. 
        if (!visible) return;

        // We have opened the page
        // Start the visibility effect timers.
        verticalScrollBarInitialVisibilityTimer.running = true;
    }

    ScrollBar.vertical: ScrollBar {
        id: scrollBar;
        policy: Qt.ScrollBarAlwaysOn;

        background: Rectangle {
            implicitWidth: verticalScrollBarWidth;
            color: "transparent";
            radius: 5;
            visible: scrollBar.visible;

            Behavior on implicitWidth {
                NumberAnimation {
                    duration: 300;
                    easing.type: Easing.InOutCubic;
                }
            }
        }
    }

    Column {
        id: graphics_page_column;
        width: parent.width - 20;
        anchors.horizontalCenterOffset: -5
        anchors.horizontalCenter: parent.horizontalCenter;
        spacing: 10;

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
        AdvancedOptions {
            id: renderingEffectsAdvancedOptions;
            isEnabled: Render.renderMethod === 0;

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

        // Procedural Materials
        SettingBoolean {
            settingText: "Procedural Materials";
            settingEnabled: Render.proceduralMaterialsEnabled;

            onSettingEnabledChanged: {
                Render.proceduralMaterialsEnabled = settingEnabled;
            }
        }

        // FPS
        SettingComboBox {
            settingText: "Refresh rate";
            options: ["Economical", "Interactive", "Real-Time", "Custom"];
            optionIndex: Performance.getRefreshRateProfile();

            onValueChanged: {
                Performance.setRefreshRateProfile(index);
                fpsAdvancedOptions.isEnabled = index == 3;
            }
        }

        // Custom FPS
        AdvancedOptions {
            id: fpsAdvancedOptions;
            isEnabled: Performance.getRefreshRateProfile() === 3;

            SettingNumber {
                settingText: "Focus Active";
                minValue: 5;
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
                settingText: "Unfocused";
                minValue: 1;
                maxValue: 9999;
                suffixText: "fps";
                settingValue: Performance.getCustomRefreshRate(2)

                onValueChanged: {
                    Performance.setCustomRefreshRate(2, value);
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

        // Resolution Scale
        SettingSlider {
            settingText: "Resolution scale";
            sliderStepSize: 0.1;
            minValue: 0.1;
            maxValue: 2;
            settingValue: Render.viewportResolutionScale.toFixed(1)

            onSliderValueChanged: {
                Render.viewportResolutionScale = value.toFixed(1)
            }
        }

        // Fullscreen Display
        SettingComboBox {
            settingText: "Fullscreen Display";

            Component.onCompleted: {
                var screens = Render.getScreens();
                var selected = Render.getFullScreenScreen();
                setOptions(screens);

                for (let i = 0; screens.length > i; i++) {
                    if (screens[i] == selected) {
                        optionIndex = i;
                        return;
                    }
                }
            }

            onValueChanged: {
                Render.setFullScreenScreen(optionText);
            }
        }

        // FOV
        SettingSlider {
            settingText: "Field of View";
            sliderStepSize: 1;
            minValue: 20;
            maxValue: 130;
            settingValue: Render.verticalFieldOfView.toFixed(1);
            roundDisplay: 0;

            onSliderValueChanged: {
                Render.verticalFieldOfView = value.toFixed(1);
            }
        }

        // Camera clipping
        SettingBoolean {
            settingText: "Allow camera clipping";
            settingEnabled: !Render.cameraClippingEnabled

            onSettingEnabledChanged: {
                Render.cameraClippingEnabled = settingEnabled ? 0 : 1;
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