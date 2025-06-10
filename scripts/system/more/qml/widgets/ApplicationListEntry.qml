import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11

Rectangle {
	readonly property var maturityColors: {
		"STABLE": "gray",
		"BETA": "lightblue",
		"ALPHA": "yellow"
	}

	width: parent.width;
	height: 75;

	color: colors.darkBackground2;

	RowLayout {
		anchors.fill: parent;

		// Application icon
		AppImage {
			Layout.leftMargin: 10;
			icon: appList[index].appIcon;
		}

		// Application information
		ColumnLayout {
			Layout.fillWidth: true;
			height: parent.height - 50;
			Layout.leftMargin: 10;

			Text {
				text: appList[index].appName;
				color: "white";
				font.pixelSize: 20;
			}

			Text {
				text: "By: " + appList[index].appAuthor;
				color: "gray";
				font.pixelSize: 18;
			}

			// Pad
			Item {
				Layout.fillHeight: true;
			}
		}

		// Pad
		Item {
			Layout.fillHeight: true;
			Layout.fillWidth: true;
		}

		ColumnLayout {
			Layout.fillWidth: true;
			Layout.fillHeight: true;
			Layout.leftMargin: 10;
			height: parent.height - 50;

			Text {
				visible: appList[index].isInstalled;
				text: "( Installed )";
				color: "gray";
				font.pixelSize: 16;
			}

			// Pad
			Item {
				Layout.fillHeight: true;
			}
		}

		Item {
			Layout.fillHeight: true;
			width: 10;
		}
	}

	MouseArea {
		anchors.fill: parent;
		hoverEnabled: true;
		propagateComposedEvents: true;	

		onPressed: {
			showAppDetailPage();
			focusedAppIndex = index;
		}

		onEntered: {
			parent.color = colors.darkBackground3;

		}

		onExited: {
			parent.color = colors.darkBackground2;
		}
	}

	Behavior on color {
		ColorAnimation {
			duration: 50;
			easing.type: Easing.InOutCubic;
		}
	}
}