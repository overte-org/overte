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
    destroyOnHidden: true
    objectName: "AssetServer"
    title: qsTr("Asset Browser")
    opacity: parent.opacity

    Settings {
        category: "Overlay.AssetServer"
        property alias width: root.width
        property alias height: root.height
        property alias x: root.x
        property alias y: root.y
    }

    OverteDialogs.AssetDialog {
        anchors.fill: parent

        id: assetDialog
    }
}
