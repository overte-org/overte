import QtQuick 2.7
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import controlsUit 1.0 as HifiControlsUit
import "./qml"

Rectangle {
    color: Qt.rgba(0.1,0.1,0.1,1)
    signal sendToScript(var message);

    property string pageVal: "local"

    // When the window is created on the script side, the window starts open.
    // Once the QML window is created wait, then send the initialized signal.
    // This signal is mostly used to close the "Desktop overlay window" script side
    // https://github.com/overte-org/overte/issues/824
    Timer {
        interval: 10
        running: true
        repeat: false
        onTriggered: {
            toScript({type: "initialized"});
        }
    }

    // User view
    Item {
        anchors.fill: parent;
        width: parent.width;

        // Navigation Bar
        Rectangle {
            id: navigationBar
            width: parent.width
            height: 40
            color:Qt.rgba(0,0,0,1)

            Item {
                height: parent.height
                width: parent.width
                anchors.fill: parent

                Rectangle {
                    width: pageVal === "local" ? 100 : 60
                    height: parent.height
                    color: pageVal === "local" ? "#505186" : "white"
                    id: localPage

                    Image {
                        source: "./img/ui/" + (pageVal === "local" ? "social_white.png" : "social_black.png")
                        sourceSize.width: 40
                        sourceSize.height: 40
                        anchors.centerIn: parent
                    }

                    Behavior on width {
                        NumberAnimation {
                            duration: 50
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            pageVal = "local";
                            scrollToBottom();
                        }
                    }
                }
                Rectangle {
                    width: pageVal === "domain" ? 100 : 60
                    height: parent.height
                    color: pageVal === "domain" ? "#505186" : "white"
                    anchors.left: localPage.right
                    anchors.leftMargin: 5
                    id: domainPage

                    Image {
                        source: "./img/ui/" + (pageVal === "domain" ? "world_white.png" : "world_black.png")
                        sourceSize.width: 30
                        sourceSize.height: 30
                        anchors.centerIn: parent
                    }

                    Behavior on width {
                        NumberAnimation {
                            duration: 50
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            pageVal = "domain";
                            scrollToBottom();
                        }
                    }
                }

                Rectangle {
                        width: pageVal === "settings" ? 100 : 60
                        height: parent.height
                        color: pageVal === "settings" ? "#505186" : "white"
                        anchors.right: parent.right
                        id: settingsPage

                        Image {
                            source: "./img/ui/" + (pageVal === "settings" ? "settings_white.png" : "settings_black.png")
                            sourceSize.width: 30
                            sourceSize.height: 30
                            anchors.centerIn: parent
                        }

                        Behavior on width {
                        NumberAnimation {
                            duration: 50
                        }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                pageVal = "settings"
                            }
                        }
                    }
                }

        }

        // Pages
        Item {
            width: parent.width
            height: parent.height - 40
            anchors.top: navigationBar.bottom
            visible: ["local", "domain"].includes(pageVal) ? true : false

            // Chat Message History
            ListView {
                id: listview;
                width: parent.width;
                height: parent.height - 40;
                clip: true;
                model: pageVal == "local" ? localMessages : domainMessages;
                orientation: ListView.Vertical;
                spacing: 5;

                delegate: ChatMessage {
                    delegateMessage: model.text;
                    delegateUsername: model.username;
                    delegateDate: model.date;
                    isSystem: model.type === "notification";
                }

                ScrollBar.vertical: ScrollBar {
                    policy: Qt.ScrollBarAlwaysOn;

                    background: Rectangle {
                        color: "transparent";
                        radius: 5;
                        visible: parent.visible;
                    }
                }

                Component.onCompleted: {
                    listview.positionViewAtEnd();
                }
            }

            ListModel {
                id: localMessages;
            }
            ListModel {
                id: domainMessages;
            }

            // Chat Entry
            Rectangle {
                width: parent.width
                height: 40
                color: Qt.rgba(0.9,0.9,0.9,1)
                anchors.bottom: parent.bottom

                Row {
                    width: parent.width
                    height: parent.height

                    TextField {
                        width: parent.width - 60;
                        height: parent.height;
                        placeholderText: pageVal.charAt(0).toUpperCase() + pageVal.slice(1) + " chat message...";
                        clip: false;
                        font.italic: text == "";
                        selectByMouse: true;

                        Keys.onPressed: {
                            if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter) && !(event.modifiers & Qt.ShiftModifier)) {
                                event.accepted = true;
                                toScript({type: "sendMessage", message: text, channel: pageVal});
                                text = ""
                            }
                        }
                        onTextChanged: {
                            if (text === "") {
                                toScript({type: "action", action: "end_typing"});
                            } else {
                                toScript({type: "action", action: "start_typing"});
                            }
                        }
                        onFocusChanged: {
                            if (!HMD.active) return;
                            if (focus) return ApplicationInterface.showVRKeyboardForHudUI(true);
                            ApplicationInterface.showVRKeyboardForHudUI(false);
                        }
                    }
                    Button {
                        width: 60
                        height:parent.height

                        Image {
                            source: "./img/ui/send_black.png"
                            sourceSize.width: 30
                            sourceSize.height: 30
                            anchors.centerIn: parent
                        }

                        onClicked: {
                            toScript({type: "sendMessage", message: parent.children[0].text, channel: pageVal});
                            parent.children[0].text = ""
                        }
                        Keys.onPressed: {
                            if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                                toScript({type: "sendMessage", message: parent.children[0].text, channel: pageVal});
                                parent.children[0].text = ""
                            }
                        }
                    }
                }
            }
        }

        Item {
            width: parent.width
            height: parent.height - 40
            anchors.top: navigationBar.bottom
            visible: ["local", "domain"].includes(pageVal) ? false : true

            Column {
                width: parent.width - 10
                height: parent.height - 10
                anchors.centerIn: parent
                spacing: 10

                // External Window
                Rectangle {
                    width: parent.width
                    height: 40
                    color: "transparent"

                    Text{
                        text: "External window"
                        color: "white"
                        font.pointSize: 12
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    CheckBox{
                        id: s_externalWindow
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter

                        onCheckedChanged: {
                            toScript({type: 'settingChange', setting: 'externalWindow', value: checked})
                        }
                    }
                }

                // Maximum saved messages
                Rectangle {
                    width: parent.width
                    height: 40
                    color: "transparent"

                    Text{
                        text: "Maximum saved messages"
                        color: "white"
                        font.pointSize: 12
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    
                    HifiControlsUit.SpinBox {
                        id: s_maximumMessages
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        decimals: 0
                        width: 100
                        height: parent.height
                        realFrom: 1
                        realTo: 1000
                        backgroundColor: "#cccccc"

                        onValueChanged: {
                            toScript({type: 'settingChange', setting: 'maximumMessages', value: value})
                        }
                    }
                }

                // Erase History
                Rectangle {
                    width: parent.width
                    height: 40
                    color: Qt.rgba(0.15,0.15,0.15,1);

                    Text{
                        text: "Erase chat history"
                        color: "white"
                        font.pointSize: 12
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Button {
                        anchors.right: parent.right
                        text: "Erase"
                        height: parent.height
                        anchors.verticalCenter: parent.verticalCenter

                        onClicked: {
                            toScript({type: "action", action: "eraseHistory"})
                        }
                    }
                }


                // Join notification
                Rectangle {
                    width: parent.width
                    height: 40
                    color: "transparent"

                    Text{
                        text: "Join notification"
                        color: "white"
                        font.pointSize: 12
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    CheckBox{
                        id: s_joinNotification
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter

                        onCheckedChanged: {
                            toScript({type: 'settingChange', setting: 'joinNotification', value: checked})
                        }
                    }
                }

                // Chat bubbles
                Rectangle {
                    width: parent.width
                    height: 40
                    color: "transparent"

                    Text{
                        text: "In-world chat bubbles"
                        color: "white"
                        font.pointSize: 12
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    CheckBox{
                        id: s_useChatBubbles
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter

                        onCheckedChanged: {
                            toScript({type: 'settingChange', setting: 'useChatBubbles', value: checked})
                        }
                    }
                }
            }
        }

    }

    function scrollToBottom() {
        listview.positionViewAtEnd();
    }

    function addMessage(username, message, date, channel, type){
        // Format content
        message = formatContent(message);
        message = embedImages(message);

        if (type === "notification"){ 
            domainMessages.append({ text: message, username, date, type });
            scrollToBottom();
            return;
        }

        if (channel === "local") localMessages.append({ text: message, username, date, type });
        if (channel === "domain") domainMessages.append({ text: message, username, date, type });

        scrollToBottom();
    }

    function formatContent(mess) {
        var arrow = /\</gi
        mess = mess.replace(arrow, "&lt;");

        var link = /https?:\/\/(www\.)?[-a-zA-Z0-9@:%._\+~#=]{1,256}\.[a-zA-Z0-9()]{1,6}\b([-a-zA-Z0-9()@:%_\+.~#?&=,\/]*)/g;
        mess = mess.replace(link, (match) => {return `<a style="color:#4EBAFD" href='` + match + `?noOpen=true'>` + match + `</a> <a href='` + match + `'>🗗</a>`});

        var newline = /\n/gi;
        mess = mess.replace(newline, "<br>");
        return mess
    }

    function embedImages(mess){
        var imageLink = /(https?:(\/){2})[\w.-]+(?:\.[\w\.-]+)+(?:\/[\w\-\._~:/?#[\]@!\$&'\(\)\*\+,;=.]*)(?:png|jpe?g|gif|bmp|svg|webp)/g;
        var matches = mess.match(imageLink);
        var newMessage = ""
        var listed = []
        var totalEmbeds = 0

        newMessage += mess

        for (var i = 0; matches && matches.length > i && totalEmbeds < 3; i++){
            if (!listed.includes(matches[i])) {
                newMessage += "<br><img src="+ matches[i] +" width='250' >"
                listed.push(matches[i]);
                totalEmbeds++
            } 
        }
        return newMessage;
    }

    // Messages from script
    function fromScript(message) {

        switch (message.type){
            case "showMessage":
                addMessage(message.displayName, message.message, `[ ${message.timeString} - ${message.dateString} ]`, message.channel, "chat");
                break;
            case "notification":
                addMessage("SYSTEM", message.message, `[ ${message.timeString} - ${message.dateString} ]`, "domain", "notification");
                break;
            case "clearMessages":
                localMessages.clear();
                domainMessages.clear();
                break;
            case "initialSettings":
                if (message.settings.externalWindow) s_externalWindow.checked = true;
                if (message.settings.maximumMessages) s_maximumMessages.value = message.settings.maximumMessages;
                if (message.settings.joinNotification) s_joinNotification.checked = true;
                if (message.settings.useChatBubbles) s_useChatBubbles.checked = true;
                break;
        }
    }

    // Send message to script
    function toScript(packet){
        sendToScript(packet)
    }
}
