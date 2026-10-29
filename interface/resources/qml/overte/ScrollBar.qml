import QtQuick
import QtQuick.Controls
import "."

ScrollBar {
    id: control
    opacity: {
        if (
            control.policy === ScrollBar.AlwaysOn ||
            (control.policy === ScrollBar.AsNeeded && interactive && control.size < 1.0)
        ) {
            return 1.0;
        } else if (control.active && control.size < 1.0) {
            return 0.75;
        } else {
            return 0.0;
        }
    }

    Behavior on opacity {
        NumberAnimation {}
    }

    property color backgroundColor: Theme.paletteActive.button

    readonly property bool hasButtons: Theme.scrollbarButtons && interactive
    readonly property int thumbWidth: Theme.scrollbarWidth - (horizontalPadding * 2)
    readonly property bool isHorizontal: orientation === Qt.Horizontal

    //horizontalPadding: Theme.borderWidth
    //verticalPadding: Theme.borderWidth
    horizontalPadding: 0
    verticalPadding: 0

    // TODO: magic numbers? minimumSize is weird and confusing,
    // 0.32 doesn't work on short scrollbars
    stepSize: 0.03
    minimumSize: 0.32

    background: Rectangle {
        color: Qt.darker(
            Theme.paletteActive.base,
            Theme.highContrast ? 1.0 : (Theme.darkMode ? 1.2 : 1.1)
        )
        implicitWidth: Theme.scrollbarWidth
        implicitHeight: Theme.scrollbarWidth
    }

    contentItem: Item {
        implicitWidth: horizontal ? 32 : thumbWidth
        implicitHeight: horizontal ? thumbWidth : 32

        Rectangle {
            // pad away from the scroll buttons, but allow a border width of sink-in
            // so there isn't a double-border when the thumbs are at their min/max
            anchors.topMargin: control.hasButtons && !control.isHorizontal ? thumbWidth - Theme.borderWidth : 0
            anchors.bottomMargin: control.hasButtons && !control.isHorizontal ? thumbWidth - Theme.borderWidth: 0
            anchors.leftMargin: control.hasButtons && control.isHorizontal ? thumbWidth - Theme.borderWidth: 0
            anchors.rightMargin: control.hasButtons && control.isHorizontal ? thumbWidth - Theme.borderWidth : 0
            anchors.fill: parent

            id: buttonBg
            radius: Theme.borderRadius
            border.width: Theme.borderWidth
            border.color: Theme.highContrast ? Theme.paletteActive.buttonText : Qt.darker(control.backgroundColor, Theme.borderDarker)
            color: Theme.highContrast ? Theme.paletteActive.buttonText : control.backgroundColor;
            gradient: Gradient {
                GradientStop {
                    position: 0.0; color: Qt.lighter(buttonBg.color, 1.05)
                }
                GradientStop {
                    position: 0.5; color: buttonBg.color
                }
                GradientStop {
                    position: 1.0; color: Qt.darker(buttonBg.color, 1.05)
                }
            }
        }
    }

    Button {
        anchors.right: !control.isHorizontal ? control.right : undefined
        anchors.rightMargin: !control.isHorizontal ? control.horizontalPadding : undefined

        anchors.left: control.isHorizontal ? control.left : undefined
        anchors.leftMargin: control.isHorizontal ? control.horizontalPadding : undefined

        anchors.top: control.top
        anchors.topMargin: control.verticalPadding

        width: thumbWidth
        height: width

        id: scrollLessButton
        visible: control.hasButtons
        focusPolicy: Qt.NoFocus

        icon.source: (
            !control.isHorizontal ?
            "./icons/triangle_up.svg" :
            "./icons/triangle_left.svg"
        )
        icon.width: width - 4
        icon.height: height - 4
        icon.color: Theme.paletteActive.buttonText
        display: AbstractButton.IconOnly
        horizontalPadding: 0
        verticalPadding: 0
        autoRepeat: true

        onClicked: {
            control.position = Math.max(0.0, control.position - control.stepSize);
        }
    }

    Button {
        anchors.right: control.isHorizontal ? control.right : undefined
        anchors.rightMargin: control.isHorizontal ? control.horizontalPadding : undefined

        anchors.left: !control.isHorizontal ? control.left : undefined
        anchors.leftMargin: !control.isHorizontal ? control.horizontalPadding : undefined

        anchors.bottom: control.bottom
        anchors.bottomMargin: control.verticalPadding

        width: thumbWidth
        height: width

        id: scrollMoreButton
        visible: control.hasButtons
        focusPolicy: Qt.NoFocus
        display: AbstractButton.IconOnly
        horizontalPadding: 0
        verticalPadding: 0
        autoRepeat: true

        icon.source: (
            !control.isHorizontal ?
            "./icons/triangle_down.svg" :
            "./icons/triangle_right.svg"
        )
        icon.width: width - 4
        icon.height: height - 4
        icon.color: Theme.paletteActive.buttonText

        onClicked: {
            control.position = Math.min(1.0 - control.size, control.position + control.stepSize);
        }
    }
}
