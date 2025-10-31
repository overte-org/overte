import QtQuick
import QtQuick.Layouts

import ".." as Overte

Rectangle {
    id: item
    color: index % 2 === 0 ? Overte.Theme.paletteActive.base : Overte.Theme.paletteActive.alternateBase

    required property int index

    required property string name
    property string description: listView.model[index].description ?? ""
    property url thumbnail: listView.model[index].thumbnail ?? ""

    property int currentUsers: listView.model[index].current_attendance ?? 0
    property int maxUsers: {
        const capacity = listView.model[index].domain.capacity;
        return (capacity !== 0) ? capacity : 9999;
    }

    readonly property color textBackgroundColor: {
        if (Overte.Theme.highContrast) {
            return Overte.Theme.darkMode ? "black" : "white"
        } else {
            return Overte.Theme.darkMode ? "#80000000" : "#80ffffff"
        }
    }

    anchors.left: parent ? parent.left : undefined
    anchors.right: parent ? parent.right : undefined
    anchors.leftMargin: 4
    anchors.rightMargin: Overte.Theme.scrollbarWidth

    implicitHeight: 128

    Component.onCompleted: {
        // Hide redundant default descriptions that don't say anything
        if (description === `A place in ${name}`) {
            description = "";
        }
    }

    Image {
        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop
        visible: !Overte.Theme.highContrast
        source: thumbnail
    }

    ColumnLayout {
        anchors.left: item.left
        anchors.top: item.top
        anchors.bottom: item.bottom
        anchors.right: controls.left
        anchors.margins: 4
        spacing: 8

        Rectangle {
            Layout.alignment: Qt.AlignLeft | Qt.AlignBottom
            Layout.maximumWidth: parent.width
            implicitWidth: titleText.implicitWidth + 16
            implicitHeight: titleText.implicitHeight + 16
            color: textBackgroundColor
            radius: Overte.Theme.borderRadius

            Overte.Label {
                anchors.margins: 8
                anchors.fill: parent

                id: titleText
                text: name
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignTop
                style: Text.Outline
                styleColor: Overte.Theme.darkMode ? "black" : "white"
            }
        }

        Item { Layout.fillHeight: true }

        Rectangle {
            visible: description !== ""

            Layout.alignment: Qt.AlignLeft | Qt.AlignBottom
            Layout.maximumWidth: parent.width
            implicitWidth: descriptionText.implicitWidth + 16
            implicitHeight: descriptionText.implicitHeight + 16
            color: textBackgroundColor
            radius: Overte.Theme.borderRadius

            Overte.Label {
                anchors.margins: 8
                anchors.fill: parent

                id: descriptionText
                text: description
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignTop
                style: Text.Outline
                styleColor: Overte.Theme.darkMode ? "black" : "white"
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                font.pixelSize: Overte.Theme.fontPixelSizeSmall
            }
        }
    }

    ColumnLayout {
        id: controls
        anchors.top: item.top
        anchors.bottom: item.bottom
        anchors.right: item.right
        anchors.margins: 8
        spacing: 4

        Overte.RoundButton {
            Layout.alignment: Qt.AlignCenter
            text: ">"
            backgroundColor: Overte.Theme.paletteActive.buttonAdd
        }

        Overte.RoundButton {
            Layout.alignment: Qt.AlignCenter
            text: "P"
            backgroundColor: Overte.Theme.paletteActive.buttonInfo
        }

        Item { Layout.fillHeight: true }

        Rectangle {
            Layout.alignment: Qt.AlignCenter
            implicitWidth: 12
            implicitHeight: 12
            radius: width
            border.width: Overte.Theme.borderWidth
            border.color: Qt.darker(Overte.Theme.paletteActive.base, Overte.Theme.borderDarker)
            color: {
                if (currentUsers === 0) {
                    return "#808080";
                } else if (currentUsers < maxUsers) {
                    return "#00ff00";
                } else {
                    return "#ff0000";
                }
            }
        }
    }
}
