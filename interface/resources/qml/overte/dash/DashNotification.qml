import QtQuick
import QtQuick.Layouts

import ".." as Overte

Rectangle {
    id: bubble
    opacity: Overte.Theme.highContrast ? 1.0 : 0.95

    color: {
        let c = Overte.Theme.paletteActive.tooltip;
        if (!Overte.Theme.highContrast) { c.a = 128; }
        return c;
    }

    radius: Overte.Theme.borderRadius
    border.width: Overte.Theme.borderWidth
    border.color: (
        Overte.Theme.highContrast ?
        Overte.Theme.paletteActive.tooltipText :
        Qt.darker(Overte.Theme.paletteActive.tooltip, Overte.Theme.borderDarker)
    );

    implicitHeight: imageSource === "" ? 64 : 320
    implicitWidth: 480

    required property int index
    required property real lifetime
    required property string text
    required property string iconSource
    required property string imageSource

    signal expired()

    Timer {
        running: true
        interval: lifetime * 1000
        onTriggered: expired()
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: Overte.Theme.borderWidth

        id: lifetimeBar
        height: 4

        radius: Overte.Theme.borderRadius
        color: Overte.Theme.paletteActive.alternateBase

        Rectangle {
            radius: Overte.Theme.borderRadius
            color: Overte.Theme.paletteActive.highlight
            height: parent.height
            width: parent.width

            NumberAnimation on width {
                running: true
                from: parent?.width ?? 1
                to: 0
                duration: bubble.lifetime * 1000
            }
        }
    }

    ColumnLayout {
        anchors.top: lifetimeBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 8
        anchors.topMargin: 8 - lifetimeBar.height

        RowLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true

            AnimatedImage {
                Layout.fillHeight: true
                Layout.minimumWidth: 40
                Layout.maximumWidth: 40
                Layout.minimumHeight: 40
                Layout.maximumHeight: 40

                visible: source.toString() !== ""

                source: bubble.iconSource
                fillMode: Image.PreserveAspectFit
                horizontalAlignment: Image.AlignRight
                verticalAlignment: Image.AlignVCenter
                sourceSize.width: 40
                sourceSize.height: 40
            }

            Overte.Label {
                Layout.fillWidth: true
                Layout.fillHeight: true

                id: text

                text: bubble.text
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                // TODO: would it be worth exposing rich text to notifications?
                textFormat: Text.PlainText
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            implicitHeight: imageBody.height

            id: imageContainer

            color: Overte.Theme.paletteActive.alternateBase
            radius: Overte.Theme.borderRadius
            visible: bubble.imageSource !== ""

            AnimatedImage {
                id: imageBody
                anchors.fill: parent
                source: bubble.imageSource
                fillMode: Image.PreserveAspectFit
                horizontalAlignment: Image.AlignHCenter
                verticalAlignment: Image.AlignVCenter
            }
        }
    }
}
