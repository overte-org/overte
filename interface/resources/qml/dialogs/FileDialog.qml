//
//  FileDialog.qml
//
//  Created by Bradley Austin Davis on 14 Jan 2016
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtCore
import QtQuick
import Qt.labs.folderlistmodel
import Qt.labs.qmlmodels
import QtQuick.Dialogs as OriginalDialogs
import QtQuick.Controls

import ".."
import controlsUit 1.0
import stylesUit 1.0
import "../windows"

import "fileDialog"

//FIXME implement shortcuts for favorite location
ModalWindow {
    id: root
    resizable: true
    implicitWidth: 480
    implicitHeight: 360 + (fileDialogItem.keyboardEnabled && fileDialogItem.keyboardRaised ? keyboard.raisedHeight + hifi.dimensions.contentSpacing.y : 0)

    minSize: Qt.vector2d(360, 240)
    draggable: true

    HifiConstants { id: hifi }

    property var filesModel: TableModel { }

    Settings {
        category: "FileDialog"
        property alias width: root.width
        property alias height: root.height
        property alias x: root.x
        property alias y: root.y
    }


    // Set from OffscreenUi::getOpenFile()
    property alias caption: root.title;
    // Set from OffscreenUi::getOpenFile()
    property alias dir: fileTableModel.folder;
    // Set from OffscreenUi::getOpenFile()
    property alias filter: selectionType.filtersString;
    // Set from OffscreenUi::getOpenFile()
    property int options; // <-- FIXME unused

    property string iconText: root.title !== "" ? hifi.glyphs.scriptUpload : ""
    property int iconSize: 40

    property bool selectDirectory: false;
    property bool showHidden: true;
    // FIXME implement
    property bool multiSelect: false;
    property bool saveDialog: false;
    property var helper: fileDialogHelper
    property alias model: fileTableView.model
    property var drives: helper.drives()

    property int titleWidth: 0

    signal selectedFile(var file);
    signal canceled();
    signal selected(int button);
    function click(button) {
        clickedButton = button;
        selected(button);
        destroy();
    }

    property int clickedButton: OriginalDialogs.MessageDialog.NoButton;
	
    Component.onCompleted: {
        fileDialogItem.keyboardEnabled = HMD.active;

        // HACK: The following lines force the model to initialize properly such that the go-up button
        // works properly from the initial screen.
        var initialFolder = folderListModel.folder;
        fileTableModel.folder = helper.pathToUrl(drives[0]);
        fileTableModel.folder = initialFolder;

        iconText = root.title !== "" ? hifi.glyphs.scriptUpload : "";

        // Clear selection when click on external frame.
        frameClicked.connect(function() { d.clearSelection(); });

        if (selectDirectory) {
            currentSelection.text = d.capitalizeDrive(helper.urlToPath(initialFolder));
            d.currentSelectionIsFolder = true;
            d.currentSelectionUrl = initialFolder;
        }

        helper.contentsChanged.connect(function() {
            if (folderListModel) {
                // Make folderListModel refresh.
                var save = folderListModel.folder;
                folderListModel.folder = "";
                folderListModel.folder = save;
            }
        });

        focusTimer.start();
    }
    
    Timer {
        id: focusTimer
        interval: 10
        running: false
        repeat: false
        onTriggered: {
            fileTableView.contentItem.forceActiveFocus();
        }
    }

    Item {
        id: fileDialogItem
        clip: true
        width: pane.width
        height: pane.height
        anchors.margins: 0

        property bool keyboardEnabled: false
        property bool keyboardRaised: false
        property bool punctuationMode: false

        MouseArea {
            // Clear selection when click on internal unused area.
            anchors.fill: parent
            drag.target: root
            onClicked: {
                d.clearSelection();
                // Defocus text field so that the keyboard gets hidden.
                // Clicking also breaks keyboard navigation apart from backtabbing to cancel 
                frame.forceActiveFocus();
            }
        }

        Row {
            id: navControls
            anchors {
                top: parent.top
                topMargin: hifi.dimensions.contentMargin.y
                left: parent.left
            }
            spacing: hifi.dimensions.contentSpacing.x

            GlyphButton {
                id: upButton
                glyph: hifi.glyphs.levelUp
                width: height
                size: 30
                enabled: fileTableModel.parentFolder && fileTableModel.parentFolder !== ""
                onClicked: d.navigateUp();
                Keys.onReturnPressed: { d.navigateUp(); }
                KeyNavigation.tab: homeButton
                KeyNavigation.backtab: upButton
                KeyNavigation.left: upButton
                KeyNavigation.right: homeButton
            }

            GlyphButton {
                id: homeButton
                property var destination: helper.home();
                glyph: hifi.glyphs.home
                size: 28
                width: height
                enabled: d.homeDestination ? true : false
                onClicked: d.navigateHome();
                Keys.onReturnPressed: { d.navigateHome(); }
                KeyNavigation.tab: fileTableView.contentItem
                KeyNavigation.backtab: upButton
                KeyNavigation.left: upButton
            }
        }

        ComboBox {
            id: pathSelector
            anchors {
                top: parent.top
                topMargin: hifi.dimensions.contentMargin.y
                left: navControls.right
                leftMargin: hifi.dimensions.contentSpacing.x
                right: parent.right
            }

            property var lastValidFolder: helper.urlToPath(fileTableModel.folder)

            function calculatePathChoices(folder) {
                var folders = folder.split("/"),
                    choices = [],
                    i, length;

                if (folders[folders.length - 1] === "") {
                    folders.pop();
                }

                choices.push(folders[0]);

                for (i = 1, length = folders.length; i < length; i++) {
                    choices.push(choices[i - 1] + "/" + folders[i]);
                }

                if (folders[0] === "") {
                    // Special handling for OSX root dir.
                    choices[0] = "/";
                }

                choices.reverse();

                if (drives && drives.length > 1) {
                    choices.push("This PC");
                }

                if (choices.length > 0) {
                    pathSelector.model = choices;
                }
            }

            onLastValidFolderChanged: {
                var folder = d.capitalizeDrive(lastValidFolder);
                calculatePathChoices(folder);
            }

            onCurrentTextChanged: {
                var folder = currentText;

                if (/^[a-zA-z]:$/.test(folder)) {
                    folder = "file:///" + folder + "/";
                } else if (folder === "This PC") {
                    folder = "file:///";
                } else {
                    folder = helper.pathToUrl(folder);
                }

                if (helper.urlToPath(folder).toLowerCase() !== helper.urlToPath(fileTableModel.folder).toLowerCase()) {
                    if (root.selectDirectory) {
                        currentSelection.text = currentText !== "This PC" ? currentText : "";
                        d.currentSelectionUrl = helper.pathToUrl(currentText);
                    }
                    fileTableModel.folder = folder;
                }
            }

            KeyNavigation.up: fileTableView.contentItem
            KeyNavigation.down: fileTableView.contentItem
            KeyNavigation.tab: fileTableView.contentItem
            KeyNavigation.backtab: fileTableView.contentItem
            KeyNavigation.left: fileTableView.contentItem
            KeyNavigation.right: fileTableView.contentItem
        }

        QtObject {
            id: d
            property var currentSelectionUrl;
            readonly property string currentSelectionPath: currentSelectionUrl === undefined ? "" : helper.urlToPath(currentSelectionUrl);
            property bool currentSelectionIsFolder;
            property var backStack: []
            property var tableViewConnection: Connections { target: fileTableView; function onCurrentRowChanged() { d.update(); } }
            property var modelConnection: Connections { target: fileTableModel; function onFolderChanged() { d.update(); } }
            property var homeDestination: helper.home();

            function capitalizeDrive(path) {
                // Consistently capitalize drive letter for Windows.
                if (/[a-zA-Z]:/.test(path)) {
                    return path.charAt(0).toUpperCase() + path.slice(1);
                }
                return path;
            }

            function update() {
                var row = fileTableView.currentRow;

                if (row === -1) {
                    if (!root.selectDirectory) {
                        currentSelection.text = "";
                        currentSelectionIsFolder = false;
                    }
                    return;
                }

                currentSelectionUrl = helper.pathToUrl(fileTableView.model.get(row).filePath);
                currentSelectionIsFolder = fileTableView.model !== filesModel ?
                            fileTableView.model.isFolder(row) :
                            fileTableModel.isFolder(row);
                if (root.selectDirectory || !currentSelectionIsFolder) {
                    currentSelection.text = capitalizeDrive(helper.urlToPath(currentSelectionUrl));
                } else {
                    currentSelection.text = "";
                }
            }

            function navigateUp() {
                if (fileTableModel.parentFolder && fileTableModel.parentFolder !== "") {
                    fileTableModel.folder = fileTableModel.parentFolder;
                    return true;
                }
            }

            function navigateHome() {
                fileTableModel.folder = homeDestination;
                return true;
            }

            function clearSelection() {
                // QT6TODO
                //fileTableView.selection.clear();
                //fileTableView.currentRow = -1;
                update();
            }
        }

        FolderListModel {
            id: folderListModel
            nameFilters: selectionType.currentFilter
            caseSensitive: false
            showDirsFirst: true
            showDotAndDotDot: false
            showFiles: !root.selectDirectory
            showHidden: root.showHidden
            Component.onCompleted: {
                showFiles = !root.selectDirectory
                showHidden = root.showHidden
            }

            onFolderChanged: {
                d.clearSelection();
                fileTableModel.update();  // Update once the data from the folder change is available.
            }

            function getItem(index, field) {
                return get(index, field);
            }
        }

        ListModel {
            // Emulates FolderListModel but contains drive data.
            id: driveListModel

            property int count: 1

            Component.onCompleted: initialize();

            function initialize() {
                var drive,
                    i;

                count = drives.length;

                for (i = 0; i < count; i++) {
                    drive = drives[i].slice(0, -1);  // Remove trailing "/".
                    append({
                       fileName: drive,
                       fileModified: new Date(0),
                       fileSize: 0,
                       filePath: drive + "/",
                       fileIsDir: true,
                       fileNameSort: drive.toLowerCase()
                    });
                }
            }

            function getItem(index, field) {
                return get(index)[field];
            }
        }

        Component {
            id: filesModelBuilder
            TableModel {
                TableModelColumn {
                    //id: fileNameColumn
                    display: "fileName"
                    //title: "Name"
                    //width: (selectDirectory ? 1.0 : 0.5) * fileTableView.width
                }
                TableModelColumn {
                    //id: fileModifiedColumn
                    display: "fileModified"
                    //title: "Date"
                    //width: 0.3 * fileTableView.width
                }
                TableModelColumn {
                    display: "fileSize"
                    //title: "Size"
                    //width: fileTableView.width - fileNameColumn.width - fileModifiedColumn.width
                }
            }
        }

        QtObject {
            id: fileTableModel

            // FolderListModel has a couple of problems:
            // 1) Files and directories sort case-sensitively: https://bugreports.qt.io/browse/QTBUG-48757
            // 2) Cannot browse up to the "computer" level to view Windows drives: https://bugreports.qt.io/browse/QTBUG-42901
            //
            // To solve these problems an intermediary ListModel is used that implements proper sorting and can be populated with
            // drive information when viewing at the computer level.

            property var folder
            property int sortOrder: Qt.AscendingOrder
            property int sortColumn: 0
            property var model: folderListModel
            property string parentFolder: calculateParentFolder();

            readonly property string rootFolder: "file:///"

            function calculateParentFolder() {
                if (model === folderListModel) {
                    if (folderListModel.parentFolder.toString() === "" && driveListModel.count > 1) {
                        return rootFolder;
                    } else {
                        return folderListModel.parentFolder;
                    }
                } else {
                    return "";
                }
            }

            onFolderChanged: {
                if (folder === rootFolder) {
                    model = driveListModel;
                    helper.monitorDirectory("");
                    update();
                } else {
                    var needsUpdate = model === driveListModel && folder === folderListModel.folder;

                    model = folderListModel;
                    folderListModel.folder = folder;
                    helper.monitorDirectory(helper.urlToPath(folder));

                    if (needsUpdate) {
                        update();
                    }
                }
            }

            function isFolder(row) {
                if (row === -1) {
                    return false;
                }
                return filesModel.getRow(row).fileIsDir;
            }

            function get(row) {
                return filesModel.getRow(row)
            }

            function update() {
                var dataFields = ["fileName", "fileModified", "fileSize"],
                    sortFields = ["fileNameSort", "fileModified", "fileSize"],
                    dataField = dataFields[sortColumn],
                    sortField = sortFields[sortColumn],
                    sortValue,
                    fileName,
                    fileIsDir,
                    comparisonFunction,
                    lower,
                    middle,
                    upper,
                    rows = 0,
                    i;

                filesModel = filesModelBuilder.createObject(root);

                comparisonFunction = sortOrder === Qt.AscendingOrder
                    ? function(a, b) { return a < b; }
                    : function(a, b) { return a > b; }

                for (i = 0; i < model.count; i++) {
                    fileName = model.getItem(i, "fileName");
                    fileIsDir = model.getItem(i, "fileIsDir");

                    sortValue = model.getItem(i, dataField);
                    if (dataField === "fileName") {
                        // Directories first by prefixing a "*".
                        // Case-insensitive.
                        sortValue = (fileIsDir ? "*" : "") + sortValue.toLowerCase();
                    }

                    lower = 0;
                    upper = rows;
                    while (lower < upper) {
                        middle = Math.floor((lower + upper) / 2);
                        var lessThan;
                        if (comparisonFunction(sortValue, filesModel.getRow(middle)[sortField])) {
                            lessThan = true;
                            upper = middle;
                        } else {
                            lessThan = false;
                            lower = middle + 1;
                        }
                    }

                    filesModel.insertRow(lower, {
                       fileName: fileName,
                       fileModified: (fileIsDir ? new Date(0) : model.getItem(i, "fileModified")),
                       fileSize: model.getItem(i, "fileSize"),
                       filePath: model.getItem(i, "filePath"),
                       fileIsDir: fileIsDir,
                       fileNameSort: (fileIsDir ? "*" : "") + fileName.toLowerCase()
                    });

                    rows++;
                }
            }
        }

        TableView {
            id: fileTableView
            // QT6TODO:
            //colorScheme: hifi.colorSchemes.light
            anchors {
                top: navControls.bottom
                topMargin: hifi.dimensions.contentSpacing.y
                left: parent.left
                right: parent.right
                bottom: currentSelection.top
                bottomMargin: hifi.dimensions.contentSpacing.y + currentSelection.controlHeight - currentSelection.height
            }
            // QT6TODO:
            //headerVisible: !selectDirectory
            //onDoubleClicked: navigateToRow(row);
            Keys.onReturnPressed: navigateToCurrentRow();
            Keys.onEnterPressed: navigateToCurrentRow();

            property int sortIndicatorColumn: 0
            property var sortIndicatorOrder: Qt.AscendingOrder
            property bool sortIndicatorVisible: true

            // QT6TODO
            model: filesModel

            function updateSort() {
                fileTableModel.sortOrder = sortIndicatorOrder;
                fileTableModel.sortColumn = sortIndicatorColumn;
                fileTableModel.update();
            }

            onSortIndicatorColumnChanged: { updateSort(); }

            onSortIndicatorOrderChanged: { updateSort(); }

            delegate: TextInput {
                text: model.display
                padding: 12
                selectByMouse: true

                onAccepted: model.display = text

                Rectangle {
                    anchors.fill: parent
                    color: "#efefef"
                    z: -1
                }
            }
            /*delegate: Item {
                clip: true

                FiraSansSemiBold {
                    text: getText();
                    elide: styleData.elideMode
                    anchors {
                        left: parent.left
                        leftMargin: hifi.dimensions.tablePadding
                        right: parent.right
                        rightMargin: hifi.dimensions.tablePadding
                        verticalCenter: parent.verticalCenter
                    }
                    size: hifi.fontSizes.tableText
                    color: hifi.colors.baseGrayHighlight
                    font.family: (styleData.row !== -1 && fileTableView.model.get(styleData.row).fileIsDir)
                        ? "Fira Sans SemiBold" : "Fira Sans"

                    function getText() {
                        if (styleData.row === -1) {
                            return styleData.value;
                        }

                        switch (styleData.column) {
                            case 1: return fileTableView.model.get(styleData.row).fileIsDir ? "" : styleData.value;
                            case 2: return fileTableView.model.get(styleData.row).fileIsDir ? "" : formatSize(styleData.value);
                            default: return styleData.value;
                        }
                    }
                    function formatSize(size) {
                        var suffixes = [ "bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" ];
                        var suffixIndex = 0
                        while ((size / 1024.0) > 1.1) {
                            size /= 1024.0;
                            ++suffixIndex;
                        }

                        size = Math.round(size*1000)/1000;
                        size = size.toLocaleString()

                        return size + " " + suffixes[suffixIndex];
                    }
                }
            }*/

            /*model: TableModel {
                TableModelColumn {
                    id: fileNameColumn
                    role: "fileName"
                    title: "Name"
                    width: (selectDirectory ? 1.0 : 0.5) * fileTableView.width
                    movable: false
                    resizable: true
                }
                TableModelColumn {
                    id: fileModifiedColumn
                    role: "fileModified"
                    title: "Date"
                    width: 0.3 * fileTableView.width
                    movable: false
                    resizable: true
                    visible: !selectDirectory
                }
                TableModelColumn {
                    role: "fileSize"
                    title: "Size"
                    width: fileTableView.width - fileNameColumn.width - fileModifiedColumn.width
                    movable: false
                    resizable: true
                    visible: !selectDirectory
                }
            }*/

            function navigateToRow(row) {
                currentRow = row;
                navigateToCurrentRow();
            }

            function navigateToCurrentRow() {
                var currentModel = fileTableView.model !== filesModel ? fileTableView.model : fileTableModel
                var row = fileTableView.currentRow
                var isFolder = currentModel.isFolder(row);
                var file = currentModel.get(row).filePath;
                if (isFolder) {
                    currentModel.folder = helper.pathToUrl(file);
                } else {
                    okAction.trigger();
                }
            }

            property string prefix: ""

            function addToPrefix(event) {
                if (!event.text || event.text === "") {
                    return false;
                }
                var newPrefix = prefix + event.text.toLowerCase();
                var matchedIndex = -1;
                for (var i = 0; i < model.count; ++i) {
                    var name = model !== filesModel ? model.get(i).fileName.toLowerCase() :
                                                      filesModel.get(i).fileName.toLowerCase();
                    if (0 === name.indexOf(newPrefix)) {
                        matchedIndex = i;
                        break;
                    }
                }

                if (matchedIndex !== -1) {
                    fileTableView.selection.clear();
                    fileTableView.selection.select(matchedIndex);
                    fileTableView.currentRow = matchedIndex;
                    fileTableView.prefix = newPrefix;
                }
                prefixClearTimer.restart();
                return true;
            }

            Timer {
                id: prefixClearTimer
                interval: 1000
                repeat: false
                running: false
                onTriggered: fileTableView.prefix = "";
            }

            Keys.onPressed: event => {
                switch (event.key) {
                case Qt.Key_Backspace:
                case Qt.Key_Tab:
                case Qt.Key_Backtab:
                    event.accepted = false;
                    break;
                case Qt.Key_Escape:
                    event.accepted = true;
                    root.click(OriginalDialogs.StandardButton.Cancel);
                    break;
                default:
                    if (addToPrefix(event)) {
                        event.accepted = true
                    } else {
                        event.accepted = false;
                    }
                    break;
                }
            }

            KeyNavigation.tab: root.saveDialog ? currentSelection : openButton
        }

        TextField {
            id: currentSelection
            label: selectDirectory ? "Directory:" : "File name:"
            anchors {
                left: parent.left
                right: selectionType.visible ? selectionType.left: parent.right
                rightMargin: selectionType.visible ? hifi.dimensions.contentSpacing.x : 0
                bottom: keyboard.top
                bottomMargin: hifi.dimensions.contentSpacing.y
            }
            readOnly: !root.saveDialog
            activeFocusOnTab: !readOnly
            onActiveFocusChanged: if (activeFocus) { selectAll(); }
            onAccepted: okAction.trigger();
            KeyNavigation.up: fileTableView.contentItem
            KeyNavigation.down: openButton
            KeyNavigation.tab: openButton
            KeyNavigation.backtab: fileTableView.contentItem
        }

        FileTypeSelection {
            id: selectionType
            anchors {
                top: currentSelection.top
                left: buttonRow.left
                right: parent.right
            }
            visible: !selectDirectory && filtersCount > 1
        }

        Keyboard {
            id: keyboard
            raised: parent.keyboardEnabled && parent.keyboardRaised
            numeric: parent.punctuationMode
            anchors {
                left: parent.left
                right: parent.right
                bottom: buttonRow.top
                bottomMargin: visible ? hifi.dimensions.contentSpacing.y : 0
            }
        }

        Row {
            id: buttonRow
            anchors {
                right: parent.right
                bottom: parent.bottom
            }
            spacing: hifi.dimensions.contentSpacing.y

            Button {
                id: openButton
                color: hifi.buttons.blue
                action: okAction
                Keys.onReturnPressed: okAction.trigger()
                KeyNavigation.right: cancelButton
                KeyNavigation.up: root.saveDialog ? currentSelection : fileTableView.contentItem
                KeyNavigation.tab: cancelButton
            }

            Button {
                id: cancelButton
                action: cancelAction
                Keys.onReturnPressed: { cancelAction.trigger() }
                KeyNavigation.left: openButton
                KeyNavigation.up: root.saveDialog ? currentSelection : fileTableView.contentItem
                KeyNavigation.backtab: openButton
            }
        }

        Action {
            id: okAction
            text: currentSelection.text ? (root.selectDirectory && fileTableView.currentRow === -1 ? "Choose" : (root.saveDialog ? "Save" : "Open")) : "Open"
            enabled: currentSelection.text || !root.selectDirectory && d.currentSelectionIsFolder ? true : false
            onTriggered: {
                if (!root.selectDirectory && !d.currentSelectionIsFolder
                        || root.selectDirectory && fileTableView.currentRow === -1) {
                    okActionTimer.start();
                } else {
                    fileTableView.navigateToCurrentRow();
                }
            }
        }

        Timer {
            id: okActionTimer
            interval: 50
            running: false
            repeat: false
            onTriggered: {
                if (!root.saveDialog) {
                    selectedFile(d.currentSelectionUrl);
                    root.destroy()
                    return;
                }

                // Handle the ambiguity between different cases
                // * typed name (with or without extension)
                // * full path vs relative vs filename only
                var selection = helper.saveHelper(currentSelection.text, root.dir, selectionType.currentFilter);

                if (!selection) {
                    desktop.messageBox({ icon: OriginalDialogs.StandardIcon.Warning, text: "Unable to parse selection" })
                    return;
                }

                if (helper.urlIsDir(selection)) {
                    root.dir = selection;
                    currentSelection.text = "";
                    return;
                }

                // Check if the file is a valid target
                if (!helper.urlIsWritable(selection)) {
                    desktop.messageBox({
                                           icon: OriginalDialogs.StandardIcon.Warning,
                                           text: "Unable to write to location " + selection
                                       })
                    return;
                }

                if (helper.urlExists(selection)) {
                    var messageBox = desktop.messageBox({
                                                            icon: OriginalDialogs.StandardIcon.Question,
                                                            buttons: OriginalDialogs.StandardButton.Yes | OriginalDialogs.StandardButton.No,
                                                            text: "Do you wish to overwrite " + selection + "?",
                                                        });
                    var result = messageBox.exec();
                    if (OriginalDialogs.StandardButton.Yes !== result) {
                        return;
                    }
                }

                selectedFile(selection);
                root.destroy();
            }
        }

        Action {
            id: cancelAction
            text: "Cancel"
            onTriggered: { canceled(); root.destroy(); }
        }
    }

    Keys.onPressed: event => {
        switch (event.key) {
        case Qt.Key_Backspace:
            event.accepted = d.navigateUp();
            break;

        case Qt.Key_Home:
            event.accepted = d.navigateHome();
            break;
		
        case Qt.Key_Escape:
            event.accepted = true;
            root.click(OriginalDialogs.StandardButton.Cancel);
            break;
		}
    }
}
