import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../" as Overte

ColumnLayout {
    property alias text: labelItem.text
    property alias value: sliderItem.value
    property alias from: sliderItem.from
    property alias to: sliderItem.to
    property alias stepSize: sliderItem.stepSize
    property alias enabled: sliderItem.enabled
    property string valueText: valueToText()
    property bool fineTweakButtons: false

    property var valueToText: () => value.toString()

    id: item
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.margins: 16
    spacing: 16

    RowLayout {
        Overte.Label {
            Layout.fillWidth: true

            id: labelItem
            wrapMode: Text.Wrap
        }

        Overte.Label {
            text: valueText
            wrapMode: Text.Wrap
        }
    }

    RowLayout {
        Overte.RoundButton {
            visible: fineTweakButtons

            icon.width: 24
            icon.height: 24
            icon.source: "../icons/triangle_left.svg"
            icon.color: Overte.Theme.paletteActive.buttonText

            onClicked: {
                sliderItem.value -= item.stepSize;
            }
        }

        Overte.Slider {
            Layout.fillWidth: true

            id: sliderItem
            snapMode: Slider.SnapAlways
        }

        Overte.RoundButton {
            visible: fineTweakButtons

            icon.width: 24
            icon.height: 24
            icon.source: "../icons/triangle_right.svg"
            icon.color: Overte.Theme.paletteActive.buttonText

            onClicked: {
                sliderItem.value += item.stepSize;
            }
        }
    }

    // an extra spacer so the slider doesn't crowd with the setting below
    Item {}
}
