import QtQuick
import QtQuick.Controls

import "."

SpinBox {
    id: control

    font.pixelSize: Theme.fontPixelSize
    font.family: Theme.fontFamily

    background: Rectangle {
        anchors.left: control.down.indicator.right
        anchors.right: control.up.indicator.left
        anchors.leftMargin: -Theme.borderWidth
        anchors.rightMargin: -Theme.borderWidth

        implicitHeight: Theme.fontPixelSize * 2
        implicitWidth: 180
        // no radius so the borders can mix into the scroll button borders
        //radius: Theme.borderRadius
        border.width: control.activeFocus ? Theme.borderWidthFocused : Theme.borderWidth
        border.color: (
            parent.activeFocus ?
            Theme.paletteActive.focusRing :
            (
                Theme.highContrast ?
                Theme.paletteActive.windowText :
                Qt.darker(Theme.paletteActive.window, Theme.borderDarker)
            )
        )
        color: Theme.paletteActive.window
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.darker(control.background.color, 1.02) }
            GradientStop { position: 0.5; color: control.background.color }
            GradientStop { position: 1.0; color: Qt.lighter(control.background.color, 1.02) }
        }
    }

    contentItem: TextInput {
        anchors.left: control.down.indicator.right
        anchors.right: control.up.indicator.left

        font: control.font
        color: Theme.paletteActive.windowText
        selectionColor: Theme.paletteActive.highlight
        selectedTextColor: Theme.paletteActive.highlightedText
        text: control.textFromValue(control.value, control.locale)

        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter

        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: Qt.ImhFormattedNumbersOnly
    }

    down.indicator: Rectangle{
        id: downIndicator
        x: 0
        width: Theme.fontPixelSize * 2
        height: parent.height
        color: (
            control.down.down ?
            Qt.darker(Theme.paletteActive.button, Theme.checkedDarker) :
            (
                (control.down.hovered && control.enabled) ?
                Qt.lighter(Theme.paletteActive.button, Theme.hoverLighter) :
                Theme.paletteActive.button
            )
        )
        opacity: control.value > control.from ? 1.0 : 0.5

        radius: 0
        topLeftRadius: Theme.borderRadius
        bottomLeftRadius: Theme.borderRadius

        border.width: Theme.borderWidth
        border.color: (
            Theme.highContrast ?
            Theme.paletteActive.windowText :
            Qt.darker(Theme.paletteActive.button, Theme.borderDarker)
        )

        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.lighter(downIndicator.color, 1.05) }
            GradientStop { position: 0.5; color: downIndicator.color }
            GradientStop { position: 1.0; color: Qt.darker(downIndicator.color, 1.05) }
        }

        Text {
            anchors.fill: parent
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
            color: Theme.paletteActive.buttonText
            font: control.font
            text: "-"
        }
    }

    up.indicator: Rectangle{
        id: upIndicator
        x: parent.width - width
        width: Theme.fontPixelSize * 2
        height: parent.height
        color: (
            control.up.down ?
            Qt.darker(Theme.paletteActive.button, Theme.checkedDarker) :
            (
                (control.up.hovered && control.enabled) ?
                Qt.lighter(Theme.paletteActive.button, Theme.hoverLighter) :
                Theme.paletteActive.button
            )
        )
        opacity: control.value < control.to ? 1.0 : 0.5

        radius: 0
        topRightRadius: Theme.borderRadius
        bottomRightRadius: Theme.borderRadius

        border.width: Theme.borderWidth
        border.color: (
            Theme.highContrast ?
            Theme.paletteActive.windowText :
            Qt.darker(Theme.paletteActive.button, Theme.borderDarker)
        )

        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.lighter(upIndicator.color, 1.05) }
            GradientStop { position: 0.5; color: upIndicator.color }
            GradientStop { position: 1.0; color: Qt.darker(upIndicator.color, 1.05) }
        }

        Text {
            anchors.fill: parent
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
            color: Theme.paletteActive.buttonText
            font: control.font
            text: "+"
        }
    }
}
