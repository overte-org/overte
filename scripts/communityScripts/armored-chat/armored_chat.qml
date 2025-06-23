import QtQuick 2.7
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import controlsUit 1.0 as HifiControlsUit

Rectangle {
    color: Qt.rgba(0.1,0.1,0.1,1)
    signal sendToScript(var message);

    property string pageVal: "local"
    property string last_message_user: ""
    property date last_message_time: new Date()

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
            load_scroll_timer.bypassDistanceCheck = true
            load_scroll_timer.running = true
        }
    }
    Timer {
        id: load_scroll_timer
        interval: 100
        running: false
        repeat: false

        property bool bypassDistanceCheck: false    // One time event. Whether we should bypass the distance check, and scroll to the bottom regardless or not.

        onTriggered: {
           scrollToBottom(bypassDistanceCheck);
           bypassDistanceCheck = false;             // Set the property to false, this was a one time event!
        }
    }

    // User view
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
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            pageVal = "local";
                            load_scroll_timer.bypassDistanceCheck = true;
                            load_scroll_timer.running = true;
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
                            pageVal = "domain";
                            load_scroll_timer.bypassDistanceCheck = true;
                            load_scroll_timer.running = true;
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
            Flickable {
                width: parent.width
                height: parent.height - 40
                contentWidth: parent.width
                contentHeight: listview.height
                clip: true
                id: messageViewFlickable

                ColumnLayout {
                    id: listview 
                    Layout.fillWidth: true

                    Repeater {
                        model: getChannel(pageVal)
                        delegate: Loader {
                            property int delegateIndex: model.index
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
                    }
                }

                ScrollBar.vertical: ScrollBar { 
                    size: 100
                    minimumSize: 0.1
                }

                rebound: Transition {
                    NumberAnimation {
                        properties: "x,y"
                        duration: 1
                    }
                }
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
                        clip: false
                        font.italic: text == ""

                        Keys.onPressed: {
                            if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter) && !(event.modifiers & Qt.ShiftModifier)) {
                                event.accepted = true;
                                toScript({type: "send_message", message: text, channel: pageVal});
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
                        id: s_external_window
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter

                        onCheckedChanged: {
                            toScript({type: 'setting_change', setting: 'external_window', value: checked})
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
                        id: s_maximum_messages
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        decimals: 0
                        width: 100
                        height: parent.height
                        realFrom: 1
                        realTo: 1000
                        backgroundColor: "#cccccc"

                        onValueChanged: {
                            toScript({type: 'setting_change', setting: 'maximum_messages', value: value})
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
                            toScript({type: "action", action: "erase_history"})
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
                        id: s_join_notification
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter

                        onCheckedChanged: {
                            toScript({type: 'setting_change', setting: 'join_notification', value: checked})
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
                        id: s_chat_bubbles
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter

                        onCheckedChanged: {
                            toScript({type: 'setting_change', setting: 'use_chat_bubbles', value: checked})
                        }
                    }
                }
            }
        }

    }

    // Templates
    Component {
        id: template_chat_message

        Rectangle {
            property int index: delegateIndex
            property string texttest: delegateText
            property string username: delegateUsername
            property string date: delegateDate

            height: Math.max(65, children[1].height + 30)
            color: index % 2 === 0 ? "transparent" : Qt.rgba(0.15,0.15,0.15,1)
            width: listview.parent.parent.width
            Layout.fillWidth: true

            Item {
                width: parent.width - 10
                anchors.horizontalCenter: parent.horizontalCenter
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

            TextEdit {
                anchors.top: parent.children[0].bottom
                x: 5
                text: texttest
                color:"white"
                font.pointSize: 12
                readOnly: true
                selectByMouse: true
                selectByKeyboard: true
                width: parent.width * 0.8
                height: contentHeight
                wrapMode: Text.Wrap
                textFormat: TextEdit.RichText

                onLinkActivated: {
                    Window.openWebBrowser(link)
                }
            }
        }
    }

    Component {
        id: template_notification

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
                    width: parent.width * 0.8
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
                    anchors.right: parent.right
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

    function scrollToBottom(bypassDistanceCheck = false, extraMoveDistance = 0) {
        const totalHeight = listview.height; // Total height of the content
        const currentPosition = messageViewFlickable.contentY; // Current position of the view
        const windowHeight = listview.parent.parent.height; // Total height of the window
        const bottomPosition = currentPosition + windowHeight;

        // Check if the view is within 300 units from the bottom
        const closeEnoughToBottom = totalHeight - bottomPosition <= 300;
        if (!bypassDistanceCheck && !closeEnoughToBottom) return;
        if (totalHeight < windowHeight) return; // No reason to scroll, we don't have an overflow.
        if (bottomPosition == totalHeight) return; // At the bottom, do nothing.

        messageViewFlickable.contentY = listview.height - listview.parent.parent.height;
        messageViewFlickable.returnToBounds();
    }


    function addMessage(username, message, date, channel, type){
        channel = getChannel(channel)

        // Format content
        message = formatContent(message);
        message = embedImages(message);

        if (type === "notification"){
            channel.append({ text: message, date: date, type: "notification" });
            last_message_user = "";
            scrollToBottom(null, 30);

            last_message_time = new Date();
            return;
        }

        var current_time = new Date();
        var elapsed_time = current_time - last_message_time;
        var elapsed_minutes = elapsed_time / (1000 * 60); 

        var last_item_index = channel.count - 1;
        var last_item = channel.get(last_item_index);

        if (last_message_user === username && elapsed_minutes < 1 && last_item){
            message = "<br>" + message 
            last_item.text = last_item.text += "\n" + message;
            load_scroll_timer.running = true;
            last_message_time = new Date();
            return;
        }

        last_message_user = username;
        last_message_time = new Date();
        channel.append({ text: message, username: username, date: date, type: type });
        load_scroll_timer.running = true;
    }

    function getChannel(id) {
        return channels[id];
    }

    function formatContent(mess) {
        var arrow = /\</gi
        mess = mess.replace(arrow, "&lt;");

        var link = /https?:\/\/(www\.)?[-a-zA-Z0-9@:%._\+~#=]{1,256}\.[a-zA-Z0-9()]{1,6}\b([-a-zA-Z0-9()@:%_\+.~#?&//=]*)/g;
        mess = mess.replace(link, (match) => {return `<a style="color:#4EBAFD" onclick='Window.openUrl("+match+")' href='` + match + `'>` + match + `</a> <a onclick='Window.openUrl(`+match+`)'>🗗</a>`});

        var newline = /\n/gi;
        mess = mess.replace(newline, "<br>");
        return mess
    }

    function embedImages(mess){
        var image_link = /(https?:(\/){2})[\w.-]+(?:\.[\w\.-]+)+(?:\/[\w\-\._~:/?#[\]@!\$&'\(\)\*\+,;=.]*)(?:png|jpe?g|gif|bmp|svg|webp)/g;
        var matches = mess.match(image_link);
        var new_message = ""
        var listed = []
        var total_emeds = 0

        new_message += mess

        for (var i = 0; matches && matches.length > i && total_emeds < 3; i++){
            if (!listed.includes(matches[i])) {
                new_message += "<br><img src="+ matches[i] +" width='250' >"
                listed.push(matches[i]);
                total_emeds++
            } 
        }
        return new_message;
    }

    // Messages from script
    function fromScript(message) {

        switch (message.type){
            case "show_message":
                addMessage(message.displayName, message.message, `[ ${message.timeString} - ${message.dateString} ]`, message.channel, "chat");
                break;
            case "notification":
                addMessage("SYSTEM", message.message, `[ ${message.timeString} - ${message.dateString} ]`, "domain", "notification");
                break;
            case "clear_messages":
                local.clear();
                domain.clear();
                break;
            case "initial_settings":
                if (message.settings.external_window) s_external_window.checked = true;
                if (message.settings.maximum_messages) s_maximum_messages.value = message.settings.maximum_messages;
                if (message.settings.join_notification) s_join_notification.checked = true;
                if (message.settings.use_chat_bubbles) s_chat_bubbles.checked = true;
                break;
        }
    }

    // Send message to script
    function toScript(packet){
        sendToScript(packet)
    }
}
