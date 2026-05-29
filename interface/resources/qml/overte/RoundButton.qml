import QtQuick
import QtQuick.Controls
import "."

Button {
    id: button
    property color backgroundColor: Theme.paletteActive.button

    palette.buttonText: Theme.paletteActive.buttonText
    font.family: Theme.fontFamily
    font.pixelSize: Theme.fontPixelSize
    horizontalPadding: 2
    verticalPadding: 2
    hoverEnabled: true
    implicitHeight: font.pixelSize * 2
    implicitWidth: font.pixelSize * 2

    background: Rectangle {
        id: buttonBg
        radius: button.height / 2
        border.width: parent.activeFocus ? Theme.borderWidthFocused : Theme.borderWidth
        border.color: (
            button.activeFocus ?
            Theme.paletteActive.focusRing :
            (
                Theme.highContrast ?
                Theme.paletteActive.buttonText :
                Qt.darker(button.backgroundColor, Theme.borderDarker)
            )
        )
        color: (
            (button.down || button.checked) ?
            Qt.darker(button.backgroundColor, Theme.checkedDarker) :
            (
                (button.hovered && button.enabled) ?
                Qt.lighter(button.backgroundColor, Theme.hoverLighter) :
                button.backgroundColor
            )
        )
        gradient: Gradient {
            GradientStop {
                position: 0.0; color: Qt.lighter(buttonBg.color, (button.down || button.checked) ? 0.9 : 1.1)
            }
            GradientStop {
                position: 0.5; color: buttonBg.color
            }
            GradientStop {
                position: 1.0; color: Qt.darker(buttonBg.color, (button.down || button.checked) ? 0.9 : 1.1)
            }
        }
    }
}
