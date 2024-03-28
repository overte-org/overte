//
//  Decoration.qml
//
//  Created by Armored Dragon with parts used from Bradley Austin Davis
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

// TODO: Remove Controls after adjusting action buttons

import QtGraphicalEffects 1.15
import QtQuick 2.5
import QtQuick.Controls 2.0

import "."
import stylesUit 1.0



Rectangle {
    HifiConstants { id: hifi }

    // Dialog frame
    id: root

    signal inflateDecorations();
    signal deflateDecorations();

    property int iconSize: hifi.dimensions.frameIconSize
    property int titleMargin: 5
    property int frameMargin: 2
    property int frameMarginLeft: frameMargin
    property int frameMarginRight: frameMargin
    property int frameMarginTop: 10 + iconSize
    property int frameMarginBottom: frameMargin
    property bool is_window: true // Controls whether or not the border should include the top bar
    property color bg_color: Qt.rgba(0.2,0.2,0.2,0.8)

    Behavior on frameMargin {
        NumberAnimation {
            duration: 100
            easing.type: Easing.InOutQuad
        }
    }
    Behavior on bg_color {
        ColorAnimation {
            duration: 100
            easing.type: Easing.InOutQuad
        }
    }
    Behavior on titleMargin {
        NumberAnimation {
            duration: 100
            easing.type: Easing.InOutQuad
        }
    }

    anchors {
        topMargin: -frameMarginTop
        leftMargin: -frameMarginLeft
        rightMargin: -frameMarginRight
        bottomMargin: -frameMarginBottom
    }
    anchors.fill: parent
    color: bg_color
    radius: 0 // hifi.dimensions.borderRadius

    // Enable dragging of the window,
    // detect mouseover of the window (including decoration)
    MouseArea {
        id: decorationMouseArea
        anchors.fill: parent
        drag.target: window
        hoverEnabled: true
        onEntered: {
            window.mouseEntered();
            frameMargin = 15;
            titleMargin = 18;
            bg_color = Qt.rgba(0.113, 0.122, 0.149, 1)
        }
        onExited: {
            if (!containsMouseGlobal()) {
                window.mouseExited();
                frameMargin = 2;
                titleMargin = 5;
                bg_color = Qt.rgba(0,0,0,0.8)
            }
        }

        function containsMouseGlobal() {
            var reticlePos = Reticle.position;
            var globalPosition = decorationMouseArea.mapToItem(desktop, 0, 0);
            var localPosition = {
                x: reticlePos.x - globalPosition.x,
                y: reticlePos.y - globalPosition.y,
            };
            return localPosition.x >= 0 && localPosition.x <= width &&
                   localPosition.y >= 0 && localPosition.y <= height;
        }

    }
    Connections {
        target: window
        function onMouseEntered() {
            if (desktop.hmdHandMouseActive) {
                // root.inflateDecorations()
            }
        }
        function onMouseExited() {
            // root.deflateDecorations();
        }
    }
    Connections {
        target: desktop
        function onHmdHandMouseActiveChanged() {
            if (desktop.hmdHandMouseActive) {
                if (decorationMouseArea.containsMouse) {
                    // root.inflateDecorations();
                }
            } else {
                // root.deflateDecorations();
            }
        }
    }

    Rectangle{
        visible: is_window
        height: frameMarginTop
        width: parent.width
        color: "black"
    }
    Row {
        visible: is_window
        id: controlsRow
        width: parent.width
        height: frameMarginTop

        // Title
        Text {
            id: titleText
            anchors {
                top: parent.top + frameMargin // FIXME: Error somewhere here
                bottom: parent.bottom
                left: parent.left
                leftMargin: titleMargin
            }
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.family: "Raleway"
            font.pixelSize: 20 // Set font size
            color: hifi.colors.white
            text: window ? window.title : ""
        }

        // Action buttons
        Row {
            anchors {
                right: parent.right
                top: parent.top
            }
            // Close
            Item {
                visible: window ? window.closable : false
                height: frameMarginTop
                width: 50

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor
                    onEntered: {
                        cursorShape: Qt.PointingHandCursor
                        closeButton.color = Qt.rgba(1,0,0,1)
                    }
                    onExited: {
                        cursorShape: Qt.ArrowCursor
                        closeButton.color = Qt.rgba(1,1,1,1)
                    }
                    onClicked: {
                        window.shown = false;
                        window.windowClosed();
                    }
                }

                Text {
                    id: closeButton
                    text: hifi.glyphs.close
                    font.family: "hifi-glyphs"
                    color: Qt.rgba(1,1,1,1)
                    font.pointSize: 18
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    Behavior on color {
                        ColorAnimation{
                            duration: 100
                            easing.type: Easing.InOutQuad
                        }
                    }
                }
            }
        }

    }
}

