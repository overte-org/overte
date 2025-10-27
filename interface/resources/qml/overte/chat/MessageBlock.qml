import QtQuick
import QtQuick.Layouts

import "../" as Overte

ColumnLayout {
    required property int index
    required property string name
    required property string body
    required property string notification
    required property string timestamp

    anchors.left: parent ? parent.left : undefined
    anchors.right: parent ? parent.right : undefined
    anchors.leftMargin: 4
    anchors.rightMargin: Overte.Theme.scrollbarWidth

    Rectangle {
        Layout.fillWidth: true
        implicitHeight: 3
        color: Overte.Theme.paletteActive.highlight

        ColorAnimation on color {
            to: Overte.Theme.paletteActive.alternateBase
            duration: 500
        }
    }

    RowLayout {
        Layout.leftMargin: 6
        Layout.rightMargin: 8
        Layout.fillWidth: true
        opacity: Overte.Theme.highContrast ? 1.0 : 0.6

        Overte.Label {
            text: notification ? notification : name
            font.pixelSize: Overte.Theme.fontPixelSizeSmall
            font.bold: true
            Layout.fillWidth: true
            wrapMode: Text.Wrap
        }

        Overte.Label {
            Layout.alignment: Qt.AlignRight
            horizontalAlignment: Text.AlignRight
            font.pixelSize: Overte.Theme.fontPixelSizeSmall
            text: timestamp
        }
    }

    Overte.BodyText {
        Layout.leftMargin: 6
        Layout.rightMargin: 16
        // if we used Layout.fillWidth, the whole text line would be selectable
        // with this hack we only use the width we really need to
        Layout.maximumWidth: parent.width - parent.anchors.leftMargin - parent.anchors.rightMargin

        visible: text.length > 0

        // MD support is cool, but it'd only work properly in the QML chat app
        // and not chat bubbles. (maybe would work with QML desktop notifications?)
        //textFormat: TextEdit.MarkdownText
        textFormat: TextEdit.RichText

        text: {
            if (notification) { return ""; }

            return (
                body
                .replace(
                    /(https?:\/\/[^\s]+)/gi,
                    `<a style="color:${Overte.Theme.paletteActive.link}" href="$1">$1</a>`
                )
            );
        }
    }
}
