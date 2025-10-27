import QtCore
import QtQuick

import ".." as Overte
import "../dialogs" as OverteDialogs

import "../../windows" as HifiWindows

// compatibility shim with Hifi FileDialog
HifiWindows.Window {
    id: root
    resizable: true
    implicitWidth: 480
    implicitHeight: 360
    minSize: Qt.vector2d(360, 240)
    destroyOnCloseButton: true
    destroyOnHidden: true
    modality: Qt.ApplicationModal

    Settings {
        category: "FileDialog"
        property alias width: root.width
        property alias height: root.height
        property alias x: root.x
        property alias y: root.y
    }

    property alias caption: root.title
    property string dir: ""
    property var filter // maybe not necessary?

    property bool selectDirectory: false
    property bool multiSelect: false // unused
    property bool saveDialog: false

    property var options

    signal selectedFile(var file)
    signal canceled

    onSelectedFile: root.destroy()
    onCanceled: root.destroy()
    onWindowClosed: canceled()

    Component.onCompleted: {
        if (dir !== "") {
            fileDialog.currentFolder = dir;
        }
    }

    OverteDialogs.FileDialog {
        anchors.fill: parent

        id: fileDialog
        visible: true

        fileMode: {
            if (root.selectDirectory) {
                return OverteDialogs.FileDialog.FileMode.OpenFolder;
            } else if (root.saveDialog) {
                return OverteDialogs.FileDialog.FileMode.SaveFile;
            } else if (root.multiSelect) {
                return OverteDialogs.FileDialog.FileMode.OpenFiles;
            } else {
                return OverteDialogs.FileDialog.FileMode.OpenFile;
            }
        }

        onAccepted: root.selectedFile(fileDialog.selectedFile)
        onRejected: root.canceled()
    }
}
