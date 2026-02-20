//
//  DefaultFrame.qml
//
//  Created by Bradley Austin Davis on 12 Jan 2016
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick
import QtQuick.Layouts

import "."
import stylesUit 1.0

// TODO: look into whether we should outright replace the Hifi desktop system
import "../overte" as Overte

Decoration {
    HifiConstants { id: hifi }

    // Dialog frame
    id: root

    property int iconSize: 24
    frameMargin: 4
    frameMarginLeft: frameMargin
    frameMarginRight: frameMargin
    frameMarginTop: (2 * frameMargin) + iconSize
    frameMarginBottom: (2 * frameMargin) + (window.resizable || DebugQML ? 18 : 0)

    onInflateDecorations: {
        if (!HMD.active) {
            return;
        }
        root.frameMargin = 18;
        root.iconSize = 32;
    }

    onDeflateDecorations: {
        root.frameMargin = 4;
        root.iconSize = 24;
    }

    Rectangle {
        anchors.fill: controlsRow
        color: Overte.Theme.paletteActive.activeWindowTitleBg
        topLeftRadius: Overte.Theme.borderRadius
        topRightRadius: Overte.Theme.borderRadius
    }

    RowLayout {
        id: controlsRow
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: Overte.Theme.borderWidth
        }
        spacing: 2
        height: root.frameMarginTop - root.frameMargin

        Overte.Label {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 4

            // Title
            id: titleText
            text: window ? window.title : ""
            verticalAlignment: Text.AlignVCenter
            color: Overte.Theme.paletteActive.activeWindowTitleFg
        }

        HiFiGlyphs {
            Layout.alignment: Qt.AlignCenter

            // "Pin" button
            visible: window.pinnable
            text: window.pinned ? hifi.glyphs.pinInverted : hifi.glyphs.pin
            color: pinClickArea.pressed ? hifi.colors.redHighlight : hifi.colors.white
            size: root.iconSize
            MouseArea {
                id: pinClickArea
                anchors.fill: parent
                hoverEnabled: true
                propagateComposedEvents: true
                onClicked: window.pinned = !window.pinned;
            }
        }

        Overte.RoundButton {
            Layout.alignment: Qt.AlignCenter
            Layout.rightMargin: 4

            visible: window ? window.closable : false
            icon.source: "../overte/icons/close.svg"
            icon.width: root.iconSize == 24 ? 12 : 24
            icon.height: root.iconSize == 24 ? 12 : 24
            icon.color: Overte.Theme.paletteActive.buttonText
            backgroundColor: (
                hovered ?
                Overte.Theme.paletteActive.buttonDestructive :
                Overte.Theme.paletteActive.button
            )

            implicitWidth: root.iconSize
            implicitHeight: root.iconSize

            onClicked: {
                window.shown = false;
                window.windowClosed();
            }
        }
    }
}
