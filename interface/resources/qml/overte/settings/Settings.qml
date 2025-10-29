import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../" as Overte
import "." as OverteSettings
import "./pages" as SettingsPages
import "../dialogs" as OverteDialogs

Rectangle {
    id: settingsRoot
    width: 480
    height: 720
    visible: true
    anchors.fill: parent
    color: Overte.Theme.paletteActive.base

    Overte.TabBar {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        id: tabBar

        Overte.TabButton { text: qsTr("General") }
        Overte.TabButton { text: qsTr("Graphics") }
        Overte.TabButton { text: qsTr("Controls") }
        Overte.TabButton { text: qsTr("Audio") }
    }

    StackLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: tabBar.bottom
        anchors.bottom: parent.bottom
        anchors.topMargin: Overte.Theme.fontPixelSize
        currentIndex: tabBar.currentIndex

        SettingsPages.General {}

        SettingsPages.Graphics {}

        SettingsPages.Controls {}

        SettingsPages.Audio {}
    }

    OverteDialogs.FileDialog {
        anchors.fill: settingsRoot
        visible: false

        id: folderDialog

        property var acceptedCallback: folder => {}
        fileMode: OverteDialogs.FileDialog.OpenFolder

        onAccepted: {
            acceptedCallback(new URL(folderDialog.selectedFile).pathname);
            close();
        }

        onRejected: close()
    }

    function openFolderPicker(callback, root) {
        folderDialog.acceptedCallback = callback;
        if (root) {
            folderDialog.currentFolder = `file://${root}`;
        }
        folderDialog.open();
    }
}
