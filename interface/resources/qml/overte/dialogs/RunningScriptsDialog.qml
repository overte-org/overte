import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import ".." as Overte

Rectangle {
    anchors.fill: parent

    id: root
    implicitWidth: 480
    implicitHeight: 720
    color: Overte.Theme.paletteActive.base

    property list<var> runningScriptsModel: []

    function refreshRunningScriptsModel() {
        let tmp = [];

        for (const script of ScriptDiscoveryService.getRunning()) {
            tmp.push({
                name: script.name,
                url: script.url,
            });
        }

        tmp.sort((a, b) => a.name.localeCompare(b.name));

        runningScriptsModel = tmp;
    }

    Connections {
        target: ScriptDiscoveryService

        function onScriptCountChanged() {
            refreshRunningScriptsModel();
        }
    }

    Component.onCompleted: refreshRunningScriptsModel()

    component ScriptDelegate: Rectangle {
        required property int index
        required property string name
        required property string url

        width: runningList.width - Overte.Theme.scrollbarWidth
        height: layout.implicitHeight

        color: (
            index % 2 == 0 ?
            Overte.Theme.paletteActive.base :
            Overte.Theme.paletteActive.alternateBase
        )

        RowLayout {
            id: layout
            anchors.fill: parent

            ColumnLayout {
                Layout.margins: 8

                Overte.Label {
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    text: name.replace(/(.*).js/, "$1")
                }

                Overte.Label {
                    Layout.alignment: Qt.AlignLeft | Qt.AlignBottom
                    Layout.fillWidth: true
                    font.pixelSize: Overte.Theme.fontPixelSizeSmall
                    opacity: Overte.Theme.highContrast ? 1.0 : 0.6
                    elide: Text.ElideRight
                    text: url.startsWith("qrc:") ? `${qsTr("Built-in:")} ${url}` : url
                }
            }

            Row {
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.margins: 8
                spacing: 2

                // TODO: url copy button
                /*Overte.RoundButton {
                    backgroundColor: Overte.Theme.paletteActive.buttonInfo

                    icon.source: "../icons/copy.svg"
                    icon.color: Overte.Theme.paletteActive.buttonText
                    icon.width: 20
                    icon.height: 20

                    implicitWidth: 28
                    implicitHeight: 28
                    horizontalPadding: 0
                    verticalPadding: 0

                    onClicked: WindowScriptingInterface.copyToClipboard(url)
                }*/

                Overte.RoundButton {
                    icon.source: "../icons/reload.svg"
                    icon.color: Overte.Theme.paletteActive.buttonText
                    icon.width: 20
                    icon.height: 20

                    implicitWidth: 28
                    implicitHeight: 28
                    horizontalPadding: 0
                    verticalPadding: 0

                    // restart the script
                    onClicked: ScriptDiscoveryService.stopScript(url, true)
                }

                Overte.RoundButton {
                    backgroundColor: Overte.Theme.paletteActive.buttonDestructive

                    icon.source: "../icons/close.svg"
                    icon.color: Overte.Theme.paletteActive.buttonText
                    icon.width: 20
                    icon.height: 20

                    implicitWidth: 28
                    implicitHeight: 28
                    horizontalPadding: 0
                    verticalPadding: 0

                    onClicked: ScriptDiscoveryService.stopScript(url)
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollBar.vertical: Overte.ScrollBar {
                policy: ScrollBar.AlwaysOn
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right
            }

            id: runningList
            clip: true
            model: root.runningScriptsModel
            delegate: ScriptDelegate {}
        }

        Overte.Ruler { Layout.fillWidth: true }

        Overte.Label {
            Layout.fillWidth: true

            horizontalAlignment: Text.AlignHCenter
            opacity: Overte.Theme.highContrast ? 1.0 : 0.6
            text: qsTr("Add new script from...")
        }

        RowLayout {
            Layout.margins: 8
            Layout.fillHeight: true

            Overte.Button {
                // force equal button widths
                Layout.preferredWidth: 1
                Layout.fillWidth: true

                text: qsTr("File")

                onClicked: {
                    console.warn("TODO");
                }
            }

            Overte.Button {
                // force equal button widths
                Layout.preferredWidth: 1
                Layout.fillWidth: true

                text: qsTr("URL")

                onClicked: addFromUrlDialog.open()
            }

            Overte.Button {
                // force equal button widths
                Layout.preferredWidth: 1
                Layout.fillWidth: true

                text: qsTr("Built-in")

                onClicked: {
                    console.warn("TODO");
                }
            }
        }
    }

    Overte.Dialog {
        id: addFromUrlDialog
        anchors.fill: parent
        maxWidth: -1

        signal accepted
        signal rejected

        onAccepted: {
            ScriptDiscoveryService.loadScript(targetUrlField.text, !oneSessionSwitch.checked);
            close();

            targetUrlField.text = "";
            oneSessionSwitch.checked = false;
        }

        onRejected: {
            close();

            targetUrlField.text = "";
            oneSessionSwitch.checked = false;
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8

            Overte.Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                opacity: Overte.Theme.highContrast ? 1.0 : 0.6
                text: qsTr("Load script from URL")
            }
            Overte.Ruler { Layout.fillWidth: true }

            Overte.TextField {
                Layout.fillWidth: true
                placeholderText: qsTr("Script URL")
                id: targetUrlField
            }

            Overte.Switch {
                id: oneSessionSwitch
                text: qsTr("Only for this session")
            }

            RowLayout {
                Layout.preferredWidth: 720
                Layout.fillWidth: true

                Overte.Button {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    text: qsTr("Cancel")

                    onClicked: {
                        addFromUrlDialog.rejected();
                    }
                }

                Item {
                    Layout.preferredWidth: 1
                    Layout.fillWidth: true
                }

                Overte.Button {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1

                    backgroundColor: Overte.Theme.paletteActive.buttonAdd
                    text: qsTr("Load")
                    enabled: targetUrlField.text !== ""

                    onClicked: {
                        addFromUrlDialog.accepted();
                    }
                }
            }
        }
    }
}
