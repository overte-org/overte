import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11
import "../."

Item {
	width: parent.width;
	height: 50;

	property bool canAddEntries: true;
	property bool canDeleteEntries: true;
	property string entryText: "";

	Rectangle {
		color: colors.darkBackground3;
		width: parent.width;
		height: parent.height;

		Item {
			width: parent.width - 10;
			height: parent.height;
			anchors.centerIn: parent;

			RowLayout {
				width: parent.width;
				height: parent.height;
				spacing: 10;

				Text {
					text: entryText;
					color: "white";
					Layout.fillWidth: true;
					id: entryTextComponent;
					wrapMode: Text.Wrap;
				}

				CustomButton {
					id: deleteEntryComponent;
					width: 25;
					buttonText: "X";
					visible: canDeleteEntries;

					onClickedFunc: () => { onRemoveEntryButton(entryText) };
				}
			}
		}
	}
}