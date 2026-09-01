import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import ".." as Overte
import "."

Rectangle {
    id: contactsList
    anchors.fill: parent
    implicitWidth: 480
    implicitHeight: 720
    color: Overte.Theme.paletteActive.base

    property string localSearchExpression: ".*"
    property string accountSearchExpression: ".*"

    property list<var> localContactsModel: []
    property list<var> accountContactsModel: []
    property var adminData: ({})

    property var waitingRequestCallbacks: ({})

    function fromScript(message) {
        const data = JSON.parse(message);

        switch (data.action) {
            case "system:auth_request": {
                if (contactsList.waitingRequestCallbacks[data.data.cookie]) {
                    contactsList.waitingRequestCallbacks[data.data.cookie](data.data);
                    delete contactsList.waitingRequestCallbacks[data.data.cookie];
                }
            } break;
        }
    }

    function authRequest(data) {
        let cookie = Date.now() + Math.floor(Math.random() * (1000 - -1000) + -1000);

        contactsList.waitingRequestCallbacks[cookie] = data.callback;

        sendToScript(JSON.stringify({
            action: "system:auth_request",
            data: {
                method: data.method ?? "GET",
                url: data.url,
                body: data.body,
                cookie: cookie,
            }
        }));
    }

    function updateLocalContacts() {
        const palData = AvatarManager.getPalData().data;
        let tmp = [];

        for (const data of palData) {
            // don't add ourselves to the contacts list
            if (!data.sessionUUID) { continue; }

            if (!contactsList.adminData[data.sessionUUID]) {
                Users.requestUsernameFromID(data.sessionUUID);
            }

            tmp.push({
                uuid: data.sessionUUID,
                name: data.sessionDisplayName ? data.sessionDisplayName : `${qsTr("Unnamed")} ${data.sessionUUID}`,
                volume: Users.getAvatarGain(data.sessionUUID),
            });
        }

        localContactsModel = tmp;
    }

    function updateAccountContactsImpl(accounts) {
        console.debug("Contacts list data received");
        let tmp = [];

        for (const entry of accounts) {
            // FIXME: /api/v1/users/connections doesn't give you the availability
            // of a connection, only whether they're on a server or not
            let status = entry.location?.online ? 2 : 0;

            let img = entry.images?.tiny ?? entry.images?.thumbnail;

            // some accounts have a bugged default avatar that doesn't exist
            if (
                img === "assets/brand-icon-256.png" ||
                !(img && img.match(/^https?:\/\//))
            ) {
                img = undefined;
            }

            tmp.push({
                user: entry.username,
                avatarUrl: img ?? "../icons/unset_avatar.svg",
                status: status,
                currentPlaceName: "",
                friend: entry.connection === "friend",
            });
        }

        tmp.sort((a, b) => (
            ((a.status !== 0 ? 1 : 0) - (b.status !== 0 ? 1 : 0)) ||
            a.user.localeCompare(b.user)
        ));

        accountContactsModel = tmp;
    }

    function updateAccountContacts() {
        if (!AccountServices.loggedIn) {
            accountContactsModel = [];
            return;
        }

        console.debug("Requesting contacts list…");
        authRequest({
            method: "GET",
            url: `${AccountServices.metaverseServerURL}/api/v1/users/connections`,
            callback: response => {
                try {
                    let data = JSON.parse(response.responseText);
                    if (data.status === "success" && data?.data?.users) {
                        updateAccountContactsImpl(data.data.users);
                    } else {
                        console.error("Failed to get contacts list");
                        console.debug(response.responseText);
                    }
                } catch (e) {
                    console.error(e);
                }
            },
        });
    }

    Connections {
        target: Users

        function usernameFromIDReply(sessionUUID, username, _fingerprint, isAdmin) {
            contactsList.adminData[sessionUUID] = {
                username: username,
                badge: isAdmin ? "../icons/admin_shield.svg" : ""
            };

            // only assignments are checked, not property changes, so force the update signal
            contactsList.adminDataChanged();
        }

        function avatarRemovedEvent(sessionUUID) {
            delete contactsList.adminData[sessionUUID];
            updateLocalContacts();
        }

        function avatarAddedEvent(sessionUUID) {
            updateLocalContacts();
        }
    }

    Connections {
        target: AccountServices

        function loggedInChanged(_loggedIn) {
            updateAccountContacts();
        }
    }

    Component.onCompleted: {
        updateLocalContacts();
        updateAccountContacts();
    }

    ColumnLayout {
        anchors.fill: contactsList

        MyAccountInfo {
            Layout.fillWidth: true
            id: myAccountInfo
        }

        Overte.TabBar {
            Layout.fillWidth: true
            id: tabBar

            Overte.TabButton { text: qsTr("Local") }
            Overte.TabButton { text: qsTr("Account") }
        }

        StackLayout {
            currentIndex: tabBar.currentIndex

            // Local
            ColumnLayout {
                Overte.Label {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    visible: !localContactsList.visible
                    opacity: Overte.Theme.highContrast ? 1.0 : 0.7
                    text: qsTr("It's just you here")

                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    id: localContactsList
                    visible: model.length > 0
                    clip: true

                    ScrollBar.vertical: Overte.ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }

                    contentWidth: contactsList.width - Overte.Theme.scrollbarWidth

                    model: {
                        const regex = new RegExp(localSearchExpression, "i");
                        let tmp = [];

                        for (const item of localContactsModel) {
                            if (item.name.match(regex) || item.user.match(regex)) {
                                tmp.push(item);
                            }
                        }

                        return tmp;
                    }
                    delegate: SessionContact {}
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: 4
                    Layout.rightMargin: 4

                    Overte.TextField {
                        Layout.fillWidth: true

                        Keys.onEnterPressed: {
                            localSearchButton.clicked();
                            forceActiveFocus();
                        }

                        Keys.onReturnPressed: {
                            localSearchButton.clicked();
                            forceActiveFocus();
                        }

                        placeholderText: qsTr("Search…")
                        id: localSearchField
                    }

                    Overte.RoundButton {
                        icon.source: "../icons/search.svg"
                        icon.width: 24
                        icon.height: 24
                        icon.color: Overte.Theme.paletteActive.buttonText
                        id: localSearchButton

                        onClicked: localSearchExpression = localSearchField.text
                    }
                }
            }

            // Account
            ColumnLayout {
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    visible: myAccountInfo.status !== MyAccountInfo.Status.LoggedOut

                    ScrollBar.vertical: Overte.ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }

                    contentWidth: contactsList.width - Overte.Theme.scrollbarWidth

                    model: {
                        const regex = new RegExp(accountSearchExpression, "i");
                        let tmp = [];

                        for (const item of accountContactsModel) {
                            if (item.user.match(regex)) {
                                tmp.push(item);
                            }
                        }

                        return tmp;
                    }
                    delegate: AccountContact {}
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: 4
                    Layout.rightMargin: 4
                    visible: myAccountInfo.status !== MyAccountInfo.Status.LoggedOut

                    Overte.TextField {
                        Layout.fillWidth: true

                        Keys.onEnterPressed: {
                            accountSearchButton.clicked();
                            forceActiveFocus();
                        }

                        Keys.onReturnPressed: {
                            accountSearchButton.clicked();
                            forceActiveFocus();
                        }

                        placeholderText: qsTr("Search…")
                        id: accountSearchField
                    }

                    Overte.RoundButton {
                        icon.source: "../icons/search.svg"
                        icon.width: 24
                        icon.height: 24
                        icon.color: Overte.Theme.paletteActive.buttonText
                        id: accountSearchButton

                        onClicked: accountSearchExpression = accountSearchField.text
                    }
                }

                Overte.Label {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: myAccountInfo.status === MyAccountInfo.Status.LoggedOut
                    wrapMode: Text.Wrap
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: qsTr("Log in to track contacts")
                }
            }
        }
    }

    Overte.Dialog {
        id: changeAvatarDialog
        anchors.fill: parent
        maxWidth: -1

        signal accepted
        signal rejected

        onAccepted: {
            myAccountInfo.avatarUrl = avatarChangeUrlField.text;

            authRequest({
                method: "POST",
                url: `${AccountServices.metaverseServerURL}/api/v1/account/${AccountServices.username}/field/images_tiny`,
                body: JSON.stringify({ set: myAccountInfo.avatarUrl }),
            });

            avatarChangeUrlField.text = "";
            close();
        }

        onRejected: {
            avatarChangeUrlField.text = "";
            close();
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8

            Overte.Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                opacity: Overte.Theme.highContrast ? 1.0 : 0.6
                text: qsTr("Change profile picture")
            }
            Overte.Ruler { Layout.fillWidth: true }

            Overte.Label {
                Layout.topMargin: 12
                Layout.bottomMargin: 12
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                font.pixelSize: Overte.Theme.fontPixelSizeSmall
                text: qsTr("Profile pictures aren't stored on the directory server, so you need to supply a URL to your profile picture.\n\nSome services that let you upload images will give you links that will expire and stop working after a while.\n\n100x100 JPEG or WebP is recommended.")
            }

            Overte.TextField {
                Layout.fillWidth: true
                placeholderText: qsTr("Profile picture URL")
                id: avatarChangeUrlField
            }

            RowLayout {
                Layout.preferredWidth: 720
                Layout.fillWidth: true

                Overte.Button {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    text: qsTr("Cancel")

                    onClicked: changeAvatarDialog.rejected();
                }

                Item {
                    Layout.preferredWidth: 1
                    Layout.fillWidth: true
                }

                Overte.Button {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1

                    backgroundColor: Overte.Theme.paletteActive.buttonAdd
                    text: qsTr("Apply")
                    enabled: (
                        avatarChangeUrlField.text !== "" &&
                        avatarChangeUrlField.text.match(/^https?:\/\//)
                    )

                    onClicked: changeAvatarDialog.accepted()
                }
            }
        }
    }
}
