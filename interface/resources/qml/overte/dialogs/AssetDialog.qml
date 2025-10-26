import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.folderlistmodel
import Qt.labs.qmlmodels

import ".."

Rectangle {
	id: dialog
	color: Theme.paletteActive.base

	property url selectedFile: ""
	property string searchExpression: ".*"

	readonly property var spawnableRegex: /\.(glb|fbx|fst|png|jpeg|jpg|webp)$/i

	ColumnLayout {
		anchors.fill: parent

		RowLayout {
			Layout.fillWidth: true
			Layout.margins: 4

			Button {
				backgroundColor: Theme.paletteActive.buttonAdd
				text: qsTr("Upload File")

				// TODO
				onClicked: {}
			}

			TextField {
				Layout.fillWidth: true
				Layout.preferredHeight: parent.height

				id: searchField
				placeholderText: qsTr("Search…")

				Keys.onEnterPressed: searchButton.click()
				Keys.onReturnPressed: searchButton.click()
			}

			RoundButton {
				id: searchButton
				icon.source: "../icons/search.svg"
				icon.width: 24
				icon.height: 24
				icon.color: Theme.paletteActive.buttonText

				onClicked: {
					searchExpression = searchField.text === "" ? /.*/ : new RegExp(searchField.text);
				}
			}
		}

		ScrollView {
			Layout.fillWidth: true
			Layout.fillHeight: true

			contentWidth: availableWidth
			rightPadding: ScrollBar.vertical.width

			background: Rectangle {
				color: Qt.darker(Theme.paletteActive.base, 1.2)
			}

			ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

			ScrollBar.vertical: ScrollBar {
				policy: ScrollBar.AlwaysOn
				interactive: true
				anchors.right: parent.right
				anchors.top: parent.top
				anchors.bottom: parent.bottom
			}

			TableView {
				id: tableView
				clip: true
				selectionBehavior: TableView.SelectRows
				selectionMode: TableView.SingleSelection
				pixelAligned: true
				rowSpacing: 1
				columnSpacing: 0
				boundsBehavior: Flickable.StopAtBounds

				model: TableModel {
					TableModelColumn {
						display: "fileName"
						textAlignment: () => Text.AlignLeft
					}
					TableModelColumn {
						display: "fileModified"
						textAlignment: () => Text.AlignRight
					}
					TableModelColumn {
						display: "fileSize"
						textAlignment: () => Text.AlignLeft
					}

					// TODO
					rows: [
						{ fileName: "bevy2.glb", fileModified: (new Date()).toLocaleDateString(null, Locale.ShortFormat), fileSize: "2.1 MiB" },
						{ fileName: "cheese.jpg", fileModified: (new Date()).toLocaleDateString(null, Locale.ShortFormat), fileSize: "12 KiB" },
						{ fileName: "metal pipe.wav", fileModified: (new Date()).toLocaleDateString(null, Locale.ShortFormat), fileSize: "200 KiB" },
					]
				}

				selectionModel: ItemSelectionModel {
					onCurrentChanged: (index, _) => {
						if (index.row === -1) { return; }

						const data = tableView.model.getRow(index.row);
						selectedFile = data.fileName;
					}
				}

				delegate: Rectangle {
					required property bool selected
					required property bool current
					required property int row

					readonly property bool rowCurrent: tableView.currentRow === row

					color: (
						rowCurrent ?
						Theme.paletteActive.highlight :
						(row % 2 === 0 ? Theme.paletteActive.base : Theme.paletteActive.alternateBase)
					)
					implicitHeight: Theme.fontPixelSize * 2
					implicitWidth: {
						let nameWidth = tableView.width;
						let mtimeWidth = tableView.width * (1 / 4);
						let sizeWidth = tableView.width * (1 / 4);

						// qt doesn't let us do stretchy columns so emulate it ourselves
						nameWidth -= sizeWidth;
						nameWidth -= mtimeWidth;

						// hide the mtime column if the window isn't big enough to fit it comfortably
						if (tableView.width < 720) {
							// can't be zero or qt complains
							nameWidth += mtimeWidth;
							mtimeWidth = 1;
						}

						switch (column) {
							case 0: return nameWidth;
							case 1: return mtimeWidth;
							case 2: return sizeWidth;
						}
					}

					Text {
						anchors.margins: 8
						anchors.fill: parent

						verticalAlignment: Text.AlignVCenter
						horizontalAlignment: column === 2 ? Text.AlignRight : Text.AlignLeft
						font.family: Theme.fontFamily
						font.pixelSize: Theme.fontPixelSize
						color: rowCurrent ? Theme.paletteActive.highlightedText : Theme.paletteActive.text
						text: display
					}
				}
			}
		}

		GridLayout {
			rows: 2
			columns: 2
			Layout.fillWidth: true
			Layout.margins: 8

			Button {
				Layout.fillWidth: true
				Layout.preferredWidth: 1

				enabled: tableView.currentRow !== -1
				text: qsTr("Delete")
				backgroundColor: Theme.paletteActive.buttonDestructive

				// TODO
				onClicked: {
					console.log("Delete", tableView.model.data(tableView.model.index(tableView.currentRow, 0)));
				}
			}

			Button {
				Layout.fillWidth: true
				Layout.preferredWidth: 1

				enabled: tableView.currentRow !== -1
				text: qsTr("Copy Link")
				backgroundColor: Theme.paletteActive.buttonInfo

				// TODO
				onClicked: {
					console.log("Copy Link", tableView.model.data(tableView.model.index(tableView.currentRow, 0)));
				}
			}

			Button {
				Layout.fillWidth: true
				Layout.preferredWidth: 1

				enabled: tableView.currentRow !== -1
				text: qsTr("Rename")

				// TODO
				onClicked: {
					console.log("Rename", tableView.model.data(tableView.model.index(tableView.currentRow, 0)));
				}
			}

			Button {
				Layout.fillWidth: true
				Layout.preferredWidth: 1

				enabled: {
					if (tableView.currentRow === -1) { return false; }

					const index = tableView.model.index(tableView.currentRow, 0);
					const data = tableView.model.data(index);
					return !!data.match(spawnableRegex);
				}
				text: qsTr("Create Entity")
				backgroundColor: Theme.paletteActive.buttonAdd

				// TODO
				onClicked: {
					console.log("Create Entity", tableView.model.data(tableView.model.index(tableView.currentRow, 0)));
				}
			}
		}
	}

	MessageDialog {
		id: replaceWarningDialog
		anchors.fill: parent
	}
}
