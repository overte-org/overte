import QtCore as QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import ".." as Overte
import "."

Rectangle {
    id: moreApps
    implicitWidth: 480
    implicitHeight: 720
    color: Overte.Theme.paletteActive.base

    property list<url> repoSources: {
        // the default setting argument doesn't work in qml, so emulate it
        let sourcesList = SettingsInterface.getValue("private/moreApp/repoSources");
        if (!sourcesList || sourcesList.length < 1) {
            sourcesList = [ "https://more.overte.org" ];
        }

        return sourcesList;
    }

    property string searchExpression: ".*"
    property list<var> rawListModel: []
    property list<var> filteredModel: []

    // can't be list<url> or equality tests fail
    property list<string> installedScripts: SettingsInterface.getValue("private/moreApp/installedScripts") ?? []
    property list<string> runningScripts: []

    // the running scripts list doesn't immediately update, so give it a bit to update
    Timer {
        id: runningScriptsRefreshTimer
        running: false
        repeat: false
        interval: 500

        onTriggered: {
            runningScripts = ScriptDiscoveryService.getRunning().map(x => x.url);
        }
    }

    function refreshRunningScripts() {
        runningScriptsRefreshTimer.start();
    }

    function refreshFilteredModel() {
        const searchRegex = new RegExp(searchExpression, "i");
        let tmp = [];

        for (const item of rawListModel) {
            // if onlyInstalled is checked and we're not running or installed, skip
            if (onlyInstalled.checked) {
                if (
                    !runningScripts.includes(item.scriptUrl) &&
                    !installedScripts.includes(item.scriptUrl)
                ) {
                    continue;
                }
            }

            if (
                item.name.match(searchRegex) ||
                item.id.match(searchRegex) ||
                item.description.match(searchRegex)
            ) {
                tmp.push(item);
            }
        }

        tmp.sort((a, b) => (
            a.name.localeCompare(b.name)
        ));

        filteredModel = tmp;
    }

    onRawListModelChanged: refreshFilteredModel()

    function parseList(list) {
        if (list.version !== 2) {
            console.warn(`App list "${list}" is version ${list.version}, expected 2`);
            return;
        }
        const repoHost = (new URL(list.baseApiUrl)).hostname;

        let tmp = [];

        for (const item of list.applicationList) {
            if (!item.appActive) { continue; }

            const scriptUrl = `${list.baseApiUrl}/${item.appBaseDirectory}/${item.appScriptVersions.Stable}`;

            tmp.push({
                appID: item.appBaseDirectory,
                name: item.appName,
                description: item.appDescription,
                scriptUrl: scriptUrl,
                author: item.appAuthor,
                icon: `${list.baseApiUrl}/${item.appBaseDirectory}/${item.appIcon}`,
                repoHost: repoHost,
            });
        }

        rawListModel = rawListModel.concat(tmp);
    }

    function fetchList(repo) {
        let xhr = new XMLHttpRequest();

        xhr.onreadystatechange = () => {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                let data;
                try {
                    data = JSON.parse(xhr.response);
                } catch(e) {
                    console.error(repo, e);
                    return;
                }
                parseList(data);
            }
        };

        xhr.open("GET", `${repo}/applications/metadata.json`);
        xhr.send();
    }

    function fetchAllLists() {
        rawListModel = [];

        for (const repo of repoSources) {
            fetchList(repo);
        }
    }

    onRepoSourcesChanged: {
        SettingsInterface.setValue("private/moreApp/repoSources", repoSources);
        refreshRunningScripts();
        fetchAllLists();
    }

    onInstalledScriptsChanged: SettingsInterface.setValue("private/moreApp/installedScripts", installedScripts)

    ColumnLayout {
        anchors.fill: parent

        RowLayout {
            Layout.margins: 4

            // TODO
            Overte.RoundButton {
                icon.source: "../icons/settings_cog.svg"
                icon.width: 24
                icon.height: 24

                onClicked: console.warn("TODO")
                visible: false
            }

            Overte.RoundButton {
                // TODO
                icon.source: checked ? "../icons/eye_closed.svg" : "../icons/eye_open.svg"
                icon.width: 24
                icon.height: 24

                id: onlyInstalled
                checkable: true
                onToggled: refreshFilteredModel()
                
                Overte.ToolTip { text: qsTr("Only show installed and running apps") }
            }

            Overte.TextField {
                Layout.fillWidth: true
                placeholderText: qsTr("Search…")
                id: searchField

                Keys.onEnterPressed: {
                    searchButton.clicked();
                    forceActiveFocus();
                }

                Keys.onReturnPressed: {
                    searchButton.clicked();
                    forceActiveFocus();
                }
            }

            Overte.RoundButton {
                icon.source: "../icons/search.svg"
                icon.width: 24
                icon.height: 24
                icon.color: Overte.Theme.paletteActive.buttonText
                id: searchButton

                onClicked: {
                    moreApps.searchExpression = searchField.text;
                    moreApps.refreshFilteredModel();
                }
            }
        }

        Overte.Label {
            Layout.fillWidth: true
            Layout.fillHeight: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: qsTr("Loading %n app source(s)…", "", repoSources.length)
            visible: rawListModel.length === 0
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: rawListModel.length > 0
            clip: true

            // QT6TODO: broken mouse input, remove when that's fixed
            interactive: false

            ScrollBar.vertical: Overte.ScrollBar {}
            contentWidth: width - ScrollBar.vertical.width

            model: filteredModel
            delegate: AppDelegate {}
        }
    }
}
