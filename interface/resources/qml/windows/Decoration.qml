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

import "."
import stylesUit 1.0

// TODO: look into whether we should outright replace the Hifi desktop system
import "../overte" as Overte

Rectangle {
    HifiConstants { id: hifi }

    signal inflateDecorations();
    signal deflateDecorations();

    property int frameMargin: 4
    property int frameMarginLeft: frameMargin
    property int frameMarginRight: frameMargin
    property int frameMarginTop: (2 * frameMargin) + iconSize
    property int frameMarginBottom: (2 * frameMargin) + (window.resizable || DebugQML ? 18 : 0)

    anchors {
        topMargin: -frameMarginTop
        leftMargin: -frameMarginLeft
        rightMargin: -frameMarginRight
        bottomMargin: -frameMarginBottom
    }
    anchors.fill: parent
    color: Overte.Theme.paletteActive.base
    border.width: Overte.Theme.borderWidth
    border.color: {
        if (Overte.Theme.highContrast) {
            return Overte.Theme.paletteActive.text;
        } else if (Overte.Theme.darkMode) {
            return Qt.lighter(Overte.Theme.paletteActive.base);
        } else {
            return Qt.darker(Overte.Theme.paletteActive.base);
        }
    }
    radius: Overte.Theme.borderRadius

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
