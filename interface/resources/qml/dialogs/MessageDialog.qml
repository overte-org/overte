//
//  MessageDialog.qml
//
//  Created by Bradley Austin Davis on 15 Jan 2016
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.5
import QtQuick.Dialogs as OriginalDialogs

import controlsUit 1.0
import stylesUit 1.0
import "../windows"

import "messageDialog"

ModalWindow {
    id: root
    HifiConstants { id: hifi }
    destroyOnCloseButton: true
    destroyOnHidden: true
    visible: true

    signal selected(int button);

    function click(button) {
        clickedButton = button;
        selected(button);
        destroy();
    }

    function exec() {
        return OffscreenUi.waitForMessageBoxResult(root);
    }

    Keys.onRightPressed: if (defaultButton === OriginalDialogs.MessageDialog.Yes) {
       yesButton.forceActiveFocus()
    } else if (defaultButton === OriginalDialogs.MessageDialog.Ok) {
       okButton.forceActiveFocus()
    }
    Keys.onTabPressed: if (defaultButton === OriginalDialogs.MessageDialog.Yes) {
       yesButton.forceActiveFocus()
    } else if (defaultButton === OriginalDialogs.MessageDialog.Ok) {
       okButton.forceActiveFocus()
    }
    property alias detailedText: detailedText.text
    property alias text: mainTextContainer.text
    property alias informativeText: informativeTextContainer.text
    property int buttons: OriginalDialogs.MessageDialog.Ok
    property int icon: OriginalDialogs.StandardIcon.NoIcon
    property string iconText: ""
    property int iconSize: 50
    onIconChanged: updateIcon();
    property int defaultButton: OriginalDialogs.MessageDialog.NoButton;
    property int clickedButton: OriginalDialogs.MessageDialog.NoButton;

    property int titleWidth: 0
    onTitleWidthChanged: d.resize();

    function updateIcon() {
        if (!root) {
            return;
        }
        iconText = hifi.glyphForIcon(root.icon);
    }

    Item {
        id: messageBox
        clip: true
        width: pane.width
        height: pane.height

        QtObject {
            id: d
            readonly property int minWidth: 1100
            readonly property int maxWidth: 1280
            readonly property int minHeight: 120
            readonly property int maxHeight: 720

            function resize() {
                var targetWidth = Math.max(titleWidth, mainTextContainer.contentWidth)
                var targetHeight = mainTextContainer.height + 3 * hifi.dimensions.contentSpacing.y
                        + (informativeTextContainer.text != "" ? informativeTextContainer.contentHeight + 3 * hifi.dimensions.contentSpacing.y : 0)
                        + buttons.height
                        + (content.state === "expanded" ? details.implicitHeight + hifi.dimensions.contentSpacing.y : 0)
                root.width = (targetWidth < d.minWidth) ? d.minWidth : ((targetWidth > d.maxWidth) ? d.maxWidth : targetWidth)
                root.height = (targetHeight < d.minHeight) ? d.minHeight: ((targetHeight > d.maxHeight) ? d.maxHeight : targetHeight)
            }
        }

        RalewaySemiBold {
            id: mainTextContainer
            onTextChanged: d.resize();
            wrapMode: Text.WordWrap
            width: messageBox.width
            size: hifi.fontSizes.menuItem
            color: hifi.colors.baseGrayHighlight
            anchors {
                top: parent.top
                horizontalCenter: parent.horizontalCenter
                margins: 0
                topMargin: hifi.dimensions.contentSpacing.y
            }
            lineHeight: 2
            lineHeightMode: Text.ProportionalHeight
            horizontalAlignment: Text.AlignHCenter
        }

        RalewaySemiBold {
            id: informativeTextContainer
            onTextChanged: d.resize();
            wrapMode: Text.WordWrap
            size: hifi.fontSizes.menuItem
            color: hifi.colors.baseGrayHighlight
            anchors {
                top: mainTextContainer.bottom
                left: parent.left
                right: parent.right
                margins: 0
                topMargin: text != "" ? hifi.dimensions.contentSpacing.y : 0
            }
        }

        Flow {
            id: buttons
            focus: true
            spacing: hifi.dimensions.contentSpacing.x
            onHeightChanged: d.resize(); onWidthChanged: d.resize();
            layoutDirection: Qt.RightToLeft
            anchors {
                top: informativeTextContainer.text == "" ? mainTextContainer.bottom : informativeTextContainer.bottom
                horizontalCenter: parent.horizontalCenter
                margins: 0
                topMargin: 2 * hifi.dimensions.contentSpacing.y
            }
            MessageDialogButton { dialog: root; text: qsTr("Close"); button: OriginalDialogs.MessageDialog.Close; }
            MessageDialogButton { dialog: root; text: qsTr("Abort"); button: OriginalDialogs.MessageDialog.Abort; }
            MessageDialogButton { dialog: root; text: qsTr("Cancel"); button: OriginalDialogs.MessageDialog.Cancel; }
            MessageDialogButton { dialog: root; text: qsTr("Restore Defaults"); button: OriginalDialogs.MessageDialog.RestoreDefaults; }
            MessageDialogButton { dialog: root; text: qsTr("Reset"); button: OriginalDialogs.MessageDialog.Reset; }
            MessageDialogButton { dialog: root; text: qsTr("Discard"); button: OriginalDialogs.MessageDialog.Discard; }
            MessageDialogButton { dialog: root; text: qsTr("No to All"); button: OriginalDialogs.MessageDialog.NoToAll; }
            MessageDialogButton {
                id: noButton
                dialog: root
                text: qsTr("No")
                button: OriginalDialogs.MessageDialog.No
                KeyNavigation.left: yesButton
                KeyNavigation.backtab: yesButton
            }
            MessageDialogButton { dialog: root; text: qsTr("Yes to All"); button: OriginalDialogs.MessageDialog.YesToAll; }
            MessageDialogButton {
                id: yesButton
                dialog: root
                text: qsTr("Yes") 
                button: OriginalDialogs.MessageDialog.Yes
                KeyNavigation.right: noButton
                KeyNavigation.tab: noButton
            }
            MessageDialogButton { dialog: root; text: qsTr("Apply"); button: OriginalDialogs.MessageDialog.Apply; }
            MessageDialogButton { dialog: root; text: qsTr("Ignore"); button: OriginalDialogs.MessageDialog.Ignore; }
            MessageDialogButton { dialog: root; text: qsTr("Retry"); button: OriginalDialogs.MessageDialog.Retry; }
            MessageDialogButton { dialog: root; text: qsTr("Save All"); button: OriginalDialogs.MessageDialog.SaveAll; }
            MessageDialogButton { dialog: root; text: qsTr("Save"); button: OriginalDialogs.MessageDialog.Save; }
            MessageDialogButton { dialog: root; text: qsTr("Open"); button: OriginalDialogs.MessageDialog.Open; }
            MessageDialogButton {
                id: okButton
                dialog: root 
                text: qsTr("OK")
                button: OriginalDialogs.MessageDialog.Ok
            }

            Button {
                id: moreButton
                text: qsTr("Show Details...")
                width: 160
                onClicked: { content.state = (content.state === "" ? "expanded" : "") }
                visible: detailedText && detailedText.length > 0
            }
            MessageDialogButton { dialog: root; text: qsTr("Help"); button: OriginalDialogs.MessageDialog.Help; }
        }

        Item {
            id: details
            width: parent.width
            implicitHeight: detailedText.implicitHeight
            height: 0
            clip: true
            anchors {
                top: buttons.bottom
                left: parent.left;
                right: parent.right;
                margins: 0
                topMargin: hifi.dimensions.contentSpacing.y
            }
            Flickable {
                id: flickable
                contentHeight: detailedText.height
                anchors.fill: parent
                anchors.topMargin: hifi.dimensions.contentSpacing.x
                anchors.bottomMargin: hifi.dimensions.contentSpacing.y
                TextEdit {
                    id: detailedText
                    size: hifi.fontSizes.menuItem
                    color: hifi.colors.baseGrayHighlight
                    width: details.width
                    wrapMode: Text.WordWrap
                    readOnly: true
                    selectByMouse: true
                    anchors.margins: 0
                }
            }
        }

        states: [
            State {
                name: "expanded"
                PropertyChanges { target: root; anchors.fill: undefined }
                PropertyChanges { target: details; height: 120 }
                PropertyChanges { target: moreButton; text: qsTr("Hide Details") }
            }
        ]

        Component.onCompleted: {
            updateIcon();
            d.resize();
        }
        onStateChanged: d.resize()
    }

    Keys.onPressed: {
        if (!visible) {
            return
        }

        if (event.modifiers === Qt.ControlModifier)
            switch (event.key) {
            case Qt.Key_A:
                event.accepted = true
                detailedText.selectAll()
                break
            case Qt.Key_C:
                event.accepted = true
                detailedText.copy()
                break
            case Qt.Key_Period:
                if (Qt.platform.os === "osx") {
                    event.accepted = true
                    content.reject()
                }
                break
        } else switch (event.key) {
            case Qt.Key_Escape:
            case Qt.Key_Back:
                event.accepted = true
                root.click(OriginalDialogs.MessageDialog.Cancel)
                break
        }
    }
}
