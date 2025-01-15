import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import QtMultimedia 5.15

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

			TextEdit {
				text: delegateUsername
				color: "lightgray"
				readOnly: true
				selectByMouse: true
				selectByKeyboard: true
			}

			TextEdit {
				anchors.right: parent.right
				text: delegateDate
				color: "lightgray"
				readOnly: true
				selectByMouse: true
				selectByKeyboard: true
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
					TextEdit {
						text: model.value || ""
						font.pointSize: 12
						wrapMode: Text.Wrap
						width: model.type === 'text' || model.type === 'mention' ? Math.min(messageBoxFlow.width, contentWidth) : 0;
						visible: model.type === 'text' || model.type === 'mention';
						readOnly: true
						selectByMouse: true
						selectByKeyboard: true

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

						TextEdit {
							text: model.value || "";
							font.pointSize: 12;
							wrapMode: Text.Wrap;
							color: "#4EBAFD";
							font.underline: true;
							width: parent.width;
							readOnly: true
							selectByMouse: true
							selectByKeyboard: true

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

							TextEdit {
								text: model.type === 'overteLocation' ? model.value.split('hifi://')[1].split('/')[0] : '';
								color: "black"
								font.pointSize: 12
								x: parent.children[0].width + 5;
								anchors.verticalCenter: parent.verticalCenter 
								readOnly: true
								selectByMouse: true
								selectByKeyboard: true
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

						AnimatedImage {
							source: model.type === 'imageEmbed' ? model.value : ''
							height: Math.min(sourceSize.height, 200);
							fillMode: Image.PreserveAspectFit
						}
					}

					Item {
						visible: model.type === 'videoEmbed';
						width: messageBoxFlow.width;
						height: 200;

						Video {
							id: videoPlayer
							source: model.type === 'videoEmbed' ? model.value : ''
							height: 200;
							width: 400;
							fillMode: Image.PreserveAspectFit
							autoLoad: false;

							onStatusChanged: {
								if (status === 7) {
									// Weird hack to make the video restart when it's over
									// Ideally you'd want to use the seek function to restart the video but it doesn't work?
									// Will need to make a more refined solution for this later. in the form of a more advanced media player.
									// For now, this is sufficient. -AD
									let originalURL = videoPlayer.source;
									videoPlayer.source = "";
									videoPlayer.source = originalURL;
								}
							}

							MouseArea {
								anchors.fill: parent
								onClicked: {
									const videoIsOver = videoPlayer.position == videoPlayer.duration
									if (videoPlayer.playbackState == MediaPlayer.PlayingState) {
										videoPlayer.pause();
									} 
									else {
										parent.play();
									}
								}
							}
						}
					}
				}
			}
		}
	}
}