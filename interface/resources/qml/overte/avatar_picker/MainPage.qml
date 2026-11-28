import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../" as Overte
import "."

ColumnLayout {
    RowLayout {
        Image {
            Layout.preferredWidth: 64
            Layout.preferredHeight: 64
            Layout.margins: 4
            id: currentIcon

            source: {
                const meshURL = MyAvatar.skeletonModelURL;
                let iconUrl = new URL(meshURL);
                iconUrl.pathname = iconUrl.pathname.replace(/[.](?:fst|glb|fbx|vrm)$/i, ".jpg");
                return iconUrl;
            }

            fillMode: Image.PreserveAspectFit
            sourceSize.width: width
            sourceSize.height: height

            // placeholder icon
            Image {
                anchors.fill: parent

                fillMode: Image.PreserveAspectFit
                source: "../icons/avatars.png"
                sourceSize.width: width
                sourceSize.height: height
                visible: currentIcon.status === Image.Error || currentIcon.status === Image.Null
            }
        }

        Overte.Label {
            Layout.margins: 4
            Layout.fillWidth: true
            id: currentName

            verticalAlignment: Text.AlignVCenter
            text: {
                const name = MyAvatar.getFullAvatarModelName();

                if (name !== "") {
                    return name;
                } else {
                    return qsTr("Unnamed");
                }
            }

            Connections {
                target: MyAvatar

                function onSkeletonModelURLChanged() {
                    const name = MyAvatar.getFullAvatarModelName();

                    if (name !== "") {
                        currentName.text = name;
                    } else {
                        currentName.text = qsTr("Unnamed");
                    }

                    console.info(name);
                }
            }
        }

        Overte.RoundButton {
            Layout.margins: 4

            Overte.ToolTip { text: qsTr("Edit current") }

            icon.source: "../icons/pencil.svg"
            icon.width: 32
            icon.height: 32
            icon.color: Overte.Theme.paletteActive.buttonText
            backgroundColor: Overte.Theme.paletteActive.buttonInfo

            implicitWidth: 44
            implicitHeight: 44

            onClicked: root.requestEdit(-1)
        }
    }

    RowLayout {
        Layout.margins: 4

        Overte.TextField {
            Layout.fillWidth: true

            id: searchField
            placeholderText: qsTr("Search…")

            Keys.onEnterPressed: {
                searchButton.click();
                forceActiveFocus();
            }
            Keys.onReturnPressed: {
                searchButton.click();
                forceActiveFocus();
            }
        }

        Overte.RoundButton {
            id: searchButton
            icon.source: "../icons/search.svg"
            icon.width: 24
            icon.height: 24
            icon.color: Overte.Theme.paletteActive.buttonText

            onClicked: {
                searchExpression = searchField.text === "" ? ".*" : searchField.text;
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.leftMargin: 8
        Layout.rightMargin: 8
        visible: root.availableTags.length !== 0

        Overte.Label {
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            text: qsTr("Tags")
        }

        ListView {
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.fillWidth: true
            implicitHeight: Overte.Theme.fontPixelSize * 2
            orientation: Qt.Horizontal
            spacing: 2
            clip: true

            model: root.availableTags
            delegate: Overte.Button {
                required property int index

                implicitHeight: Overte.Theme.fontPixelSize * 2
                text: qsTr(ListView.view.model[index])
                checkable: true
                checked: true

                palette.buttonText: checked ? Overte.Theme.paletteActive.highlightedText : Overte.Theme.paletteActive.button
                backgroundColor: checked ? Overte.Theme.paletteActive.highlight : Overte.Theme.paletteActive.button
            }
        }
    }

    GridView {
        Layout.fillWidth: true
        Layout.fillHeight: true

        // QT6TODO: remove this once mouse inputs work properly
        interactive: false

        clip: true
        // scales the cells to never leave dead space, but looks bad when scaling window
        //cellWidth: (width - ScrollBar.vertical.width) / Math.floor(3 * (width / 480))
        cellWidth: Math.floor((480 - ScrollBar.vertical.width) / 3)
        cellHeight: cellWidth + Overte.Theme.fontPixelSize + 6

        ScrollBar.vertical: Overte.ScrollBar {
            policy: ScrollBar.AsNeeded
            interactive: true
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
        }

        delegate: AvatarItem {}

        model: {
            const searchRegex = new RegExp(searchExpression, "i");
            let tmp = [];

            for (const item of root.avatarModel) {
                if (item.name.match(searchRegex)) {
                    let modelItem = item;

                    if (!modelItem.iconUrl) {
                        modelItem.iconUrl = "../icons/avatars.svg";
                    }

                    if (!modelItem.tags) { modelItem.tags = []; }

                    tmp.push(modelItem);
                }
            }

            return tmp;
        }
    }

    RowLayout {
        Layout.margins: 4
        spacing: 8

        Overte.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            text: qsTr("%1 avatar(s)").arg(avatarModel.length)
        }

        Overte.Label {
            Layout.fillWidth: true
            visible: editable
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            text: qsTr("Add new avatar")
        }

        Overte.RoundButton {
            icon.source: "../icons/plus.svg"
            icon.width: 24
            icon.height: 24
            icon.color: Overte.Theme.paletteActive.buttonText
            backgroundColor: Overte.Theme.paletteActive.buttonAdd
            implicitWidth: 48
            implicitHeight: 48
            visible: editable

            onClicked: addNewDialog.open()
        }
    }
}
