//
//  MappingPreference.qml
//
//  Created by Bradley Austin Davis on 18 Jan 2016
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.5

import "../../dialogs"
import controlsUit 1.0
//import "../../controlsUit/Shortcut.qml"

Preference {
    id: root
    //height: dataMapping.controlHeight + hifi.dimensions.controlInterlineHeight
    //property QKeySequence value;

    Component.onCompleted: {
        //dataMapping.text = preference.value;
        dataMapping.sequence = preference.value;
    }

    function save() {
        //preference.value = dataMapping.text;
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
