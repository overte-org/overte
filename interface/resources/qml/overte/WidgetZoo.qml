import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import "." as Overte

// debugging test case to view the themed widgets
Rectangle {
    id: root
    width: 480
    height: 720
    visible: true
    color: Overte.Theme.paletteActive.base

    Overte.TabBar {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        id: tabBar
        Overte.TabButton { text: "Tab 1"; width: 120 }
        Overte.TabButton { text: "Tab 2"; width: 120 }
        Overte.TabButton { text: "Tab 3"; width: 120 }
        Overte.TabButton { text: "Tab 4"; width: 120 }
        Overte.TabButton { text: "Tab 5"; width: 120 }
        Overte.TabButton { text: "Tab 6"; width: 120 }
        Overte.TabButton { text: "Tab 7"; width: 120 }
        Overte.TabButton { text: "Tab 8"; width: 120 }
    }

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: tabBar.bottom
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        anchors.topMargin: 16
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Overte.Button {
                text: "Button"
            }
            Overte.Button {
                text: "Destroy"
                backgroundColor: Overte.Theme.paletteActive.buttonDestructive
            }
            Overte.Button {
                text: "Add"
                backgroundColor: Overte.Theme.paletteActive.buttonAdd
            }
            Overte.Button {
                text: "Info"
                backgroundColor: Overte.Theme.paletteActive.buttonInfo
            }
            Overte.RoundButton {
                icon.source: "./icons/folder.svg"
                icon.width: 24
                icon.height: 24
            }
        }

        Overte.TextField {
            Layout.fillWidth: true
            placeholderText: "Text field"
        }
        Overte.TextArea {
            Layout.fillWidth: true
            Layout.preferredHeight: 64
            placeholderText: "Text area"
        }
        Overte.TextArea {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            placeholderText: "Code area"
            font.family: Overte.Theme.monoFontFamily
            text: "function doSomething() {\n  console.info(\"Hello, world\");\n}\n"
        }
        Overte.Switch {
            text: "Switch"
        }
        Overte.Slider {
            value: 0.5
        }
        Overte.SpinBox {
            editable: true
        }
        Overte.Ruler { Layout.fillWidth: true }
        Row {
            spacing: 16

            Overte.ComboBox {
                model: [
                    "ComboBox",
                    "Singing Dogs",
                    "Golden Combs",
                    "Swirly Bombs",
                ]
            }

            Overte.Label {
                font.pixelSize: Overte.Theme.fontPixelSizeSmall
                text: "This is a note label."
            }
        }

        Overte.BodyText {
            Layout.fillWidth: true
            text: "This is body text. It's meant for selectable text documents or chat messages."
        }

        ScrollView {
            id: scrollView
            Layout.fillWidth: true
            Layout.preferredHeight: 256

            clip: true
            contentWidth: 1024
            contentHeight: 1024

            ScrollBar.vertical: Overte.ScrollBar {
                anchors.top: scrollView.top
                anchors.bottom: scrollView.bottom
                anchors.right: scrollView.right
                anchors.bottomMargin: Theme.scrollbarWidth
            }
            ScrollBar.horizontal: Overte.ScrollBar {
                anchors.bottom: scrollView.bottom
                anchors.left: scrollView.left
                anchors.right: scrollView.right
                anchors.rightMargin: Theme.scrollbarWidth
            }

            Overte.Label {
                text: "ScrollView and ScrollBar"
            }
        }
    }
}
