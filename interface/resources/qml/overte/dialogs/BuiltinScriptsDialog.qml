import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.qmlmodels

import ".." as Overte

Rectangle {
    anchors.fill: parent
    implicitWidth: 400
    implicitHeight: 500
    visible: false
    color: Overte.Theme.paletteActive.base
    id: picker

    signal accepted(file: url)
    signal rejected

    onAccepted: visible = false
    onRejected: visible = false

    function open() {
        visible = true;
    }

    function getDirectoryModel(parentIndex = undefined) {
        const DISPLAY_ROLE = 0;
        const PATH_ROLE = 256;
        const sourceModel = ScriptDiscoveryService.scriptsModel;
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

        Overte.Label {
            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
            opacity: Overte.Theme.highContrast ? 1.0 : 0.6
            wrapMode: Text.Wrap
            text: qsTr("Load built-in script")
        }

        Overte.Ruler {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBottom
        }

        TreeView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            id: treeView
            clip: true

            // QT6TODO: remove this once mouse input is working properly again,
            // this needs to be false until then or the expander arrows are entirely unusable
            interactive: false

            ScrollBar.vertical: Overte.ScrollBar {
                policy: ScrollBar.AlwaysOn
            }

            contentWidth: width - ScrollBar.vertical.width
            flickableDirection: Flickable.VerticalFlick
            selectionModel: ItemSelectionModel {}

            model: TreeModel {
                id: treeModel

                TableModelColumn { display: "name" }

                rows: getDirectoryModel()
            }

            delegate: TreeDelegate {}
        }

        RowLayout {
            Layout.margins: 8
            Layout.fillWidth: true

            Item { Layout.fillWidth: true }
            
            Overte.Button {
                Layout.preferredWidth: 128
                text: qsTr("Cancel")

                onClicked: rejected()
            }

            Overte.Button {
                Layout.preferredWidth: 128

                enabled: {
                    if (treeView.selectionModel.currentIndex === -1) { return false; }

                    const data = treeModel.getRow(treeView.selectionModel.currentIndex);
                    return !data?.rows;
                }
                text: qsTr("Load")
                backgroundColor: Overte.Theme.paletteActive.buttonAdd

                onClicked: {
                    const data = treeModel.getRow(treeView.selectionModel.currentIndex);
                    accepted(data.path);
                }
            }
        }
    }
}
