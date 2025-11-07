import QtCore as QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs as QtDialogs

import "../" as Overte
import "."

Rectangle {
    id: placePicker
    anchors.fill: parent
    color: Overte.Theme.paletteActive.base
    implicitWidth: 480
    implicitHeight: 720

    property bool hasHomeButton: true

    readonly property string protocolSignature: WindowScriptingInterface.protocolSignature()

    QtCore.Settings {
        id: filters
        category: "placesApp"
        property string searchExpression: ".*"
        property bool includeIncompatible: false
        property list<string> maturity: [
            "everyone",
            "teen",
            "mature",
            "adult",
            "unrated",
        ]
        property list<string> favoritedPlaceIds: []
    }

    function goBack() {
        let cookie = Date.now() + Math.floor(Math.random() * (1000 - -1000) + -1000);

        sendToScript(JSON.stringify({
            action: "system:location_go_back",
            data: { cookie: cookie },
        }));
    }

    function goForward() {
        let cookie = Date.now() + Math.floor(Math.random() * (1000 - -1000) + -1000);

        sendToScript(JSON.stringify({
            action: "system:location_go_forward",
            data: { cookie: cookie },
        }));
    }

    function goToLocation(path) {
        let cookie = Date.now() + Math.floor(Math.random() * (1000 - -1000) + -1000);

        sendToScript(JSON.stringify({
            action: "system:location_go_to",
            data: {
                cookie: cookie,
                path: path,
            },
        }));
    }

    // The mv.overte.org directory listing is about 300KiB
    // and takes about 10 seconds to download. Is there something
    // we could use to safely cache the last results and only refresh
    // after a set period?
    property list<var> rawPlaces: []
    property real downloadProgress: 0.0

    function filterPlaces() {
        // TODO: federation support
        const hostname = (new URL(AccountServices.metaverseServerURL)).hostname;
        const searchExpression = new RegExp(filters.searchExpression, "i");
        const ONE_DAY_SECS = 60 * 60 * 24;
        let tmp = [];

        for (let place of rawPlaces) {
            const compatibleProtocol = place.domain.protocol_version === protocolSignature;
            // ?status=active should filter out dead places already
            //const recentHeartbeat = ((Date.now() - parseInt(place.domain.time_of_last_heartbeat_s)) / 1000) < ONE_DAY_SECS;
            const filterName = Boolean(place.name.match(searchExpression));
            const filterMaturity = filters.maturity.includes(place.maturity);
            const filterHasUsers = !onlyShowActiveButton.checked || place.current_attendance > 0;

            if (
                (compatibleProtocol || filters.includeIncompatible) &&
                filterName &&
                filterMaturity &&
                filterHasUsers
            ) {
                place.directoryHost = hostname;
                place.compatibleProtocol = compatibleProtocol;
                tmp.push(place);
            }
        }

        tmp.sort((a, b) => (
            // if "show incompatible servers" is on, sort compatible ones first
            ((b.compatibleProtocol ? 1 : 0) - (a.compatibleProtocol ? 1 : 0)) ||
            // sort favorited places up
            ((filters.favoritedPlaceIds.includes(b.placeId) ? 1 : 0) - (filters.favoritedPlaceIds.includes(a.placeId) ? 1 : 0)) ||
            // sort by UUID, so the listing has a stable order that can't be cheated
            a.placeId.localeCompare(b.placeId)
        ));

        gridView.model = tmp;
    }

    Component.onCompleted: {
        let fileSize = 1;
        let xhr = new XMLHttpRequest();

        xhr.onreadystatechange = () => {
            if (xhr.readyState === XMLHttpRequest.HEADERS_RECEIVED) {
                const length = xhr.getResponseHeader("Content-Length");
                if (length !== "") { fileSize = Number(length); }
            } else if (xhr.readyState === XMLHttpRequest.LOADING) {
                downloadProgress = xhr.response.length / fileSize;
            } else if (xhr.readyState === XMLHttpRequest.DONE) {
                console.debug("Finished downloading place list");
                downloadProgress = 1.0;

                try {
                    const body = JSON.parse(xhr.responseText);
                    rawPlaces = body.data.places;
                    filterPlaces();
                } catch (e) {
                    console.error(e);
                }
            }
        };

        console.debug("Downloading place list…");
        downloadProgress = 0.0;

        // TODO: federation support, multiple directory sources
        xhr.open("GET", `${AccountServices.metaverseServerURL}/api/v1/places?status=online`);
        xhr.send();
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.margins: 4

            Overte.RoundButton {
                visible: placePicker.hasHomeButton

                icon.source: "../icons/triangle_left.svg"
                icon.width: 24
                icon.height: 24
                icon.color: Overte.Theme.paletteActive.buttonText

                onClicked: placePicker.goBack()
            }

            Overte.RoundButton {
                visible: placePicker.hasHomeButton

                icon.source: "../icons/triangle_right.svg"
                icon.width: 24
                icon.height: 24
                icon.color: Overte.Theme.paletteActive.buttonText

                onClicked: placePicker.goForward()
            }

            Overte.RoundButton {
                visible: placePicker.hasHomeButton

                icon.source: "../icons/home.svg"
                icon.width: 24
                icon.height: 24
                icon.color: Overte.Theme.paletteActive.buttonText

                onClicked: placePicker.goToLocation(LocationBookmarks.getHomeLocationAddress())

                Overte.ToolTip { text: qsTr("Go to Home bookmark") }
            }

            Overte.TextField {
                Layout.fillWidth: true

                id: searchField
                placeholderText: qsTr("Search…")

                Keys.onEnterPressed: {
                    searchButton.click();
                    forceActiveFocus();
                }
                Keys.onReturnPressed: {
                    searchButton.click();
                    forceActiveFocus();
                }
            }

            Overte.RoundButton {
                id: searchButton
                icon.source: (
                    searchField.text.match(/(?:hifi|https?|file):\/\//) ?
                    "../icons/send.svg" :
                    "../icons/search.svg"
                )
                icon.width: 24
                icon.height: 24
                icon.color: Overte.Theme.paletteActive.buttonText

                onClicked: {
                    if (searchField.text.startsWith("hifi://")) {
                        placePicker.goToLocation(infoDialog.placeUrl);
                        searchField.text = "";
                    } else {
                        filters.searchExpression = searchField.text === "" ? ".*" : searchField.text;
                        filterPlaces();
                    }
                }
            }

            Overte.RoundButton {
                id: onlyShowActiveButton
                icon.source: "../icons/users.svg"
                icon.width: 24
                icon.height: 24
                icon.color: Overte.Theme.paletteActive.buttonText
                checkable: true

                Overte.ToolTip { text: qsTr("Only show places with users") }

                onClicked: filterPlaces()
            }

            Overte.RoundButton {
                id: filterButton
                icon.source: "../icons/filter.svg"
                icon.width: 24
                icon.height: 24
                icon.color: Overte.Theme.paletteActive.buttonText

                onClicked: filterDialog.open()

                Overte.ToolTip { text: qsTr("Search options") }
            }
        }

        Overte.TabBar {
            Layout.fillWidth: true
            id: tabBar

            Overte.TabButton { text: qsTr("Public") }
            Overte.TabButton { text: qsTr("Bookmarks") }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: rawPlaces.length === 0

            Item { Layout.fillHeight: true }

            Overte.Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: qsTr("Loading…")
            }

            Rectangle {
                Layout.margins: 8
                Layout.leftMargin: 64
                Layout.rightMargin: 64
                Layout.fillWidth: true

                implicitWidth: 256
                implicitHeight: Overte.Theme.fontPixelSize + (Overte.Theme.borderWidth * 2)
                color: Overte.Theme.paletteActive.base
                border.color: Qt.darker(color, Overte.Theme.borderDarker)
                border.width: Overte.Theme.borderWidth
                radius: Overte.Theme.borderRadius

                Rectangle {
                    x: parent.border.width
                    y: parent.border.width
                    height: parent.height - (parent.border.width * 2)
                    width: downloadProgress * (parent.width - (parent.border.width * 2))
                    radius: parent.radius - Overte.Theme.borderWidth
                    color: Overte.Theme.paletteActive.highlight
                }
            }

            Item { Layout.fillHeight: true }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            GridView {
                // QT6TODO: remove this once mouse inputs work properly
                interactive: false

                id: gridView
                visible: rawPlaces.length !== 0
                // fit two cells onto the default tablet
                cellWidth: (480 - Overte.Theme.scrollbarWidth) / 2
                cellHeight: Math.floor(cellWidth * 0.6)
                clip: true

                ScrollBar.vertical: Overte.ScrollBar {}
                rightMargin: ScrollBar.vertical.width

                model: []
                delegate: PlaceItem {}
            }
        }

        RowLayout {
            Layout.margins: 2
            Layout.fillWidth: true

            Overte.Label {
                Layout.fillWidth: true
                verticalAlignment: Text.AlignVCenter
                text: (
                    gridView.model.length !== 0 ?
                    qsTr("%1 place(s)").arg(gridView.model.length) :
                    ""
                )
            }

            Overte.RoundButton {
                id: settingsButton
                icon.source: "../icons/settings_cog.svg"
                icon.width: 24
                icon.height: 24

                onClicked: console.error("TODO: settings")
                visible: false
            }
        }
    }

    Overte.Dialog {
        id: filterDialog
        visible: false
        anchors.fill: placePicker

        function open() {
            filterControlMaturityEveryone.checked = filters.maturity.includes("everyone");
            filterControlMaturityTeen.checked = filters.maturity.includes("teen");
            filterControlMaturityMature.checked = filters.maturity.includes("mature");
            filterControlMaturityAdult.checked = filters.maturity.includes("adult");
            filterControlIncludeIncompatible.checked = filters.includeIncompatible;

            visible = true;
            opacity = Overte.Theme.reducedMotion ? 1 : 0;
        }

        function accept() {
            let maturityList = [];

            if (filterControlMaturityEveryone.checked) {
                maturityList.push("everyone");
            }

            if (filterControlMaturityTeen.checked) {
                maturityList.push("teen");
            }

            if (filterControlMaturityMature.checked) {
                maturityList.push("mature");
            }

            if (filterControlMaturityAdult.checked) {
                maturityList.push("adult");
            }

            // if everything is enabled or disabled, include unrated places too
            if (maturityList.length === 0 || maturityList.length === 4) {
                maturityList.push("unrated");
            }

            filters.includeIncompatible = filterControlIncludeIncompatible.checked;
            filters.maturity = maturityList;
            placePicker.filterPlaces();
            close();
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8

            Overte.Label {
                Layout.fillWidth: true
                text: qsTr("Filters")
                horizontalAlignment: Text.AlignHCenter
                opacity: Overte.Theme.highContrast ? 1.0 : 0.6
            }

            Overte.Ruler { Layout.fillWidth: true }

            RowLayout {
                Overte.Label {
                    Layout.fillWidth: true
                    text: qsTr("Maturity ratings")
                }

                GridLayout {
                    Layout.alignment: Qt.AlignRight
                    columns: 2

                    Overte.Switch {
                        id: filterControlMaturityEveryone
                        text: qsTr("Everyone")
                    }

                    Overte.Switch {
                        id: filterControlMaturityTeen
                        text: qsTr("Teen")
                    }

                    Overte.Switch {
                        id: filterControlMaturityMature
                        text: qsTr("Mature")
                    }

                    Overte.Switch {
                        id: filterControlMaturityAdult
                        text: qsTr("Adult")
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true

                Overte.Label {
                    Layout.fillWidth: true
                    text: qsTr("Show incompatible servers")
                }

                Overte.Switch { id: filterControlIncludeIncompatible }
            }

            RowLayout {
                Layout.preferredWidth: 480

                Overte.Button {
                    Layout.preferredWidth: 1
                    Layout.fillWidth: true
                    text: qsTr("Cancel")
                    onClicked: filterDialog.close()
                }

                Item { Layout.fillWidth: true }

                Overte.Button {
                    Layout.preferredWidth: 1
                    Layout.fillWidth: true
                    backgroundColor: Overte.Theme.paletteActive.buttonAdd
                    text: qsTr("Apply")
                    onClicked: filterDialog.accept()
                }
            }
        }
    }

    Overte.Dialog {
        id: infoDialog
        visible: false
        anchors.fill: placePicker

        property string placeName: ""
        property string placeDesc: ""
        property string domainName: ""
        property string directoryHost: ""
        property int currentUsers: 0
        property int maxUsers: 0
        property list<string> managers: []
        property url placeUrl: ""
        property bool compatible: true
        property string placeId: ""

        function open(index) {
            const data = gridView.model[index];
            placeName = data.name;
            domainName = data.domain.name;
            placeDesc = data.description;
            directoryHost = data.directoryHost;
            compatible = data.compatibleProtocol;

            // ignore the redundant default place description
            if (
                placeDesc === `A place in ${domainName}` ||
                placeDesc === `A place in ${placeName}`
            ) {
                placeDesc = "";
            }

            currentUsers = data.current_attendance;
            maxUsers = data.domain.capacity;
            managers = data.managers;
            placeUrl = data.finalPlaceUrl ?? `hifi://${placeName}${data.path}`;
            placeId = data.placeId;

            visible = true;
            opacity = Overte.Theme.reducedMotion ? 1 : 0;
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8

            ColumnLayout {
                Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                Layout.fillWidth: true

                Overte.Label {
                    // leave space for the close button
                    Layout.rightMargin: 44
                    Layout.fillWidth: true
                    font.bold: true
                    text: infoDialog.placeName
                }

                Overte.Label {
                    // leave space for the close button
                    Layout.rightMargin: 44
                    font.pixelSize: Overte.Theme.fontPixelSizeSmall
                    opacity: Overte.Theme.highContrast ? 1.0 : 0.6
                    text: `${infoDialog.domainName} @ ${infoDialog.directoryHost}`
                }

                Overte.Label {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    opacity: Overte.Theme.highContrast ? 1.0 : 0.6
                    font.pixelSize: Overte.Theme.fontPixelSizeSmall
                    text: qsTr("Managed by %1").arg(infoDialog.managers.join(", "))
                }

                Overte.Ruler { Layout.fillWidth: true }

                Overte.Label {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    wrapMode: Text.Wrap
                    font.pixelSize: Overte.Theme.fontPixelSizeSmall
                    text: infoDialog.placeDesc
                }
            }

            RowLayout {
                implicitWidth: 480
                Layout.preferredWidth: 480
                Layout.fillWidth: true

                Overte.Button {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.preferredWidth: 1
                    text: qsTr("Copy URL")
                    onClicked: WindowScriptingInterface.copyToClipboard(infoDialog.placeUrl)
                }

                Overte.Button {
                    // TODO: separate place portal handler, the old one was
                    // baked into places.js so we can't reuse it
                    visible: false
                    enabled: false

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.preferredWidth: 1
                    backgroundColor: Overte.Theme.paletteActive.buttonInfo
                    text: qsTr("Portal")
                    /*onClicked: {
                        // FIXME: This is defined in the previous places.js,
                        // we'll need a separate script for handling portal spawns
                        Messages.sendMessage("com.overte.places.portalRezzer", JSON.stringify({
                            action: "REZ_PORTAL",
                            position: Vec3.sum(
                                MyAvatar.position,
                                Vec3.multiply(2, Quat.getForward(MyAvatar.orientation))
                            ),
                            url: infoDialog.placeUrl,
                            name: infoDialog.placeName,
                            placeID: infoDialog.placeName,
                        }));
                    }*/
                }

                Overte.Button {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.preferredWidth: 1
                    backgroundColor: Overte.Theme.paletteActive.buttonAdd
                    enabled: infoDialog.compatible
                    text: infoDialog.compatible ? qsTr("Join") : qsTr("Incompatible")
                    onClicked: placePicker.goToLocation(infoDialog.placeUrl)
                }
            }
        }

        Row {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 8
            spacing: 4

            Overte.RoundButton {
                implicitWidth: 28
                implicitHeight: 28

                icon.source: "../icons/gold_star.svg"
                icon.width: 20
                icon.height: 20
                icon.color: (
                    checked ?
                    Overte.Theme.paletteActive.buttonText :
                    Overte.Theme.paletteActive.placeholderText
                )

                backgroundColor: checked ? Overte.Theme.paletteActive.buttonFavorite : Overte.Theme.paletteActive.button
                checkable: true
                checked: filters.favoritedPlaceIds.includes(infoDialog.placeId)

                onToggled: {
                    if (!checked) {
                        const index = filters.favoritedPlaceIds.indexOf(infoDialog.placeId);
                        filters.favoritedPlaceIds.splice(index);
                    } else {
                        filters.favoritedPlaceIds.push(infoDialog.placeId);
                    }

                    filterPlaces();
                }

                Overte.ToolTip { text: qsTr("Favorite\nSorts this place before unfavorited places.") }
            }

            Overte.RoundButton {
                implicitWidth: 28
                implicitHeight: 28

                backgroundColor: Overte.Theme.paletteActive.buttonDestructive

                icon.source: "../icons/close.svg"
                icon.width: 18
                icon.height: 18
                icon.color: Overte.Theme.paletteActive.buttonText

                onClicked: infoDialog.close()
            }
        }
    }
}
