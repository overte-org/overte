import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import "../."

Item {
	width: parent.width;

	property var entries: [];

	property bool canAddEntries: true;
	property bool canDeleteEntries: true;

	property var onAddEntryButtonClickedFunc;

	ColumnLayout {
		width: parent.width;
		height: parent.height;

		Flickable {
			contentHeight: Math.min(entryListElement.height, 200);
			Layout.fillHeight: true;
			width: parent.width;
			clip: true;

			Column {
				width: parent.width - 10;
				id: entryListElement;

				Repeater {
					model: entries.length;
					delegate: CustomListElement {
						entryText: entries[index].entryText;
						canAddEntries: canAddEntries;
						canDeleteEntries: canDeleteEntries;
					}
				}
			}

			ScrollBar.vertical: ScrollBar {
				policy: Qt.ScrollBarAlwaysOn;

				background: Rectangle {
					color: "transparent";
					radius: 5;
					visible: parent.visible;
				}
			}
		}

		CustomButton {
			height: 40;
			Layout.fillWidth: true;
			buttonText: "Add Entry";

			onClickedFunc: onAddEntryButtonClickedFunc;
			Layout.bottomMargin: 10;
		}
	}
}