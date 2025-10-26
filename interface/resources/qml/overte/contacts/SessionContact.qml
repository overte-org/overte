import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import ".." as Overte

Rectangle {
	id: control
	color: index % 2 === 0 ? Overte.Theme.paletteActive.base : Overte.Theme.paletteActive.alternateBase
	implicitWidth: ListView.view.contentWidth
	implicitHeight: 96

	required property int index

	// session UUID or directory username
	required property string user
	required property string name
	required property real volume
	required property url badgeIconSource

	ColumnLayout {
		anchors.fill: parent
		anchors.margins: 4
		anchors.leftMargin: 8
		anchors.rightMargin: 8

		RowLayout {
			Layout.fillWidth: true
			spacing: 16

			Image {
				Layout.leftMargin: 6
				Layout.preferredWidth: 24
				Layout.preferredHeight: 24
				source: badgeIconSource
			}

			ColumnLayout {
				Layout.fillWidth: true

				Overte.Label {
					Layout.fillWidth: true
					elide: Text.ElideRight
					text: name
				}

				Overte.BodyText {
					font.pixelSize: Overte.Theme.fontPixelSizeSmall
					visible: user !== ""
					//elide: Text.ElideRight
					text: user
					palette.buttonText: Overte.Theme.paletteActive.link
					opacity: Overte.Theme.highContrast ? 1.0 : 0.7
				}
			}
		}

		RowLayout {
			Overte.RoundButton {
				icon.source: volumeSlider.value === 0.0 ? "../icons/speaker_muted.svg" : "../icons/speaker_active.svg"
				icon.width: 24
				icon.height: 24
				icon.color: Overte.Theme.paletteActive.buttonText

				onClicked: volumeSlider.value = 0.0
			}

			Overte.Slider {
				Layout.fillWidth: true
				id: volumeSlider
				from: 0.0
				to: 1.2
				stepSize: 0.1
				snapMode: Slider.SnapAlways
				value: volume

				onMoved: {
					// TODO
				}
			}

			Overte.Label {
				Layout.preferredWidth: Overte.Theme.fontPixelSize * 3
				text: `${Math.round(volumeSlider.value * 100)}%`
			}
		}
	}
}
