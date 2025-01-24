import QtQuick 2.5
import QtQuick.Controls 1.4

Item {
    id: root
    property var window

    Binding { target: root; property:'window'; value: parent.parent; when: Boolean(parent.parent) }
    Binding { target: window; property: 'shown'; value: false; when: Boolean(window) }
    Component.onDestruction: chat_bar && chat_bar.destroy()

    property alias chat_bar: chat_bar

    Rectangle {
        id: chat_bar
        parent: desktop
        x: 0
        y: parent.height - height
        width: parent.width
        height: 50
        z: 99
        visible: false

        Rectangle {
            width: parent.width
            height: parent.height
            color: Qt.rgba(0.95,0.95,0.95,1)
        }

        TextInput {
            id: textArea
            x: 5
            width: parent.width
            height: parent.height
            text: ""
            color: "#000"
            clip: false
            font.pointSize: 18
            verticalAlignment: Text.AlignVCenter

            Keys.onReturnPressed: { _onEnterPressed(); }
            Keys.onEnterPressed: { _onEnterPressed(); }
            Keys.onLeftPressed: { moveLeft(); }
            Keys.onRightPressed: { moveRight(); }

            function moveLeft(){
                if (cursorPosition > 0){
                    cursorPosition--
                }
            }
            function moveRight(){
                if (cursorPosition < text.length){
                    cursorPosition++
                }
            }
        }

        Text {
            text: "Local message..."
            font.pointSize: 16
            color: "gray"
            x: 5
            width: parent.width
            anchors.verticalCenter: parent.verticalCenter
            visible: textArea.text == ""
            font.italic: true
        }

        Rectangle {
            id: button
            x: parent.width - width
            y: 0
            width: 64
            height: parent.height
            clip: false
            color: "#262626"

            Image {
                id: image
                width: 30
                height: 30
                fillMode: Image.PreserveAspectFit
                anchors.centerIn: parent
                source: "./img/ui/send_white.png"
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    _onEnterPressed();
                }
            }
        }

    }

    function _onEnterPressed() {
        changeVisibility(false)
        toScript({type: "send_message", message: textArea.text, channel: "local"})
        textArea.text = "";
    }

    function changeVisibility(state){
        chat_bar.visible = state
        if (state) textArea.forceActiveFocus();
        else root.parent.forceActiveFocus();
    }

    // Messages from script
    function fromScript(message) {
        switch (message.type){
            case "change_visibility":
                changeVisibility(message.value)
                break;
        }
    }

    // Send message to script
    function toScript(packet){
        sendToScript(packet)
    }
}