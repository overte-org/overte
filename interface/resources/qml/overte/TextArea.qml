import QtQuick
import QtQuick.Controls
import "."

TextArea {
    id: textArea
    selectByMouse: true

    property color backgroundColor: Theme.paletteActive.window

    color: Theme.paletteActive.windowText
    placeholderTextColor: Theme.paletteActive.placeholderText
    font.pixelSize: Theme.fontPixelSize
    font.family: Theme.fontFamily
    selectionColor: Theme.paletteActive.highlight
    selectedTextColor: Theme.paletteActive.highlightedText

    leftPadding: 6
    rightPadding: 6
    topPadding: 8
    bottomPadding: 8

    background: Rectangle {
        radius: Theme.borderRadius
        border.width: parent.activeFocus ? Theme.borderWidthFocused : Theme.borderWidth
        border.color: textArea.activeFocus ?
            Theme.paletteActive.focusRing :
            (Theme.highContrast ? Theme.paletteActive.windowText : Qt.darker(textArea.backgroundColor, Theme.borderDarker))
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.darker(textArea.backgroundColor, 1.02) }
            GradientStop { position: 0.5; color: textArea.backgroundColor }
            GradientStop { position: 1.0; color: Qt.lighter(textArea.backgroundColor, 1.02) }
        }
    }
}
