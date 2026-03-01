import QtQuick
import QtQuick.Controls
import "."

ToolTip {
    id: control
    delay: 500

    // NOTE: this default is for Controls,
    // if you're using something else then override this
    visible: parent.hovered

    font.family: Theme.fontFamily
    font.pixelSize: Theme.fontPixelSizeSmall

    background: Item {
        // drop shadow
        Rectangle {
            x: 3
            y: 3
            width: parent.width
            height: parent.height

            radius: Theme.borderRadius
            color: "#a0000000"
        }

        Rectangle {
            x: 0
            y: 0
            width: parent.width
            height: parent.height

            radius: Theme.borderRadius
            border.width: Theme.borderWidth
            border.color: Theme.highContrast ? Theme.paletteActive.tooltipText : Qt.darker(Theme.paletteActive.tooltip, Theme.borderDarker)
            color: Theme.paletteActive.tooltip
        }
    }

    contentItem: Text {
        text: control.text
        font: control.font
        color: Theme.paletteActive.tooltipText
        wrapMode: Text.Wrap
    }
}
