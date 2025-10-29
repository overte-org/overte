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

    // FIXME: "Could not convert argument 0 from [object Object] to EntityItemProperties"
    //readonly property var spawnableRegex: /\.(gltf|glb|vrm|fbx|fst|obj|png|jpeg|jpg|webp)$/i
    readonly property var spawnableRegex: /\.(gltf|glb|vrm|fbx|fst|obj)$/i

    property var assetMappingModel: ({})

    Component.onCompleted: refreshModel()

    function refreshModel() {
        Assets.getAllMappings((error, maps) => {
            if (error === "") {
                assetMappingModel = maps;
                tableModel.rows = getDirectoryModel();
            } else {
                console.error(error);
            }
        });
    }

    // this seems suboptimal but i can't figure out how else to use the Assets models
    function getDirectoryModel() {
        let tmp = [];

        for (const [name, hash] of Object.entries(assetMappingModel)) {
            if (!name.slice(1).match(new RegExp(searchExpression, "i"))) {
                continue;
            }

            tmp.push({
                // trim off the leading slash
                name: name.slice(1),
                path: name,
                hash: hash,
            });
        }

        tmp.sort((a, b) => a.name.localeCompare(b.name));

        return tmp;
    }

    component TableDelegate: Rectangle {
        required property TableView tableView
        required property int row
        required property bool current

        property string name: tableModel.rows[row]?.name ?? "undefined"

        implicitWidth: Math.max(tableView.contentWidth, label.implicitWidth)
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

        Overte.Label {
            id: label
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.left: parent.left
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
                enabled: false
                onClicked: {}
            }

            Overte.TextField {
                Layout.fillWidth: true
                Layout.preferredHeight: parent.height

                id: searchField
                placeholderText: qsTr("Search…")

                Keys.onEnterPressed: {
                    searchButton.click();
                    searchField.forceActiveFocus();
                }

                Keys.onReturnPressed: {
                    searchButton.click();
                    searchField.forceActiveFocus();
                }
            }

            Overte.RoundButton {
                id: searchButton
                icon.source: "../icons/search.svg"
                icon.width: 24
                icon.height: 24
                icon.color: Overte.Theme.paletteActive.buttonText

                onClicked: {
                    searchExpression = searchField.text === "" ? ".*" : searchField.text;

                    // refresh the listing model
                    tableModel.rows = getDirectoryModel();
                }
            }
        }

        TableView {
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

            ScrollBar.vertical: Overte.ScrollBar {
                policy: ScrollBar.AlwaysOn
                interactive: true
            }

            ScrollBar.horizontal: Overte.ScrollBar {
                policy: ScrollBar.AsNeeded
                interactive: true
            }

            contentWidth: width - ScrollBar.vertical.width

            model: TableModel {
                id: tableModel

                TableModelColumn { display: "name" }

                rows: []
            }

            selectionModel: ItemSelectionModel {}
            delegate: TableDelegate {}
        }

        GridLayout {
            rows: 2
            columns: 2
            Layout.fillWidth: true
            Layout.margins: 8

            Overte.Button {
                Layout.fillWidth: true
                Layout.preferredWidth: 1

                // TODO: might work, don't have a way of testing atm
                //enabled: tableView.currentRow !== -1
                enabled: false
                backgroundColor: Overte.Theme.paletteActive.buttonDestructive
                text: qsTr("Delete")

                onClicked: {
                    const data = tableModel.rows[tableView.currentRow];
                    deleteWarningDialog.entryToDelete = data.name;
                    deleteWarningDialog.open();
                }
            }

            Overte.Button {
                Layout.fillWidth: true
                Layout.preferredWidth: 1

                enabled: tableView.currentRow !== -1
                backgroundColor: Overte.Theme.paletteActive.buttonInfo
                text: qsTr("Copy Link")

                onClicked: {
                    const data = tableModel.rows[tableView.currentRow];
                    WindowScriptingInterface.copyToClipboard(`atp:${data.path}`);
                }
            }

            Overte.Button {
                Layout.fillWidth: true
                Layout.preferredWidth: 1

                // TODO: might work, don't have a way of testing atm
                // enabled: tableView.currentRow !== -1
                enabled: false
                text: qsTr("Rename")

                onClicked: {
                    const data = tableModel.rows[tableView.currentRow];
                    renameDialog.entryToRename = data.name;
                    renameDialog.open();
                }
            }

            Overte.Button {
                Layout.fillWidth: true
                Layout.preferredWidth: 1

                enabled: {
                    if (tableView.currentRow === -1) { return false; }
                    return tableModel.rows[tableView.currentRow].name.match(spawnableRegex);
                }
                backgroundColor: Overte.Theme.paletteActive.buttonAdd
                text: qsTr("Create Entity")

                onClicked: {
                    const data = tableModel.rows[tableView.currentRow];
                    spawnEntity(data.path);
                }
            }
        }
    }

    Overte.MessageDialog {
        id: deleteWarningDialog
        anchors.fill: parent

        property string entryToDelete: ""
        text: qsTr("Are you sure you want to delete %1?").arg(entryToDelete)

        onAccepted: {
            // put the leading slash back on
            Assets.deleteMapping(`/${entryToDelete}`);
            entryToDelete = "";
            close();
        }

        onRejected: {
            entryToDelete = "";
            close();
        }
    }

    Overte.Dialog {
        id: renameDialog
        anchors.fill: parent

        property string entryToRename: ""

        signal accepted
        signal rejected

        onAccepted: {
            // put the leading slash back on
            Assets.renameMapping(`/${entryToRename}`, `/${assetRenameField.text}`, refreshModel);
            assetRenameField.text = "";
            entryToRename = "";
            close();
        }

        onRejected: {
            assetRenameField.text = "";
            entryToRename = "";
            close();
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8

            Overte.Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                opacity: Overte.Theme.highContrast ? 1.0 : 0.6
                text: qsTr("Rename asset")
            }
            Overte.Ruler { Layout.fillWidth: true }

            Overte.TextField {
                Layout.preferredWidth: 720
                Layout.fillWidth: true
                placeholderText: qsTr("New asset name")
                text: renameDialog.entryToRename
                id: assetRenameField
            }

            RowLayout {
                Overte.Button {
                    Layout.minimumWidth: 128
                    Layout.fillWidth: true
                    text: qsTr("Cancel")

                    onClicked: renameDialog.rejected()
                }

                Item { Layout.fillWidth: true }

                Overte.Button {
                    Layout.minimumWidth: 128
                    Layout.fillWidth: true

                    enabled: assetRenameField.text !== ""
                    backgroundColor: Overte.Theme.paletteActive.buttonAdd
                    text: qsTr("Rename")

                    onClicked: renameDialog.accepted()
                }
            }
        }
    }

    function spawnEntity(path) {
        let isImage = path.match(/\.(png|jpeg|jpg|webp)$/i);
        let isMesh = path.match(/\.(gltf|glb|vrm|fbx|fst|obj)$/i);

        // FIXME: "Could not convert argument 0 from [object Object] to EntityItemProperties"
        if (isImage) {
            /*Entities.addEntity({
                type: "Image",
                imageURL: `atp:${path}`,
                emissive: true,
                keepAspectRatio: true,
                rotation: MyAvatar.orientation,
                position: Vec3.sum(
                    MyAvatar.position,
                    Vec3.multiply(2, Quat.getForward(MyAvatar.orientation))
                ),
            });*/
        } else if (isMesh) {
            /*Entities.addEntity({
                type: "Model",
                modelURL: `atp:${path}`,
                shapeType: "simple-hull",
                rotation: MyAvatar.orientation,
                position: Vec3.sum(
                    MyAvatar.position,
                    Vec3.multiply(2, Quat.getForward(MyAvatar.orientation))
                ),
            });*/
            Entities.addModelEntity(
                /* name */          path,
                /* modelUrl */      `atp:${path}`,
                /* textures */      "",
                /* shapeType */     "simple-hull",
                /* dynamic */       false,
                /* collisionless */ false,
                /* grabbable */     true,
                /* position */      Vec3.sum(
                    MyAvatar.position,
                    Vec3.multiply(2, Quat.getForward(MyAvatar.orientation))
                ),
                /* gravity */       Vec3.ZERO
            );
        } else {
            console.error(`spawnEntity called with path that wasn't image or mesh! ${path}`);
        }
    }
}
