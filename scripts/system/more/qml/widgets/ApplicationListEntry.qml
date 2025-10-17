import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11

Rectangle {
	width: parent.width;
	height: 80;

    // JS function inside QML
    function truncateText(str, maxLength){
        let cleaned = str.replace(/<br\s*\/?>/gi, '');
        if (cleaned.length > maxLength) {
            return cleaned.slice(0, maxLength) + '\u2026';
        } else {
            return cleaned;
        }
    }

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
				text: truncateText(appList[index].appDescription, 55);
				color: colors.lightText2;
				font.pixelSize: 12;
			}

			Text {
				text: "By: " + appList[index].appAuthor;
				color: colors.lightText2;
				font.pixelSize: 14;
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
				text: appList[index].isRunning ? "\u2B24" : "\u2BC0";
				color: appList[index].isRunning ? colors.greenIndicatorText : colors.redIndicatorText;
				font.pixelSize: 16;
			}

			Text {
				visible: !appList[index].isInstalled && appList[index].isRunning;
				text: "\u25B2";
				color: colors.yellowIndicatorText;
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