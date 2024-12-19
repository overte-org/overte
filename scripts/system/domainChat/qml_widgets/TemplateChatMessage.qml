import QtQuick 2.7
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Component {
	id: template_chat_message

	Rectangle {
		property int index: delegateIndex

		height: Math.max(65, children[1].height + 30)
		color: index % 2 === 0 ? "transparent" : Qt.rgba(0.15,0.15,0.15,1)
		width: listview.parent.parent.width
		Layout.fillWidth: true

		Item {
			width: parent.width - 10
			anchors.horizontalCenter: parent.horizontalCenter
			height: 22

			Text {
				text: delegateUsername
				color: "lightgray"
			}

			Text {
				anchors.right: parent.right
				text: delegateDate
				color: "lightgray"
			}
		}

		Flow {
			anchors.top: parent.children[0].bottom;
			width: parent.width * 0.8
			x: 5
			id: messageBoxFlow

			Repeater {
				model: delegateText;

				RowLayout {
					Text {
						text: model.value || ""
						font.pointSize: 12
						wrapMode: Text.Wrap
						width: model.type === 'text' || model.type === 'mention' ? Math.min(messageBoxFlow.width, contentWidth) : 0;
						visible: model.type === 'text' || model.type === 'mention';

						color: {
							switch (model.type) {
								case "mention":
									return "purple";
								default:
									return "white";
							}
						}
					}

					RowLayout {
						width: children[0].contentWidth;
						visible: model.type === 'url';

						Text {
							text: model.value || "";
							font.pointSize: 12;
							wrapMode: Text.Wrap;
							color: "#4EBAFD";
							font.underline: true;
							width: parent.width;

							MouseArea {
								anchors.fill: parent;

								onClicked: {
									Window.openWebBrowser(model.value);
								}
							}
						}

						Text {
							text: "🗗";
							font.pointSize: 10;
							wrapMode: Text.Wrap;
							color: "white";

							MouseArea {
								anchors.fill: parent;

								onClicked: {
									Qt.openUrlExternally(model.value);
								}
							}
						}
					}

					RowLayout {
						visible: model.type === 'overteLocation';
						width: Math.min(messageBoxFlow.width, children[0].children[1].contentWidth + 35);
						height: 20;
						Layout.leftMargin: 5
						Layout.rightMargin: 5

						Rectangle {
							width: parent.width;
							height: 20;
							color: "lightgray"
							radius: 2;

							Image {
								source: "../img/ui/world_black.png"
								width: 18;
								height: 18;
								sourceSize.width: 18
								sourceSize.height: 18
								anchors.left: parent.left
								anchors.verticalCenter: parent.verticalCenter 
								anchors.leftMargin: 2
								anchors.rightMargin: 10
							}

							Text {
								text: model.type === 'overteLocation' ? model.value.split('hifi://')[1].split('/')[0] : '';
								color: "black"
								font.pointSize: 12
								x: parent.children[0].width + 5;
								anchors.verticalCenter: parent.verticalCenter 
							}

							MouseArea {
								anchors.fill: parent;

								onClicked: {
									Window.openUrl(model.value);
								}
							}
						}
					}

					Item {
						Layout.fillWidth: true
						visible: model.type === 'messageEnd'
					}

					Item {
						visible: model.type === 'imageEmbed';
						width: messageBoxFlow.width;
						height: 200

						Image {
							source: model.type === 'imageEmbed' ? model.value : ''
							sourceSize.width: 400
							sourceSize.height: 200
						}
					}
				}
			}
		}
	}
}