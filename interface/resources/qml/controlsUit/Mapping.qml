//
//  Mapping.qml
//
//  Based on a file created by David Rowe on 17 Feb 2016
//

import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import "../stylesUit"
import "." as HifiControls

//Item {
HifiControls.TextField {
    id: mapping

    /*property int colorScheme: hifi.colorSchemes.light
    readonly property bool isLightColorScheme: colorScheme == hifi.colorSchemes.light
    readonly property bool isFaintGrayColorScheme: colorScheme == hifi.colorSchemes.faintGray
    property bool isSearchField: false
    property string label: ""
    property real controlHeight: height + (mappingLabel.visible ? mappingLabel.height + 1 : 0)
    property bool hasDefocusedBorder: true;
    property bool hasRoundedBorder: false
    property int roundedBorderRadius: 4
    property bool error: false;
    property bool hasClearButton: false;
    property string leftPermanentGlyph: "";
    property string centerPlaceholderGlyph: "";
    property int styleRenderType: Text.NativeRendering

    //placeholderText: mapping.placeholderText

    font.family: "Fira Sans"
    font.pixelSize: hifi.fontSizes.mappingInput
    height: implicitHeight + 3  // Make surrounding box higher so that highlight is vertically centered.
    property alias mappingLabel: mappingLabel

    y: mappingLabel.visible ? mappingLabel.height + mappingLabel.anchors.bottomMargin : 0*/

    Keys.onPressed: {
        switch (event.key) {
            case Qt.Key_Return:
            case Qt.Key_Enter:
                event.accepted = true;

                // emit accepted signal manually
                if (acceptableInput) {
                    accepted();
                }
                break;
            case Qt.Key_Escape:
                shortcut.sequence = '';
                mapping.text = '';
                break;
            default:
                shortcut.sequence = event.key;
                mapping.text = shortcut.nativeText;
        }
    }

    /*style: TextFieldStyle {
        id: style;
        textColor: {
            if (isLightColorScheme) {
                if (mapping.activeFocus) {
                    hifi.colors.black
                } else {
                    hifi.colors.lightGray
                }
            } else if (isFaintGrayColorScheme) {
                if (mapping.activeFocus) {
                    hifi.colors.black
                } else {
                    hifi.colors.lightGray
                }
            } else {
                if (mapping.activeFocus) {
                    hifi.colors.white
                } else {
                    hifi.colors.lightGrayText
                }
            }
        }
        background: Rectangle {
            color: {
            if (isLightColorScheme) {
                if (mapping.activeFocus) {
                    hifi.colors.white
                } else {
                    hifi.colors.mappingLightBackground
                }
            } else if (isFaintGrayColorScheme) {
                if (mapping.activeFocus) {
                    hifi.colors.white
                } else {
                    hifi.colors.faintGray50
                }
            } else {
                if (mapping.activeFocus) {
                    hifi.colors.black
                } else {
                    hifi.colors.baseGrayShadow
                }
            }
        }
            border.color: mapping.error ? hifi.colors.redHighlight :
            (mapping.activeFocus ? hifi.colors.primaryHighlight : (hasDefocusedBorder ? (isFaintGrayColorScheme ? hifi.colors.lightGrayText : hifi.colors.lightGray) : color))
            border.width: mapping.activeFocus || hasRoundedBorder || mapping.error ? 1 : 0
            radius: isSearchField ? mapping.height / 2 : (hasRoundedBorder ? roundedBorderRadius : 0)

            HiFiGlyphs {
                text: mapping.leftPermanentGlyph;
                color: textColor;
                size: hifi.fontSizes.mappingSearchIcon;
                anchors.left: parent.left;
                anchors.verticalCenter: parent.verticalCenter;
                anchors.leftMargin: hifi.dimensions.textPadding - 2;
                visible: text;
            }

            HiFiGlyphs {
                text: mapping.centerPlaceholderGlyph;
                color: textColor;
                size: parent.height;
                anchors.horizontalCenter: parent.horizontalCenter;
                anchors.verticalCenter: parent.verticalCenter;
                visible: text && !mapping.focus && mapping.text === "";
            }

            HiFiGlyphs {
                text: hifi.glyphs.search
                color: textColor
                size: hifi.fontSizes.mappingSearchIcon
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: hifi.dimensions.textPadding - 2
                visible: isSearchField
            }

            HiFiGlyphs {
                text: hifi.glyphs.error
                color: textColor
                size: 40
                anchors.right: parent.right
                anchors.rightMargin: hifi.dimensions.textPadding - 2
                anchors.verticalCenter: parent.verticalCenter
                visible: hasClearButton && mapping.text !== "";

                MouseArea {
                    anchors.fill: parent;
                    onClicked: {
                        mapping.text = "";
                    }
                }
            }
        }
        placeholderTextColor: isFaintGrayColorScheme ? hifi.colors.lightGrayText : hifi.colors.lightGray
        selectedTextColor: hifi.colors.black
        selectionColor: hifi.colors.primaryHighlight
        padding.left: hasRoundedBorder ? mapping.height / 2 : ((isSearchField || mapping.leftPermanentGlyph !== "") ? mapping.height - 2 : 0) + hifi.dimensions.textPadding
        padding.right: (hasClearButton ? mapping.height - 2 : 0) + hifi.dimensions.textPadding
        renderType: mapping.styleRenderType
    }*/

    HifiControls.Label {
        id: mappingLabel
        text: mapping.label
        colorScheme: mapping.colorScheme
        anchors.left: parent.left

        Binding on anchors.right {
            when: mapping.right
            value: mapping.right
        }
        Binding on wrapMode {
            when: mapping.right
            value: Text.WordWrap
        }

        anchors.bottom: parent.top
        anchors.bottomMargin: 3
        visible: label != ""
    }

    Shortcut {
        id: shortcut
        enabled: false
    }
}
