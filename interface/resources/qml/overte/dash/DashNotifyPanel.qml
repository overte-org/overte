import QtQuick

import ".." as Overte
import "."

Item {
    id: root
    anchors.fill: parent

    property var maxBubbles: 3

    function fromScript(rawMsg) {
        const msg = JSON.parse(rawMsg);

        if (msg?.notify === undefined) { return; }

        listView.model.insert(0, {
            text: msg.notify.text ?? "<Notification text is missing!!>",
            iconSource: msg.notify.icon ?? "",
            imageSource: msg.notify.image ?? "",
            lifetime: msg.notify.lifetime ?? 5,
        });

        if (listView.model.count > maxBubbles) {
            listView.model.remove(listView.model.count - 1, 1);
        }
    }

    Component.onCompleted: {
        eventBridge.scriptEventReceived.connect(fromScript);
    }

    ListView {
        id: listView
        anchors.fill: parent
        verticalLayoutDirection: ListView.BottomToTop
        spacing: 2
        model: ListModel {}

        add: Transition {
            NumberAnimation {
                property: "opacity"
                from: 0
                to: 0.95
                duration: 500
                easing.type: Easing.OutExpo
            }
            NumberAnimation {
                property: "y"
                duration: 500
                easing.type: Easing.OutExpo
            }
        }

        displaced: Transition {
            NumberAnimation {
                property: "y"
                duration: 500
                easing.type: Easing.OutExpo
            }
        }

        remove: Transition {
            NumberAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: 500
                easing.type: Easing.OutExpo
            }
        }

        delegate: DashNotification {
            anchors.left: parent?.left
            anchors.right: parent?.right

            onExpired: listView.model.remove(index, 1)
        }
    }
}
