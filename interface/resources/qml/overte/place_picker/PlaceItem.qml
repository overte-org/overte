import QtQuick
import QtQuick.Layouts

import ".." as Overte

Rectangle {
    id: item
    color: compatible ? Overte.Theme.paletteActive.alternateBase : Overte.Theme.paletteActive.buttonDestructive

    implicitWidth: gridView.cellWidth
    implicitHeight: gridView.cellHeight

    required property int index
    readonly property var modelData: gridView.model[index]

    required property string name
    readonly property string domainName: modelData.domain.name
    readonly property url thumbnail: modelData.thumbnail ?? ""
    readonly property bool compatible: modelData.compatibleProtocol ?? true
    readonly property url placeUrl: `hifi://${name}${modelData.path}`

    readonly property int currentUsers: modelData.current_attendance ?? 0
    readonly property int maxUsers: {
        const capacity = modelData.domain.capacity;
        return (capacity !== 0) ? capacity : 9999;
    }

    readonly property color textBackgroundColor: {
        if (Overte.Theme.highContrast) {
            return Overte.Theme.darkMode ? "black" : "white"
        } else {
            return Overte.Theme.darkMode ? "#d0000000" : "#e0ffffff"
        }
    }

    readonly property bool hovered: mouseArea.containsMouse || infoButton.hovered || joinButton.hovered

    Overte.Label {
        anchors.margins: Overte.Theme.borderWidth
        anchors.fill: item
        visible: thumbnailImage.status !== Image.Ready
        opacity: Overte.Theme.highContrast ? 1.0 : 0.6
        wrapMode: Text.Wrap
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: thumbnailImage.status === Image.Loading ? "Loading…" : "No icon"
    }

    Image {
        anchors.margins: Overte.Theme.borderWidth
        anchors.fill: item
        id: thumbnailImage

        source: thumbnail
        sourceSize.width: width
        sourceSize.height: height
        fillMode: Image.PreserveAspectCrop
    }

    Rectangle {
        anchors.margins: Overte.Theme.borderWidth
        anchors.top: item.top
        anchors.left: item.left

        width: Math.min(titleText.implicitWidth + 8, item.width - (2 * item.border.width))
        height: Math.min(titleText.implicitHeight + 8, item.height - (2 * item.border.width))
        color: textBackgroundColor

        Overte.Label {
            anchors.margins: 4
            anchors.fill: parent

            id: titleText
            text: name
            font.bold: true
            font.pixelSize: Overte.Theme.fontPixelSizeSmall
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignTop
        }
    }

    Rectangle {
        anchors.margins: Overte.Theme.borderWidth
        anchors.bottom: item.bottom
        anchors.left: item.left

        width: Math.min(userCountText.implicitWidth + 8, item.width - (2 * item.border.width))
        height: Math.min(userCountText.implicitHeight + 8, item.height - (2 * item.border.width))
        color: textBackgroundColor

        Overte.Label {
            anchors.margins: 4
            anchors.fill: parent

            id: userCountText
            text: {
                if (maxUsers === 9999) {
                    return `${domainName}: ${currentUsers}`;
                } else {
                    return `${domainName}: ${currentUsers}/${maxUsers}`;
                }
            }
            color: {
                if (currentUsers === 0) {
                    return Overte.Theme.paletteActive.userCountEmpty;
                } else if (currentUsers < maxUsers) {
                    return Overte.Theme.paletteActive.userCountActive;
                } else {
                    return Overte.Theme.paletteActive.userCountFull;
                }
            }
            font.bold: true
            font.pixelSize: Overte.Theme.fontPixelSizeXSmall
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignTop
            elide: Text.ElideLeft
        }
    }

    MouseArea {
        anchors.fill: item
        hoverEnabled: true
        propagateComposedEvents: true
        id: mouseArea
    }

    Overte.RoundButton {
        anchors.margins: 4
        anchors.top: item.top
        anchors.right: item.right
        id: infoButton

        implicitWidth: 32
        implicitHeight: 32
        horizontalPadding: 0
        verticalPadding: 0

        opacity: {
            if (item.hovered) {
                return Overte.Theme.highContrast || hovered ? 1.0 : 0.9;
            } else {
                return 0.0;
            }
        }
        backgroundColor: Overte.Theme.paletteActive.buttonInfo

        icon.source: "../icons/info.svg"
        icon.width: 24
        icon.height: 24
        icon.color: Overte.Theme.paletteActive.buttonText

        onClicked: infoDialog.open(index)
    }

    Overte.RoundButton {
        anchors.margins: 4
        anchors.bottom: item.bottom
        anchors.right: item.right
        id: joinButton

        implicitWidth: 40
        implicitHeight: 40
        horizontalPadding: 0
        verticalPadding: 0

        enabled: item.compatible
        visible: item.compatible
        opacity: {
            if (item.hovered) {
                return Overte.Theme.highContrast || hovered ? 1.0 : 0.9;
            } else {
                return 0.0;
            }
        }
        backgroundColor: Overte.Theme.paletteActive.buttonAdd

        icon.source: "../icons/triangle_right.svg"
        icon.width: 24
        icon.height: 24
        icon.color: Overte.Theme.paletteActive.buttonText

        onClicked: placePicker.goToLocation(placeUrl)
    }
}
