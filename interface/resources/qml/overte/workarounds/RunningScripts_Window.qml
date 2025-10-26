import ".." as Overte
import "../dialogs" as OverteDialogs

import "../../windows" as HifiWindows

HifiWindows.Window {
    objectName: "RunningScripts"
    title: qsTr("Running Scripts")
    opacity: parent.opacity
    resizable: true
    minSize: Qt.vector2d(384, 192)

    OverteDialogs.RunningScriptsDialog {}
}
