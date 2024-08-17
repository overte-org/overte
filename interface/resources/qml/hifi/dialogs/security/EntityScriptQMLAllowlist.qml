//
//  EntityScriptQMLAllowlist.qml
//  interface/resources/qml/hifi/dialogs/security
//
//  Created by Kalila L. on 2019.12.05 | realities.dev | somnilibertas@gmail.com
//  Copyright 2019 Kalila L.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
// Security Settings for the Entity Script QML Allowlist

import Hifi 1.0 as Hifi
import QtQuick 2.8
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.12
import stylesUit 1.0 as HifiStylesUit
import controlsUit 1.0 as HiFiControls
import PerformanceEnums 1.0
import "../../../windows"


Rectangle {
    id: parentBody;

    function getAllowlistAsText() {
        var allowlist = Settings.getValue("private/settingsSafeURLS");
        var arrayAllowlist = allowlist.split(",").join("\n");
        return arrayAllowlist;
    }

    function setAllowlistAsText(allowlistText) {
        Settings.setValue("private/settingsSafeURLS", allowlistText.text);

        var originalSetString = allowlistText.text;
        var originalSet = originalSetString.split(' ').join('');

        var check = Settings.getValue("private/settingsSafeURLS");
        var arrayCheck = check.split(",").join("\n");

        setAllowlistSuccess(arrayCheck === originalSet);
    }

    function setAllowlistSuccess(success) {
        if (success) {
            notificationText.text = "Successfully saved settings.";
        } else {
            notificationText.text = "Error! Settings not saved.";
        }
    }

    function toggleAllowlist(enabled) {
        Settings.setValue("private/allowlistEnabled", enabled);
        console.info("Toggling Allowlist to:", enabled);
    }

    function initCheckbox() {
        var check = Settings.getValue("private/allowlistEnabled", false);

        if (check) {
            allowlistEnabled.toggle();
        }
    }


    anchors.fill: parent
    width: parent.width;
    height: 120;
    color: "#80010203";

    HifiStylesUit.RalewayRegular {
        id: titleText;
        text: "Entity Script / QML Allowlist"
        // Text size
        size: 24;
        // Style
        color: "white";
        elide: Text.ElideRight;
        // Anchors
        anchors.top: parent.top;
        anchors.left: parent.left;
        anchors.leftMargin: 20;
        anchors.right: parent.right;
        anchors.rightMargin: 20;
        height: 60;

        CheckBox {
            Component.onCompleted: {
                initCheckbox();
            }

            id: allowlistEnabled;

            anchors.right: parent.right;
            anchors.top: parent.top;
            anchors.topMargin: 10;
            onToggled: {
                toggleAllowlist(allowlistEnabled.checked)
            }

            Label {
                text: "Enabled"
                color: "white"
                font.pixelSize: 18;
                anchors.right: parent.left;
                anchors.top: parent.top;
                anchors.topMargin: 10;
            }
        }
    }

    Rectangle {
        id: textAreaRectangle;
        color: "black";
        width: parent.width;
        height: 250;
        anchors.top: titleText.bottom;

        ScrollView {
            id: textAreaScrollView
            anchors.fill: parent;
            width: parent.width
            height: parent.height
            contentWidth: parent.width
            contentHeight: parent.height
            clip: false;

            TextArea {
                id: allowlistTextArea
                text: getAllowlistAsText();
                onTextChanged: notificationText.text = "";
                width: parent.width;
                height: parent.height;
                font.family: "Ubuntu";
                font.pointSize: 12;
                color: "white";
            }
        }

        Button {
            id: saveChanges
            anchors.topMargin: 5;
            anchors.leftMargin: 20;
            anchors.rightMargin: 20;
            x: textAreaRectangle.x + textAreaRectangle.width - width - 15;
            y: textAreaRectangle.y + textAreaRectangle.height - height;
            contentItem: Text {
                text: saveChanges.text
                font.family: "Ubuntu";
                font.pointSize: 12;
                opacity: enabled ? 1.0 : 0.3
                color: "black"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            text: "Save Changes"
            onClicked: setAllowlistAsText(allowlistTextArea)

            HifiStylesUit.RalewayRegular {
                id: notificationText;
                text: ""
                // Text size
                size: 16;
                // Style
                color: "white";
                elide: Text.ElideLeft;
                // Anchors
                anchors.right: parent.left;
                anchors.rightMargin: 10;
            }
        }

        HifiStylesUit.RalewayRegular {
            id: descriptionText;
            text:
    "The allowlist checks scripts and QML as they are loaded.<br/>
    Therefore, if a script is cached or has no reason to load again,<br/>
    removing it from the allowlist will have no effect until<br/>
    it is reloaded.<br/>
    Separate your allowlisted domains by line, not commas. e.g.
    <blockquote>
        <b>https://google.com/</b><br/>
        <b>hifi://the-spot/</b><br/>
        <b>127.0.0.1</b><br/>
        <b>https://mydomain.here/</b>
    </blockquote>
    Ensure there are no spaces or whitespace.<br/><br/>
    For QML files, you can only allowlist each file individually<br/>
    ending with '.qml'."
            // Text size
            size: 16;
            // Style
            color: "white";
            elide: Text.ElideRight;
            textFormat: Text.RichText;
            // Anchors
            anchors.top: parent.bottom;
            anchors.topMargin: 90;
            anchors.left: parent.left;
            anchors.leftMargin: 20;
            anchors.right: parent.right;
            anchors.rightMargin: 20;
        }
    }
}
