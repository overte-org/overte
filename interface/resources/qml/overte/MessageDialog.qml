import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs

import "."

Dialog {
    id: root
    implicitWidth: layout.implicitWidth
    implicitHeight: layout.implicitHeight

    readonly property var buttonRoles: [
        { label: qsTr("Ok"), flag: MessageDialog.Ok, role: MessageDialog.AcceptRole },
        { label: qsTr("Save"), flag: MessageDialog.Save, role: MessageDialog.AcceptRole },
        { label: qsTr("Yes"), flag: MessageDialog.Yes, role: MessageDialog.YesRole },
        { label: qsTr("Cancel"), flag: MessageDialog.Cancel, role: MessageDialog.RejectRole },
        { label: qsTr("Discard"), flag: MessageDialog.Discard, role: MessageDialog.DestructiveRole },
        { label: qsTr("No"), flag: MessageDialog.No, role: MessageDialog.NoRole },
    ]

    property string text: "Oops! Your ModalDialog doesn't have any text."
    property string descriptiveText: ""
    property int buttons: MessageDialog.Ok | MessageDialog.Cancel
    property int result: MessageDialog.Ok

    property list<var> buttonDataModel: {
        let list = [];

        for (let role of buttonRoles) {
            if ((role.flag & buttons) !== 0) {
                list.push(role);
            }
        }

        return list.reverse();
    }

    signal accepted
    signal rejected
    signal buttonClicked(button: int, role: int)

    function reject() {
        rejected();
        close();
    }

    function accept() {
        accepted();
        close();
    }

    onButtonClicked: (button, role) => {
        result = button;

        switch (role) {
            case MessageDialog.AcceptRole:
            case MessageDialog.YesRole:
            accept();
            break;

            case MessageDialog.RejectRole:
            case MessageDialog.NoRole:
            reject();
            break;
        }
    }

    ColumnLayout {
        id: layout
        anchors.fill: parent
        anchors.margins: 8

        Label {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 8

            visible: root.text !== ""
            text: root.text
            wrapMode: Text.Wrap
        }

        Label {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 8

            visible: root.descriptiveText !== ""
            text: root.descriptiveText
            wrapMode: Text.Wrap
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignRight

            Repeater {
                model: buttonDataModel
                delegate: Button {
                    required property string label
                    required property int flag
                    required property int role

                    Layout.fillWidth: true
                    Layout.minimumWidth: 96
                    Layout.preferredWidth: 96

                    text: label
                    onClicked: buttonClicked(flag, role);
                }
            }
        }
    }
}
