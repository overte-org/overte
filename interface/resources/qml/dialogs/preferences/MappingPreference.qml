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
    height: mapping.height + hifi.dimensions.controlInterlineHeight

    Component.onCompleted: {
        mapping.keyval = preference.value;
        mapping.bakkeyval = mapping.keyval;

        //mapping.text = mapping.keyval;
        //mapping.text = valueToKey(mapping.keyval);
        //mapping.text = preference.value.toString();
        //mapping.text = preference.value.valueOf();
        //mapping.text = Key(preference.value).toString();
        //mapping.text = Qt.Key(mapping.keyval).toString();
        //mapping.text = Qt.valueToKey(mapping.keyval);
        mapping.text = preference.displayValue.substring(4);
        mapping.baktext = mapping.text;
    }

    function save() {
        if (mapping.keyval != mapping.bakkeyval) {
            preference.value = mapping.keyval;
            preference.save();
            mapping.bakkeyval = mapping.keyval;
            mapping.baktext = mapping.text;
        }
    }

    Mapping {
        id: mapping
        //label: preference.label // Not sure why this doesn't work.
        label: root.label
        //text: keyval;	// Needed?

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
    }
}
