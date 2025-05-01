//
//  Created by Ada <ada@thingvellir.net> on 2025-04-14
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

    function bringToView(item) {
        var yTop = item.mapToItem(contentItem, 0, 0).y;
        var yBottom = yTop + item.height;

        var surfaceTop = contentY;
        var surfaceBottom = contentY + height;

        if(yTop < surfaceTop || yBottom > surfaceBottom) {
            contentY = yTop - height / 2 + item.height
        }
    }

    Component.onCompleted: {
        page = config.createObject(flick.contentItem);
    }
    Component {
        id: config
        Rectangle {
            id: openXrConfiguration
            anchors.fill: parent

            property int leftMargin: 75
            property int countDown: 0

            HifiConstants { id: hifi }

            MouseArea {
                id: mouseArea

                anchors.fill: parent
                propagateComposedEvents: true
                onPressed: {
                    mouse.accepted = false;
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
                property string text: "Calibrate hands"
                property int padding: 12

                width: glyphButton.width + calibrationText.width + padding
                height: hifi.dimensions.controlLineHeight
                anchors.top: bottomSeperator.bottom
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
                        : hifi.buttonXRdisabledTextColor[calibrationButton.colorScheme]
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
                        calibrationTimer.start();

                        ScriptDiscoveryService.loadScript("file:///~//system/controllers/openXRCalibration.js");
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
                    InputConfiguration.calibratePlugin("OpenXR")
                }
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

            NumberAnimation {
                id: numberAnimation
                target: openXrConfiguration
                property: "countDown"
                to: 0
            }
        }
    }
}
