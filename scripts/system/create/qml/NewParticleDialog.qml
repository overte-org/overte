//
//  NewParticleDialog.qml
//  qml/hifi
//
//  Created by HifiExperiments on 11/22/23
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.5
import QtQuick.Dialogs 1.2 as OriginalDialogs

import stylesUit 1.0
import controlsUit 1.0
import hifi.dialogs 1.0

Rectangle {
    id: newParticleDialog
    // width: parent.width
    // height: parent.height
    HifiConstants { id: hifi }
    color: hifi.colors.baseGray;
    signal sendToScript(var message);
    property bool keyboardEnabled: false
    property bool keyboardRaised: false
    property bool punctuationMode: false
    property bool keyboardRasied: false

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

        Column {
            id: column2
            height: 400
            spacing: 10

            CheckBox {
                id: procedural
                text: qsTr("Procedural (GPU) Particles?")
                checked: false
            }

            Row {
                id: row1
                width: 200
                height: 400
                spacing: 5

                Button {
                    id: button1
                    text: qsTr("Create")
                    z: -1
                    onClicked: {
                        newParticleDialog.sendToScript({
                            method: "newParticleDialogAdd",
                            params: {
                                procedural: procedural.checked
                            }
                        });
                    }
                }

                Button {
                    id: button2
                    z: -1
                    text: qsTr("Cancel")
                    onClicked: {
                        newParticleDialog.sendToScript({method: "newParticleDialogCancel"})
                    }
                }
            }
        }
    }

    Keyboard {
        id: keyboard
        raised: parent.keyboardEnabled && parent.keyboardRaised
        numeric: parent.punctuationMode
        anchors {
            bottom: parent.bottom
            bottomMargin: 40
            left: parent.left
            right: parent.right
        }
    }
}
