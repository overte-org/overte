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
import Hifi 1.0 as Hifi

import stylesUit 1.0


Rectangle {
    id: root
    HifiConstants { id: hifi }
    width: parent ? parent.width : 100
    height: parent ? parent.height : 100
    color: hifi.colors.darkGray

    signal moved(vector2d position);
    signal resized(size size);

    property var channel;

    ScrollView {
        anchors.fill: parent
        clip: true

        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn

        TextArea {
            id: textArea
            width: root.width
            height: parent.height
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
        }
    }

    function fromScript(line) {
        var MAX_LINE_COUNT = 2000;
        var TRIM_LINES = 500;
        if (textArea.lineCount > MAX_LINE_COUNT) {
            var lines = textArea.text.split('\n');
            lines.splice(0, TRIM_LINES);
            textArea.text = lines.join('\n');
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
        const message = line.message.replace(/\n/g, "<br>")

        // white-space: pre-wrap preserves white space at the start of lines
        textArea.append(`<span style="color: ${line.color}; white-space: pre-wrap">[${line.date}] [${line.scriptFileName}] ${line.type.length > 0 ? line.type+' - ' : ""}${message}</span>`);
    }

    function clearWindow() {
        textArea.remove(0,textArea.length);
    }
}


