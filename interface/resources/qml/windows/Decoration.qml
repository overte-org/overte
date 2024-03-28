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

    Behavior on frameMargin {
        NumberAnimation {
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
    color: Qt.rgba(0,0,0,0.8)
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
        }
        onExited: {
            if (!containsMouseGlobal()) {
                window.mouseExited();
                frameMargin = 2;
                titleMargin = 5;
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
        height: frameMarginTop
        width: parent.width
        color: "black"
    }
    Row {
        id: controlsRow
        width: parent.width
        height: frameMarginTop

        // Title
        Text {
            id: titleText
            anchors {
                top: parent.top + frameMargin
                bottom: parent.bottom
                left: parent.left
                leftMargin: titleMargin
            }
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
            Button {
                text: "X"
                visible: window ? window.closable : false
                height: frameMarginTop
                width: 100
                onClicked: {
                        window.shown = false;
                        window.windowClosed();
                    }
            }
        }

    }
}

