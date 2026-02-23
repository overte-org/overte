import QtQuick
import QtQuick.Controls
import "."

TextField {
    id: textField
    selectByMouse: true

    property color backgroundColor: Theme.paletteActive.window

    color: Theme.paletteActive.windowText
    placeholderTextColor: Theme.paletteActive.placeholderText
    font.pixelSize: Theme.fontPixelSize
    font.family: Theme.fontFamily
    selectionColor: Theme.paletteActive.highlight
    selectedTextColor: Theme.paletteActive.highlightedText

    opacity: enabled ? 1.0 : 0.5

    leftPadding: 6
    rightPadding: 6
    topPadding: 8
    bottomPadding: 8

    background: Rectangle {
        implicitHeight: Theme.fontPixelSize * 2
        radius: Theme.borderRadius
        border.width: parent.activeFocus ? Theme.borderWidthFocused : Theme.borderWidth
        border.color: parent.activeFocus ?
            Theme.paletteActive.focusRing :
            (Theme.highContrast ? Theme.paletteActive.windowText : Qt.darker(parent.backgroundColor, Theme.borderDarker))
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.darker(textField.backgroundColor, 1.02) }
            GradientStop { position: 0.5; color: textField.backgroundColor }
            GradientStop { position: 1.0; color: Qt.lighter(textField.backgroundColor, 1.02) }
        }
    }
}
