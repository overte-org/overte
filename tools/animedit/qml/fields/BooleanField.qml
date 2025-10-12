import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 1.6
import QtQuick.Layouts 1.3
import QtQuick.Dialogs


Row {
    id: row
    height: 20
    width: 300
    anchors.left: parent.left
    anchors.leftMargin: 0
    anchors.right: parent.right
    anchors.rightMargin: 0

    property string key
    property bool value
    property bool optional

    function setValue(newValue) {
        parent.fieldChanged(key, newValue);
    }

    Text {
        id: keyText
        y: 5
        width: 100
        text: key + ":"
        font.pixelSize: 12
        color: optional ? "blue" : "black"
    }

    CheckBox {
        id: valueCheckBox
        x: 100
        y: 5
        width: 200
        checked: value ? Qt.Checked : Qt.Unchecked
        onCheckedChanged: {
            setValue(checked);
        }
    }
}
