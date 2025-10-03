import QtQuick 2.7
import QtQuick.Controls 2.2
import QtWebChannel 1.0
import controls 1.0
import hifi.toolbars 1.0
import Qt5Compat.GraphicalEffects
import controlsUit 1.0 as HifiControls
import stylesUit 1.0


WebView {
    id: entityListToolWebView
    url: Qt.resolvedUrl("../html/entityList.html")
    enabled: true
    blurOnCtrlShift: false
}
