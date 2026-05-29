import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../" as Overte
import "."

ColumnLayout {
    id: editor

    required property string bookmarkToReplace
    required property string avatarName
    required property string avatarURL
    required property real avatarScale
    required property string iconURL
    required property var entityData

    ScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true

        ScrollBar.vertical: Overte.ScrollBar {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            policy: ScrollBar.AsNeeded
        }
        contentWidth: width - ScrollBar.vertical.width

        component TextSetting: Column {
            anchors.left: column.left
            anchors.right: column.right
            spacing: 4

            required property string label
            property string description
            property alias text: inputField.text
            property alias enabled: inputField.enabled

            Overte.Label {
                anchors.left: parent.left
                anchors.right: parent.right
                wrapMode: Text.Wrap
                text: parent.label
            }

            Overte.Label {
                anchors.left: parent.left
                anchors.right: parent.right
                font.pixelSize: Overte.Theme.fontPixelSizeSmall
                wrapMode: Text.Wrap
                text: parent.description
                visible: text !== ""
            }

            Overte.TextField {
                anchors.left: parent.left
                anchors.right: parent.right
                id: inputField
            }
        }

        component SliderSetting: Column {
            anchors.left: column.left
            anchors.right: column.right
            spacing: 4

            required property string label
            property string description
            property alias from: inputSlider.from
            property alias to: inputSlider.to
            property alias value: inputSlider.value
            property alias stepSize: inputSlider.stepSize
            property alias snapMode: inputSlider.snapMode

            Overte.Label {
                anchors.left: parent.left
                anchors.right: parent.right
                wrapMode: Text.Wrap
                text: parent.label
            }

            Overte.Label {
                anchors.left: parent.left
                anchors.right: parent.right
                font.pixelSize: Overte.Theme.fontPixelSizeSmall
                wrapMode: Text.Wrap
                text: parent.description
                visible: text !== ""
            }

            Overte.Slider {
                anchors.left: parent.left
                anchors.right: parent.right
                id: inputSlider
            }

            Overte.Label {
                anchors.left: parent.left
                anchors.right: parent.right
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: Overte.Theme.fontPixelSizeSmall
                text: inputSlider.value.toFixed(1)
            }
        }

        Column {
            anchors.fill: parent
            anchors.margins: 8

            id: column
            spacing: Overte.Theme.fontPixelSize
            padding: 8

            Column {
                anchors.left: column.left
                anchors.right: column.right
                visible: bookmarkToReplace === ""
                spacing: 4

                Overte.Label {
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    opacity: Overte.Theme.highContrast ? 1.0 : 0.6
                    text: qsTr("Editing your current avatar info")
                }

                Overte.Ruler { width: parent.width }
            }

            TextSetting {
                id: settingName
                label: qsTr("Name")
                text: editor.avatarName
                enabled: bookmarkToReplace !== ""
            }

            TextSetting {
                id: settingURL
                label: qsTr("Model URL")
                description: qsTr("Supported formats are FST, glTF, and FBX. FST files contain metadata about an avatar, such as its name, material overrides, and flow bone properties.")
                text: editor.avatarURL
            }

            TextSetting {
                id: settingIconURL
                label: qsTr("Icon URL (optional)")
                description: qsTr("If left empty, the game will attempt to use a .jpg file with the same name and path as the model URL.")
                text: editor.iconURL
            }

            SliderSetting {
                id: settingScale
                label: qsTr("Model scale")
                description: qsTr("A scale of 1 uses the avatar model's natural height. Some domains may have their own avatar scale limits. Note that very large or very small avatars may exhibit buggy behavior.")
                value: editor.avatarScale
                stepSize: 0.1
                from: 0.1
                to: 5.0
            }

            Column {
                anchors.left: column.left
                anchors.right: column.right
                spacing: 4

                Overte.Label {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    wrapMode: Text.Wrap
                    text: qsTr("Avatar Entities")
                }

                Overte.Label {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    font.pixelSize: Overte.Theme.fontPixelSizeSmall
                    wrapMode: Text.Wrap
                    text: qsTr("Avatar entities follow you across domains and are saved as part of the avatar settings. Avatar entities are commonly used for clothes and other wearables. You can edit your avatar entities in further detail using the Create app. Note that some domains may not allow avatar entities.")
                }

                Overte.Ruler {
                    anchors.left: parent.left
                    anchors.right: parent.right
                }

                EntityEditor {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    model: editor.entityData
                    id: entityEditor
                }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true

        Overte.Button {
            // make the buttons equal size
            Layout.fillWidth: true
            Layout.preferredWidth: 1

            text: qsTr("Cancel")
            onClicked: stack.pop()
        }

        Item {
            // make the buttons equal size
            Layout.fillWidth: true
            Layout.preferredWidth: 1
        }

        Overte.Button {
            // make the buttons equal size
            Layout.fillWidth: true
            Layout.preferredWidth: 1

            backgroundColor: Overte.Theme.paletteActive.buttonAdd
            text: bookmarkToReplace !== "" ? qsTr("Save") : qsTr("Apply")

            onClicked: {
                if (bookmarkToReplace !== "") {
                    // AvatarBookmarks has its own separate format,
                    // { properties: { id } }
                    let entityData = [];

                    for (let item of entityEditor.model) {
                        entityData.push({ properties: item });
                    }

                    // editing an existing bookmark
                    AvatarBookmarks.setBookmarkData(bookmarkToReplace, {
                        version: 3,
                        avatarUrl: settingURL.text,
                        avatarIcon: settingIconURL.text,
                        avatarScale: settingScale.value,
                        // FIXME: yes, it's "avatarEntites", not "avatarEntities"
                        avatarEntites: entityData,
                    });
                } else {
                    // editing the current unsaved avatar state
                    MyAvatar.skeletonModelURL = settingURL.text;
                    MyAvatar.scale = settingScale.value;

                    // marked as deprecated, but this is the only way
                    // of editing avatar entities in QML, since QVariantMap
                    // isn't convertible to EntityItemProperties

                    // updateAvatarEntities has its own separate format,
                    // { id, properties: {} }
                    let entityData = [];

                    for (let item of entityEditor.model) {
                        let props = {};
                        Object.assign(props, item);
                        delete props.id;

                        entityData.push({
                            id: item.id,
                            properties: props,
                        });
                    }

                    AvatarBookmarks.updateAvatarEntities(entityData);
                }

                stack.pop();
            }
        }
    }
}
