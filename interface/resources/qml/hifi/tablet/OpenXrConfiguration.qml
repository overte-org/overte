//
//  Created by Ada <ada@thingvellir.net> on 2025-06-15
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.5
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.2

import stylesUit 1.0
import "../../controls"
import controlsUit 1.0 as HifiControls
import "."


Flickable {
    id: flick
    width: parent.width
    height: parent.height
    anchors.fill: parent
    contentHeight: 550
    flickableDirection: Flickable.VerticalFlick
    property var page: null;

    ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AlwaysOn
        parent: flick.parent
        anchors.top: flick.top
        anchors.right: flick.right
        anchors.bottom: flick.bottom
        z: 100  // Display over top of separators.

        background: Item {
            implicitWidth: verticalScrollWidth
            Rectangle {
                color: hifi.colors.baseGrayShadow
                radius: 4
                anchors {
                    fill: parent
                    bottomMargin: 1
                }
            }
        }
        contentItem: Item {
            implicitWidth: verticalScrollShaft
            Rectangle {
                radius: verticalScrollShaft/2
                color: hifi.colors.white30
                anchors {
                    fill: parent
                    topMargin: 1
                    bottomMargin: 1
                }
            }
        }
    }

    Component.onCompleted: {
        page = config.createObject(flick.contentItem);
        page.displayConfiguration();
    }
    Component {
        id: config
        Rectangle {
            id: openXrConfiguration
            anchors.fill: parent

            property int leftMargin: 75
            property int countDown: 0
            property var displayInformation: null

            property var lastConfiguration:  null
            property bool isConfiguring: false

            HifiConstants { id: hifi }

            Component { id: screen; CalibratingScreen {} }

            MouseArea {
                id: mouseArea

                anchors.fill: parent
                propagateComposedEvents: true
                onPressed: {
                    mouse.accepted = false;
                }
            }

            Row {
                id: otherConfig
                anchors.left: parent.left
                anchors.leftMargin: leftMargin + 10
                spacing: 10

                HifiControls.CheckBox {
                    id: hapticsBox
                    width: 15
                    height: 15
                    boxRadius: 7

                    onClicked: {
                        sendConfigurationSettings();
                    }
                }

                RalewayBold {
                    size: 12
                    text: "Enable Controller Haptics"
                    color: hifi.colors.lightGrayText
                }
            }

            RalewayBold {
                id: bodyTracking

                text: "Body Tracking"
                size: 12

                color: hifi.colors.white

                anchors.top: otherConfig.bottom
                anchors.left: parent.left
                anchors.leftMargin: leftMargin
                anchors.topMargin: 10
            }

            RalewayRegular {
                id: info

                text: "See Recommended Placement"
                color: hifi.colors.blueHighlight
                size: 12
                anchors {
                    left: bodyTracking.right
                    leftMargin: 10
                    verticalCenter: bodyTracking.verticalCenter
                    topMargin: 10
                    top: bodyTracking.bottom
                }

                Rectangle {
                    id: selected
                    color: hifi.colors.blueHighlight

                    width: info.width
                    height: 1

                    anchors {
                        top: info.bottom
                        topMargin: 1
                        left: info.left
                        right: info.right
                    }

                    visible: false
                }

                MouseArea {
                    anchors.fill: parent;
                    hoverEnabled: true

                    onEntered: {
                        selected.visible = true;
                    }

                    onExited: {
                        selected.visible = false;
                    }
                    onClicked: {
                        stack.messageVisible = true;
                    }
                }
            }

            color: hifi.colors.baseGray

            Rectangle {
                id: calibrationButton
                property int color: hifi.buttons.blue
                property int colorScheme: hifi.colorSchemes.light
                property string glyph: hifi.glyphs.avatar1
                property bool enabled: true
                property bool pressed: false
                property bool hovered: false
                property int size: 32
                property string text: "Calibrate"
                property int padding: 12

                width: glyphButton.width + calibrationText.width + padding
                height: hifi.dimensions.controlLineHeight
                anchors.top: bodyTracking.bottom
                anchors.topMargin: 15
                anchors.left: parent.left
                anchors.leftMargin: leftMargin

                radius: hifi.buttons.radius

                gradient: Gradient {
                    GradientStop {
                        position: 0.2
                        color: {
                            if (!calibrationButton.enabled) {
                                hifi.buttons.disabledColorStart[calibrationButton.colorScheme]
                            } else if (calibrationButton.pressed) {
                                hifi.buttons.pressedColor[calibrationButton.color]
                            } else if (calibrationButton.hovered) {
                                hifi.buttons.hoveredColor[calibrationButton.color]
                            } else {
                                hifi.buttons.colorStart[calibrationButton.color]
                            }
                        }
                    }

                    GradientStop {
                        position: 1.0
                        color: {
                            if (!calibrationButton.enabled) {
                                hifi.buttons.disabledColorFinish[calibrationButton.colorScheme]
                            } else if (calibrationButton.pressed) {
                                hifi.buttons.pressedColor[calibrationButton.color]
                            } else if (calibrationButton.hovered) {
                                hifi.buttons.hoveredColor[calibrationButton.color]
                            } else {
                                hifi.buttons.colorFinish[calibrationButton.color]
                            }
                        }
                    }
                }

                HiFiGlyphs {
                    id: glyphButton
                    color: enabled ? hifi.buttons.textColor[calibrationButton.color]
                        : hifi.buttons.disabledTextColor[calibrationButton.colorScheme]
                    text: calibrationButton.glyph
                    size: calibrationButton.size

                    anchors {
                        top: parent.top
                        bottom: parent.bottom
                        bottomMargin: 1
                    }
                }

                RalewayBold {
                    id: calibrationText
                    font.capitalization: Font.AllUppercase
                    color: enabled ? hifi.buttons.textColor[calibrationButton.color]
                        : hifi.buttons.disabledTextColor[calibrationButton.colorScheme]
                    size: hifi.fontSizes.buttonLabel
                    text: calibrationButton.text

                    anchors {
                        left: glyphButton.right
                        top: parent.top
                        topMargin: 7
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        calibrationTimer.interval = timeToCalibrate.realValue * 1000
                        openXrConfiguration.countDown = timeToCalibrate.realValue;
                        var calibratingScreen = screen.createObject();
                        stack.push(calibratingScreen);
                        calibratingScreen.canceled.connect(cancelCalibration);
                        calibratingScreen.restart.connect(restartCalibration);
                        calibratingScreen.start(calibrationTimer.interval, timeToCalibrate.realValue);
                        calibrationTimer.start();
                    }

                    onPressed: {
                        calibrationButton.pressed = true;
                    }

                    onReleased: {
                        calibrationButton.pressed = false;
                    }

                    onEntered: {
                        calibrationButton.hovered = true;
                    }

                    onExited: {
                        calibrationButton.hovered = false;
                    }
                }
            }

            Timer {
                id: calibrationTimer
                repeat: false
                interval: 20
                onTriggered: {
                    InputConfiguration.calibratePlugin("OpenXR");
                    stack.currentItem.success();
                }
            }

            Timer {
                id: displayTimer
                repeat: false
                interval: 3000
                onTriggered: {
                }
            }

            Component.onDestruction: {
                var settings = InputConfiguration.configurationSettings("OpenXR");
                var data = {};
                UserActivityLogger.logAction("mocap_ui_close_dialog", data);
            }

            HifiControls.SpinBox {
                id: timeToCalibrate
                width: 70
                anchors.top: calibrationButton.bottom
                anchors.topMargin: 20
                anchors.left: parent.left
                anchors.leftMargin: leftMargin

                minimumValue: 0
                maximumValue: 5
                realValue: 5
                realStepSize: 1.0
                colorScheme: hifi.colorSchemes.dark

                onRealValueChanged: {
                    calibrationTimer.interval = realValue * 1000;
                    openXrConfiguration.countDown = realValue;
                    numberAnimation.duration = calibrationTimer.interval;
                }
            }

            RalewayBold {
                id: delayTextInfo
                size: 10
                text: "Delay Before Calibration Starts"
                color: hifi.colors.white

                anchors {
                    left: timeToCalibrate.right
                    leftMargin: 20
                    verticalCenter: timeToCalibrate.verticalCenter
                }
            }

            RalewayRegular {
                size: 12
                text: "sec"
                color: hifi.colors.lightGray

                anchors {
                    left: delayTextInfo.right
                    leftMargin: 10
                    verticalCenter: delayTextInfo.verticalCenter
                }
            }

            Separator {
                id: advanceSeperator
                width: parent.width
                anchors.top: timeToCalibrate.bottom
                anchors.topMargin: 10
            }


            NumberAnimation {
                id: numberAnimation
                target: openXrConfiguration
                property: "countDown"
                to: 0
            }

            function cancelCalibration() {
                calibrationTimer.stop();
            }

            function restartCalibration() {
                calibrationTimer.restart();
            }

            function displayConfiguration() {
                isConfiguring = true;

                var settings = InputConfiguration.configurationSettings("OpenXR");
                hapticsBox.checked = settings["enable_haptics"];

                isConfiguring = false;
            }

            function sendConfigurationSettings() {
                var settings = InputConfiguration.configurationSettings("OpenXR");

                settings["enable_haptics"] = hapticsBox.checked;

                InputConfiguration.setConfigurationSettings(settings, "OpenXR");
            }
        }
    }
}
