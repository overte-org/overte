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

    property int status: MyAccountInfo.Status.Invisible
    property string avatarImgSource: "file:///home/ada/art/doodles/bevy blep avi.png"

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
                root.avatarImgSource :
                "../icons/unset_avatar.svg"
            )
            status: root.status
            Layout.preferredWidth: 64
            Layout.preferredHeight: 64
            Layout.leftMargin: 8
            Layout.rightMargin: 8
        }

        ColumnLayout {
            Layout.rightMargin: 8
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

            Overte.TextField {
                Layout.fillWidth: true
                verticalAlignment: Text.AlignVCenter

                id: displayName
                text: "ada.tv"
                placeholderText: qsTr("Display Name")
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
                    }

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
