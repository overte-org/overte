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

HifiControls.TextField {
    id: mapping
    property int keyval;
    property int bakkeyval;
    property string baktext;
    //anchors.left: mappingLabel.right

    Keys.onPressed: {
        switch (event.key) {
            case Qt.Key_Return:
            case Qt.Key_Enter:
                event.accepted = true;

                // emit accepted signal manually
                if (acceptableInput) {
                    accepted();
                }
                save();
                break;
            case Qt.Key_Escape:
                if (keyval == bakkeyval) {	// Clear mapping.
                    keyval = 0;
                    mapping.text = '';
                } else {	// Reset to existing mapping.
                    keyval = bakkeyval;
                    mapping.text = baktext;
                }
                event.accepted = true;
                break;
            default:
                keyval = event.key;

                if (event.text.length == 0) {
                    if (event.modifiers & Qt.ControlModifier) {
                        mapping.keyval = 0x01000021	// "COMMAND" on Mac...
                        mapping.text = "Control"
                        event.accepted = true;
                    }
                    if (event.modifiers & Qt.AltModifier) {
                        mapping.keyval = 0x01000023
                        mapping.text = "Alt"
                        event.accepted = true;
                    }
                    if (event.modifiers & Qt.ShiftModifier) {
                        mapping.keyval = 0x01000020
                        mapping.text = "Shift"
                        event.accepted = true;
                    }
                } else {
                    mapping.text = event.text.toUpperCase();
                    event.accepted = true;
                }
        }
    }

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
}
