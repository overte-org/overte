//
//  MicBar.qml
//  qml/hifi/audio
//
//  Created by Zach Pomerantz on 6/14/2017
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.5
import Qt5Compat.GraphicalEffects
import stylesUit 1.0

import TabletScriptingInterface 1.0

Rectangle {
    id: micBar
    HifiConstants { id: hifi; }

    property var muted: AudioScriptingInterface.muted;
    readonly property var level: AudioScriptingInterface.inputLevel;
    readonly property var clipping: AudioScriptingInterface.clipping;
    property var pushToTalk: AudioScriptingInterface.pushToTalk;
    property var pushingToTalk: AudioScriptingInterface.pushingToTalk;

    readonly property var userSpeakingLevel: 0.4;
    property bool gated: false;
    Component.onCompleted: {
        AudioScriptingInterface.noiseGateOpened.connect(function() { gated = false; });
        AudioScriptingInterface.noiseGateClosed.connect(function() { gated = true; });
        HMD.displayModeChanged.connect(function() {
            muted = AudioScriptingInterface.muted;
            pushToTalk = AudioScriptingInterface.pushToTalk;
        });
        AudioScriptingInterface.mutedChanged.connect(function() {
            muted = AudioScriptingInterface.muted;
        });
        AudioScriptingInterface.pushToTalkChanged.connect(function() {
            pushToTalk = AudioScriptingInterface.pushToTalk;
        });
        AudioScriptingInterface.pushingToTalkChanged.connect(function() {
            pushingToTalk = AudioScriptingInterface.pushingToTalk;
        });
    }

    property bool standalone: false;
    property var dragTarget: null;

    width: 240;
    height: 50;

    radius: 5;

    color: "#00000000";
    border {
        width: mouseArea.containsMouse || mouseArea.containsPress ? 2 : 0;
        color: colors.border;
    }

    // borders are painted over fill, so reduce the fill to fit inside the border
    Rectangle {
        color: standalone ? colors.fill : "#00000000";
        width: 236;
        height: 46;

        radius: 5;

        anchors {
            verticalCenter: parent.verticalCenter;
            horizontalCenter: parent.horizontalCenter;
        }
    }

    MouseArea {
        id: mouseArea;

        anchors {
            left: icon.left;
            right: bar.right;
            top: icon.top;
            bottom: icon.bottom;
        }

        hoverEnabled: true;
        scrollGestureEnabled: false;
        onClicked: {
            if (pushToTalk) {
                return;
            }
            AudioScriptingInterface.muted = !muted;
            Tablet.playSound(TabletEnums.ButtonClick);
        }
        drag.target: dragTarget;
        onContainsMouseChanged: {
            if (containsMouse) {
                Tablet.playSound(TabletEnums.ButtonHover);
            }
        }
    }

    QtObject {
        id: colors;

        readonly property string unmutedColor: "#FFF";
        readonly property string mutedColor: "#E2334D";
        readonly property string gutter: "#575757";
        readonly property string greenStart: "#39A38F";
        readonly property string greenEnd: "#1FC6A6";
        readonly property string yellow: "#C0C000";
        readonly property string red: colors.mutedColor;
        readonly property string fill: "#55000000";
        readonly property string border: standalone ? "#80FFFFFF" : "#55FFFFFF";
        readonly property string icon: muted ? colors.mutedColor : unmutedColor;
    }

    Item {
        id: icon;

        anchors {
            left: parent.left;
            leftMargin: 5;
            verticalCenter: parent.verticalCenter;
        }

        width: 40;
        height: 40;

        Item {
            Image {
                readonly property string unmutedIcon: "../../../icons/tablet-icons/mic-unmute-i.svg";
                readonly property string mutedIcon: "../../../icons/tablet-icons/mic-mute-i.svg";
                readonly property string pushToTalkIcon: "../../../icons/tablet-icons/mic-ptt-i.svg";
                readonly property string clippingIcon: "../../../icons/tablet-icons/mic-clip-i.svg";
                readonly property string gatedIcon: "../../../icons/tablet-icons/mic-gate-i.svg";

                id: image;
                source: (pushToTalk && !pushingToTalk) ? pushToTalkIcon : muted ? mutedIcon :
                    clipping ? clippingIcon : gated ? gatedIcon : unmutedIcon;

                width: 30;
                height: 30;
                anchors {
                    left: parent.left;
                    leftMargin: 5;
                    top: parent.top;
                    topMargin: 5;
                }
            }

            ColorOverlay {
                anchors { fill: image }
                source: image;
                color: colors.icon;
            }
        }
    }

    Item {
        id: status;

        visible: (pushToTalk && !pushingToTalk) || muted;

        anchors {
            left: parent.left;
            leftMargin: 50;
            verticalCenter: parent.verticalCenter;
        }

        width: 170;
        height: 8

        Text {
            anchors {
                horizontalCenter: parent.horizontalCenter;
                verticalCenter: parent.verticalCenter;
            }

            color: colors.icon;

            text: (pushToTalk && !pushingToTalk) ? (HMD.active ? "MUTED PTT" : "MUTED PTT-(T)") : (muted ? "MUTED" : "MUTE");
            font.pointSize: 12;
        }

        Rectangle {
            anchors {
                left: parent.left;
                verticalCenter: parent.verticalCenter;
            }

            width: pushToTalk && !pushingToTalk ? (HMD.active ? 27 : 25) : 50;
            height: 4;
            color: colors.icon;
        }

        Rectangle {
            anchors {
                right: parent.right;
                verticalCenter: parent.verticalCenter;
            }

            width: pushToTalk && !pushingToTalk ? (HMD.active ? 27 : 25) : 50;
            height: 4;
            color: colors.icon;
        }
    }

    Item {
        id: bar;

        visible: !status.visible;

        anchors.fill: status;

        width: status.width;

        Rectangle { // base
            radius: 4;
            anchors { fill: parent }
            color: colors.gutter;
        }

        Rectangle { // mask
            id: mask;
            width: gated ? 0 : parent.width * level;
            radius: 5;
            anchors {
                bottom: parent.bottom;
                bottomMargin: 0;
                top: parent.top;
                topMargin: 0;
                left: parent.left;
                leftMargin: 0;
            }
        }

        LinearGradient {
            anchors { fill: mask }
            source: mask
            start: Qt.point(0, 0);
            end: Qt.point(170, 0);
            gradient: Gradient {
                GradientStop {
                    position: 0;
                    color: colors.greenStart;
                }
                GradientStop {
                    position: 0.5;
                    color: colors.greenEnd;
                }
                GradientStop {
                    position: 1;
                    color: colors.yellow;
                }
            }
        }

        Rectangle {
            id: gatedIndicator;
            visible: gated && !AudioScriptingInterface.clipping

            radius: 4;
            width: 2 * radius;
            height: 2 * radius;
            color: "#0080FF";
            anchors {
                right: parent.left;
                verticalCenter: parent.verticalCenter;
            }
        }

        Rectangle {
            id: clippingIndicator;
            visible: AudioScriptingInterface.clipping

            radius: 4;
            width: 2 * radius;
            height: 2 * radius;
            color: colors.red;
            anchors {
                left: parent.right;
                verticalCenter: parent.verticalCenter;
            }
        }
    }
}
