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

    QtCore.Settings {
        id: settings
        category: "moreApp"
        property list<var> installedScripts: []
        property list<var> repoSources: []
    }

    property alias installedScripts: settings.installedScripts
    property alias repoSources: settings.repoSources

    property string searchExpression: ".*"
    property list<var> rawListModel: []
    property list<var> filteredModel: []

    property list<var> runningScripts: []

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

    function parseList(list, repo) {
        if (list.version !== 2) {
            console.warn(`App list "${list}" is version ${list.version}, expected 2`);
            return;
        }
        const repoHost = (new URL(repo)).hostname;

        let tmp = [];

        for (const item of list.applicationList) {
            if (!item.appActive) { continue; }

            const baseUrl = `${repo}/applications`;
            const scriptUrl = `${baseUrl}/${item.appBaseDirectory}/${item.appScriptVersions.Stable}`;

            tmp.push({
                appID: item.appBaseDirectory,
                name: item.appName,
                description: item.appDescription,
                scriptUrl: scriptUrl,
                author: item.appAuthor,
                icon: `${baseUrl}/${item.appBaseDirectory}/${item.appIcon}`,
                repoHost: repoHost,
            });
        }

        rawListModel = rawListModel.concat(tmp);
    }

    property var pendingRequests: ({})

    function fetchList(repo) {
        let xhr = new XMLHttpRequest();

        pendingRequests[repo] = {
            abort: () => xhr.abort(),
            expectedSize: 1,
            currentSize: 0,
        };

        xhr.onreadystatechange = () => {
            if (xhr.readyState === XMLHttpRequest.HEADERS_RECEIVED) {
                const length = xhr.getResponseHeader("Content-Length");
                if (length !== "") {
                    pendingRequests[repo].expectedSize = Number(length);
                }
            } else if (xhr.readyState === XMLHttpRequest.LOADING) {
                pendingRequests[repo].currentSize = xhr.response.length;
            } else if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status !== 200) {
                    console.warn(repo, xhr.status, xhr.statusText);
                    downloadProgressLabel.updateText();
                    delete pendingRequests[repo];
                    pendingRequestsChanged();
                    return;
                }

                pendingRequests[repo].currentSize = xhr.response.length;

                try {
                    const data = JSON.parse(xhr.response);
                    parseList(data, repo);
                } catch(e) {
                    console.error(repo, e);
                }

                delete pendingRequests[repo];
            }

            downloadProgressLabel.updateText();
            pendingRequestsChanged();
        };

        xhr.open("GET", `${repo}/applications/metadata.json`);
        xhr.send();

        pendingRequestsChanged();
        downloadProgressLabel.updateText();
    }

    function clearAllPendingRequests() {
        for (const req of Object.values(pendingRequests)) {
            req.abort();
        }

        pendingRequests = [];
    }

    function fetchAllLists() {
        // cancel any downloads that are already in-flight
        // so we don't accidentally get two responses for the same repo
        clearAllPendingRequests();

        rawListModel = [];
        filteredModel = [];

        for (const repo of repoSources) {
            fetchList(repo);
        }
    }

    onRepoSourcesChanged: {
        refreshRunningScripts();
        fetchAllLists();
    }

    Component.onCompleted: {
        // don't put this in the default because it'll load
        // the default first, *then* the actual repos stored
        // in the settings. this way we don't double up on requests
        if (repoSources.length === 0) {
            repoSources.push("https://more.overte.org");
        }
    }

    Component.onDestruction: {
        // we're quitting, cancel any downloads that are already in-flight
        // so they don't try to update items that have been destroyed
        clearAllPendingRequests();
    }

    Component {
        id: settingsPage

        ColumnLayout {
            RowLayout {
                Layout.fillWidth: true

                Overte.RoundButton {
                    icon.source: "../icons/triangle_left.svg"
                    icon.width: 24
                    icon.height: 24
                    icon.color: Overte.Theme.paletteActive.buttonText
                    onClicked: stack.pop()
                }

                Overte.Label {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: qsTr("Settings - More Apps")
                }
            }

            Item {
                Layout.fillWidth: true
                implicitHeight: 12
            }

            Overte.Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignBottom
                text: qsTr("App Sources")
                opacity: Overte.Theme.highContrast ? 1.0 : 0.6
            }

            Overte.Ruler { Layout.fillWidth: true }

            ListView {
                Layout.fillWidth: true

                // QT6TODO: remove this when mouse inputs are working properly
                interactive: false

                clip: true
                ScrollBar.vertical: Overte.ScrollBar {
                    policy: ScrollBar.AlwaysOn
                }
                contentWidth: width - ScrollBar.vertical.width

                implicitHeight: (Overte.Theme.fontPixelSizeSmall * 3) * 6

                model: moreApps.repoSources
                delegate: Rectangle {
                    required property int index
                    readonly property var text: moreApps.repoSources[index]

                    width: ListView.view.contentWidth
                    implicitHeight: Overte.Theme.fontPixelSizeSmall * 3
                    color: index % 2 === 0 ? Overte.Theme.paletteActive.base : Overte.Theme.paletteActive.alternateBase

                    Overte.Label {
                        anchors.margins: 4
                        anchors.left: parent.left
                        anchors.right: sourceRemoveButton.right
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: Overte.Theme.fontPixelSizeSmall
                        elide: Text.ElideRight

                        // FIXME: shouldn't ever be undefined, but sometimes is anyway?
                        text: parent.text ?? "undefined"
                    }

                    Overte.RoundButton {
                        id: sourceRemoveButton
                        anchors.margins: 4
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter

                        backgroundColor: (
                            (hovered || Overte.Theme.highContrast) ?
                            Overte.Theme.paletteActive.buttonDestructive :
                            Overte.Theme.paletteActive.button
                        )

                        horizontalPadding: 0
                        verticalPadding: 0
                        implicitWidth: 32
                        implicitHeight: 32

                        icon.source: "../icons/close.svg"
                        icon.width: 18
                        icon.height: 18
                        icon.color: Overte.Theme.paletteActive.buttonText

                        onClicked: {
                            // FIXME: this throws an error about not being able to
                            // resolve "moreApps", but it seems to work fine anyway?
                            moreApps.repoSources.splice(index, 1)
                            moreApps.repoSourcesChanged()
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true

                Overte.TextField {
                    Layout.fillWidth: true

                    id: newSourceField
                    placeholderText: qsTr("App list source URL")
                }

                Overte.RoundButton {
                    icon.source: "../icons/plus.svg"
                    icon.width: 24
                    icon.height: 24
                    icon.color: Overte.Theme.paletteActive.buttonText
                    backgroundColor: Overte.Theme.paletteActive.buttonAdd

                    enabled: !!newSourceField.text.match(/^https?:\/\/.+/i)

                    onClicked: {
                        moreApps.repoSources.push(newSourceField.text.replace(/\/$/g, ""));
                        moreApps.repoSourcesChanged();
                        newSourceField.text = "";
                    }
                }
            }

            // spacer
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    Overte.StackView {
        id: stack
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: downloadProgressLabel.top

        initialItem: ColumnLayout {
            RowLayout {
                Layout.margins: 4

                Overte.RoundButton {
                    icon.source: "../icons/settings_cog.svg"
                    icon.width: 24
                    icon.height: 24

                    onClicked: stack.push(settingsPage.createObject(stack))
                }

                Overte.RoundButton {
                    // TODO: is the eye icon acceptable here? it feels too vague to grok
                    icon.source: checked ? "../icons/eye_closed.svg" : "../icons/eye_open.svg"
                    icon.width: 24
                    icon.height: 24

                    id: onlyInstalled
                    checkable: true
                    onToggled: refreshFilteredModel()

                    backgroundColor: (
                        checked ?
                        Overte.Theme.paletteActive.highlight :
                        Overte.Theme.paletteActive.button
                    )
                    color: (
                        checked ?
                        Overte.Theme.paletteActive.highlightedText :
                        Overte.Theme.paletteActive.buttonText
                    )

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

    Overte.Label {
        anchors.margins: 4
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        id: downloadProgressLabel
        visible: Object.values(pendingRequests).length !== 0

        function updateText() {
            const requests = Object.values(pendingRequests);

            if (requests.length === 0) {
                this.text = "";
                return;
            }

            let accum = 0;

            for (const req of requests) {
                accum = (req.currentSize / req.expectedSize);
            }

            const percent = Math.round((accum / requests.length) * 100);
            this.text = `${qsTr("Downloading…")} ${percent}%`;
        }
    }
}
