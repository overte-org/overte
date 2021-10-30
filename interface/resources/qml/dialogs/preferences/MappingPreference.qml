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
    //property alias shortcut: mapping.shortcut
    height: control.height + hifi.dimensions.controlInterlineHeight

    /*mapping.onEditingFinished: {
        mapping.shortcut.sequence = mapping.text;
    }*/

    Component.onCompleted: {
        mapping.label = preference.label;
        mapping.shortcut.sequence = preference.value;
        mapping.text = mapping.shortcut.nativeText;
    }

    function save() {
        //preference.value = mapping.shortcut.sequence;
        preference.value = mapping.shortcut.portableText;
        preference.save();
    }

    Mapping {
        id: mapping
        //placeholderText: preference.placeholderText
        //label: root.label
        label: preference.name
        //colorScheme: hifi.colorSchemes.dark
        //shortcut.sequence: preference.value
        //text: shortcut.sequence.nativeText

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
    }
}
