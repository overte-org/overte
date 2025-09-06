//
//  Decoration.qml
//
//  Created by Bradley Austin Davis on 12 Jan 2016
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.5
import Qt5Compat.GraphicalEffects

import "."
import stylesUit 1.0

Rectangle {
    HifiConstants { id: hifi }

    signal inflateDecorations();
    signal deflateDecorations();

    property int frameMargin: 9
    property int frameMarginLeft: frameMargin
    property int frameMarginRight: frameMargin
    property int frameMarginTop: 2 * frameMargin + iconSize
    property int frameMarginBottom: iconSize + 11

    anchors {
        topMargin: -frameMarginTop
        leftMargin: -frameMarginLeft
        rightMargin: -frameMarginRight
        bottomMargin: -frameMarginBottom
    }
    anchors.fill: parent
    color: hifi.colors.baseGrayHighlight40
    border {
        width: hifi.dimensions.borderWidth
        color: hifi.colors.faintGray50
    }
    radius: hifi.dimensions.borderRadius

    // Enable dragging of the window,
    // detect mouseover of the window (including decoration)
    MouseArea {
        id: decorationMouseArea
        anchors.fill: parent
        drag.target: window
        hoverEnabled: true
        onEntered: window.mouseEntered();
        onExited: {
            if (!containsMouseGlobal()) {
                window.mouseExited();
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
                root.inflateDecorations()
            }
        }
        function onMouseExited() {
            root.deflateDecorations();
        }
    }
    Connections {
        target: desktop
        function onHmdHandMouseActiveChanged() {
            if (desktop.hmdHandMouseActive) {
                if (decorationMouseArea.containsMouse) {
                    root.inflateDecorations();
                }
            } else {
                root.deflateDecorations();
            }
        }
    }
}

