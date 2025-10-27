import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.folderlistmodel
import Qt.labs.qmlmodels

import ".." as Overte

Rectangle {
    id: dialog
    color: Overte.Theme.paletteActive.base

    property url selectedFile: ""
    property string searchExpression: ".*"

    readonly property var spawnableRegex: /\.(gltf|glb|vrm|fbx|fst|obj|png|jpeg|jpg|webp)$/i

    // TODO: how does this work?
    readonly property var assetProxyModel: Assets.proxyModel

    function getDirectoryModel(parentIndex = undefined) {
        const DISPLAY_ROLE = 0x100;
        const PATH_ROLE = 0x103;
        const sourceModel = assetProxyModel;
        const sourceModelRootLength = sourceModel.rowCount(parentIndex);
        let tmp = [];

        for (let i = 0; i < sourceModelRootLength; i++) {
            const index = sourceModel.index(i, 0, parentIndex);

            const name = sourceModel.data(index, DISPLAY_ROLE);
            const path = sourceModel.data(index, PATH_ROLE);
            const hasChildren = sourceModel.hasChildren(index);

            if (hasChildren) {
                tmp.push({
                    name: name,
                    path: path,
                    rows: getDirectoryModel(index)
                });
            } else {
                tmp.push({
                    name: name,
                    path: path,
                });
            }
        }

        tmp.sort((a, b) => ((b.rows ? 1 : 0) - (a.rows ? 1 : 0)) || a.name.localeCompare(b.name));

        console.log(JSON.stringify(tmp));

        return tmp;
    }

    component TreeDelegate: Rectangle {
        required property TreeView treeView
        required property int depth
        required property int row
        required property bool current
        required property bool expanded
        required property bool hasChildren

        property string name: treeView.model.getRow(treeView.index(row, 0)).name

        implicitWidth: treeView.contentWidth
        implicitHeight: Overte.Theme.fontPixelSize * 2

        color: {
            if (current) {
                return Overte.Theme.paletteActive.highlight;
            } else if (row % 2 === 0) {
                return Overte.Theme.paletteActive.base;
            } else {
                return Overte.Theme.paletteActive.alternateBase;
            }
        }

        // IconImage and ColorImage are private in Qt 6.10,
        // so hijack AbstractButton's icon
        Overte.Button {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.leftMargin: Overte.Theme.fontPixelSize * depth
            id: indicator

            visible: hasChildren
            width: parent.height
            height: width

            flat: true
            horizontalPadding: 0
            verticalPadding: 0

            icon.source: expanded ? "../icons/triangle_down.svg" : "../icons/triangle_right.svg"
            icon.width: Math.min(24, width)
            icon.height: Math.min(24, width)
            icon.color: current ? Overte.Theme.paletteActive.highlightedText : Overte.Theme.paletteActive.buttonText

            onClicked: treeView.toggleExpanded(row)
        }

        Overte.Label {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.left: indicator.right
            anchors.leftMargin: 4
            verticalAlignment: Text.AlignVCenter
            color: current ? Overte.Theme.paletteActive.highlightedText : Overte.Theme.paletteActive.buttonText
            text: name
        }
    }

    ColumnLayout {
        anchors.fill: parent

        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 4

            Overte.Button {
                backgroundColor: Overte.Theme.paletteActive.buttonAdd
                text: qsTr("Upload File")

                // TODO
                onClicked: {}
            }

            Overte.TextField {
                Layout.fillWidth: true
                Layout.preferredHeight: parent.height

                id: searchField
                placeholderText: qsTr("Search…")

                Keys.onEnterPressed: searchButton.click()
                Keys.onReturnPressed: searchButton.click()
            }

            Overte.RoundButton {
                id: searchButton
                icon.source: "../icons/search.svg"
                icon.width: 24
                icon.height: 24
                icon.color: Overte.Theme.paletteActive.buttonText

                onClicked: {
                    searchExpression = searchField.text === "" ? /.*/ : new RegExp(searchField.text);
                }
            }
        }

        TreeView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: tableView
            clip: true
            selectionBehavior: TableView.SelectRows
            selectionMode: TableView.SingleSelection
            pixelAligned: true
            rowSpacing: 1
            columnSpacing: 0
            boundsBehavior: Flickable.StopAtBounds

            // QT6TODO: remove this once mouse input is working properly again,
            // this needs to be false until then or the expander arrows are entirely unusable
            interactive: false

            ScrollBar.vertical: Overte.ScrollBar {
                policy: ScrollBar.AlwaysOn
                interactive: true
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
            }
            contentWidth: width - ScrollBar.vertical.width

            model: TreeModel {
                TableModelColumn { display: "name" }

                rows: getDirectoryModel()
            }

            selectionModel: ItemSelectionModel {}
            delegate: TreeDelegate {}
        }

        GridLayout {
            rows: 2
            columns: 2
            Layout.fillWidth: true
            Layout.margins: 8

            Overte.Button {
                Layout.fillWidth: true
                Layout.preferredWidth: 1

                enabled: tableView.currentRow !== -1
                text: qsTr("Delete")
                backgroundColor: Overte.Theme.paletteActive.buttonDestructive

                // TODO
                onClicked: {
                    console.log("Delete");
                }
            }

            Overte.Button {
                Layout.fillWidth: true
                Layout.preferredWidth: 1

                enabled: tableView.currentRow !== -1
                text: qsTr("Copy Link")
                backgroundColor: Overte.Theme.paletteActive.buttonInfo

                // TODO
                onClicked: {
                    console.log("Copy Link");
                }
            }

            Overte.Button {
                Layout.fillWidth: true
                Layout.preferredWidth: 1

                enabled: tableView.currentRow !== -1
                text: qsTr("Rename")

                // TODO
                onClicked: {
                    console.log("Rename");
                }
            }

            Overte.Button {
                Layout.fillWidth: true
                Layout.preferredWidth: 1

                enabled: {
                    if (tableView.currentRow === -1) { return false; }

                    const index = tableView.model.index(tableView.currentRow, 0);
                    const data = tableView.model.data(index);
                    return !!data.match(spawnableRegex);
                }
                text: qsTr("Create Entity")
                backgroundColor: Overte.Theme.paletteActive.buttonAdd

                // TODO
                onClicked: {
                    console.log("Create Entity");
                }
            }
        }
    }

    Overte.MessageDialog {
        id: replaceWarningDialog
        anchors.fill: parent
    }
}
