import QtCore as QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import ".." as Overte
import "."

Rectangle {
    id: root
    radius: 8
    border.width: Overte.Theme.borderWidth
    border.color: Qt.darker(color, 1.2)
    color: Overte.Theme.paletteActive.base

    implicitHeight: 96
    implicitWidth: 320

    enum Status {
        LoggedOut = -1,
        Invisible,
        FriendsOnly,
        Contacts,
        Everyone
    }

    // cache the last avatar image url so there's not a few
    // seconds of placeholder avatar while the profile loads
    QtCore.Settings {
        id: settings
        category: "MyAccountInfo"
        property url cachedAvatarUrl: "../icons/unset_avatar.svg"
    }

    Connections {
        target: AccountServices

        function loggedInChanged(loggedIn) {
            updateStatus();
        }
    }

    Component.onCompleted: updateStatus()

    function updateStatus() {
        if (status === MyAccountInfo.Status.LoggedOut && AccountServices.loggedIn) {
        contactsList.authRequest({
            url: `${AccountServices.metaverseServerURL}/api/v1/account/${AccountServices.username}/field/images_tiny`,
            callback: response => {
                const data = JSON.parse(response.responseText);
                if (data.data) {
                    root.avatarUrl = data.data;
                }
            },
        });
        }

        if (!AccountServices.loggedIn) {
            status = MyAccountInfo.Status.LoggedOut;
        } else {
            switch (AccountServices.findableBy) {
                case "none": status = MyAccountInfo.Status.Invisible; break;
                case "friends": status = MyAccountInfo.Status.FriendsOnly; break;
                case "contacts": status = MyAccountInfo.Status.Contacts; break;
                case "all": status = MyAccountInfo.Status.Everyone; break;
            }
        }
    }

    property int status: MyAccountInfo.Status.LoggedOut

    property string avatarUrl: settings.cachedAvatarUrl

    onAvatarUrlChanged: settings.cachedAvatarUrl = avatarUrl

    readonly property color currentStatusColor: {
        switch (status) {
            case MyAccountInfo.Status.LoggedOut:
                return Overte.Theme.paletteActive.statusOffline;

            case MyAccountInfo.Status.Invisible:
                return Overte.Theme.paletteActive.statusOffline;

            case MyAccountInfo.Status.FriendsOnly:
                return Overte.Theme.paletteActive.statusFriendsOnly;

            case MyAccountInfo.Status.Contacts:
                return Overte.Theme.paletteActive.statusContacts;

            case MyAccountInfo.Status.Everyone:
                return Overte.Theme.paletteActive.statusEveryone;
        }
    }

    readonly property string currentStatusName: {
        switch (status) {
            case MyAccountInfo.Status.LoggedOut:
                return qsTr("Logged Out");

            case MyAccountInfo.Status.Invisible:
                return qsTr("Invisible");

            case MyAccountInfo.Status.FriendsOnly:
                return qsTr("Friends Only");

            case MyAccountInfo.Status.Contacts:
                return qsTr("Contacts");

            case MyAccountInfo.Status.Everyone:
                return qsTr("Everyone");
        }
    }

    RowLayout {
        anchors.fill: parent

        AccountAvatar {
            id: avatarImg
            source: (
                root.status !== MyAccountInfo.Status.LoggedOut ?
                root.avatarUrl :
                "../icons/unset_avatar.svg"
            )
            status: root.status
            retainWhileLoading: true
            Layout.preferredWidth: 64
            Layout.preferredHeight: 64
            Layout.leftMargin: 8
            Layout.rightMargin: 8

            Overte.Button {
                anchors.fill: parent
                anchors.margins: Overte.Theme.borderWidth

                backgroundColor: Overte.Theme.paletteActive.buttonAdd
                opacity: hovered ? (Overte.Theme.highContrast ? 1.0 : 0.7) : 0
                enabled: AccountServices.loggedIn
                visible: AccountServices.loggedIn

                icon.source: "../icons/plus.svg"
                icon.width: 24
                icon.height: 24
                icon.color: Overte.Theme.paletteActive.buttonText

                onClicked: changeAvatarDialog.open()

                Overte.ToolTip { text: qsTr("Change profile picture") }
            }
        }

        ColumnLayout {
            Layout.rightMargin: 8
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

            Overte.TextField {
                Layout.fillWidth: true
                verticalAlignment: Text.AlignVCenter

                id: displayName
                text: MyAvatar.displayName
                placeholderText: AccountServices.username

                onEditingFinished: MyAvatar.displayName = text
            }

            RowLayout {
                Layout.fillWidth: true

                Overte.Label {
                    text: root.status === MyAccountInfo.Status.Invisible ? "○" : "●"
                    color: root.currentStatusColor
                }

                Overte.ComboBox {
                    Layout.fillWidth: true

                    id: statusBox
                    flat: true
                    color: root.status === MyAccountInfo.Status.Invisible ? Overte.Theme.paletteActive.windowText : root.currentStatusColor
                    textRole: "text"
                    valueRole: "value"
                    visible: root.status !== MyAccountInfo.Status.LoggedOut

                    onActivated: index => {
                        root.status = index;

                        switch (index) {
                            case 0: AccountServices.findableBy = "none"; break;
                            case 1: AccountServices.findableBy = "friends"; break;
                            case 2: AccountServices.findableBy = "connections"; break;
                            case 3: AccountServices.findableBy = "all"; break;
                        }
                    }

                    currentIndex: root.status
                    model: [
                        { value: MyAccountInfo.Status.Invisible, text: qsTr("Invisible") },
                        { value: MyAccountInfo.Status.FriendsOnly, text: qsTr("Friends Only") },
                        { value: MyAccountInfo.Status.Contacts, text: qsTr("Contacts") },
                        { value: MyAccountInfo.Status.Everyone, text: qsTr("Everyone") },
                    ]
                }

                Overte.Label {
                    Layout.fillWidth: true
                    visible: root.status === MyAccountInfo.Status.LoggedOut
                    verticalAlignment: Text.AlignVCenter
                    text: qsTr("Logged Out")
                }
            }
        }
    }
}
