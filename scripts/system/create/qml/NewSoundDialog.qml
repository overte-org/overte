//
//  NewSoundDialog.qml
//  qml/hifi
//
//  Created by HifiExperiments on April 7th, 2024
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.2 as OriginalDialogs

import stylesUit 1.0
import controlsUit 1.0
import hifi.dialogs 1.0

Rectangle {
    id: newSoundDialog
    // width: parent.width
    // height: parent.height
    HifiConstants { id: hifi }
    color: hifi.colors.baseGray;
    signal sendToScript(var message);
    property bool keyboardEnabled: false
    property bool punctuationMode: false

    function errorMessageBox(message) {
        try {
            return desktop.messageBox({
                icon: hifi.icons.warning,
                defaultButton: OriginalDialogs.StandardButton.Ok,
                title: "Error",
                text: message
            });
        } catch(e) {
            Window.alert(message);
        }
    }

    Item {
        id: column1
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.bottomMargin: 10
        anchors.topMargin: 10
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: keyboard.top

        Text {
            id: text1
            text: qsTr("Sound URL <i></i>&nbsp;&nbsp;&nbsp;")
            color: "#ffffff"
            font.pixelSize: 12
        }

        Button {
            id: pasteBtn
            text: "Paste"
            font.pixelSize: 11
            height: 16
            width: 40
            radius: 4
            anchors.top: text1.top
            anchors.left: text1.right
            anchors.bottom: text1.bottom
            onClicked: {
                soundURL.paste()
            }
        }

        TextInput {
            id: soundURL
            height: 20
            text: qsTr("")
            color: "white"
            anchors.top: pasteBtn.bottom
            anchors.topMargin: 5
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.right: parent.right
            anchors.rightMargin: 0
            font.pixelSize: 12

            onAccepted: {
                newSoundDialog.keyboardEnabled = false;
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    newSoundDialog.keyboardEnabled = HMD.active
                    parent.focus = true;
                    parent.forceActiveFocus();
                    soundURL.cursorPosition = soundURL.positionAt(mouseX, mouseY, TextInput.CursorBetweenCharaters);
                }
            }
        }

        Rectangle {
            id: textInputBox
            color: "white"
            anchors.fill: soundURL
            opacity: 0.1
        }

        Row {
            id: row1
            height: 400
            spacing: 30
            anchors.top: soundURL.bottom
            anchors.topMargin: 5
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.right: parent.right
            anchors.rightMargin: 0

            Column {
                id: column3
                height: 400
                spacing: 10

                Row {
                    id: row3
                    width: 200
                    height: 400
                    spacing: 5

                    anchors.horizontalCenter: column3.horizontalCenter
                    anchors.horizontalCenterOffset: 0

                    Button {
                        id: button1
                        text: qsTr("Create")
                        z: -1
                        onClicked: {
                            newSoundDialog.sendToScript({
                                method: "newSoundDialogAdd",
                                params: {
                                    textInput: soundURL.text
                                }
                            });
                        }
                    }

                    Button {
                        id: button2
                        z: -1
                        text: qsTr("Cancel")
                        onClicked: {
                            newSoundDialog.sendToScript({method: "newSoundDialogCancel"})
                        }
                    }
                }
            }
        }
    }

    Keyboard {
        id: keyboard
        raised: parent.keyboardEnabled
        numeric: parent.punctuationMode
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
    }
}
