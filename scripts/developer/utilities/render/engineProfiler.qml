//
//  EngineProfiler.qml
//
//  Created by Sam Gateau on 06/07/2018
//  Copyright 2016 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or https://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//
import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3

import stylesUit 1.0
import controlsUit 1.0 as HifiControls

import "../lib/jet/qml" as Jet

Item {
    HifiConstants { id: hifi;}
    id: root;   
    anchors.fill: parent

    property var rootConfig: Render.getConfig("")


    Jet.TaskTimeFrameView {
        rootConfig: root.rootConfig
        anchors.fill: root        
    }
}