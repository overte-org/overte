import QtQuick
import QtQuick.Layouts
import QtMultimedia

import ".." as Overte

Rectangle {
	property url source: "file:///home/ada/var/livestream/clips/corrupted-seagull.mp4"

	anchors.fill: parent

	id: root
	radius: 8
	color: Qt.darker(Overte.Theme.paletteActive.base)
	border.width: Overte.Theme.borderWidth
	border.color: Overte.Theme.paletteActive.base

	implicitWidth: 854
	implicitHeight: 480

	MediaPlayer {
		id: mediaPlayer
		source: root.source
		audioOutput: audioOutput
		videoOutput: videoOutput
	}

	AudioOutput {
		id: audioOutput
		volume: volumeSlider.value
	}

	VideoOutput {
		anchors.fill: parent
		anchors.margins: root.border.width

		id: videoOutput
	}

	Item {
		id: controlsOverlay
		anchors.fill: root
		anchors.margins: root.border.width
		opacity: 0.0

		transitions: Transition {
			NumberAnimation {
				properties: "opacity"
				easing.type: Easing.OutExpo
				duration: 500
			}
		}

		states: [
			State {
				name: "idle"
				when: !hoverArea.hovered

				PropertyChanges {
					target: controlsOverlay
					opacity: 0.0
				}
			},
			State {
				name: "hovered"
				when: hoverArea.hovered

				PropertyChanges {
					target: controlsOverlay
					opacity: 1.0
				}
			}
		]

		HoverHandler {
			id: hoverArea
			target: parent
		}

		Rectangle {
			anchors.top: controlsOverlay.top
			anchors.left: controlsOverlay.left
			anchors.margins: 8
			width: childrenRect.width
			height: childrenRect.height
			radius: 8
			color: Overte.Theme.highContrast ? "black" : "#80000000"
			visible: titleLabel.text !== "" || statusLabel.text !== ""

			ColumnLayout {
				id: statusColumn

				Overte.Label {
					Layout.margins: 8

					id: titleLabel
					visible: text !== ""

					// metadata title > extracted filename > raw url
					text: {
						const metaTitle = mediaPlayer.metaData.value(MediaMetaData.Title);

						if (metaTitle) {
							return metaTitle;
						} else {
							const file = new URL(root.source).pathname.replace(/^.+?([^\/]+?)(?:\..+)?$/, "$1")

							if (file) {
								return file;
							} else {
								return root.source;
							}
						}
					}
				}

				Overte.Label {
					Layout.margins: 8

					id: statusLabel
					visible: text !== ""

					text: {
						switch (mediaPlayer.mediaStatus) {
							case MediaPlayer.LoadingMedia:
							return qsTr("Loading…");

							case MediaPlayer.BufferingMedia:
							return qsTr("Buffering…");

							case MediaPlayer.StalledMedia:
							return qsTr("Stalled! Connection may have been lost.");

							case MediaPlayer.InvalidMedia:
							return qsTr("Media file is not playable.");

							default: return "";
						}
					}
				}
			}
		}

		Overte.RoundButton {
			anchors.top: controlsOverlay.top
			anchors.right: controlsOverlay.right
			anchors.margins: 8

			icon.color: Overte.Theme.paletteActive.buttonText
			icon.source: "../icons/info.svg"
			icon.width: 24
			icon.height: 24

			id: infoButton
			checkable: true
		}

		Rectangle {
			anchors.right: infoButton.left
			anchors.top: infoButton.top
			anchors.rightMargin: 8
			radius: 8
			color: Overte.Theme.highContrast ? "black" : "#80000000"
			visible: infoButton.checked

			width: Math.min(controlsOverlay.width / 2, metadataColumn.implicitWidth + 16)
			height: metadataColumn.implicitHeight + 16

			ColumnLayout {
				id: metadataColumn
				anchors.fill: parent
				anchors.margins: 8

				Overte.Label {
					Layout.fillWidth: true
					font.family: Overte.Theme.monoFontFamily
					font.pixelSize: Overte.Theme.fontPixelSizeSmall
					elide: Text.ElideMiddle
					text: root.source
				}

				Overte.Label {
					Layout.fillWidth: true
					text: {
						const containerNames = new Map([
							[0, "WMV"],
							[1, "AVI"],
							[2, "Matroska (mkv)"],
							[3, "MPEG-4 (mp4)"],
							[4, "Ogg"],
							[5, "QuickTime (mov)"],
							[6, "WebM"],
							[7, "MPEG-4 Audio"],
							[8, "AAC"],
							[9, "WMA"],
							[10, "MP3"],
							[11, "FLAC"],
							[12, "WAV"],
						]);
						const vcodecNames = new Map([
							[0, "MPEG-1"],
							[1, "MPEG-2"],
							[2, "MPEG-4"],
							[3, "H.264"],
							[4, "H.265"],
							[5, "VP8"],
							[6, "VP9"],
							[7, "AV1"],
							[8, "Theora"],
							[9, "WMV Video"],
							[10, "Motion JPEG"],
						]);
						const acodecNames = new Map([
							[0, "MP3"],
							[1, "AAC"],
							[2, "Dolby AC3"],
							[3, "Dolby EAC3"],
							[4, "FLAC"],
							[5, "Dolby TrueHD"],
							[6, "Opus"],
							[7, "Ogg Vorbis"],
							[8, "WAV"],
							[9, "WMA"],
							[10, "ALAC"],
						]);
						let chunks = [];

						if (
							mediaPlayer.mediaStatus === MediaPlayer.NoMedia ||
							mediaPlayer.mediaStatus === MediaPlayer.InvalidMedia
						) {
							return qsTr("Unloaded");
						}

						const containerName = containerNames.get(mediaPlayer.metaData.value(MediaMetaData.FileFormat));
						chunks.push(`${qsTr("Container:")} ${containerName ?? qsTr("Other")}`);

						if (mediaPlayer.hasVideo) {
							const track = mediaPlayer.videoTracks[mediaPlayer.activeVideoTrack];
							const codecName = vcodecNames.get(track.value(MediaMetaData.VideoCodec));
							const fps = track.value(MediaMetaData.VideoFrameRate);
							const resolution = track.value(MediaMetaData.Resolution);
							chunks.push(`${qsTr("Video:")} ${resolution.width}x${resolution.height}@${fps.toFixed(2)} ${codecName ?? qsTr("Other")}`);
						}

						if (mediaPlayer.hasAudio) {
							const track = mediaPlayer.audioTracks[mediaPlayer.activeAudioTrack];
							const codecName = acodecNames.get(track.value(MediaMetaData.AudioCodec));
							const bitrate = Math.round(track.value(MediaMetaData.AudioBitRate) / 1000);
							chunks.push(`${qsTr("Audio:")} ${bitrate} kbps ${codecName ?? qsTr("Other")}`);
						}

						return chunks.join("\n");
					}
				}
			}
		}

		Rectangle {
			anchors.left: controlsOverlay.left
			anchors.right: controlsOverlay.right
			anchors.bottom: controlsOverlay.bottom
			anchors.margins: 8
			height: mediaControlsLayout.implicitHeight + 16
			radius: 16
			color: Overte.Theme.highContrast ? "black" : "#80000000"

			RowLayout {
				anchors.fill: parent
				anchors.margins: 8
				id: mediaControlsLayout

				Overte.RoundButton {
					implicitWidth: 48
					implicitHeight: 48
					icon.source: (
						mediaPlayer.playbackState === MediaPlayer.PlayingState ?
						"../icons/pause.svg" :
						"../icons/triangle_right.svg"
					)
					icon.color: Overte.Theme.paletteActive.buttonText
					icon.width: 32
					icon.height: 32

					onClicked: {
						if (mediaPlayer.playbackState === MediaPlayer.PlayingState) {
							mediaPlayer.pause();
						} else {
							mediaPlayer.play();
						}
					}
				}

				ColumnLayout {
					RowLayout {
						Overte.Label {
							font.family: Overte.Theme.monoFontFamily
							text: {
								const totalSeconds = Math.floor(mediaPlayer.position / 1000);

								const seconds = (totalSeconds % 60).toString().padStart(2, "0");
								const minutes = (Math.floor(totalSeconds / 60) % 60).toString().padStart(2, "0");
								const hours = Math.floor(totalSeconds / 60 / 60).toString().padStart(2, "0");

								return `${hours}:${minutes}:${seconds}`;
							}
						}

						Item { Layout.fillWidth: true }

						Overte.Label {
							font.family: Overte.Theme.monoFontFamily
							text: {
								const totalSeconds = Math.floor(mediaPlayer.duration / 1000);

								const seconds = (totalSeconds % 60).toString().padStart(2, "0");
								const minutes = (Math.floor(totalSeconds / 60) % 60).toString().padStart(2, "0");
								const hours = Math.floor(totalSeconds / 60 / 60).toString().padStart(2, "0");

								return `${hours}:${minutes}:${seconds}`;
							}
						}
					}

					RowLayout {
						Overte.RoundButton {
							icon.source: "../icons/skip_backward.svg"
							icon.width: 24
							icon.height: 24
							icon.color: Overte.Theme.paletteActive.buttonText
							onClicked: mediaPlayer.position -= 5000
						}

						Overte.Slider {
							Layout.fillWidth: true
							value: mediaPlayer.position / mediaPlayer.duration

							onMoved: {
								mediaPlayer.position = value * mediaPlayer.duration;
							}
						}

						Overte.RoundButton {
							icon.source: "../icons/skip_forward.svg"
							icon.width: 24
							icon.height: 24
							icon.color: Overte.Theme.paletteActive.buttonText
							onClicked: mediaPlayer.position += 5000
						}
					}
				}

				Overte.RoundButton {
					id: muteButton
					icon.source: {
						if (audioOutput.muted) {
							return "../icons/speaker_muted.svg";
						} else if (audioOutput.volume === 0) {
							return "../icons/speaker_inactive.svg"
						} else {
							return "../icons/speaker_active.svg";
						}
					}
					icon.color: Overte.Theme.paletteActive.buttonText
					icon.width: 24
					icon.height: 24
					checkable: true
					checked: audioOutput.muted

					onClicked: audioOutput.muted = !audioOutput.muted;
				}

				Overte.Slider {
					Layout.preferredWidth: 96

					id: volumeSlider
					value: 0.75

					onValueChanged: {
						audioOutput.muted = false;
					}
				}
			}
		}
	}
}
