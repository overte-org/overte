import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

// hack to get access to IconImage,
// why isn't it part of stock QtQuick.Controls?
import QtQuick.Controls.impl

import ".." as Overte

Rectangle {
    id: scriptLog
    anchors.fill: parent
    color: Overte.Theme.paletteActive.base

    property ListModel model: ListModel {}

    function fromScript(rawMsg) {
        scriptLog.model.append(JSON.parse(rawMsg));
    }

    function transparent(c, a = 0.1) {
        return Qt.rgba(c.r, c.g, c.b, a);
    }

    readonly property list<color> priorityColors: [
        Overte.Theme.paletteActive.logPriorityDebug,
        Overte.Theme.paletteActive.logPriorityInfo,
        Overte.Theme.paletteActive.logPriorityWarn,
        Overte.Theme.paletteActive.logPriorityError,
    ]

    readonly property list<string> priorityIcons: [
        "",
        "../icons/log_info.svg",
        "../icons/log_warn.svg",
        "../icons/log_error.svg",
    ]

    component LogLine: Rectangle {
        required property int priority
        required property string text
        property string source

        width: listView.contentWidth
        height: label.implicitHeight + 8

        color: transparent(priorityColors[priority])
        border.color: priorityColors[priority]
        border.width: 1

        // IconImage isn't documented and technically private
        IconImage {
            id: icon

            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.margins: 4
            anchors.leftMargin: 6
            sourceSize.width: 16
            sourceSize.height: 16
            width: 16
            height: 16

            visible: priorityIcons[priority] !== ""
            color: parent.border.color
            source: priorityIcons[priority]
        }

        TextEdit {
            id: label
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.left: icon.right
            anchors.margins: 4
            anchors.leftMargin: 6

            readOnly: true
            selectionColor: Overte.Theme.paletteActive.highlight
            selectedTextColor: Overte.Theme.paletteActive.highlightedText
            font.family: Overte.Theme.monoFontFamily
            font.pixelSize: Overte.Theme.fontPixelSizeSmall
            color: Overte.Theme.paletteActive.text
            wrapMode: Text.Wrap
            text: parent.text
        }
    }

    Overte.RoundButton {
        id: clearButton

        anchors.margins: 2
        anchors.top: parent.top
        anchors.right: parent.right

        icon.source: "../icons/delete.svg"
        icon.width: 24
        icon.height: 24
        icon.color: Overte.Theme.paletteActive.buttonText
        backgroundColor: (
            hovered ?
            Overte.Theme.paletteActive.buttonDestructive :
            Overte.Theme.paletteActive.button
        )

        Overte.ToolTip { text: qsTr("Clear log") }

        onClicked: scriptLog.model.clear()
    }

    ListView {
        id: listView

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: clearButton.bottom
        anchors.topMargin: 4

        ScrollBar.vertical: Overte.ScrollBar {
            interactive: true
            policy: ScrollBar.AlwaysOn
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
        }
        contentWidth: width - ScrollBar.vertical.width

        delegate: LogLine {}
        model: scriptLog.model
    }
}
