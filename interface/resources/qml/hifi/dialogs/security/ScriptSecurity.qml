//
//  ScriptPermissions.cpp
//  libraries/script-engine/src/ScriptPermissions.cpp
//
//  Created by dr Karol Suprynowicz on 2024/03/24.
//  Copyright 2024 Overte e.V.
//
//  Based on EntityScriptQMLWhitelist.qml
//  Created by Kalila L. on 2019.12.05 | realities.dev | somnilibertas@gmail.com
//  Copyright 2019 Kalila L.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
// Security settings for the script engines

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

    function getWhitelistAsText() {
        var whitelist = Settings.getValue("private/scriptPermissionGetAvatarURLSafeURLs");
        var arrayWhitelist = whitelist.replace(",", "\n");
        return arrayWhitelist;
    }

    function setWhitelistAsText(whitelistText) {
        Settings.setValue("private/scriptPermissionGetAvatarURLSafeURLs", whitelistText.text);
        notificationText.text = "Whitelist saved.";
    }

    function setAvatarProtection(enabled) {
        Settings.setValue("private/scriptPermissionGetAvatarURLEnable", enabled);
        console.info("Setting Protect Avatar URLs to:", enabled);
    }

    anchors.fill: parent
    width: parent.width;
    height: 120;
    color: "#80010203";

    HifiStylesUit.RalewayRegular {
        id: titleText;
        text: "Protect Avatar URLs"
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
            id: whitelistEnabled;

            checked: Settings.getValue("private/scriptPermissionGetAvatarURLEnable", true);

            anchors.right: parent.right;
            anchors.top: parent.top;
            anchors.topMargin: 10;
            onToggled: {
                setAvatarProtection(whitelistEnabled.checked)
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
                id: whitelistTextArea
                text: getWhitelistAsText();
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
            onClicked: setWhitelistAsText(whitelistTextArea)
          
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
    }
}
