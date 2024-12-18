import QtQuick 2.7
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import controlsUit 1.0 as HifiControlsUit
import "./qml_widgets"

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
            load_scroll_timer.running = true
        }
    }
    Timer {
        id: load_scroll_timer
        interval: 100
        running: false
        repeat: false
        onTriggered: {
           scrollToBottom(true);
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
                            pageVal = "domain"
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
                            property var delegateText: model.text
                            property string delegateUsername: model.username
                            property string delegateDate: model.date

                            sourceComponent: {
                                if (model.type === "chat") return template_chat_message;
                                if (model.type === "notification") return template_notification;
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
            }
        }

    }

    // Templates
    TemplateChatMessage { id: template_chat_message }
    TemplateNotification { id: template_notification }

    property var channels: {
        "local": local,
        "domain": domain,
    }

    function scrollToBottom(bypassDistanceCheck = false, extraMoveDistance = 0) {
        const totalHeight = listview.height;                    // Total height of the content
        const currentPosition = messageViewFlickable.contentY;  // Current position of the view
        const windowHeight = listview.parent.parent.height;     // Total height of the window
        const bottomPosition = currentPosition + windowHeight;

        // Check if the view is within 300 units from the bottom
        const closeEnoughToBottom = totalHeight - bottomPosition <= 300;
        if (!bypassDistanceCheck && !closeEnoughToBottom) return;
        if (totalHeight < windowHeight) return;                 // No reason to scroll, we don't have an overflow.
        if (bottomPosition == totalHeight) return;              // At the bottom, do nothing.

        messageViewFlickable.contentY = listview.height - listview.parent.parent.height;
        messageViewFlickable.returnToBounds();
    }


    function addMessage(username, message, date, channel, type){
        channel = getChannel(channel)

        // Format content

        if (type === "notification"){
            channel.append({ text: message, date: date, type: "notification" });
            scrollToBottom(null, 30);

            return;
        }

        channel.append({ text: message, username: username, date: date, type: type });
        load_scroll_timer.running = true;
    }

    function getChannel(id) {
        return channels[id];
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
                break;
        }
    }

    // Send message to script
    function toScript(packet){
        sendToScript(packet)
    }
}
