import QtQuick

import ".." as Overte
import "."

Rectangle {
    id: nodeGraph
    color: Qt.darker(Overte.Theme.paletteActive.base, 1.1)

    Overte.Switch {
        onToggled: Overte.Theme.darkMode = checked
    }

    Node {
        x: 64
        y: 64

        titleColor: "#00a000"
        title: qsTr("Flush Entity Properties")
        helpText: qsTr("Commits all changes to an entity's properties at once.")
        inputs: [
            { type: "exec" },
            { type: "entity" },
        ]
        outputs: [
            { type: "exec" },
        ]
    }
}
