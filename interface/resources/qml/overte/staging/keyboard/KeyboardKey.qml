import QtQuick

import "../"

Button {
    property int unshifted_keycode: 0
    property int shifted_keycode: 0

    property string unshifted_char: ""
    property string shifted_char: ""

    property string legend: ""
    property real span: 1.0

    id: control
    text: legend
    focusPolicy: Qt.NoFocus
    horizontalPadding: 4
    verticalPadding: 4

    onPressed: keyboardRoot.keyPressed(this)
    onReleased: keyboardRoot.keyReleased(this)

    Connections {
        target: keyboardRoot

        function onModifiersChanged() {
            const value = keyboardRoot.modifiers;

            switch (unshifted_keycode) {
                case Qt.Key_Shift:
                    checked = (value & Qt.ShiftModifier) !== 0;
                    break;

                case Qt.Key_Control:
                    checked = (value & Qt.ControlModifier) !== 0;
                    break;

                case Qt.Key_Alt:
                    checked = (value & Qt.AltModifier) !== 0;
                    break;

                case Qt.Key_Meta:
                    checked = (value & Qt.MetaModifier) !== 0;
                    break;
            }
        }
    }

    contentItem: Text {
        text: control.text
        font.family: control.font.family
        font.pixelSize: control.height / 3
        color: Theme.paletteActive.buttonText
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideNone
    }

    background: Rectangle {
        id: controlBg
        radius: Theme.borderRadius
        border.width: Theme.borderWidth
        border.color: control.checked ?
            Theme.paletteActive.focusRing :
            (Theme.highContrast ? Theme.paletteActive.controlText : Qt.darker(control.backgroundColor, Theme.borderDarker))
        color: (control.down || control.checked) ? Qt.darker(control.backgroundColor, Theme.checkedDarker) :
            control.hovered ? Qt.lighter(control.backgroundColor, Theme.hoverLighter) :
            control.backgroundColor;
        gradient: Gradient {
            GradientStop {
                position: 0.0; color: Qt.lighter(controlBg.color, (control.down || control.checked) ? 0.9 : 1.1)
            }
            GradientStop {
                position: 0.5; color: controlBg.color
            }
            GradientStop {
                position: 1.0; color: Qt.darker(controlBg.color, (control.down || control.checked) ? 0.9 : 1.1)
            }
        }
    }
}
