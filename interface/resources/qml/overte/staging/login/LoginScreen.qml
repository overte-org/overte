import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Effects

import "../" as Overte
import "./pages" as Pages

Rectangle {
    anchors.fill: parent
    implicitWidth: 480
    implicitHeight: 720
    id: root

    color: Overte.Theme.paletteActive.base

    Image {
        visible: !Overte.Theme.highContrast
        anchors.fill: parent
        source: "./assets/background.png"

        layer.enabled: true
        layer.effect: MultiEffect {
            blurEnabled: true
            blurMax: 64
            blur: 1.0
            contrast: Overte.Theme.darkMode ? -0.7 : -0.8
            brightness: Overte.Theme.darkMode ? -0.2 : 0.25
        }
    }

    ColumnLayout {
        anchors.fill: parent

        Image {
            Layout.fillWidth: true

            id: logo
            fillMode: Image.PreserveAspectFit

            source: Overte.Theme.darkMode ? "./assets/logo_dark.png" : "./assets/logo_light.png"
        }

        Overte.StackView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: stack
            initialItem: Pages.StartPage {}
        }
    }
}
