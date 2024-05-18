import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

Rectangle {
    color: Qt.rgba(0.1,0.1,0.1,1)
    signal sendToScript(var message);

    property string pageVal: "local"
    property string last_message_user: ""
    property date last_message_time: new Date()

    // TODO: Find a better way to do this
    // When the window is created on the script side, the window starts open.
    // Once the QML window is created wait, then send the initialized signal.
    // This signal is mostly used to close the "Desktop overlay window" script side
    // https://github.com/overte-org/overte/issues/824
    Timer {
        interval: 100
        running: true
        onTriggered: {
           toScript({type: "initialized"})
        }
    }
    // Component.onCompleted: {
    //     toScript({type: "initialized"})
    // }

    Item {
        anchors.fill: parent

        // Navigation Bar
        Rectangle {
            id: navigation_bar
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
                    id: local_page

                    Image {
                        source: "./img/ui/" + (pageVal === "local" ? "social_white.png" : "social_black.png")
                        sourceSize.width: 40
                        sourceSize.height: 40
                        anchors.centerIn: parent
                    }

                    Behavior on width {
                        NumberAnimation {
                            duration: 50
                            // easing.type: Easeing.InOutQuad
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            pageVal = "local";
                        }
                    }
                }
                Rectangle {
                    width: pageVal === "domain" ? 100 : 60
                    height: parent.height
                    color: pageVal === "domain" ? "#505186" : "white"
                    anchors.left: local_page.right
                    anchors.leftMargin: 5
                    id: domain_page

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
                            // addMessage("usertest", "Clicked", "Now", "domain", "notification");
                            pageVal = "domain"
                        }
                    }
                }

                Rectangle {
                        width: pageVal === "settings" ? 100 : 60
                        height: parent.height
                        color: pageVal === "settings" ? "#505186" : "white"
                        anchors.right: parent.right
                        id: settings_page

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
            anchors.top: navigation_bar.bottom
            visible: ["local", "domain"].includes(pageVal) ? true : false


            // Chat Message History
            ListView {
                width: parent.width
                height: parent.height - 40
                clip: true
                interactive: true
                spacing: 5
                id: listview

                delegate: Loader {
                    width: parent.width
                    property int delegateIndex: index
                    property string delegateText: model.text
                    property string delegateUsername: model.username
                    property string delegateDate: model.date

                    sourceComponent: {
                        if (model.type === "chat") {
                            return template_chat_message;
                        } else if (model.type === "notification") {
                            return template_notification;
                        }
                    }
                }

                model: getChannel(pageVal)

            }

            ListModel {
                id: local
            }

            ListModel {
                id: domain
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
                        width: parent.width - 60
                        height: parent.height
                        placeholderText: pageVal.charAt(0).toUpperCase() + pageVal.slice(1) + " chat message..."

                        onAccepted: {
                            toScript({type: "send_message", message: text, channel: pageVal});
                            text = ""
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
                            toScript({type: "send_message", message: parent.children[0].text, channel: pageVal});
                            parent.children[0].text = ""
                        }
                        Keys.onPressed: {
                            if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                                toScript({type: "send_message", message: parent.children[0].text, channel: pageVal});
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
            anchors.top: navigation_bar.bottom
            visible: ["local", "domain"].includes(pageVal) ? false : true

            Column {
                width: parent.width - 10
                height: parent.height - 10
                anchors.centerIn: parent
                spacing: 0

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
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter

                        onCheckedChanged: {
                            toScript({type: 'setting_change', setting: 'external_window', value: checked})
                        }
                    }
                }

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
                            toScript({type: "action", action: "erase_history"})
                        }
                    }
                }
            }
        }

    }

    Component {
        id: template_chat_message

        Rectangle{
            property int index: delegateIndex
            property string texttest: delegateText
            property string username: delegateUsername
            property string date: delegateDate

            width: parent.width
            height: Math.max(65, children[1].height + 30)
            color: index % 2 === 0 ? "transparent" : Qt.rgba(0.15,0.15,0.15,1)

            Item {
                width: parent.width
                height: 22

                Text{
                    text: username
                    color: "lightgray"
                }

                Text{
                    anchors.right: parent.right
                    text: date
                    color: "lightgray"
                }
            }

            TextEdit{
                anchors.top: parent.children[0].bottom
                text: texttest
                color:"white"
                font.pointSize: 12
                readOnly: true
                selectByMouse: true
                selectByKeyboard: true
                width: parent.width * 0.8
                height: contentHeight // Adjust height to fit content
                wrapMode: Text.Wrap
            }
        }
    }

    Component {
        id: template_notification

        // width: (Math.min(parent.width * 0.8, Math.max(contentWidth, parent.width))) - parent.children[0].width

        Rectangle{
            property int index: delegateIndex
            property string texttest: delegateText
            property string username: delegateUsername
            property string date: delegateDate
            color: "#171717"
            width: parent.width
            height: 40

            Item {
                width: 10
                height: parent.height

                Rectangle {
                    height: parent.height
                    width: 5
                    color: "#505186"
                }
            }


            Item {
                width: parent.width - parent.children[0].width - 5
                height: parent.height
                anchors.left: parent.children[0].right

                TextEdit{
                    text: texttest
                    color:"white"
                    font.pointSize: 12
                    readOnly: true
                    width: parent.width
                    selectByMouse: true
                    selectByKeyboard: true
                    height: parent.height
                    wrapMode: Text.Wrap
                    verticalAlignment: Text.AlignVCenter
                    font.italic: true
                }

                Text {
                    text: date
                    color:"white"
                    font.pointSize: 12
                    anchors.right: parent.children[0].right
                    height: parent.height
                    wrapMode: Text.Wrap
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.italic: true
                }
            }

        }

    }



    property var channels: {
        "local": local,
        "domain": domain,
    }

    function scrollToBottom() {
        listview.positionViewAtIndex(listview.count - 1, ListView.End);
        listview.positionViewAtEnd();
        listview.contentY = listview.contentY + 50;
    }


    function addMessage(username, message, date, channel, type){
        channel = getChannel(channel)

        if (type === "notification"){
            channel.append({ text: message, date: date, type: "notification" });
            last_message_user = "";
            scrollToBottom();
            last_message_time = new Date();
            return;
        }

        var current_time = new Date();
        var elapsed_time = current_time - last_message_time;
        var elapsed_minutes = elapsed_time / (1000 * 60); 

        var last_item_index = channel.count - 1;
        var last_item = channel.get(last_item_index);

        if (last_message_user === username && elapsed_minutes < 1 && last_item){
            last_item.text = last_item.text += "\n" + message;
            scrollToBottom()
            last_message_time = new Date();
            return;
        }

        last_message_user = username;
        last_message_time = new Date();
        channel.append({ text: message, username: username, date: date, type: type });
        scrollToBottom();
    }

    function getChannel(id) {
        return channels[id];
    }

    // Messages from script
    function fromScript(message) {
        let time = new Date().toLocaleTimeString(undefined, { hour12: false });
        let date = new Date().toLocaleDateString(undefined, {  month: "long", day: "numeric", });

        switch (message.type){
            case "show_message":
                addMessage(message.displayName, message.message, `[ ${time} - ${date} ]`, message.channel, "chat");
                break;
            case "avatar_connected":
                addMessage("SYSTEM", message.message, `[ ${time} - ${date} ]`, "domain", "notification");
                break;
        }
    }

    // Send message to script
    function toScript(packet){
        sendToScript(packet)
    }
}
