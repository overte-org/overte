//
//  MappingPreference.qml
//
//  Based heavily on a file created by Bradley Austin Davis on 18 Jan 2016
//

import QtQuick 2.5

import "../../dialogs"
import controlsUit 1.0

Preference {
    id: root
    property alias shortcut: shortcut
    height: control.height + hifi.dimensions.controlInterlineHeight

    Component.onCompleted: {
        shortcut.sequence = preference.value;
    }

    function save() {
        //preference.value = shortcut.sequence;
        preference.value = shortcut.postableText;
        preference.save();
    }


    Item {
        id: control
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: Math.max(labelText.height, shortcut.height)

        Label {
            id: labelText
            text: root.label + ":"
            colorScheme: hifi.colorSchemes.dark
            anchors {
                left: parent.left
                right: shortcut.left
                rightMargin: hifi.dimensions.labelPadding
                verticalCenter: parent.verticalCenter
            }
            horizontalAlignment: Text.AlignRight
            wrapMode: Text.Wrap
        }

        Shortcut {
            id: shortcut
            sequence: preference.value;
            //placeholderText: preference.placeholderText
            colorScheme: hifi.colorSchemes.dark

            /*anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }*/
        }
    }
}
