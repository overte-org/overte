import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../" as Overte
import "."

Column {
    id: entityEditor
    spacing: 4

    required property var model

    Overte.Label {
        width: entityEditor.width
        font.pixelSize: Overte.Theme.fontPixelSizeSmall
        wrapMode: Text.Wrap
        text: qsTr("Currently, only Model entities can be added from here. In the future, you will be able to create any entity type through the Create app.")
    }

    Overte.Label {
        width: entityEditor.width
        text: qsTr("Entity name")
    }

    Overte.TextField {
        width: entityEditor.width
        id: addEntityName
    }

    Overte.Label {
        width: entityEditor.width
        text: qsTr("Model URL")
    }

    Overte.TextField {
        width: entityEditor.width
        id: addEntityURL
    }

    RowLayout {
        width: entityEditor.width

        Overte.Label {
            Layout.fillWidth: true
            text: qsTr("Skin model to avatar skeleton")
        }

        Overte.Switch {
            id: addEntitySkinned
        }
    }

    Overte.Label {
        visible: !addEntitySkinned.checked
        width: entityEditor.width
        text: qsTr("Parent joint name or index")
    }

    Overte.TextField {
        visible: !addEntitySkinned.checked
        width: entityEditor.width
        id: addEntityJoint
    }

    Overte.Button {
        width: entityEditor.width
        text: qsTr("Add entity")

        icon.source: "../icons/plus.svg"
        icon.width: 24
        icon.height: 24
        icon.color: Overte.Theme.paletteActive.buttonText
        backgroundColor: Overte.Theme.paletteActive.buttonAdd

        enabled: addEntityName.text.trim() !== "" && addEntityURL.text.trim() !== ""

        onClicked: {
            let joint = undefined;

            if (!addEntitySkinned.checked) {
                joint = MyAvatar.getJointIndex(addEntityJoint.text);

                if (joint === -1) {
                    if (addEntityJoint.text.match(/[0-9]+/)) {
                        joint = parseInt(addEntityJoint.text);
                    } else {
                        joint = undefined;
                    }
                }
            }

            entityEditor.model.push({
                properties: {
                    id: Uuid.generate(),
                    type: "Model",
                    name: addEntityName.text,
                    parentID: MyAvatar.SELF_ID,
                    parentJointIndex: joint,
                    modelURL: addEntityURL.text,
                    relayParentJoints: addEntitySkinned.checked,
                    useOriginalPivot: true,
                },
            });

            // force an update on the repeater, since
            // JS arrays don't trigger change signals
            entityEditor.modelChanged();

            addEntityName.text = "";
            addEntityURL.text = "";
            addEntityJoint.text = "";
            addEntitySkinned.checked = false;
        }
    }

    Overte.Ruler { width: entityEditor.width }

    Repeater {
        model: entityEditor.model

        RowLayout {
            width: entityEditor.width

            required property int index
            required property var modelData

            Column {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Overte.Label {
                    width: parent.width
                    elide: Text.ElideLeft
                    text: {
                        if (modelData.name) {
                            return modelData.name;
                        } else {
                            return modelData.id ?? "No ID";
                        }
                    }
                }

                Overte.Label {
                    width: parent.width
                    opacity: Overte.Theme.highContrast ? 1.0 : 0.6
                    font.pixelSize: Overte.Theme.fontPixelSizeSmall
                    elide: Text.ElideRight
                    text: {
                        if (
                            modelData.type === "Model" &&
                            modelData.modelURL
                        ) {
                            return modelData.modelURL;
                        } else {
                            return modelData.type ?? "Unknown";
                        }
                    }
                }
            }

            Overte.RoundButton {
                backgroundColor: Overte.Theme.paletteActive.buttonDestructive

                icon.source: "../icons/delete.svg"
                icon.width: 24
                icon.height: 24

                implicitWidth: 32
                implicitHeight: 32

                horizontalPadding: 0
                verticalPadding: 0

                onClicked: {
                    entityEditor.model.splice(index, 1);

                    // force an update on the repeater, since
                    // JS arrays don't trigger change signals
                    entityEditor.modelChanged();
                }
            }
        }
    }
}
