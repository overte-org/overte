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
            implicitWidth: 10
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
            implicitWidth: 10
            Rectangle {
                radius: 5
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
                id: hapticsRow
                anchors.left: parent.left
                anchors.leftMargin: leftMargin + 10
                anchors.topMargin: 10
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

            Row {
                id: handTrackingRow
                anchors.left: parent.left
                anchors.leftMargin: leftMargin + 10
                anchors.top: hapticsRow.bottom
                anchors.topMargin: 10
                spacing: 10

                HifiControls.CheckBox {
                    id: handTrackingBox
                    width: 15
                    height: 15
                    boxRadius: 7

                    onClicked: {
                        sendConfigurationSettings();
                    }
                }

                RalewayBold {
                    size: 12
                    text: "Enable Hand Tracking"
                    color: hifi.colors.lightGrayText
                }
            }

            RalewayBold {
                id: bodyTrackingTitle

                text: "Body Tracking"
                size: 12

                color: hifi.colors.white

                anchors.top: handTrackingRow.bottom
                anchors.left: parent.left
                anchors.leftMargin: leftMargin
                anchors.topMargin: 10
            }

            RalewayRegular {
                id: bodyTrackingInfo

                text: "Body tracking support is experimental on OpenXR. Calibration is incorrect, and tracking only works the Monado runtime currently. Other runtime vendors' body tracking extensions may be supported in the future."
                size: 12

                color: hifi.colors.white
                wrapMode: Text.Wrap

                anchors.top: bodyTrackingTitle.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: leftMargin
                anchors.topMargin: 10
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
                anchors.top: bodyTrackingInfo.bottom
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
                handTrackingBox.checked = settings["enable_hand_tracking"];

                isConfiguring = false;
            }

            function sendConfigurationSettings() {
                var settings = InputConfiguration.configurationSettings("OpenXR");

                settings["enable_haptics"] = hapticsBox.checked;
                settings["enable_hand_tracking"] = handTrackingBox.checked;

                InputConfiguration.setConfigurationSettings(settings, "OpenXR");
            }
        }
    }
}
