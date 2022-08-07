//
//  FiraSansMedium.qml
//
//  Created by Dale Glass on 7 Aug 2022
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.7

Text {
    id: root
    property real size: 32
    font.pixelSize: size
    verticalAlignment: Text.AlignVCenter
    horizontalAlignment: Text.AlignLeft
    font.family: "Fira Sans"
    font.weight: Font.DemiBold
}
