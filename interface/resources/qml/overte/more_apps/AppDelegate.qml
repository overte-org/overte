import QtQuick
import QtQuick.Layouts

import ".." as Overte

Rectangle {
    id: item
    width: ListView.view.contentWidth
    implicitHeight: column.implicitHeight
    color: index % 2 === 0 ? Overte.Theme.paletteActive.base : Overte.Theme.paletteActive.alternateBase

    required property int index

    required property string name
    required property string description
    required property url icon
    required property string author
    required property string scriptUrl
    required property string appID
    required property string repoHost

    property bool running: moreApps.runningScripts.includes(scriptUrl)
    property bool installed: moreApps.installedScripts.includes(scriptUrl)
    property bool processing: false

    Connections {
        target: moreApps
        ignoreUnknownSignals: false

        function onRunningScriptsChanged() {
            processing = false;
        }
    }

    ColumnLayout {
        anchors.fill: parent
        id: column

        RowLayout {
            Layout.margins: 8
            Layout.fillWidth: true

            Overte.RoundButton {
                id: dropdownButton
                checkable: true
                icon.source: checked ? "../icons/triangle_up.svg" : "../icons/triangle_down.svg"
                icon.width: 24
                icon.height: 24
                icon.color: Overte.Theme.paletteActive.buttonText
            }

            Rectangle {
                Layout.preferredWidth: 64
                Layout.preferredHeight: 64
                color: Overte.Theme.paletteActive.appIconBackground
                radius: Overte.Theme.borderRadius

                border.width: Math.max(2, Overte.Theme.borderWidth)
                border.color: {
                    if (installed && running) {
                        return Overte.Theme.paletteActive.appInstalledRunning;
                    } else if (installed && !running) {
                        return Overte.Theme.paletteActive.appInstalledNotRunning;
                    } else if (!installed && running) {
                        return Overte.Theme.paletteActive.appNotInstalledRunning;
                    } else {
                        return Overte.Theme.paletteActive.appNotInstalled;
                    }
                }

                Image {
                    anchors.fill: parent
                    anchors.margins: 8
                    source: icon
                    sourceSize.width: width
                    sourceSize.height: height
                }
            }

            ColumnLayout {
                Layout.fillWidth: true

                Overte.Label {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    text: name
                }

                Overte.Label {
                    Layout.fillWidth: true
                    font.pixelSize: Overte.Theme.fontPixelSizeSmall
                    opacity: Overte.Theme.highContrast ? 1.0 : 0.7
                    visible: !dropdownButton.checked
                    elide: Text.ElideRight
                    text: description
                }

                Overte.Label {
                    Layout.fillWidth: true
                    font.pixelSize: Overte.Theme.fontPixelSizeSmall
                    opacity: Overte.Theme.highContrast ? 1.0 : 0.7
                    visible: dropdownButton.checked
                    wrapMode: Text.Wrap
                    text: {
                        if (running && installed) {
                            return qsTr("Running");
                        } else if (running && !installed) {
                            return qsTr("Running, not installed");
                        } else if (!running && installed) {
                            return qsTr("Paused");
                        } else {
                            return "";
                        }
                    }
                }
            }

            Overte.RoundButton {
                visible: installed || running
                enabled: !processing
                icon.source: "../icons/delete.svg"
                icon.width: 24
                icon.height: 24
                icon.color: Overte.Theme.paletteActive.buttonText
                backgroundColor: Overte.Theme.paletteActive.buttonDestructive

                onClicked: {
                    processing = true;
                    ScriptDiscoveryService.stopScript(scriptUrl);

                    // only variable assignments are automatically tracked by Qt
                    const indexOf = moreApps.installedScripts.indexOf(scriptUrl);
                    moreApps.installedScripts.splice(indexOf);
                    moreApps.installedScriptsChanged();

                    moreApps.refreshRunningScripts();
                    moreApps.refreshFilteredModel();
                }
            }

            Overte.RoundButton {
                enabled: !processing
                visible: installed && running
                icon.source: "../icons/reload.svg"
                icon.width: 24
                icon.height: 24
                icon.color: Overte.Theme.paletteActive.buttonText
                onClicked: ScriptDiscoveryService.stopScript(scriptUrl, true)
            }

            Overte.RoundButton {
                enabled: !processing
                visible: installed
                icon.source: running ? "../icons/pause.svg" : "../icons/triangle_right.svg"
                icon.width: 24
                icon.height: 24
                icon.color: Overte.Theme.paletteActive.buttonText
                onClicked: {
                    if (running) {
                        ScriptDiscoveryService.stopScript(scriptUrl, false);
                    } else {
                        ScriptDiscoveryService.loadScript(scriptUrl, true);
                    }

                    processing = true;
                    moreApps.refreshRunningScripts();
                    moreApps.refreshFilteredModel();
                }
            }

            Overte.RoundButton {
                id: installButton
                enabled: !processing
                visible: !installed
                backgroundColor: Overte.Theme.paletteActive.buttonAdd

                icon.source: "../icons/plus.svg"
                icon.width: 24
                icon.height: 24
                icon.color: Overte.Theme.paletteActive.buttonText

                onClicked: {
                    processing = true;
                    ScriptDiscoveryService.loadScript(scriptUrl, true);

                    // only variable assignments are automatically tracked by Qt
                    moreApps.installedScripts.push(scriptUrl);
                    moreApps.installedScriptsChanged();

                    moreApps.refreshRunningScripts();
                    moreApps.refreshFilteredModel();
                }
            }
        }

        RowLayout {
            Layout.leftMargin: 8
            Layout.rightMargin: 8
            Layout.fillWidth: true
            visible: dropdownButton.checked

            Overte.Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignLeft
                font.pixelSize: Overte.Theme.fontPixelSizeSmall
                opacity: Overte.Theme.highContrast ? 1.0 : 0.7
                text: qsTr("By %1").arg(author)
            }

            Overte.Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                font.pixelSize: Overte.Theme.fontPixelSizeSmall
                opacity: Overte.Theme.highContrast ? 1.0 : 0.7
                text: `${appID} @ ${repoHost}`
            }
        }

        Overte.Label {
            Layout.leftMargin: 8
            Layout.rightMargin: 8
            Layout.bottomMargin: 8
            Layout.fillWidth: true
            visible: dropdownButton.checked
            wrapMode: Text.Wrap
            text: description
        }
    }
}
