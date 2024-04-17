//
//  NewPolyVoxDialog.qml
//  Created by dr Karol Suprynowicz on 2022.05.17.
//  based on NewModelDialog.qml
//  qml/hifi
//
//  Copyright 2017 High Fidelity, Inc.
//  Copyright 2020 Vircadia contributors
//  Copyright 2022 Overte e.V.
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
    id: newPolyVoxDialog
    // width: parent.width
    // height: parent.height
    HifiConstants { id: hifi }
    color: hifi.colors.baseGray;
    signal sendToScript(var message);
    property bool keyboardEnabled: false
    property bool keyboardRaised: false
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
        anchors.topMargin: 0
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: keyboard.top
        
        ComboBox {
            id: texturePreset
            currentIndex: 0

            property var texturePresetArray: ["Material presets",
                                            "Grass + ground",
                                            "Bricks",
                                            "Stone",
                                            "Concrete",
                                            "Rock"]

            width: 200
            z: 100
            transformOrigin: Item.Center
            model: texturePresetArray
            
            onCurrentIndexChanged: {
                switch (currentIndex) {
                    // Clear texture entries
                    case 0:
                        xTextureURL.text = "";
                        yTextureURL.text = "";
                        zTextureURL.text = "";
                        break;
                    // Grass + ground
                    case 1:
                        xTextureURL.text = "qrc:///serverless/Textures/ground_5-2K/2K-ground_5-diffuse.jpg";
                        yTextureURL.text = "qrc:///serverless/Textures/ground_grass_gen_05.png";
                        zTextureURL.text = "qrc:///serverless/Textures/ground_5-2K/2K-ground_5-diffuse.jpg";
                        break;
                    // Bricks
                    case 2:
                        xTextureURL.text = "qrc:///serverless/Textures/2K-wall_stone_2-diffuse_l.jpg";
                        yTextureURL.text = "qrc:///serverless/Textures/2K-stone_floor_3-diffuse_l.jpg";
                        zTextureURL.text = "qrc:///serverless/Textures/2K-wall_stone_2-diffuse_l.jpg";
                        break;
                    // Stone
                    case 3:
                        xTextureURL.text = "qrc:///serverless/Textures/wall_l.png";
                        yTextureURL.text = "qrc:///serverless/Textures/floor_l.png";
                        zTextureURL.text = "qrc:///serverless/Textures/wall_l.png";
                        break;
                    // Concrete
                    case 4:
                        xTextureURL.text = "qrc:///serverless/Textures/concrete_12-2K/2K-concrete_12-diffuse.jpg";
                        yTextureURL.text = "qrc:///serverless/Textures/concrete_12-2K/2K-concrete_12-diffuse.jpg";
                        zTextureURL.text = "qrc:///serverless/Textures/concrete_12-2K/2K-concrete_12-diffuse.jpg";
                        break;
                    // Rock
                    case 5:
                        xTextureURL.text = "qrc:///serverless/Textures/Rock026_2K-JPG/Rock026_2K_Color.jpg";
                        yTextureURL.text = "qrc:///serverless/Textures/Rock026_2K-JPG/Rock026_2K_Color.jpg";
                        zTextureURL.text = "qrc:///serverless/Textures/Rock026_2K-JPG/Rock026_2K_Color.jpg";
                        break;
                }
            }
        }

        Text {
            id: text1
            anchors.top: texturePreset.bottom
            anchors.topMargin: 5
            text: qsTr("X Texture URL")
            color: "#ffffff"
            font.pixelSize: 12
        }

        TextInput {
            id: xTextureURL
            height: 20
            text: qsTr("")
            color: "white"
            anchors.top: text1.bottom
            anchors.topMargin: 5
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.right: parent.right
            anchors.rightMargin: 0
            font.pixelSize: 12

            onAccepted: {
                newPolyVoxDialog.keyboardEnabled = false;
            }

            onTextChanged : {
                if (xTextureURL.text.length === 0){
                    button1.enabled = false;
                } else {
                    button1.enabled = true;
                }
            }
            
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    newPolyVoxDialog.keyboardEnabled = HMD.active
                    parent.focus = true;
                    parent.forceActiveFocus();
                    xTextureURL.cursorPosition = xTextureURL.positionAt(mouseX, mouseY, TextInput.CursorBetweenCharaters);
                }
            }
        }

        Rectangle {
            id: textInputBox1
            color: "white"
            anchors.fill: xTextureURL
            opacity: 0.1
        }

        Text {
            id: text2
            text: qsTr("Y Texture URL")
            color: "#ffffff"
            font.pixelSize: 12
            anchors.top: textInputBox1.bottom
            anchors.topMargin: 5
        }

        TextInput {
            id: yTextureURL
            height: 20
            text: qsTr("")
            color: "white"
            anchors.top: text2.bottom
            anchors.topMargin: 5
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.right: parent.right
            anchors.rightMargin: 0
            font.pixelSize: 12

            onAccepted: {
                newPolyVoxDialog.keyboardEnabled = false;
            }

            onTextChanged : {
                if (yTextureURL.text.length === 0){
                    button1.enabled = false;
                } else {
                    button1.enabled = true;
                }
            }
            
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    newPolyVoxDialog.keyboardEnabled = HMD.active
                    parent.focus = true;
                    parent.forceActiveFocus();
                    yTextureURL.cursorPosition = yTextureURL.positionAt(mouseX, mouseY, TextInput.CursorBetweenCharaters);
                }
            }
        }

        Rectangle {
            id: textInputBox2
            color: "white"
            anchors.fill: yTextureURL
            opacity: 0.1
        }
        
        Text {
            id: text3
            text: qsTr("Z Texture URL")
            color: "#ffffff"
            font.pixelSize: 12
            anchors.top: textInputBox2.bottom
            anchors.topMargin: 5
        }

        TextInput {
            id: zTextureURL
            height: 20
            text: qsTr("")
            color: "white"
            anchors.top: text3.bottom
            anchors.topMargin: 5
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.right: parent.right
            anchors.rightMargin: 0
            font.pixelSize: 12

            onAccepted: {
                newPolyVoxDialog.keyboardEnabled = false;
            }

            onTextChanged : {
                if (zTextureURL.text.length === 0){
                    button1.enabled = false;
                } else {
                    button1.enabled = true;
                }
            }
            
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    newPolyVoxDialog.keyboardEnabled = HMD.active
                    parent.focus = true;
                    parent.forceActiveFocus();
                    zTextureURL.cursorPosition = zTextureURL.positionAt(mouseX, mouseY, TextInput.CursorBetweenCharaters);
                }
            }
        }

        Rectangle {
            id: textInputBox3
            color: "white"
            anchors.fill: zTextureURL
            opacity: 0.1
        }

        Text {
            id: textVolumeSize
            text: qsTr("Volume Size (number of voxels along the edge)")
            color: "#ffffff"
            font.pixelSize: 12
            anchors.top: zTextureURL.bottom
            anchors.topMargin: 5
        }
            
        Row {
            id: rowVolumeSize
            height: 50
            spacing: 30
            anchors.top: textVolumeSize.bottom
            anchors.topMargin: 5
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.right: parent.right
            anchors.rightMargin: 0
            
            Text {
                id: textVolumeSizeX
                text: qsTr("X")
                color: "#ffffff"
                font.pixelSize: 12
            }

            TextInput {
                id: volumeSizeX
                height: 20
                width: 50
                anchors.left: textVolumeSizeX.right
                anchors.leftMargin: 3
                text: qsTr("20")
                color: "white"
                font.pixelSize: 12
                validator: IntValidator{bottom: 8; top: 64;}

                onAccepted: {
                    newPolyVoxDialog.keyboardEnabled = false;
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        newPolyVoxDialog.keyboardEnabled = HMD.active
                        parent.focus = true;
                        parent.forceActiveFocus();
                        volumeSizeX.cursorPosition = volumeSizeX.positionAt(mouseX, mouseY, TextInput.CursorBetweenCharaters);
                    }
                }
            }

            Rectangle {
                id: textInputBoxVolumeSizeX
                color: "white"
                anchors.fill: volumeSizeX
                opacity: 0.1
            }

            Text {
                id: textVolumeSizeY
                text: qsTr("Y")
                color: "#ffffff"
                font.pixelSize: 12
                anchors.left: volumeSizeX.right
                anchors.leftMargin: 5
            }

            TextInput {
                id: volumeSizeY
                height: 20
                width: 50
                anchors.left: textVolumeSizeY.right
                anchors.leftMargin: 3
                text: qsTr("20")
                color: "white"
                font.pixelSize: 12
                validator: IntValidator{bottom: 8; top: 64;}

                onAccepted: {
                    newPolyVoxDialog.keyboardEnabled = false;
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        newPolyVoxDialog.keyboardEnabled = HMD.active
                        parent.focus = true;
                        parent.forceActiveFocus();
                        volumeSizeY.cursorPosition = volumeSizeY.positionAt(mouseX, mouseY, TextInput.CursorBetweenCharaters);
                    }
                }
            }

            Rectangle {
                id: textInputBoxVolumeSizeY
                color: "white"
                anchors.fill: volumeSizeY
                opacity: 0.1
            }
            Text {
                id: textVolumeSizeZ
                text: qsTr("X")
                color: "#ffffff"
                font.pixelSize: 12
                anchors.left: volumeSizeY.right
                anchors.leftMargin: 5
            }

            TextInput {
                id: volumeSizeZ
                height: 20
                width: 50
                anchors.left: textVolumeSizeZ.right
                anchors.leftMargin: 3
                text: qsTr("20")
                color: "white"
                font.pixelSize: 12
                validator: IntValidator{bottom: 8; top: 64;}

                onAccepted: {
                    newPolyVoxDialog.keyboardEnabled = false;
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        newPolyVoxDialog.keyboardEnabled = HMD.active
                        parent.focus = true;
                        parent.forceActiveFocus();
                        volumeSizeZ.cursorPosition = volumeSizeZ.positionAt(mouseX, mouseY, TextInput.CursorBetweenCharaters);
                    }
                }
            }

            Rectangle {
                id: textInputBoxVolumeSizeZ
                color: "white"
                anchors.fill: volumeSizeZ
                opacity: 0.1
            }
        }

        Row {
            id: row1
            height: 400
            spacing: 30
            anchors.top: rowVolumeSize.bottom
            anchors.topMargin: 5
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.right: parent.right
            anchors.rightMargin: 0

            Column {
                id: column2
                width: 200
                height: 600
                spacing: 10


                CheckBox {
                    id: grabbable
                    text: qsTr("Grabbable")
                }

                CheckBox {
                    id: collisions
                    text: qsTr("Collisions")
                }

                Row {
                    id: row2
                    width: 200
                    height: 400
                    spacing: 20

                }
            }

            Column {
                id: column3
                height: 400
                spacing: 10

                Text {
                    id: text4
                    text: qsTr("Voxel type")
                    color: "#ffffff"
                    font.pixelSize: 12
                }

                ComboBox {
                    id: surfaceStyle
                    currentIndex: 3

                    property var surfaceStyleArray: ["Marching Cubes",
                                                  "Cubic",
                                                  "Edged Cubic",
                                                  "Edged Marching Cubes"]

                    width: 200
                    z: 100
                    transformOrigin: Item.Center
                    model: surfaceStyleArray
                }

                Text {
                    id: textInitialShape
                    text: qsTr("Initial shape")
                    color: "#ffffff"
                    font.pixelSize: 12
                }

                ComboBox {
                    id: initialShape
                    currentIndex: 0

                    property var initialShapeArray: ["Box",
                                                  "Plane, 1/4 full",
                                                  "Plane, 3/4 full",
                                                  "Single voxel",
                                                     ]

                    width: 200
                    z: 100
                    transformOrigin: Item.Center
                    model: initialShapeArray
                }

                Row {
                    id: row3
                    width: 200
                    height: 400
                    spacing: 5

                    anchors.horizontalCenter: column3.horizontalCenter
                    anchors.horizontalCenterOffset: -20

                    Button {
                        id: button1
                        text: qsTr("Create")
                        z: -1
                        enabled: false
                        onClicked: {
                            newPolyVoxDialog.sendToScript({
                                method: "newPolyVoxDialogAdd",
                                params: {
                                    xTextureURL: xTextureURL.text,
                                    yTextureURL: yTextureURL.text,
                                    zTextureURL: zTextureURL.text,
                                    volumeSizeX: volumeSizeX.text,
                                    volumeSizeY: volumeSizeY.text,
                                    volumeSizeZ: volumeSizeZ.text,
                                    surfaceStyleIndex: surfaceStyle.currentIndex,
                                    initialShapeIndex: initialShape.currentIndex,
                                    grabbable: grabbable.checked,
                                    collisions: collisions.checked,
                                }
                            });
                        }
                    }

                    Button {
                        id: button2
                        z: -1
                        text: qsTr("Cancel")
                        onClicked: {
                            newPolyVoxDialog.sendToScript({method: "newPolyVoxDialogCancel"})
                        }
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
