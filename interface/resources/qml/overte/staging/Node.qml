import QtQuick
import QtQuick.Layouts
import QtQuick.Effects

import ".." as Overte

Rectangle {
    property color titleColor: "#a000a0"
    property string title: "Error"
    property string bodyText: ""
    property string helpText: ""

    property list<var> inputs: []
    property list<var> outputs: []

    DragHandler {
        dragThreshold: Overte.Theme.fontPixelSize
        cursorShape: Qt.DragMoveCursor
    }

    width: {
        let accum = 0;

        accum += titleLayout.implicitWidth;

        return accum;
    }

    height: {
        let accum = 0;

        if (title !== "") { accum += titlebar.height + 4; }

        let inputsAccum = 0, outputsAccum = 0;
        for (const input of inputs) { inputsAccum += 24 + 4; }
        for (const output of outputs) { outputsAccum += 24 + 4; }
        accum += Math.max(inputsAccum, outputsAccum) + 4;

        return accum;
    }

    id: control
    radius: Overte.Theme.borderRadius
    topLeftRadius: control.radius * 3
    topRightRadius: control.radius * 3

    border.width: Overte.Theme.borderWidth
    border.color: (
        Overte.Theme.highContrast ?
        Overte.Theme.paletteActive.buttonText :
        Qt.darker(Overte.Theme.paletteActive.base, Overte.Theme.borderDarker)
    )
    gradient: Gradient {
        GradientStop {
            position: 0.0; color: Qt.lighter(Overte.Theme.paletteActive.base, 1.1)
        }
        GradientStop {
            position: 0.5; color: Overte.Theme.paletteActive.base
        }
        GradientStop {
            position: 1.0; color: Qt.darker(Overte.Theme.paletteActive.base, 1.1)
        }
    }

    Rectangle {
        anchors.fill: titleLayout
        visible: control.title !== ""
        id: titlebar

        topLeftRadius: control.radius * 2
        topRightRadius: control.radius * 2

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: Qt.lighter(control.titleColor, 1.1)
            }
            GradientStop {
                position: 0.2
                color: Qt.darker(control.titleColor, 1.1)
            }
            GradientStop {
                position: 0.8
                color: control.titleColor
            }
            GradientStop {
                position: 1.0
                color: Qt.darker(control.titleColor, 1.3)
            }
        }
    }

    RowLayout {
        anchors.left: control.left
        anchors.right: control.right
        anchors.top: control.top
        anchors.margins: Overte.Theme.borderWidth

        id: titleLayout
        spacing: 4

        Overte.Label {
            Layout.margins: 6
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: Overte.Theme.fontPixelSize * 1.5
            visible: control.title !== ""
            verticalAlignment: Text.AlignVCenter
            text: control.title
            id: titleText
            color: "white"

            layer.enabled: true
            layer.effect: MultiEffect {
                shadowEnabled: true
                shadowVerticalOffset: 2
                shadowHorizontalOffset: 2
                shadowColor: Qt.darker(control.titleColor, 2.0)
                shadowBlur: 0.2
            }
        }

        Overte.RoundButton {
            Layout.margins: 4
            Layout.alignment: Qt.AlignCenter
            visible: control.helpText !== ""
            implicitWidth: Overte.Theme.fontPixelSize + 8
            implicitHeight: Overte.Theme.fontPixelSize + 8
            text: "?"
        }
    }

    Column {
        anchors.left: control.left
        anchors.top: titlebar.bottom
        anchors.topMargin: 4
        spacing: 4
        id: leftPlugs

        Repeater {
            model: control.inputs
            delegate: NodePlug {
                required property string type

                x: -10
                color: Overte.Theme.paletteActive.scriptTypeColors[type] ?? "#ff00ff"
            }
        }
    }

    Overte.Label {
        anchors.margins: 8
        anchors.bottom: control.bottom
        anchors.left: leftPlugs.right
        anchors.right: rightPlugs.left
        anchors.top: control.title !== "" ? titlebar.bottom : control.top
        visible: control.bodyText !== ""
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: control.height / 2
        text: control.bodyText
    }

    Column {
        anchors.right: control.right
        anchors.top: titlebar.bottom
        anchors.topMargin: 4
        spacing: 4
        id: rightPlugs

        Repeater {
            model: control.outputs
            delegate: NodePlug {
                required property string type
    
                x: 10
                color: Overte.Theme.paletteActive.scriptTypeColors[type] ?? "#ff00ff"
            }
        }
    }
}
