//
//  NewMaterialDialog.qml
//  qml/hifi
//
//  Created by Sam Gondelman on January 17th, 2018
//  Copyright 2018 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Dialogs as OriginalDialogs

import stylesUit 1.0
import controlsUit 1.0
import hifi.dialogs 1.0

Rectangle {
    id: newMaterialDialog
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
            text: qsTr("Material URL <i>(Optional)</i>&nbsp;&nbsp;&nbsp;")
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
                materialURL.paste()
            }
        }

        TextInput {
            id: materialURL
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
                newMaterialDialog.keyboardEnabled = false;
            }
            
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    newMaterialDialog.keyboardEnabled = HMD.active
                    parent.focus = true;
                    parent.forceActiveFocus();
                    materialURL.cursorPosition = materialURL.positionAt(mouseX, mouseY, TextInput.CursorBetweenCharaters);
                }
            }
        }

        Rectangle {
            id: textInputBox
            color: "white"
            anchors.fill: materialURL
            opacity: 0.1
        }

        Row {
            id: row1
            height: 400
            spacing: 30
            anchors.top: materialURL.bottom
            anchors.topMargin: 5
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.right: parent.right
            anchors.rightMargin: 0

            Column {
                id: column3
                height: 400
                spacing: 10

                /*Text {
                    id: text3
                    text: qsTr("Material Mode")
                    color: "#ffffff"
                    font.pixelSize: 12
                }

                ComboBox {
                    id: materialMappingMode
                    property var materialArray: ["UV space material",
                                                 "3D projected material"]

                    width: 200
                    z: 100
                    transformOrigin: Item.Center
                    model: materialArray
                }*/

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
                            newMaterialDialog.sendToScript({
                                method: "newMaterialDialogAdd",
                                params: {
                                    textInput: materialURL.text,
                                    //comboBox: materialMappingMode.currentIndex
                                }
                            });
                        }
                    }

                    Button {
                        id: button2
                        z: -1
                        text: qsTr("Cancel")
                        onClicked: {
                            newMaterialDialog.sendToScript({method: "newMaterialDialogCancel"})
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
