import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import "../widgets"

ColumnLayout {
	width: parent.width;
	height: parent.height;
	id: applicationListPage;
	spacing: 10;

	// Header
	RowLayout {
		Layout.margins: 10
		Layout.bottomMargin: 0;
		Layout.alignment: Qt.AlignVCenter;
		height: 60;

		Rectangle {
			color: "white";
			Layout.fillWidth: true;
			height: parent.height - 20;
			radius: 10;

			TextField {
				id: searchArea;
				placeholderText: "Search...";
				color: "black";
				anchors.fill: parent;

				onTextChanged: {
					onSearchChanged(searchArea.text);
				}

				background: Rectangle { // Custom background for the TextField
					color: "transparent" // Make it transparent
				}
			}
		}

		CustomButton {
			height: parent.height - 20;
			implicitWidth: 55;
			buttonText: "";
			buttonIcon: "../../img/menu.svg";

			onClickedFunc: () => { showSettingsPage() }
		}
	}

	// Scroll area
	Flickable {
		Layout.margins: 10;
		Layout.topMargin: 0;
		Layout.fillHeight: true;
		Layout.fillWidth: true;
		contentHeight: appListColumn.height;
		clip: true;
		
		Column {
			id: appListColumn
			width: parent.width;

			Repeater {
				model: appList.length;
				delegate: ApplicationListEntry { }
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
}
