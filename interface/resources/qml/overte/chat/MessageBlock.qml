import QtQuick
import QtQuick.Layouts

import "../" as Overte

ColumnLayout {
    required property int index
    required property string name
    required property string body
    required property string notification
    required property real timestamp
    required property list<string> imageEmbeds

    id: messageBlock

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
            Layout.fillWidth: true
            id: nameLabel
            text: notification ? notification : name
            font.pixelSize: Overte.Theme.fontPixelSizeSmall
            font.bold: true
            elide: Text.ElideRight
            children: truncated ? [nameTooltipArea] : []

            MouseArea {
                id: nameTooltipArea
                anchors.fill: parent
                cursorShape: Qt.WhatsThisCursor
                hoverEnabled: true

                Overte.ToolTip {
                    visible: parent.containsMouse
                    text: nameLabel.text
                }
            }
        }

        Overte.Label {
            Layout.alignment: Qt.AlignRight
            horizontalAlignment: Text.AlignRight
            font.pixelSize: Overte.Theme.fontPixelSizeSmall
            text: (new Date(timestamp)).toLocaleTimeString(undefined, Locale.ShortFormat)

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.WhatsThisCursor
                hoverEnabled: true

                Overte.ToolTip {
                    visible: parent.containsMouse
                    text: (new Date(timestamp)).toLocaleString(undefined, Locale.LongFormat)
                }
            }
        }
    }

    Overte.BodyText {
        Layout.leftMargin: 6
        Layout.rightMargin: 16
        // If we used Layout.fillWidth, then the entire block width would be selectable.
        // With this hack we only use the width we really need to
        Layout.maximumWidth: parent.width - parent.anchors.leftMargin - parent.anchors.rightMargin

        visible: text.length > 0

        // MD support is cool, but it'd only work properly in the QML chat
        // app and not chat bubbles, where formatting isn't supported in Text entities.
        //textFormat: TextEdit.MarkdownText
        textFormat: TextEdit.RichText

        text: {
            if (notification) { return ""; }

            return (
                body
                .replace(
                    /(https?:\/\/\S+)/gi,
                    `<a style="color:${Overte.Theme.paletteActive.link}" href="$1">$1</a>`
                )
            );
        }
    }

    Repeater {
        model: imageEmbeds

        Rectangle {
            required property string modelData

            color: Overte.Theme.paletteActive.alternateBase
            radius: Overte.Theme.borderRadius

            Layout.maximumWidth: messageBlock.width - 64
            Layout.maximumHeight: 320

            implicitWidth: (
                embeddedImage.status === Image.Ready ?
                embeddedImage.sourceSize.width + (Overte.Theme.borderWidth * 2) :
                messageBlock.width - 64
            )
            implicitHeight: Math.max(
                Overte.Theme.fontPixelSize * 2,
                embeddedImage.sourceSize.width + (Overte.Theme.borderWidth * 2)
            )

            AnimatedImage {
                anchors.fill: parent
                anchors.margins: Overte.Theme.borderWidth

                id: embeddedImage
                fillMode: Image.PreserveAspectFit
                autoTransform: true
                source: modelData
                //sourceSize.width: width
                //sourceSize.height: height

                MouseArea {
                    anchors.fill: parent
                    id: embedMouseArea

                    hoverEnabled: true
                    propagateComposedEvents: true
                    cursorShape: Qt.PointingHandCursor
                    acceptedButtons: Qt.LeftButton

                    onClicked: Qt.openUrlExternally(modelData)
                }
            }

            Overte.ToolTip {
                text: modelData
                visible: embedMouseArea.containsMouse
            }

            Overte.Label {
                visible: embeddedImage.status !== Image.Ready
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                opacity: Overte.Theme.highContrast ? 1.0 : 0.6
                text: (
                    embeddedImage.status === Image.Loading ?
                    qsTr("Loading image… %1%%").arg(Math.floor(embeddedImage.progress * 100)) :
                    qsTr("Error loading image")
                )
            }
        }
    }
}
