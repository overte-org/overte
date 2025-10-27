import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs as QtDialogs
import Qt.labs.folderlistmodel
import Qt.labs.qmlmodels

import ".."
import "."

Rectangle {
    enum FileMode {
        OpenFile,
        OpenFiles,
        SaveFile,
        OpenFolder
    }

    id: fileDialog
    color: Theme.paletteActive.base
    visible: false

    signal accepted
    signal rejected

    property url currentFolder: StandardPaths.writableLocation(StandardPaths.HomeLocation)
    property list<string> nameFilters: ["*"]
    property int fileMode: FileDialog.OpenFile

    property url selectedFile: ""
    property list<url> selectedFiles: []

    // TODO, FIXME
    property list<url> history: []
    property int historyIndex: 0

    property string searchExpression: ".*"

    onAccepted: {
        visible = false;
    }

    onRejected: {
        visible = false;
    }

    function open() {
        visible = true;
    }

    function urlToPathString(urlRaw) {
        const url = new URL(urlRaw);

        if (url.protocol !== "file:") {
            return url;
        }

        return url.pathname;
    }

    function directoryRowActivated(currentData) {
        const parentDir = new URL(currentFolder).pathname.match(/^\/?(.*)/)[1];
        const path = `file:///${parentDir}${parentDir ? "/" : ""}${currentData.fileName}`;

        if (currentData.fileIsDir) {
            currentFolder = path;
            tableView.selectionModel.clear();
        } else {
            selectedFile = path;
            selectedFiles = [];

            let possibleConflict = false;

            for (const index of tableView.selectionModel.selectedRows(0)) {
                const rowData = tableView.model.getRow(index.row);
                selectedFiles.push(`${currentFolder.toString()}/${rowData.fileName}`);

                if (rowData.fileName === saveName.text) {
                    possibleConflict = true;
                }
            }

            if (fileMode === FileDialog.FileMode.SaveFile && possibleConflict) {
                replaceWarningDialog.text = qsTr("Are you sure you want to replace %1?").arg(saveName.text);
                replaceWarningDialog.open();
            } else {
                fileDialog.accepted();
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 8
            Layout.rightMargin: 8
            Layout.topMargin: 8

            RoundButton {
                icon.source: "../icons/triangle_left.svg"
                icon.width: 24
                icon.height: 24
                icon.color: Theme.paletteActive.buttonText

                enabled: historyIndex > 0

                // TODO
                onClicked: {}
            }
            RoundButton {
                icon.source: "../icons/triangle_right.svg"
                icon.width: 24
                icon.height: 24
                icon.color: Theme.paletteActive.buttonText

                enabled: historyIndex < history.length - 1

                // TODO
                onClicked: {}
            }

            RoundButton {
                icon.source: "../icons/arrow_up.svg"
                icon.width: 24
                icon.height: 24
                icon.color: Theme.paletteActive.buttonText

                enabled: folderModel.parentFolder.toString() !== ""

                onClicked: {
                    currentFolder = folderModel.parentFolder;
                }
            }

            TextField {
                Layout.fillWidth: true
                Layout.preferredHeight: parent.height

                text: urlToPathString(fileDialog.currentFolder)
                visible: pathEditToggle.checked

                Keys.onEnterPressed: {
                    fileDialog.currentFolder = `file:///${text.match(/^\/?(.*)/)[1]}`;
                }
                Keys.onReturnPressed: {
                    fileDialog.currentFolder = `file:///${text.match(/^\/?(.*)/)[1]}`;
                }
            }

            ListView {
                Layout.fillWidth: true
                Layout.preferredHeight: parent.height

                id: breadcrumbBar
                visible: !pathEditToggle.checked
                clip: true
                pixelAligned: true
                boundsBehavior: Flickable.StopAtBounds
                orientation: Qt.Horizontal

                onCountChanged: {
                    positionViewAtEnd();
                }

                model: {
                    let breadcrumbs = [];

                    let parts = new URL(currentFolder).pathname.split("/").filter(Boolean);

                    if (SystemInformation.kernelType !== "winnt") {
                        parts.unshift("");
                    }

                    for (let i = 0; i < parts.length; i++) {
                        let targetUrl = parts.slice(1, i + 1).join("/");
                        if (targetUrl === "") {
                            targetUrl = "file:///";
                        } else {
                            targetUrl = "file:///" + targetUrl;
                        }

                        breadcrumbs.push({
                            label: parts[i] === "" ? "/" : parts[i],
                            targetUrl: targetUrl,
                        });
                    }

                    return breadcrumbs;
                }

                delegate: Button {
                    required property string label
                    required property url targetUrl

                    height: breadcrumbBar.height
                    text: label

                    onClicked: {
                        currentFolder = targetUrl;
                    }
                }
            }

            RoundButton {
                checkable: true
                icon.source: "../icons/pencil.svg"
                icon.color: Theme.paletteActive.buttonText
                icon.width: 24
                icon.height: 24

                id: pathEditToggle
            }
        }

        RowLayout {
            Layout.fillWidth: true

            TextField {
                Layout.fillWidth: true
                Layout.leftMargin: 8
                Layout.preferredHeight: parent.height

                id: searchField
                placeholderText: qsTr("Search…")

                Keys.onEnterPressed: searchButton.click()
                Keys.onReturnPressed: searchButton.click()
            }

            RoundButton {
                Layout.rightMargin: 8

                id: searchButton
                icon.source: "../icons/search.svg"
                icon.width: 24
                icon.height: 24
                icon.color: Theme.paletteActive.buttonText

                onClicked: {
                    searchExpression = searchField.text === "" ? ".*" : searchField.text;

                    // force a refresh
                    folderModel.folder = "";
                    folderModel.folder = fileDialog.currentFolder;
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: folderModel.status === FolderListModel.Null

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 8

                Label {
                    Layout.fillWidth: true

                    text: qsTr("Invalid or unreachable folder")
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.Wrap
                }

                Button {
                    Layout.alignment: Qt.AlignHCenter

                    text: qsTr("Go Home")
                    onClicked: {
                        currentFolder = StandardPaths.writableLocation(StandardPaths.HomeLocation);
                    }
                }
            }
        }

        TableView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: folderModel.status !== FolderListModel.Null

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AlwaysOn
                interactive: true
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
            }

            rightMargin: ScrollBar.vertical.width

            id: tableView
            clip: true
            selectionBehavior: TableView.SelectRows
            selectionMode: (
                fileDialog.fileMode === FileDialog.FileMode.OpenFiles ?
                TableView.ExtendedSelection :
                TableView.SingleSelection
            )
            pixelAligned: true
            rowSpacing: 1
            columnSpacing: 0
            boundsBehavior: Flickable.StopAtBounds

            model: TableModel {
                TableModelColumn {
                    display: "fileName"
                    textAlignment: () => Text.AlignLeft
                }
                TableModelColumn {
                    display: "fileModified"
                    textAlignment: () => Text.AlignRight
                }
                TableModelColumn {
                    display: "fileSize"
                    textAlignment: () => Text.AlignLeft
                }
            }

            FolderListModel {
                id: folderModel
                nameFilters: fileDialog.nameFilters
                folder: fileDialog.currentFolder
                showDirsFirst: true
                showFiles: fileDialog.fileMode !== FileDialog.OpenFolder
                showOnlyReadable: true
                caseSensitive: false
                sortCaseSensitive: false

                // TableModel can't have a ListModel for its rows,
                // so we have to turn it into a JS array
                onStatusChanged: {
                    if (folderModel.status === FolderListModel.Ready) {
                        if (folder === "") { return; }

                        let data = [];

                        for (let i = 0; i < folderModel.count; i++) {
                            let datum = {
                                fileName: folderModel.get(i, "fileName"),
                                fileModified: folderModel.get(i, "fileModified").toLocaleString(null, Locale.ShortFormat),
                                fileSize: folderModel.get(i, "fileSize"),
                                fileIsDir: folderModel.get(i, "fileIsDir"),
                            };

                            if (!datum.fileName.match(new RegExp(searchExpression, "i"))) {
                                continue;
                            }

                            if (datum.fileSize > 1024 * 1024 * 1024) {
                                let value = datum.fileSize / (1024 * 1024 * 1024);
                                value = Math.round(value * 100) / 100;
                                datum.fileSize = `${value} GiB`;
                            } else if (datum.fileSize > 1024 * 1024) {
                                let value = datum.fileSize / (1024 * 1024);
                                value = Math.round(value * 100) / 100;
                                datum.fileSize = `${value} MiB`;
                            } else if (datum.fileSize > 1024) {
                                let value = datum.fileSize / 1024;
                                value = Math.round(value * 100) / 100;
                                datum.fileSize = `${value} KiB`;
                            } else {
                                datum.fileSize = qsTr("%n byte(s)", "", datum.fileSize);
                            }

                            if (datum.fileIsDir) {
                                datum.fileSize = qsTr("Folder");
                            }

                            data.push(datum);
                        }

                        tableView.model.rows = data;
                    } else {
                        tableView.model.rows = [];
                    }

                    tableView.positionViewAtRow(0, TableView.AlignVCenter, Qt.point(0, 0), Qt.rect(0, 0, 0, 0));
                }
            }

            selectionModel: ItemSelectionModel {
                onCurrentChanged: (index, _) => {
                    if (index.row === -1) { return; }

                    const data = tableView.model.getRow(index.row);
                    selectedFile = `${currentFolder.toString()}/${data.fileName}`;
                }
            }

            delegate: Rectangle {
                required property bool selected
                required property string display
                required property int textAlignment

                color: (
                    selected ?
                    Theme.paletteActive.highlight :
                    (row % 2 !== 0) ? Theme.paletteActive.alternateBase : Theme.paletteActive.base
                )

                id: cell
                implicitHeight: {
                    // hide the mtime column if the window isn't big enough to fit it comfortably
                    if (column === 1 && tableView.width < 720) {
                        return 0;
                    } else {
                        return text.implicitHeight * 2
                    }
                }
                implicitWidth: {
                    let nameWidth = tableView.width;
                    let mtimeWidth = tableView.width * (1 / 4);
                    let sizeWidth = tableView.width * (1 / 4);

                    // qt doesn't let us do stretchy columns so emulate it ourselves
                    nameWidth -= sizeWidth;
                    nameWidth -= mtimeWidth;

                    // hide the mtime column if the window isn't big enough to fit it comfortably
                    if (tableView.width < 720) {
                        // can't be zero or qt complains
                        nameWidth += mtimeWidth;
                        mtimeWidth = 1;
                    }

                    // how come there's one extra pixel?
                    nameWidth -= tableView.rightMargin + 1;

                    switch (column) {
                        case 0: return nameWidth;
                        case 1: return mtimeWidth;
                        case 2: return sizeWidth;
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    propagateComposedEvents: true

                    onPressed: mouse => {
                        parent.forceActiveFocus();

                        const index = tableView.model.index(row, column);

                        if (
                            fileMode === FileDialog.FileMode.OpenFiles &&
                            (mouse.modifiers & Qt.ControlModifier) === Qt.ControlModifier
                        ) {
                            tableView.selectionModel.select(
                                index,
                                ItemSelectionModel.Toggle | ItemSelectionModel.Rows
                            );
                        } else if (
                            fileMode === FileDialog.FileMode.OpenFiles &&
                            (mouse.modifiers & Qt.ShiftModifier) === Qt.ShiftModifier
                        ) {
                            const prevRow = tableView.selectionModel.currentIndex.row;
                            const currentRow = row;
                            const start = prevRow < currentRow ? prevRow : currentRow;
                            const end = prevRow < currentRow ? currentRow : prevRow;

                            // select everything between the previous "current" row and the next "current" row
                            for (let i = start; i <= end; i++) {
                                tableView.selectionModel.select(
                                    tableView.model.index(i, 0),
                                    ItemSelectionModel.Select | ItemSelectionModel.Rows
                                );
                            }
                        } else {
                            tableView.selectionModel.select(
                                index,
                                ItemSelectionModel.ClearAndSelect | ItemSelectionModel.Rows
                            );
                        }

                        tableView.selectionModel.setCurrentIndex(index, ItemSelectionModel.Current);

                        if (fileMode === FileDialog.FileMode.SaveFile) {
                            const rowData = tableView.model.getRow(index.row);
                            saveName.text = !rowData.fileIsDir ? rowData.fileName : "";
                        }
                    }

                    onDoubleClicked: {
                        directoryRowActivated(tableView.model.getRow(row));
                    }
                }

                Text {
                    id: text
                    anchors.fill: parent
                    anchors.leftMargin: 6
                    anchors.rightMargin: 6

                    visible: parent.implicitWidth > 1
                    text: cell.display
                    color: (
                        selected ?
                        Theme.paletteActive.highlightedText :
                        Theme.paletteActive.text
                    )
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontPixelSize
                    horizontalAlignment: cell.textAlignment
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.Wrap
                }
            }
        }

        RowLayout {
            visible: fileMode === FileDialog.FileMode.SaveFile

            Layout.fillWidth: true
            Layout.leftMargin: 8
            Layout.rightMargin: 8

            TextField {
                Layout.fillWidth: true

                id: saveName
                placeholderText: qsTr("Saved file name")
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 8
            Layout.rightMargin: 8
            Layout.bottomMargin: 8

            Label {
                Layout.fillWidth: true
                Layout.rightMargin: 8
                Layout.preferredHeight: parent.height

                text: nameFilters.join(", ")
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            Button {
                implicitWidth: 128
                text: qsTr("Cancel")

                Component.onCompleted: {
                    clicked.connect(fileDialog.rejected)
                }
            }

            Button {
                implicitWidth: 128
                backgroundColor: Theme.paletteActive.buttonAdd
                text: {
                    let text = fileMode === FileDialog.FileMode.SaveFile ? qsTr("Save") : qsTr("Open");

                    const currentRow = tableView.selectionModel.currentIndex.row;
                    if (currentRow !== -1) {
                        const currentData = tableView.model.getRow(currentRow);

                        if (currentData.fileIsDir) {
                            text = qsTr("Open");
                        }
                    }

                    return text;
                }

                enabled: tableView.selectionModel.hasSelection
                onClicked: {
                    const currentRow = tableView.selectionModel.currentIndex.row;
                    const currentData = tableView.model.getRow(currentRow);
                    directoryRowActivated(currentData);
                }
            }
        }
    }

    MessageDialog {
        id: replaceWarningDialog
        anchors.fill: parent
        buttons: QtDialogs.MessageDialog.Yes | QtDialogs.MessageDialog.No

        onAccepted: {
            fileDialog.accepted();
        }
    }
}
