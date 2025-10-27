import QtQuick
import QtQuick.Layouts

import "../" as Overte

Rectangle {
    id: root
    implicitWidth: 720
    implicitHeight: 480
    color: Overte.Theme.paletteActive.base

    Overte.TabBar {
        id: tabBar
        anchors.top: root.top
        anchors.left: root.left
        anchors.right: root.right

        Overte.TabButton { width: implicitWidth; text: qsTr("Keyboard") }
        Overte.TabButton { width: implicitWidth; text: qsTr("Oculus Touch") }
        Overte.TabButton { width: implicitWidth; text: qsTr("Mixed Reality") }
        Overte.TabButton { width: implicitWidth; text: qsTr("Index") }
        Overte.TabButton { width: implicitWidth; text: qsTr("Vive") }
    }

    StackLayout {
        anchors.top: tabBar.bottom
        anchors.left: root.left
        anchors.right: root.right
        anchors.bottom: root.bottom
        currentIndex:    tabBar.currentIndex

        // Keyboard
        Image {
            Layout.fillWidth: true
            Layout.fillHeight: true
            fillMode: Image.PreserveAspectFit
            source: Overte.Theme.darkMode ? "./assets/controls_dark.png" : "./assets/controls_light.png"
        }

        // Oculus Touch
        Image {
            Layout.fillWidth: true
            Layout.fillHeight: true
            fillMode: Image.PreserveAspectFit
            source: Overte.Theme.darkMode ? "./assets/controls_dark.png" : "./assets/controls_light.png"
        }

        // Mixed Reality
        Image {
            Layout.fillWidth: true
            Layout.fillHeight: true
            fillMode: Image.PreserveAspectFit
            source: Overte.Theme.darkMode ? "./assets/controls_dark.png" : "./assets/controls_light.png"
        }
    
        // Index
        Image {
            Layout.fillWidth: true
            Layout.fillHeight: true
            fillMode: Image.PreserveAspectFit
            source: Overte.Theme.darkMode ? "./assets/controls_dark.png" : "./assets/controls_light.png"
        }

        // Vive
        Image {
            Layout.fillWidth: true
            Layout.fillHeight: true
            fillMode: Image.PreserveAspectFit
            source: Overte.Theme.darkMode ? "./assets/controls_dark.png" : "./assets/controls_light.png"
        }
    }
}
