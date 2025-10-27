import QtQuick
import QtQuick.Effects

import ".." as Overte

Rectangle {
    required property url source
    required property int status

    color: "transparent"

    implicitWidth: 64 + border.width
    implicitHeight: 64 + border.width

    id: avatar
    radius: Overte.Theme.borderRadius
    border.width: Math.max(2, Overte.Theme.borderWidth)
    border.color: {
        switch (status) {
            case -1: return Overte.Theme.paletteActive.statusOffline;
            case 0: return Overte.Theme.paletteActive.statusOffline;
            case 1: return Overte.Theme.paletteActive.statusFriendsOnly;
            case 2: return Overte.Theme.paletteActive.statusContacts;
            case 3: return Overte.Theme.paletteActive.statusEveryone;
        }
    }

    Image {
        anchors.fill: avatar
        anchors.margins: avatar.border.width
        fillMode: Image.PreserveAspectCrop

        id: avatarImage
        source: avatar.source

        layer.enabled: true
        layer.effect: MultiEffect {
            anchors.fill: avatarImage
            source: avatarImage
            maskEnabled: true
            maskSource: mask
        }
    }

    Item {
        id: mask
        anchors.fill: avatar
        visible: false

        layer.enabled: true
        Rectangle {
            anchors.fill: parent
            radius: avatar.radius
            color: "black"
        }
    }
}
