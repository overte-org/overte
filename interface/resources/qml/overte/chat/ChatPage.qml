import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../" as Overte
import "../settings" as OverteSettings
import "." as OverteChat

ColumnLayout {
    id: chatPage

    RowLayout {
        Layout.fillWidth: true

        // push everything to the right
        Item { Layout.fillWidth: true }

        Overte.RoundButton {
            Overte.ToolTip { text: qsTr("Clear chat history") }

            backgroundColor: (
                hovered ?
                Overte.Theme.paletteActive.buttonDestructive :
                Overte.Theme.paletteActive.button
            )

            implicitWidth: 36
            implicitHeight: 36
            horizontalPadding: 2
            verticalPadding: 2

            icon.source: "../icons/delete.svg"
            icon.width: 24
            icon.height: 24
            icon.color: Overte.Theme.paletteActive.buttonText

            onClicked: root.messagesCleared()
        }

        Overte.RoundButton {
            Overte.ToolTip { text: qsTr("Broadcast messages to whole domain") }

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

            implicitWidth: 36
            implicitHeight: 36
            horizontalPadding: 2
            verticalPadding: 2

            icon.source: "../icons/broadcast.svg"
            icon.width: 24
            icon.height: 24
            icon.color: color

            checkable: true
            checked: root.settingBroadcast
            onToggled: {
                root.settingBroadcast = checked;
                root.sendSettingsUpdate();
            }
        }

        Overte.RoundButton {
            Overte.ToolTip { text: qsTr("Settings") }

            implicitWidth: 36
            implicitHeight: 36
            horizontalPadding: 2
            verticalPadding: 2

            icon.source: "../icons/settings_cog.svg"
            icon.width: 24
            icon.height: 24

            onClicked: stack.push("./SettingsPage.qml")
        }
    }

    Overte.Label {
        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: chatLog.model.count === 0

        id: noMessagesLabel
        text: qsTr("No messages")
        opacity: 0.5
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    ScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical: Overte.ScrollBar {
            interactive: true
            policy: ScrollBar.AlwaysOn
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
        }
        visible: chatLog.model.count > 0

        ListView {
            id: chatLog
            clip: true
            spacing: 8

            model: ListModel {}
            delegate: MessageBlock {}

            highlightMoveDuration: Overte.Theme.reducedMotion ? 1 : 500
            highlightResizeDuration: Overte.Theme.reducedMotion ? 1 : 500

            Connections {
                target: root

                function onMessagesCleared() {
                    chatLog.model.clear();
                }

                function onMessagePushed(name, body, timestamp) {
                    const imageUrlRegex = /(https?:\/\/\S+\.(?:png|jpg|jpeg|gif|webp|svg)\S*)/gmi;
                    let detectedImageUrls = body.match(imageUrlRegex) ?? [];

                    chatLog.model.append({
                        name: name,
                        body: (
                            body
                            .replace(/\&/g, "&amp;")
                            .replace(/\[/g, "&#91;")
                            .replace(/\]/g, "&#93;")
                            .replace(/\</g, "&lt;")
                            .replace(/\>/g, "&gt;")
                            .replace(/\'/g, "&apos;")
                            .replace(/\"/g, "&quot;")
                            .replace(/\n/g, "<br>")
                            // strip any image links, as they're
                            // embedded and clickable anyway
                            .replace(imageUrlRegex, "")
                        ),
                        notification: "",
                        timestamp: timestamp,
                        imageEmbeds: detectedImageUrls.map(e => ({ modelData: e })),
                    });
                    chatLog.currentIndex = chatLog.model.count - 1;
                }

                function onNotificationPushed(text, timestamp) {
                    chatLog.model.append({
                        name: "",
                        body: "",
                        notification: text,
                        timestamp: timestamp,
                        imageEmbeds: [],
                    });
                    chatLog.currentIndex = chatLog.model.count - 1;
                }
            }
        }
    }

    Overte.Label {
        Layout.fillWidth: true
        Layout.leftMargin: 8
        Layout.rightMargin: 8

        id: typingIndicator

        text: ""
        opacity: Overte.Theme.highContrast ? 1.0 : 0.6
        wrapMode: Text.Wrap
        font.pixelSize: Overte.Theme.fontPixelSizeSmall
        font.italic: true

        Connections {
            target: root

            function onTypingIndicatorNamesChanged() {
                const values = Object.values(root.typingIndicatorNames);

                if (values.length === 0) {
                    typingIndicator.text = "";
                } else {
                    typingIndicator.text = values.join(", ") + qsTr(" typing…");
                }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.fillHeight: false

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumHeight: Overte.Theme.fontPixelSize * 6

            ScrollBar.vertical: Overte.ScrollBar {
                interactive: false
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
            }

            Overte.TextArea {
                id: messageInput
                placeholderText: (
                    root.settingBroadcast ?
                    qsTr("Broadcast chat message…") :
                    qsTr("Local chat message…")
                )
                wrapMode: TextEdit.Wrap
                KeyNavigation.priority: KeyNavigation.BeforeItem
                KeyNavigation.tab: messageSend

                Keys.onPressed: event => {
                    if (
                        (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) &&
                        !(event.modifiers & Qt.ShiftModifier)
                    ) {
                        messageSend.clicked();
                        event.accepted = true;
                    }
                }

                property bool hadText: false

                onTextChanged: {
                    if (text === "") {
                        toScript({event: "end_typing"});
                        hadText = false;
                    } else if (!hadText) {
                        toScript({event: "start_typing"});
                        hadText = true;
                    }
                }
            }
        }

        Overte.RoundButton {
            id: messageSend
            Layout.fillHeight: true
            Layout.preferredWidth: 40
            horizontalPadding: 2
            verticalPadding: 2

            icon.source: "../icons/send.svg"
            icon.width: 24
            icon.height: 24
            icon.color: Overte.Theme.paletteActive.buttonText

            onClicked: {
                let text = messageInput.text.trim();
                messageInput.text = "";

                if (text === "") { return; }

                toScript({event: "send_message", body: text});
            }
        }
    }
}
