import QtQuick
import QtQuick.Controls

import ".." as Overte
import "."

Item {
    id: root
    anchors.fill: parent

    property string source: ""
    property string title: "Window title"

    property bool closed: false
    property bool hidden: false
    property bool focused: false

    property bool dragging: false

    property bool pinnable: false
    property bool pinned: false

    property real depthGradient: Overte.Theme.highContrast ? 1 : 1.1

    function readEvent(event) {
        switch (event.event) {
            case "set_props": {
                if (event.title !== undefined) { root.title = event.title; }
                if (event.source_url !== undefined) { root.source = event.source_url; }
                if (event.pinnable !== undefined) { root.pinnable = event.pinnable; }
            } break;

            case "close": { root.closed = true; } break;
            case "focus": { root.focused = true; } break;
            case "unfocus": { root.focused = false; } break;
            case "pin": { root.pinned = true; } break;
            case "unpin": { root.pinned = false; } break;
            case "hide": { root.hidden = true; } break;
            case "unhide": { root.hidden = false; } break;
        }
    }

    function fromScript(rawMsg) {
        let msg;
        try { msg = JSON.parse(rawMsg); } catch (_) {}

        if (msg?.dash_window?.event) {
            readEvent(msg.dash_window);
        } else {
            loader.item?.fromScript(rawMsg);
        }
    }

    function toScript(msg) {
        eventBridge.emitWebEvent(JSON.stringify(msg));
    }

    function pushWindowEvent(event) {
        root.toScript({ dash_window: event });
    }

    Component.onCompleted: {
        eventBridge.scriptEventReceived.connect(fromScript);
        root.pushWindowEvent({ event: "window_spawned" });
    }

    state: {
        if (root.closed || root.source === "") {
            return "CLOSED";
        } else if (root.hidden) {
            return "HIDDEN";
        } else {
            return "OPEN";
        }
    }

    states: [
        State {
            name: "CLOSED"
            PropertyChanges { target: root; opacity: 0 }
            PropertyChanges {
                target: bodyPanel
                y: Overte.Theme.reducedMotion ? 0 : bodyPanel.height
            }
        },
        State {
            name: "HIDDEN"
            PropertyChanges { target: root; opacity: 0 }
            PropertyChanges {
                target: bodyPanel
                y: Overte.Theme.reducedMotion ? 0 : bodyPanel.height
            }
        },
        State {
            name: "OPEN"
            PropertyChanges { target: root; opacity: 1 }
            PropertyChanges { target: bodyPanel; y: 0 }
        },
    ]

    transitions: [
        Transition {
            from: "CLOSED"
            to: "OPEN"
            PropertyAnimation { target: root; property: "opacity"; duration: 250 }
            PropertyAnimation {
                target: bodyPanel
                property: "y"
                duration: 500
                easing.type: Easing.OutExpo
            }
        },
        Transition {
            from: "OPEN"
            to: "CLOSED"
            PropertyAnimation {
                target: root
                property: "opacity"
                duration: 150
            }
            PropertyAnimation {
                target: bodyPanel
                property: "y"
                duration: 250
                easing.type: Easing.OutExpo
            }

            onRunningChanged: {
                if (!running && root.state === "CLOSED") {
                    root.pushWindowEvent({ event: "finished_closing" });
                }
            }
        },

        Transition {
            from: "HIDDEN"
            to: "OPEN"
            PropertyAnimation { target: root; property: "opacity"; duration: 250 }
            PropertyAnimation {
                target: bodyPanel
                property: "y"
                duration: 500
                easing.type: Easing.OutExpo
            }
        },
        Transition {
            from: "OPEN"
            to: "HIDDEN"
            PropertyAnimation {
                target: root
                property: "opacity"
                duration: 150
            }
            PropertyAnimation {
                target: bodyPanel
                property: "y"
                duration: 250
                easing.type: Easing.OutExpo
            }

            onRunningChanged: {
                if (!running && root.state === "HIDDEN") {
                    root.pushWindowEvent({ event: "finished_hiding" });
                }
            }
        },
    ]

    Rectangle {
        id: bodyPanel

        x: 0
        y: 0
        width: root.width
        height: root.height - titlebar.height - 8

        color: Overte.Theme.paletteActive.base
        radius: Overte.Theme.borderRadius
        border.width: Overte.Theme.borderWidth
        border.color: (
            root.focused ?
            Overte.Theme.paletteActive.focusRing :
            Qt.darker(color, Overte.Theme.borderDarker)
        )

        gradient: Gradient {
            GradientStop {
                position: 0;
                color: Qt.lighter(bodyPanel.color, root.depthGradient)
            }
            GradientStop {
                position: 1;
                color: Qt.darker(bodyPanel.color, root.depthGradient)
            }
        }

        Loader {
            anchors.fill: parent
            anchors.margins: Math.max(parent.radius, parent.border.width)

            id: loader
            source: root.source
            clip: true
        }

        Overte.Label {
            anchors.fill: parent
            anchors.margins: Math.max(parent.radius, parent.border.width)
            visible: loader.status === Loader.Error
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.Wrap

            text: qsTr("Error loading %1").arg(root.source)
        }
    }

    Rectangle {
        id: titlebar
        anchors.left: root.left
        anchors.right: root.right
        anchors.bottom: root.bottom
        implicitHeight: 48

        color: (
            loader.status === Loader.Error ?
            Overte.Theme.paletteActive.buttonDestructive :
            Overte.Theme.paletteActive.base
        )
        radius: Overte.Theme.borderRadius
        border.width: Overte.Theme.borderWidth
        border.color: Qt.darker(color, Overte.Theme.borderDarker)

        gradient: Gradient {
            GradientStop {
                position: 0;
                color: Qt.lighter(titlebar.color, root.depthGradient)
            }
            GradientStop {
                position: 1;
                color: Qt.darker(titlebar.color, root.depthGradient)
            }
        }

        Row {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.margins: 6

            id: titleButtons
            spacing: 2

            Overte.RoundButton {
                id: closeButton
                anchors.bottom: parent.bottom
                anchors.top: parent.top
                implicitWidth: height
                focusPolicy: Qt.NoFocus

                icon.source: "../icons/close.svg"
                icon.width: 24
                icon.height: 24
                icon.color: Overte.Theme.paletteActive.buttonText
                backgroundColor: (
                    hovered ?
                    Overte.Theme.paletteActive.buttonDestructive :
                    Overte.Theme.paletteActive.button
                );

                onClicked: root.closed = true
            }

            Overte.RoundButton {
                visible: root.pinnable

                id: pinButton
                anchors.bottom: parent.bottom
                anchors.top: parent.top
                implicitWidth: height
                focusPolicy: Qt.NoFocus

                checked: root.pinned

                icon.source: (
                    root.pinned ?
                    "../icons/triangle_down.svg" :
                    "../icons/triangle_up.svg"
                )
                icon.width: 24
                icon.height: 24
                icon.color: Overte.Theme.paletteActive.buttonText
                backgroundColor: (
                    checked ?
                    Overte.Theme.paletteActive.buttonFavorite :
                    Overte.Theme.paletteActive.button
                );

                onClicked: {
                    root.pinned = !root.pinned;
                    root.pushWindowEvent({ event: root.pinned ? "pin" : "unpin" });
                }
            }
        }

        Overte.Label {
            id: titleText
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: dragThumb.left
            anchors.left: titleButtons.right
            anchors.leftMargin: 8
            anchors.rightMargin: 8

            elide: Text.ElideRight
            textFormat: Text.PlainText
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            text: root.title
            opacity: Overte.Theme.highContrast || root.focused ? 1 : 0.5
        }

        Row {
            id: dragThumb
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.top: parent.top
            anchors.margins: 4
            anchors.rightMargin: 12
            spacing: 8

            Repeater {
                model: 3

                Rectangle {
                    height: 24
                    width: 2
                    y: (dragThumb.height - height) / 2

                    property color thumbColor: (
                        Overte.Theme.highContrast ?
                        Overte.Theme.paletteActive.text :
                        titlebar.color
                    )
                    property real depthGradient: (
                        Overte.Theme.highContrast ?
                        1.0 :
                        1.5
                    )

                    gradient: Gradient {
                        GradientStop {
                            position: 0;
                            color: Qt.darker(thumbColor, depthGradient)
                        }
                        GradientStop {
                            position: 1;
                            color: Qt.lighter(thumbColor, depthGradient)
                        }
                    }
                }
            }
        }

        MouseArea {
            id: dragArea
            anchors.left: titleButtons.right
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.top: parent.top
            anchors.margins: 4
            implicitWidth: height
            acceptedButtons: Qt.LeftButton

            onPressed: {
                root.dragging = true;
                root.pushWindowEvent({ event: "begin_drag" });
            }

            onReleased: {
                root.dragging = false;
                root.pushWindowEvent({ event: "finish_drag" });
            }
        }
    }
}
