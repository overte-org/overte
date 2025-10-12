//
//  DefaultFrame.qml
//
//  Created by Bradley Austin Davis on 12 Jan 2016
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.5
import Qt5Compat.GraphicalEffects

import "."
import stylesUit 1.0

Frame {
    HifiConstants { id: hifi }
    property alias horizontalSpacers: decoration.horizontalSpacers
    property alias verticalSpacers: decoration.verticalSpacers

    ToolFrameDecoration {
        id: decoration
    }
}

