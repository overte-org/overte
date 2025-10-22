//
//  QueryDialog.qml
//
//  Created by Bradley Austin Davis on 22 Jan 2016
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.7
import QtQuick.Dialogs as OriginalDialogs
import QtQuick.Controls 2.3

import controlsUit 1.0
import stylesUit 1.0
import "../windows"

TabletModalWindow {
    id: root
    HifiConstants { id: hifi }
    signal selected(var result);
    signal canceled();
    layer.enabled: true
    property int icon: hifi.icons.none
    property string iconText: ""
    property int iconSize: 35

    MouseArea {
        width: parent.width
        height: parent.height
    }

    property bool keyboardOverride: true
    onIconChanged: updateIcon();

    property var items;
    property string label: "" 
    property var result;
    property alias current: textResult.text 

    // For text boxes
    property alias placeholderText: textResult.placeholderText

    // For combo boxes
    property bool editable: true;

    property int titleWidth: 0
    onTitleWidthChanged: d.resize();

    property bool keyboardEnabled: false
    property bool keyboardRaised: false
    property bool punctuationMode: false

    onKeyboardRaisedChanged: d.resize();

    function updateIcon() {
        if (!root) {
            return;
        }
        iconText = hifi.glyphForIcon(root.icon);
    }

    TabletModalFrame {
        id: modalWindowItem
        width: parent.width - 12
        height: 240
        anchors.horizontalCenter: parent.horizontalCenter
        
       QtObject {
            id: d
            readonly property int minWidth: modalWindowItem.width
            readonly property int maxWidth: modalWindowItem.width
            readonly property int minHeight: 120
            readonly property int maxHeight: 720

            function resize() {
                var targetWidth = Math.max(titleWidth, modalWindowItem.width)
                var targetHeight = (items ? comboBox.controlHeight : textResult.controlHeight) + 5 * hifi.dimensions.contentSpacing.y + buttons.height
                modalWindowItem.width = (targetWidth < d.minWidth) ? d.minWidth : ((targetWidth > d.maxWdith) ? d.maxWidth : targetWidth);
                modalWindowItem.height = ((targetHeight < d.minHeight) ? d.minHeight : ((targetHeight > d.maxHeight) ? d.maxHeight : targetHeight)) + modalWindowItem.frameMarginTop
                modalWindowItem.y = (root.height - (modalWindowItem.height + ((keyboardEnabled && keyboardRaised) ? (keyboard.raisedHeight + 2 * hifi.dimensions.contentSpacing.y) : 0))) / 2
            }
        }
        
        Item {
            anchors {
                top: parent.top
                bottom: buttons.top;
                left: parent.left;
                right: parent.right;
                margins: 0
                bottomMargin: 2 * hifi.dimensions.contentSpacing.y
                topMargin: modalWindowItem.frameMarginTop
            }

            // FIXME make a text field type that can be bound to a history for autocompletion
            TextField {
                id: textResult
                label: root.label
                focus: items ? false : true
                visible: items ? false : true
                anchors {
                    left: parent.left;
                    right: parent.right;
                    bottom: parent.bottom
                    leftMargin: 5
                    rightMargin: 5
                }
            }

            ComboBox {
                id: comboBox
                label: root.label
                focus: true
                visible: items ? true : false
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                    rightMargin: 5
                }
                model: items ? items : []
            }
        }
        
        Flow {
            id: buttons
            focus: true
            spacing: hifi.dimensions.contentSpacing.x
            onHeightChanged: d.resize(); onWidthChanged: d.resize();
            layoutDirection: Qt.RightToLeft
            anchors {
                bottom: parent.bottom
                right: parent.right
                margins: 0
                rightMargin: hifi.dimensions.borderRadius
                bottomMargin: hifi.dimensions.contentSpacing.y
            }
            Button { action: cancelAction }
            Button { action: acceptAction }
        }

        Action {
            id: cancelAction
            text: qsTr("Cancel")
            shortcut: "Esc"
            onTriggered: {
                root.canceled();
                root.destroy();
            }
        }
        Action {
            id: acceptAction
            text: qsTr("OK")
            shortcut: "Return"
            onTriggered: {
                root.result = items ? comboBox.currentText : textResult.text
                root.selected(root.result);
                root.destroy();
            }
        }
    }

    Keyboard {
        id: keyboard
        raised: keyboardEnabled && keyboardRaised
        numeric: punctuationMode
        anchors {
            left: parent.left
            right: parent.right
            top: modalWindowItem.bottom
        }
    }
    Keys.onPressed: {
        if (!visible) {
            return
        }

        switch (event.key) {
        case Qt.Key_Escape:
        case Qt.Key_Back:
            cancelAction.trigger()
            event.accepted = true;
            break;

        case Qt.Key_Return:
        case Qt.Key_Enter:
            acceptAction.trigger()
            event.accepted = true;
            break;
        }
    }

   Component.onCompleted: {
       keyboardEnabled = HMD.active;
       updateIcon();
       d.resize();
       textResult.forceActiveFocus();
   }
}
