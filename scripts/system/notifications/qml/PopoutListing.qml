import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3

Item {
	property string listingId: "";
	property string listingMessage: "";
	property string listingDetails: "";
	property string listingTime: "";
	property bool isOpen: false;

	width: parent && parent.width - 10;
	height: columnElement.height;
	x: 5;
	clip: true;
	id: root;
	

	Behavior on height {
		NumberAnimation {
			duration: 50;
			easing.type: Easing.InOutCubic;
		}
	}

	Column {
		id: columnElement;
		width: parent.width;
		spacing: 0;

		// Actual message
		Item {
			width: parent.width;
			height: 70;

			Text {
				width: parent.width;
				text: listingMessage;
				color: "white";
				font.pixelSize: 18;
				wrapMode: Text.Wrap;
				y: 5;
			}
		}

		// Message details
		Item {
			id: messageDetailsElement;
			width: parent.width;
			clip: true;

			Rectangle {
				anchors.fill: parent;
				color: Qt.rgba(1,1,1,0.05);
			}

			ColumnLayout {
				width: parent.width;
				spacing: 0;

				Text {
					text: listingTime;
					Layout.fillWidth: true;
					height: contentHeight + 10;
					color: "white";
					font.pixelSize: 18;
					wrapMode: Text.Wrap;
					horizontalAlignment: Text.AlignRight;
				}

				Text {
					Layout.fillWidth: true;
					text: listingDetails;
					width: parent.width;
					height: contentHeight + 10;
					color: "white";
					font.pixelSize: 18;
					wrapMode: Text.Wrap;
				}
			}

		}
	}

	Rectangle {
		id: notificationElement;
		anchors.fill: parent;
		color: "transparent";
		z: -1;
	}

	Rectangle {
		color: Qt.rgba(1,1,1,0.1);
		height: 2;
		width: parent.width;
		y: parent.height - height;
	}

	MouseArea {
		anchors.fill: parent;
		hoverEnabled: true;
		propagateComposedEvents: true;	

		onPressed: {
			isOpen = !isOpen;
			messageDetailsElement.height = isOpen ? messageDetailsElement.children[1].height + 10 : 0;
			root.height = 60 + messageDetailsElement.height + 10;
		}

		onEntered: {
			notificationElement.color = colors.darkBackground2;
		}

		onExited: {
			notificationElement.color = "transparent";
		}
	}
}