import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs as QtDialogs

import "../" as Overte
import "."

Rectangle {
    id: root
    anchors.fill: parent
    color: Overte.Theme.paletteActive.base
    implicitWidth: 480
    implicitHeight: 720

    readonly property string protocolSignature: "6xYA55jcXgPHValo3Ba3/A=="

    Component.onCompleted: {
        let xhr = new XMLHttpRequest();

        xhr.onreadystatechange = () => {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                console.debug("Finished downloading place list");
                try {
                    const body = JSON.parse(xhr.responseText);

                    let accum = [];
                    for (const place of body.data.places) {
                        if (
                            place.domain.protocol_version === protocolSignature
                        ) {
                            accum.push(place);
                        }
                    }

                    listView.model = accum;

                    console.debug("Finished parsing place list");
                } catch (e) {}
            }
        };

        console.debug("Downloading place list…");
        xhr.open("GET", "https://mv.overte.org/server/api/v1/places");
        xhr.send();
    }

    ColumnLayout {
        anchors.fill: parent

        RowLayout {
            Layout.margins: 4

            Overte.RoundButton {
                id: settingsButton
                icon.source: "../icons/settings_cog.svg"
                icon.width: 24
                icon.height: 24
                icon.color: Overte.Theme.paletteActive.buttonText

                onClicked: print("TODO")
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
                icon.source: "../icons/search.svg"
                icon.width: 24
                icon.height: 24
                icon.color: Overte.Theme.paletteActive.buttonText

                onClicked: {
                    searchExpression = searchField.text === "" ? ".*" : searchField.text;
                }
            }
        }

        Overte.TabBar {
            Layout.fillWidth: true
            id: tabBar

            Overte.TabButton { text: qsTr("Public") }
            Overte.TabButton { text: qsTr("Bookmarks") }
        }

        Overte.Label {
            Layout.fillWidth: true
            Layout.fillHeight: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            visible: listView.model.length === 0
            text: qsTr("Loading…")
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: listView
                visible: model.length !== 0
                spacing: 2
                clip: true

                ScrollBar.vertical: Overte.ScrollBar {}

                model: []
                delegate: PlaceItem {}
            }
        }

        Overte.Label {
            Layout.margins: 8
            Layout.fillWidth: true
            visible: listView.model.length !== 0
            verticalAlignment: Text.AlignVCenter
            text: qsTr("%1 place(s)").arg(listView.model.length)
        }
    }
}
