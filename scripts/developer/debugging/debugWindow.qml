//
//  debugWindow.qml
//
//  Brad Hefta-Gaub, created on 12/19/2016.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.0
import Hifi 1.0 as Hifi
import TabletScriptingInterface 1.0

import stylesUit 1.0


ColumnLayout {
    id: root
    HifiConstants { id: hifi }
    width: parent ? parent.width : 100
    height: parent ? parent.height : 100

    signal moved(vector2d position);
    signal resized(size size);

    property var channel;

    Rectangle {
        Layout.fillHeight: true
        Layout.preferredWidth: parent.width
        color: hifi.colors.darkGray

        ScrollView {
            id: logView
            anchors.fill: parent
            clip: true

            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn

            TextArea {
                id: textArea
                width: root.width
                height: parent.height - bottomRow.height
                wrapMode: TextArea.Wrap
                placeholderText: qsTr("Debug messages from your running scripts will appear here...")
                placeholderTextColor: hifi.colors.lightGrayText
                readOnly: true
                selectByMouse: true
                textFormat: TextEdit.RichText

                background: Rectangle {
                    color: "transparent"
                }
                text:""

                onTextChanged: {
                    if (logView.scrollDelay) {
                        // Scroll to the bottom, but only once within a short period of time
                        // rather than for every message.
                        scrollTimer.restart();
                    } else {
                        // Scroll to the bottom, immediately!
                        logView.scrollToBottom();
                        logView.scrollDelay = true;
                    }
                }
            }

            // Do we want to use the timer for the next scroll (true),
            // or scroll immediately (false);
            property bool scrollDelay: true

            Timer {
                id: scrollTimer
                interval: 1000; running: false; repeat: false;
                onTriggered: {
                    logView.scrollToBottom();
                }
            }

            function scrollToBottom() { // This is not strictly necessary now, but is kept as a fail-safe
                logView.ScrollBar.vertical.position = 1.0 - logView.ScrollBar.vertical.size;
            }
        }

    }

    RowLayout {
        id: bottomRow
        Layout.preferredWidth: parent.width

        // Helpful text
        Text {
            Layout.fillWidth: true
            Layout.preferredWidth: parent.width
            Layout.maximumHeight: selectScriptName.height
            text: "Clear view and filter logs by script name"
            color: hifi.colors.lightGrayText
            horizontalAlignment: Text.AlignRight
            wrapMode: Text.WordWrap
            clip: true


        }

        Text {
            text: "-->"
            color: hifi.colors.lightGrayText
            horizontalAlignment: Text.AlignHCenter
            clip:true
        }

        // Drop down menu with all seen script names
        ComboBox {
            id: selectScriptName
            Layout.preferredWidth: 150
            Layout.preferredHeight: 35;
            padding: 0
            model: scriptNames

            background: Rectangle {
                id: comboBoxBackground;
                color: "#333";
                radius: 10;
            }

            contentItem: Text {
                anchors.centerIn: parent;
                width: parent.width;
                text: selectScriptName.displayText;
                horizontalAlignment: Text.AlignHCenter;
                verticalAlignment: Text.AlignVCenter;
                elide: Text.ElideRight;
                color: "white";
            }

            popup: Popup {
                width: selectScriptName.width
                padding: 1

                contentItem: ListView {
                    clip: true
                    implicitHeight: contentHeight
                    model: selectScriptName.popup.visible ? selectScriptName.delegateModel : null
                    currentIndex: selectScriptName.highlightedIndex

                    ScrollIndicator.vertical: ScrollIndicator { }
                }

                background: Rectangle {
                    color: Qt.rgba(0,0,0,0.9)
                    radius: 10
                }

                onVisibleChanged: {
                    Tablet.playSound(TabletEnums.ButtonClicked);
                }
            }

            indicator: Canvas {
                id: canvas
                x: selectScriptName.width - width - selectScriptName.rightPadding
                y: selectScriptName.topPadding + (selectScriptName.availableHeight - height) / 2
                width: 12
                height: 8
                contextType: "2d"

                Connections {
                    target: selectScriptName
                    function onPressedChanged() { canvas.requestPaint(); }
                }

                onPaint: {
                    context.reset();
                    context.moveTo(0, 0);
                    context.lineTo(width, 0);
                    context.lineTo(width / 2, height);
                    context.closePath();
                    context.fillStyle = "white";
                    context.fill();
                }
            }

            delegate: ItemDelegate {
                contentItem: Text {
                    text: model.name
                    color: "white"
                    font: selectScriptName.font
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }
                background: Rectangle {
                    color: highlighted ? "gray" : "transparent";
                    radius: 10
                }
                highlighted: scriptNames.highlightedIndex == index
            }

            onActivated: {
                console.log("index: ", index);
                console.log("currentText:", currentText);
                if (index == 0) {
                    currentFilter = null;
                } else {
                    currentFilter = currentText;
                }
                textArea.clear()
            }

        }
    }

    ListModel {
        id: scriptNames
        ListElement {
            name: "No Filter"
        }
    }

    property var scriptNamesSet: new Set()

    property var currentFilter: null;

    function fromScript(line) {
        const MAX_LINE_COUNT = 2000; // post-wrap lines
        const TRIM_LINES = 50; // pre-wrap lines
        if (textArea.lineCount > MAX_LINE_COUNT) { // textArea.lineCount represents the number of visual line breaks after text wrap, so is not an accurate count of the number of log lines.

            let charIndex = -1;
            for (let i = 0; i < TRIM_LINES; i++) {
                charIndex = textArea.text.indexOf('</span>', charIndex + 1); // This seems to result in removing ~3x the number of log lines then we requested.
            }
            // Here we use .remove, as editing the .text property directly results in breaking the built-in automatic scrolling when new lines are added.
            textArea.remove(0, charIndex + 7); // + the length of </span>
        }
        switch(line.type) {
            case "WARNING":
                line.color = hifi.colors.orangeHighlight
                break;
            case "ERROR":
                line.color = hifi.colors.redHighlight
                break;
            case "INFO":
                line.color = hifi.colors.indigoHighlight
                break
            default:
                line.color = hifi.colors.faintGray
        }

        // Preserved formatting of text when used in html
        const message = line.message
            .replace(/</g, "&lt;")
            .replace(/>/g, "&gt;")
            .replace(/\n/g, "<br>");

        if (!currentFilter || currentFilter == line.scriptFileName) {
            // `white-space: pre-wrap` preserves white space at the start of lines
            textArea.append(`<span style="color: ${line.color}; white-space: pre-wrap">[${line.date}] [${line.scriptFileName}] ${line.type.length > 0 ? line.type+' - ' : ""}${message}</span>`);
        }

        if (!scriptNamesSet.has(line.scriptFileName)) {
            const scriptNameItem = {"name": line.scriptFileName};
            scriptNamesSet.add(line.scriptFileName);
            scriptNames.append(scriptNameItem);
        }
    }

    function clearWindow() {
        textArea.remove(0,textArea.length);
    }
}


