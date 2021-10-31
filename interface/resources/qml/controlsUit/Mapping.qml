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
