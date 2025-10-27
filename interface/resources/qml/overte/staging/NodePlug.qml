import QtQuick

import ".." as Overte

Rectangle {
    property bool active: false

    implicitWidth: 24
    implicitHeight: 24

    id: plug
    radius: height / 2
    border.width: Overte.Theme.borderWidth
    border.color: Qt.darker(color, Overte.Theme.borderDarker)
    color: "#a000a0"

    gradient: Gradient {
        GradientStop {
            position: 0.0
            color: Qt.lighter(plug.color, 1.3)
        }
        GradientStop {
            position: 0.5
            color: plug.color
        }
        GradientStop {
            position: 1.0
            color: Qt.darker(plug.color, 1.3)
        }
    }

    Rectangle {
        anchors.fill: plug
        anchors.margins: plug.height / 5
        radius: height / 2

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: Qt.lighter(plug.color, plug.active ? 2.5 : 0.6)
            }
            GradientStop {
                position: 0.5
                color: Qt.lighter(plug.color, plug.active ? 1.5 : 0.7)
            }
            GradientStop {
                position: 1.0
                color: Qt.lighter(plug.color, plug.active ? 1.0 : 0.8)
            }
        }
    }
}
