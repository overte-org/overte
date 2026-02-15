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
    property bool dragging: false
    property bool focused: false

    function fromScript(rawMsg) {
        const msg = JSON.parse(rawMsg);
        console.debug(rawMsg);

        switch (msg.event) {
            case "open": {
                root.title = msg.title ?? "";
                root.source = msg.qmlSource;
            } break;

            case "set_title": {
                root.title = msg.title;
            } break;

            case "close": {
                root.closed = true;
            } break;

            case "focus": {
                root.focused = true;
            } break;

            case "unfocus": {
                root.focused = false;
            } break;
        }
    }

    function toScript(msg) {
        eventBridge.emitWebEvent(JSON.stringify(msg));
    }

    Component.onCompleted: {
        eventBridge.scriptEventReceived.connect(fromScript);
        toScript({ event: "window_spawned" });
    }

    state: (closed || source === "") ? "CLOSED" : "OPEN"

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
            name: "OPEN"
            PropertyChanges { target: root; opacity: 1 }
            PropertyChanges { target: bodyPanel; y: 0 }
        }
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
                if (!running) {
                    root.toScript({ event: "finished_closing" });
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

        Loader {
            anchors.fill: parent
            anchors.margins: Math.max(parent.radius, parent.border.width)

            id: loader
            source: root.source
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

        color: Overte.Theme.paletteActive.base
        radius: Overte.Theme.borderRadius
        border.width: Overte.Theme.borderWidth
        border.color: Qt.darker(color, Overte.Theme.borderDarker)

        Overte.RoundButton {
            id: closeButton
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.top: parent.top
            anchors.margins: 6
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

        Overte.Label {
            id: titleText
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: dragThumb.left
            anchors.left: closeButton.right
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
                    radius: 2
                    height: 32
                    width: 4
                    y: (dragThumb.height - height) / 2

                    property color thumbColor: (
                        Overte.Theme.highContrast ?
                        Overte.Theme.paletteActive.text :
                        Overte.Theme.paletteActive.base
                    )

                    gradient: Gradient {
                        GradientStop {
                            position: 0;
                            color: Qt.darker(thumbColor, Overte.Theme.depthDarker)
                        }
                        GradientStop {
                            position: 1;
                            color: Qt.lighter(thumbColor, Overte.Theme.depthLighter)
                        }
                    }
                }
            }
        }

        MouseArea {
            id: dragArea
            anchors.left: closeButton.right
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.top: parent.top
            anchors.margins: 4
            implicitWidth: height
            acceptedButtons: Qt.LeftButton

            onPressed: {
                root.dragging = true;
                root.toScript({ event: "begin_drag" });
            }

            onReleased: {
                root.dragging = false;
                root.toScript({ event: "finish_drag" });
            }
        }
    }
}
