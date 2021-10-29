//
//  Shortcut.qml
//
//  Basically a copy of a file created by David Rowe on 17 Feb 2016
//

import QtQuick 2.7
import QtQuick.Controls 2.2

import "../stylesUit"
import "." as HifiControls

Shortcut {
    id: shortcut

    property int colorScheme: hifi.colorSchemes.light
    readonly property bool isLightColorScheme: colorScheme == hifi.colorSchemes.light
    //readonly property bool isFaintGrayColorScheme: colorScheme == hifi.colorSchemes.faintGray
    //property bool isSearchField: false
    property string label: ""
    property real controlHeight: height + (shortcutLabel.visible ? shortcutLabel.height + 1 : 0)
    property bool hasDefocusedBorder: true;
    property bool hasRoundedBorder: false
    property int roundedBorderRadius: 4
    property bool error: false;
    property bool hasClearButton: false;
    property string leftPermanentGlyph: "";
    property string centerPlaceholderGlyph: "";
    property int styleRenderType: Text.NativeRendering
    /*property AnchorLine anchors.left
    property AnchorLine anchors.right
    property AnchorLine anchors.bottom*/

    Keys.onPressed: {
        switch (event.key) {
            case Qt.Key_Return:
            case Qt.Key_Enter:
                event.accepted = true;

                // emit accepted signal manually
                if (acceptableInput) {
                    accepted();
                }
        }
    }

    /*HifiControls.Label {
        id: shortcutLabel
        text: shortcut.label
        colorScheme: shortcut.colorScheme
        anchors.left: parent.left
        anchors.bottom: parent.top
        anchors.bottomMargin: 2
        visible: label != ""
    }*/
}
