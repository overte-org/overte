//
//  TabletCustomQueryDialog.qml
//
//  Created by Vlad Stelmahovsky on 3/27/17
//  Copyright 2017 High Fidelity, Inc.
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
    id: root;
    HifiConstants { id: hifi; }

    anchors.fill: parent
    width: parent.width
    height: parent.height

    title: ""
    visible: true;
    property bool keyboardOverride: true

    signal selected(var result);
    signal canceled();

    property int icon: hifi.icons.none;
    property string iconText: "";
    property int iconSize: 35;
    onIconChanged: updateIcon();

    property var textInput;
    property var comboBox;
    property var checkBox;
    onTextInputChanged: {
        if (textInput && textInput.text !== undefined) {
            textField.text = textInput.text;
        }
    }
    onComboBoxChanged: {
        if (comboBox && comboBox.index !== undefined) {
            comboBoxField.currentIndex = comboBox.index;
        }
    }
    onCheckBoxChanged: {
        if (checkBox && checkBox.checked !== undefined) {
            checkBoxField.checked = checkBox.checked;
        }
    }

    property bool keyboardEnabled: false
    property bool keyboardRaised: false
    property bool punctuationMode: false
    onKeyboardRaisedChanged: d.resize();

    property var warning: "";
    property var result;

    property var implicitCheckState: null;

    property int titleWidth: 0;
    onTitleWidthChanged: d.resize();

    MouseArea {
        width: parent.width
        height: parent.height
    }

    function updateIcon() {
        if (!root) {
            return;
        }
        iconText = hifi.glyphForIcon(root.icon);
    }

    function updateCheckbox() {
        if (checkBox.disableForItems) {
            var currentItemInDisableList = false;
            for (var i in checkBox.disableForItems) {
                if (comboBoxField.currentIndex === checkBox.disableForItems[i]) {
                    currentItemInDisableList = true;
                    break;
                }
            }

            if (currentItemInDisableList) {
                checkBoxField.enabled = false;
                if (checkBox.checkStateOnDisable !== null && checkBox.checkStateOnDisable !== undefined) {
                    root.implicitCheckState = checkBoxField.checked;
                    checkBoxField.checked = checkBox.checkStateOnDisable;
                }
                root.warning = checkBox.warningOnDisable;
            } else {
                checkBoxField.enabled = true;
                if (root.implicitCheckState !== null) {
                    checkBoxField.checked = root.implicitCheckState;
                    root.implicitCheckState = null;
                }
                root.warning = "";
            }
        }
    }

    TabletModalFrame {
        id: modalWindowItem
        width: parent.width - 6
        height: 240
        anchors {
            verticalCenter: parent.verticalCenter
            horizontalCenter: parent.horizontalCenter
        }

        MouseArea {
            // Clicking dialog background defocuses active control.
            anchors.fill: parent
            onClicked: parent.forceActiveFocus();
        }

        QtObject {
            id: d;
            readonly property int minWidth: 470
            readonly property int maxWidth: 470
            readonly property int minHeight: 120
            readonly property int maxHeight: 720

            function resize() {
                var targetWidth = Math.max(titleWidth, 470);
                var targetHeight = (textField.visible ? textField.controlHeight + hifi.dimensions.contentSpacing.y : 0) +
                        (extraInputs.visible ? extraInputs.height + hifi.dimensions.contentSpacing.y : 0) +
                        (buttons.height + 3 * hifi.dimensions.contentSpacing.y) +
                        ((keyboardEnabled && keyboardRaised) ? (keyboard.raisedHeight + hifi.dimensions.contentSpacing.y) : 0);

                root.width = (targetWidth < d.minWidth) ? d.minWidth : ((targetWidth > d.maxWdith) ? d.maxWidth : targetWidth);
                modalWindowItem.height = (targetHeight < d.minHeight) ? d.minHeight : ((targetHeight > d.maxHeight) ?
                                                                                           d.maxHeight : targetHeight);
            }
        }

        Item {
            anchors {
                top: parent.top;
                bottom: extraInputs.visible ? extraInputs.top : buttons.top;
                left: parent.left;
                right: parent.right;
                leftMargin: 12
                rightMargin: 12
            }

            // FIXME make a text field type that can be bound to a history for autocompletion
            TextField {
                id: textField;
                label: root.textInput.label;
                focus: root.textInput ? true : false;
                visible: root.textInput ? true : false;
                anchors {
                    left: parent.left;
                    right: parent.right;
                    bottom: keyboard.top;
                    bottomMargin: hifi.dimensions.contentSpacing.y;
                }
            }

            property alias keyboardOverride: root.keyboardOverride
            property alias keyboardRaised: root.keyboardRaised
            property alias punctuationMode: root.punctuationMode
            Keyboard {
                id: keyboard
                raised: keyboardEnabled && keyboardRaised
                numeric: punctuationMode
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                    bottomMargin: raised ? hifi.dimensions.contentSpacing.y : 0
                }
            }
        }

        Item {
            id: extraInputs;
            visible: Boolean(root.checkBox || root.comboBox);
            anchors {
                left: parent.left;
                right: parent.right;
                bottom: buttons.top;
                bottomMargin: hifi.dimensions.contentSpacing.y;
                leftMargin: 12
                rightMargin: 12
            }
            height: comboBoxField.controlHeight;
            onHeightChanged: d.resize();
            onWidthChanged: d.resize();
            z: 20

            CheckBox {
                id: checkBoxField;
                text: root.checkBox.label;
                focus: Boolean(root.checkBox);
                visible: Boolean(root.checkBox);
                anchors {
                    left: parent.left;
                    bottom: parent.bottom;
                    leftMargin: 6;  // Magic number to align with warning icon
                    bottomMargin: 6;
                }
            }

            ComboBox {
                id: comboBoxField;
                label: root.comboBox.label;
                focus: Boolean(root.comboBox);
                visible: Boolean(root.comboBox);
                Binding on x {
                    when: comboBoxField.visible
                    value: !checkBoxField.visible ? buttons.x : acceptButton.x
                }

                Binding on width {
                    when: comboBoxField.visible
                    value: !checkBoxField.visible ? buttons.width : buttons.width - acceptButton.x
                }
                anchors {
                    right: parent.right;
                    bottom: parent.bottom;
                }
                model: root.comboBox ? root.comboBox.items : [];
                onAccepted: {
                    updateCheckbox();
                    focus = true;
                }
            }
        }

        Row {
            id: buttons;
            focus: true;
            spacing: hifi.dimensions.contentSpacing.x;
            layoutDirection: Qt.RightToLeft;
            onHeightChanged: d.resize();
            onWidthChanged: {
                d.resize();
                resizeWarningText();
            }
            z: 10

            anchors {
                bottom: parent.bottom;
                left: parent.left;
                right: parent.right;
                bottomMargin: hifi.dimensions.contentSpacing.y;
                leftMargin: 12
                rightMargin: 12
            }

            function resizeWarningText() {
                var rowWidth = buttons.width;
                var buttonsWidth = acceptButton.width + cancelButton.width + hifi.dimensions.contentSpacing.x * 2;
                var warningIconWidth = warningIcon.width + hifi.dimensions.contentSpacing.x;
                warningText.width = rowWidth - buttonsWidth - warningIconWidth;
            }

            Button {
                id: cancelButton;
                action: cancelAction;
            }

            Button {
                id: acceptButton;
                action: acceptAction;
            }

            Text {
                id: warningText;
                visible: Boolean(root.warning);
                text: root.warning;
                wrapMode: Text.WordWrap;
                font.italic: true;
                maximumLineCount: 3;
            }

            HiFiGlyphs {
                id: warningIcon;
                visible: Boolean(root.warning);
                text: hifi.glyphs.alert;
                size: hifi.dimensions.controlLineHeight;
                width: 20  // Line up with checkbox.
            }
        }

        Action {
            id: cancelAction;
            text: qsTr("Cancel");
            shortcut: "Esc";
            onTriggered: {
                root.result = null;
                root.canceled();
                root.destroy();
            }
        }

        Action {
            id: acceptAction;
            text: qsTr("Add");
            shortcut: "Return";
            onTriggered: {
                var result = {};
                if (textInput) {
                    result.textInput = textField.text;
                }
                if (comboBox) {
                    result.comboBox = comboBoxField.currentIndex;
                    result.comboBoxText = comboBoxField.currentText;
                }
                if (checkBox) {
                    result.checkBox = checkBoxField.enabled ? checkBoxField.checked : null;
                }
                root.result = JSON.stringify(result);
                root.selected(root.result);
                root.destroy();
            }
        }
    }

    Keys.onPressed: {
        if (!visible) {
            return;
        }

        switch (event.key) {
        case Qt.Key_Escape:
        case Qt.Key_Back:
            cancelAction.trigger();
            event.accepted = true;
            break;

        case Qt.Key_Return:
        case Qt.Key_Enter:
            acceptAction.trigger();
            event.accepted = true;
            break;
        }
    }

    Component.onCompleted: {
        keyboardEnabled = HMD.active;
        updateIcon();
        updateCheckbox();
        d.resize();
        textField.forceActiveFocus();
    }
}
