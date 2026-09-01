import QtQuick 2.0
import QtQuick.Window 2.3
import QtQuick.Controls 2.3
import '../../../scripts/developer/tests' as Tests

ApplicationWindow {
    width: 640
    height: 480
    visible: true

    Tests.ControlsGallery {
        anchors.fill: parent
    }
}
