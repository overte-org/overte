//
//  RunningScripts.qml
//
//  Created by Bradley Austin Davis on 12 Jan 2016
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Dialogs as OriginalDialogs
import Qt.labs.settings 1.0

import stylesUit 1.0
import controlsUit 1.0 as HifiControls
import "../../windows" as Windows
import "../"

Windows.ScrollingWindow {
    id: root
    objectName: "RunningScripts"
    title: "Running Scripts"
    resizable: true
    destroyOnHidden: false
    implicitWidth: 424
    opacity: parent.opacity
    implicitHeight: isHMD ? 695 : 728
    minSize: Qt.vector2d(424, 300)

    HifiConstants { id: hifi }

    property var scripts: ScriptDiscoveryService;
    property var scriptsModel: scripts.scriptsModelFilter
    property var runningScriptsModel: ListModel { }
    property bool developerMenuEnabled: false
    property bool isHMD: false

    Settings {
        category: "Overlay.RunningScripts"
        property alias x: root.x
        property alias y: root.y
    }

    Component {
        id: letterBoxMessage
        Windows.Window {
            implicitWidth: 400
            implicitHeight: 300
            minSize: Qt.vector2d(424, 300)
            DesktopLetterboxMessage {
                visible: true
                headerGlyph: hifi.glyphs.lock
                headerText: "Developer Mode only"
                text: ( "In order to edit, delete or reload this script," +
                        " turn on Developer Mode by going to:" +
                        " Menu > Settings > Developer Menus")
                popupRadius: 0
                headerGlyphSize: 20
                headerTextMargin: 2
                headerGlyphMargin: -3
            }
        }
    }


    Timer {
        id: refreshTimer
        interval: 100
        repeat: false
        running: false
        onTriggered: updateRunningScripts();
    }


    Timer {
        id: checkMenu
        interval: 1000
        repeat: true
        running: false
        onTriggered: developerMenuEnabled = MenuInterface.isOptionChecked("Developer Menu");
    }

    Component {
        id: listModelBuilder
        ListModel { }
    }

    Connections {
        target: ScriptDiscoveryService
        function onScriptCountChanged() {
            runningScriptsModel = listModelBuilder.createObject(root);
            refreshTimer.restart();
        }
    }

    Component.onCompleted: {
        isHMD = HMD.active;
        updateRunningScripts();
        developerMenuEnabled = MenuInterface.isOptionChecked("Developer Menu");
        checkMenu.restart();
    }

    function updateRunningScripts() {
        function simplify(path) {
            // trim URI querystring/fragment
            path = (path+'').replace(/[#?].*$/,'');
            // normalize separators and grab last path segment (ie: just the filename)
            path = path.replace(/\\/g, '/').split('/').pop();
            // return lowercased because we want to sort mnemonically
            return path.toLowerCase();
        }
        var runningScripts = ScriptDiscoveryService.getRunning();
        runningScripts.sort(function(a,b) {
            a = simplify(a.path);
            b = simplify(b.path);
            return a < b ? -1 : a > b ? 1 : 0;
        });
        // Calling  `runningScriptsModel.clear()` here instead of creating a new object
        // triggers some kind of weird heap corruption deep inside Qt.  So instead of
        // modifying the model in place, possibly triggering behaviors in the table
        // instead we create a new `ListModel`, populate it and update the
        // existing model atomically.
        var newRunningScriptsModel = listModelBuilder.createObject(root);
        for (var i = 0; i < runningScripts.length; ++i) {
            newRunningScriptsModel.append(runningScripts[i]);
        }
        runningScriptsModel = newRunningScriptsModel;
    }

    function loadScript(script) {
        scripts.loadOneScript(script);
    }

    function reloadScript(script) {
        scripts.stopScript(script, true);
    }

    function stopScript(script) {
        scripts.stopScript(script);
    }

    function reloadAll() {
        console.log("Reload all scripts");
        if (!developerMenuEnabled) {
            for (var index = 0; index < runningScriptsModel.count; index++) {
                var url = runningScriptsModel.get(index).url;
                var fileName = url.substring(url.lastIndexOf('/')+1);
                if (canEditScript(fileName)) {
                    scripts.stopScript(url, true);
                }
            }
        } else {
            scripts.reloadAllScripts();
        }
    }

    function loadDefaults() {
        console.log("Load default scripts");
        scripts.loadOneScript(scripts.defaultScriptsPath + "/defaultScripts.js");
    }

    function stopAll() {
        console.log("Stop all scripts");
        for (var index = 0; index < runningScriptsModel.count; index++) {
            var url = runningScriptsModel.get(index).url;
            var fileName = url.substring(url.lastIndexOf('/')+1);
            if (canEditScript(fileName)) {
                scripts.stopScript(url);
            }
        }
    }


    function canEditScript(script) {
        if ((script === "controllerScripts.js") || (script === "defaultScripts.js")) {
            return developerMenuEnabled;
        }

        return true;
    }

    Column {
        width: pane.contentWidth

        HifiControls.ContentSection {
            name: "Currently Running"
            isFirst: true

            HifiControls.VerticalSpacer {}

            Row {
                spacing: hifi.dimensions.contentSpacing.x

                HifiControls.Button {
                    text: "Reload All"
                    color: hifi.buttons.black
                    onClicked: reloadAll()
                }

                HifiControls.Button {
                    text: "Remove All"
                    color: hifi.buttons.red
                    onClicked: stopAll()
                }

                HifiControls.Button {
                    text: "Load Defaults"
                    color: hifi.buttons.black
                    onClicked: loadDefaults()
                }
            }

            HifiControls.VerticalSpacer {
                height: hifi.dimensions.controlInterlineHeight + 2  // Add space for border
            }

            HifiControls.Table {
                model: runningScriptsModel
                id: table
                height: 185
                colorScheme: hifi.colorSchemes.dark
                anchors.left: parent.left
                anchors.right: parent.right
                expandSelectedRow: true

                itemDelegate: Item {
                    property bool canEdit: canEditScript(styleData.value);
                    anchors {
                        left: parent ? parent.left : undefined
                        leftMargin: hifi.dimensions.tablePadding
                        right: parent ? parent.right : undefined
                        rightMargin: hifi.dimensions.tablePadding
                    }

                    FiraSansSemiBold {
                        id: textItem
                        text: styleData.value
                        size: hifi.fontSizes.tableText
                        color: table.colorScheme == hifi.colorSchemes.light
                                   ? (styleData.selected ? hifi.colors.black : hifi.colors.baseGrayHighlight)
                                   : (styleData.selected ? hifi.colors.black : hifi.colors.lightGrayText)
                        anchors {
                            left: parent.left
                            right: parent.right
                            top: parent.top
                            topMargin: 3
                        }

                        HiFiGlyphs {
                            id: reloadButton
                            text: ((canEditScript(styleData.value)) ? hifi.glyphs.reload : hifi.glyphs.lock)
                            color: reloadButtonArea.pressed ? hifi.colors.white : parent.color
                            size: 21
                            anchors {
                                top: parent.top
                                right: stopButton.left
                                verticalCenter: parent.verticalCenter
                            }
                            MouseArea {
                                id: reloadButtonArea
                                anchors { fill: parent; margins: -2 }
                                onClicked: {
                                    if (canEdit) {
                                        reloadScript(model.url)
                                    } else {
                                        letterBoxMessage.createObject(desktop)
                                    }
                                }
                            }
                        }

                        HiFiGlyphs {
                            id: stopButton
                            text: hifi.glyphs.closeSmall
                            color: stopButtonArea.pressed ? hifi.colors.white : parent.color
                            visible: canEditScript(styleData.value)
                            anchors {
                                top: parent.top
                                right: parent.right
                                verticalCenter: parent.verticalCenter
                            }
                            MouseArea {
                                id: stopButtonArea
                                anchors { fill: parent; margins: -2 }
                                onClicked: {
                                    if (canEdit) {
                                        stopScript(model.url);
                                    }
                                }
                            }
                        }

                    }

                    FiraSansSemiBold {
                        text: runningScriptsModel.get(styleData.row) ? runningScriptsModel.get(styleData.row).url : ""
                        elide: Text.ElideMiddle
                        size: hifi.fontSizes.tableText
                        color: table.colorScheme == hifi.colorSchemes.light
                                   ? (styleData.selected ? hifi.colors.black : hifi.colors.lightGray)
                                   : (styleData.selected ? hifi.colors.black : hifi.colors.lightGrayText)
                        anchors {
                            top: textItem.bottom
                            left: parent.left
                            right: parent.right
                        }
                        visible: styleData.selected
                    }
                }

                TableViewColumn {
                    role: "name"
                }
            }

            HifiControls.VerticalSpacer {
                height: hifi.dimensions.controlInterlineHeight + 2  // Add space for border
            }
        }

        HifiControls.ContentSection {
            name: "Load Scripts"

            HifiControls.VerticalSpacer {}

            Row {
                spacing: hifi.dimensions.contentSpacing.x

                HifiControls.QueuedButton {
                    text: "from URL"
                    color: hifi.buttons.black
                    height: 26
                    onClickedQueued: ApplicationInterface.loadScriptURLDialog()
                }

                HifiControls.QueuedButton {
                    text: "from Disk"
                    color: hifi.buttons.black
                    height: 26
                    onClickedQueued: ApplicationInterface.loadDialog()
                }
            }

            HifiControls.VerticalSpacer {}

            HifiControls.TextField {
                id: filterEdit
                isSearchField: true
                anchors.left: parent.left
                anchors.right: parent.right
                colorScheme: hifi.colorSchemes.dark
                placeholderText: "Filter"
                onTextChanged: scriptsModel.filterRegExp =  new RegExp("^.*" + text + ".*$", "i")
                Component.onCompleted: scriptsModel.filterRegExp = new RegExp("^.*$", "i")
            }

            HifiControls.VerticalSpacer {
                height: hifi.dimensions.controlInterlineHeight + 2  // Add space for border
            }

            HifiControls.Tree {
                id: treeView
                height: 155
                treeModel: scriptsModel
                colorScheme: hifi.colorSchemes.dark
                anchors.left: parent.left
                anchors.right: parent.right

                TableViewColumn {
                    role: "display";
                }

                onActivated: {
                    var path = scriptsModel.data(index, 0x100)
                    if (path) {
                        loadScript(path)
                    }
                }
            }

            HifiControls.VerticalSpacer {
                height: hifi.dimensions.controlInterlineHeight + 2  // Add space for border
            }

            HifiControls.TextField {
                id: selectedScript
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.rightMargin: loadButton.width + hifi.dimensions.contentSpacing.x

                colorScheme: hifi.colorSchemes.dark
                readOnly: true

                Connections {
                    target: treeView.selection
                    function onCurrentIndexChanged() {
                        var path = scriptsModel.data(treeView.selection.currentIndex, 0x100)
                        if (path) {
                            selectedScript.text = path
                        } else {
                            selectedScript.text = ""
                        }
                    }
                }
            }

            Item {
                // Take the loadButton out of the column flow.
                id: loadButtonContainer
                anchors.top: selectedScript.top
                anchors.right: parent.right

                HifiControls.Button {
                    id: loadButton
                    anchors.right: parent.right

                    text: "Load"
                    color: hifi.buttons.blue
                    enabled: selectedScript.text != ""
                    onClicked: root.loadScript(selectedScript.text)
                }
            }

            HifiControls.VerticalSpacer {
                height: hifi.dimensions.controlInterlineHeight - (!isHMD ? 3 : 0)
            }

            HifiControls.TextAction {
                id: directoryButton
                icon: hifi.glyphs.script
                iconSize: 24
                text: "Reveal Scripts Folder"
                onClicked: fileDialogHelper.openDirectory(scripts.defaultScriptsPath)
                colorScheme: hifi.colorSchemes.dark
                anchors.left: parent.left
                visible: !isHMD
            }

            HifiControls.VerticalSpacer {
                height: hifi.dimensions.controlInterlineHeight - 3
                visible: !isHMD
            }
        }
    }
}
