import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs as QtDialogs

import "../" as Overte
import "."

Rectangle {
    id: root
    anchors.fill: parent
    color: Overte.Theme.paletteActive.base
    implicitWidth: 480
    implicitHeight: 720

    property bool editable: true
    property list<string> availableTags: []
    property string searchExpression: ".*"
    property var avatarModel: []

    function updateBookmarkModel() {
        const data = AvatarBookmarks.getBookmarks();
        let tmp = [];

        for (const [name, avatar] of Object.entries(data)) {
            let iconUrl;

            if (avatar.avatarIcon !== "") {
                iconUrl = avatar.avatarIcon;
            } else {
                iconUrl = new URL(avatar.avatarUrl);
                iconUrl.pathname = iconUrl.pathname.replace(/[.](?:fst|glb|fbx|vrm)$/i, ".jpg");
                iconUrl = iconUrl.toString();
            }

            tmp.push({
                name: name,
                avatarUrl: avatar.avatarUrl,
                iconUrl: iconUrl,
            });
        }

        tmp.sort((a, b) => a.name.localeCompare(b.name));

        avatarModel = tmp;
    }

    Component.onCompleted: updateBookmarkModel()

    Connections {
        target: AvatarBookmarks

        function onBookmarkAdded() { updateBookmarkModel(); }
        function onBookmarkDeleted() { updateBookmarkModel(); }
    }

    function loadBookmark(name) {
        AvatarBookmarks.loadBookmark(name);
        currentName.text = name;
    }

    Overte.StackView {
        id: stack
        anchors.fill: parent

        initialItem: MainPage {}
    }

    property int requestedDeleteIndex: -1

    function requestDelete(index, name) {
        requestedDeleteIndex = index;
        deleteWarningDialog.text = qsTr("Are you sure you want to delete %1?").arg(name);
        deleteWarningDialog.open();
    }

    function requestEdit(index) {
        if (index === -1) {
            // getAvatarEntitiesVariant has its own separate format,
            // { id, properties: {} }
            let entityDataRaw = MyAvatar.getAvatarEntitiesVariant();
            let entityData = [];

            for (let entry of entityDataRaw) {
                let data = entry.properties;
                data.id = entry.id;
                entityData.push(data);
            }

            stack.push("./EditorPage.qml", {
                bookmarkToReplace: "",
                avatarName: MyAvatar.getFullAvatarModelName(),
                avatarURL: MyAvatar.skeletonModelURL,
                avatarScale: MyAvatar.scale,
                iconURL: "",
                entityData: entityData,
            });
        } else {
            // AvatarBookmarks has its own separate format,
            // { properties: { id } }
            const name = avatarModel[index].name;
            const data = AvatarBookmarks.getBookmark(name);

            const modelURL = data?.avatarUrl ?? "";
            const iconURL = data?.avatarIcon ?? "";
            const scale = data?.avatarScale ?? 1.0;

            // FIXME: yes, it's "avatarEntites", not "avatarEntities"
            let entityData = (data?.avatarEntites ?? []).map(e => e.properties);

            stack.push("./EditorPage.qml", {
                bookmarkToReplace: name,
                avatarName: name,
                avatarURL: modelURL,
                avatarScale: scale,
                iconURL: iconURL,
                entityData: entityData,
            });
        }
    }

    Overte.MessageDialog {
        id: deleteWarningDialog
        anchors.fill: parent
        buttons: QtDialogs.MessageDialog.Yes | QtDialogs.MessageDialog.No

        onAccepted: {
            AvatarBookmarks.removeBookmark(avatarModel[requestedDeleteIndex].name);
            requestedDeleteIndex = -1;
        }
    }

    Overte.Dialog {
        id: addNewDialog
        anchors.fill: parent
        maxWidth: -1

        property string avatarName: ""
        property string avatarUrl: ""

        signal accepted
        signal rejected

        onAccepted: {
            const prevData = AvatarBookmarks.getBookmark(addNewDialog.avatarName);

            if (addNewDialog.avatarName !== avatarNameField.text) {
                AvatarBookmarks.removeBookmark(addNewDialog.avatarName);
            }

            // Qt's V4 doesn't support { ...spread } syntax :(
            let newData = prevData;

            if (avatarUrlField.text !== "") {
                newData.avatarUrl = avatarUrlField.text;
            }

            AvatarBookmarks.setBookmarkData(avatarNameField.text, newData);

            avatarName = "";
            avatarUrl = "";
            avatarNameField.text = "";
            avatarUrlField.text = "";
            close();
        }

        onRejected: {
            avatarName = "";
            avatarUrl = "";
            avatarNameField.text = "";
            avatarUrlField.text = "";
            close();
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8

            Overte.Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                opacity: Overte.Theme.highContrast ? 1.0 : 0.6
                text: qsTr("Add new avatar")
            }
            Overte.Ruler { Layout.fillWidth: true }

            Overte.TextField {
                Layout.fillWidth: true
                placeholderText: qsTr("Avatar name")
                text: addNewDialog.avatarName
                id: avatarNameField
            }

            Overte.TextField {
                Layout.fillWidth: true
                placeholderText: qsTr("Avatar URL (.fst, .glb, .vrm, .fbx)")
                text: addNewDialog.avatarUrl
                id: avatarUrlField
            }

            RowLayout {
                Layout.preferredWidth: 720
                Layout.fillWidth: true

                Overte.Button {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    text: qsTr("Cancel")

                    onClicked: addNewDialog.rejected()
                }

                Overte.Button {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1

                    backgroundColor: Overte.Theme.paletteActive.buttonAdd
                    enabled: avatarUrlField.text.trim() === "" || avatarNameField.text.trim() === ""
                    text: qsTr("Add Current")

                    onClicked: {
                        avatarUrlField.text = MyAvatar.skeletonModelURL;
                        addNewDialog.accepted();
                    }
                }

                Overte.Button {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1

                    backgroundColor: Overte.Theme.paletteActive.buttonAdd
                    text: qsTr("Add")
                    enabled: avatarNameField.text !== "" && avatarUrlField.text !== ""

                    onClicked: addNewDialog.accepted()
                }
            }
        }
    }
}
