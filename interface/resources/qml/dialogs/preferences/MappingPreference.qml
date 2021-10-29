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
    //height: dataMapping.controlHeight + hifi.dimensions.controlInterlineHeight

    Component.onCompleted: {
        dataMapping.sequence = preference.value;
    }

    function save() {
        preference.value = dataMapping.sequence;
        preference.save();
    }

    Shortcut {
        id: dataMapping
        //placeholderText: preference.placeholderText
        //label: root.label
        colorScheme: hifi.colorSchemes.dark

        /*anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }*/
    }
}
