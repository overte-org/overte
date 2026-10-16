import QtQuick
import QtQuick.Controls
import "."

Switch {
    id: control
    font.pixelSize: Theme.fontPixelSize
    font.family: Theme.fontFamily
    opacity: control.enabled ? 1.0 : 0.5

    Rectangle {
        anchors.fill: indicator
        anchors.margins: -Theme.borderWidthFocused
        color: Theme.paletteActive.focusRing
        visible: control.activeFocus
        radius: indicator.radius
    }

    indicator: Rectangle {
        implicitWidth: 48
        implicitHeight: 24
        x: control.leftPadding
        y: parent.height / 2 - height / 2
        radius: 24
        color: {
            if (Theme.highContrast) {
                return control.checked ? Theme.paletteActive.buttonText : Theme.paletteActive.button;
            } else if (control.checked) {
                return Theme.paletteActive.highlight;
            } else {
                return Qt.darker(Theme.paletteActive.base, Theme.checkedDarker);
            }
        }
        border.color: (
            Theme.highContrast ?
            Theme.paletteActive.buttonText :
            Qt.darker(color, Theme.borderDarker)
        )
        border.width: Theme.borderWidth

        Rectangle {
            x: control.checked ? parent.width - width : 0
            width: 24
            height: 24
            radius: 24
            color: (
                control.down ?
                Qt.darker(Theme.paletteActive.button, Theme.depthDarker) :
                (
                    control.hovered && control.enabled ?
                    Qt.lighter(Theme.paletteActive.button, Theme.depthLighter) :
                    Theme.paletteActive.button
                )
            );
            border.color: {
                if (Theme.highContrast) {
                    return Theme.paletteActive.buttonText;
                } else {
                    return Qt.darker(Theme.paletteActive.button, Theme.borderDarker);
                }
            }
            border.width: Theme.borderWidth
        }
    }

    contentItem: Text {
        text: control.text
        font: control.font
        color: Theme.paletteActive.text
        opacity: enabled ? 1.0 : 0.3
        verticalAlignment: Text.AlignVCenter
        leftPadding: control.indicator.width + control.spacing
    }
}
