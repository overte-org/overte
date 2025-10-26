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
            // TODO: replace this kinda hacky thing we currently do
            // with real URLs stored either in the FST or bookmark entry
            let iconUrl = new URL(avatar.avatarUrl); 
            iconUrl.pathname = iconUrl.pathname.replace(/[.](?:fst|glb|fbx|vrm)$/i, ".jpg");

            tmp.push({
                name: name,
                avatarUrl: avatar.avatarUrl,
                iconUrl: iconUrl.toString(),
                description: "",
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

	ColumnLayout {
		anchors.fill: parent

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
							modelItem.iconUrl = "../icons/no_avatar_icon.svg";
						}

						if (!modelItem.tags) { modelItem.tags = []; }
						if (!modelItem.description) { modelItem.description = ""; }

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

				onClicked: {
					editDialog.editExisting = false;
					editDialog.avatarName = "";
					editDialog.avatarUrl = "";
					editDialog.avatarDescription = "";

                    avatarNameField.text = "";
                    avatarUrlField.text = "";
                    avatarDescriptionField.text = "";

					editDialog.open();
				}
			}
		}
	}

	property int requestedDeleteIndex: -1
	property int requestedEditIndex: -1

	function requestDelete(index, name) {
		requestedDeleteIndex = index;
		deleteWarningDialog.text = qsTr("Are you sure you want to delete %1?").arg(name);
		deleteWarningDialog.open();
	}

	function requestEdit(index) {
		requestedEditIndex = index;
		editDialog.editExisting = true;
		editDialog.avatarName = avatarModel[index].name;
		editDialog.avatarUrl = avatarModel[index].avatarUrl;
		editDialog.avatarDescription = avatarModel[index].description;
		editDialog.open();
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
		id: editDialog
		anchors.fill: parent
		maxWidth: -1

		property bool editExisting: false
		property string avatarName: ""
		property string avatarUrl: ""
		property string avatarDescription: ""

		signal accepted
		signal rejected

		onAccepted: {
            if (editExisting) {
                AvatarBookmarks.removeBookmark(editDialog.avatarName);
            }

            AvatarBookmarks.addBookmark(avatarNameField.text, editDialog.avatarUrl);
            close();
		}

		onRejected: close()

		ColumnLayout {
			anchors.fill: parent
			anchors.margins: 8

			Overte.Label {
				Layout.fillWidth: true
				horizontalAlignment: Text.AlignHCenter
				opacity: Overte.Theme.highContrast ? 1.0 : 0.6
				text: editDialog.editExisting ? qsTr("Edit avatar") : qsTr("Add new avatar")
			}
			Overte.Ruler { Layout.fillWidth: true }

			Overte.TextField {
				Layout.fillWidth: true
				placeholderText: qsTr("Avatar name")
				text: editDialog.avatarName
				id: avatarNameField
			}

			Overte.TextField {
				Layout.fillWidth: true
				placeholderText: qsTr("Avatar URL (.fst, .glb, .vrm, .fbx)")
				text: editDialog.avatarUrl
				id: avatarUrlField
			}

			ScrollView {
                // TODO: support avatar descriptions
                visible: false

				Layout.preferredHeight: Overte.Theme.fontPixelSize * 8
				Layout.fillWidth: true

				ScrollBar.vertical: Overte.ScrollBar {
					interactive: false
					anchors.right: parent.right
					anchors.top: parent.top
					anchors.bottom: parent.bottom
				}

				contentWidth: availableWidth

				Overte.TextArea {
					id: avatarDescriptionField
					placeholderText: qsTr("Description (optional)")
					text: editDialog.avatarDescription
					wrapMode: Text.Wrap
					font.pixelSize: Overte.Theme.fontPixelSizeSmall
				}
			}

			RowLayout {
				Layout.preferredWidth: 720
				Layout.fillWidth: true

				Overte.Button {
					Layout.fillWidth: true
					Layout.preferredWidth: 1
					text: qsTr("Cancel")

					onClicked: {
						editDialog.rejected();
						requestedEditIndex = -1;
					}
				}

				Item {
                    visible: editDialog.editExisting
					Layout.preferredWidth: 1
					Layout.fillWidth: true
				}

				Overte.Button {
                    visible: !editDialog.editExisting
					Layout.fillWidth: true
					Layout.preferredWidth: 1

					enabled: avatarNameField.text !== ""
					text: qsTr("Add Current")

					onClicked: {
                        avatarUrlField.text = MyAvatar.skeletonModelURL;
						editDialog.accepted();
						requestedEditIndex = -1;
					}
				}

				Overte.Button {
					Layout.fillWidth: true
					Layout.preferredWidth: 1

					backgroundColor: Overte.Theme.paletteActive.buttonAdd
					text: editDialog.editExisting ? qsTr("Apply") : qsTr("Add")
					enabled: avatarNameField.text !== "" && avatarUrlField.text !== ""

					onClicked: {
						editDialog.accepted();
						requestedEditIndex = -1;
					}
				}
			}
		}
	}
}
