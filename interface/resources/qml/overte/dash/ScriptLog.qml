import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

// hack to get access to IconImage,
// why isn't it part of stock QtQuick.Controls?
import QtQuick.Controls.impl

import ".." as Overte

Rectangle {
    anchors.fill: parent
    color: Overte.Theme.paletteActive.base

    property ListModel model: ListModel {}
    property ListModel filteredModel: ListModel {}

    function refreshFilteredModel() {
        filteredModel.clear();

        for (let i = 0; i < model.count; i++) {
            const datum = model.get(i);

            if (datum.priority === 0 && !filterDebug.checked) { continue; }
            if (datum.priority === 1 && !filterInfo.checked) { continue; }
            if (datum.priority === 2 && !filterWarn.checked) { continue; }
            if (datum.priority === 3 && !filterError.checked) { continue; }

            filteredModel.append(datum);
        }
    }

    function fromScript(rawMsg) {
        model.append(JSON.parse(rawMsg));
        refreshFilteredModel();
    }

    function transparent(c, a = 0.1) {
        return Qt.rgba(c.r, c.g, c.b, a);
    }

    readonly property list<color> priorityColors: [
        Overte.Theme.paletteActive.logPriorityDebug,
        Overte.Theme.paletteActive.logPriorityInfo,
        Overte.Theme.paletteActive.logPriorityWarn,
        Overte.Theme.paletteActive.logPriorityError,
    ]

    readonly property list<string> priorityIcons: [
        "",
        "../icons/log_info.svg",
        "../icons/log_warn.svg",
        "../icons/log_error.svg",
    ]

    component LogLine: Rectangle {
        required property int priority
        required property string text
        property string source

        width: listView.contentWidth
        height: label.implicitHeight + 8

        visible: {
            switch (priority) {
                case 0: return filterDebug.checked;
                case 1: return filterInfo.checked;
                case 2: return filterWarn.checked;
                case 3: return filterError.checked;
            }
        }

        color: transparent(priorityColors[priority])
        border.color: priorityColors[priority]
        border.width: 1

        // IconImage isn't documented and technically private
        IconImage {
            id: icon

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.margins: 4
            anchors.leftMargin: 6
            sourceSize.width: 16
            sourceSize.height: 16
            width: 16
            height: 16

            visible: priorityIcons[priority] !== ""
            color: parent.border.color
            source: priorityIcons[priority]
        }

        TextEdit {
            id: label
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.left: icon.right
            anchors.margins: 4
            anchors.leftMargin: 6

            readOnly: true
            selectionColor: Overte.Theme.paletteActive.highlight
            selectedTextColor: Overte.Theme.paletteActive.highlightedText
            font.family: "monospace"
            font.pixelSize: Overte.Theme.fontPixelSizeSmall
            color: Overte.Theme.paletteActive.text
            wrapMode: Text.Wrap
            text: parent.text
        }
    }

    RowLayout {
        id: toolbar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 2
        height: childrenRect.height

        Overte.TextField {
            Layout.fillWidth: true
            placeholderText: qsTr("Search log")
        }

        Overte.RoundButton {
            icon.source: "../icons/search.svg"
            icon.width: 24
            icon.height: 24
        }

        Overte.RoundButton {
            icon.source: "../icons/delete.svg"
            icon.width: 24
            icon.height: 24
            icon.color: Overte.Theme.paletteActive.buttonText
            backgroundColor: (
                hovered ?
                Overte.Theme.paletteActive.buttonDestructive :
                Overte.Theme.paletteActive.button
            )

            Overte.ToolTip { text: qsTr("Clear log") }

            onClicked: toScript("clear log button clicked?")
        }
    }

    ListView {
        id: listView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: filters.top
        anchors.top: toolbar.bottom
        anchors.topMargin: 4

        ScrollBar.vertical: Overte.ScrollBar {
            interactive: true
            policy: ScrollBar.AlwaysOn
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
        }
        contentWidth: width - ScrollBar.vertical.width

        delegate: LogLine {}
        model: filteredModel
    }

    RowLayout {
        id: filters
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 2
        height: childrenRect.height

        Overte.RoundButton {
            id: filterError
            checkable: true
            checked: true

            icon.source: "../icons/log_error.svg"
            icon.width: 24
            icon.height: 24
            icon.color: Overte.Theme.paletteActive.buttonText

            backgroundColor: checked ? transparent(priorityColors[3], 0.5) : Overte.Theme.paletteActive.button
            onToggled: refreshFilteredModel()

            Overte.ToolTip { text: qsTr("Show error messages") }
        }

        Overte.RoundButton {
            id: filterWarn
            checkable: true
            checked: true

            icon.source: "../icons/log_warn.svg"
            icon.width: 24
            icon.height: 24
            icon.color: Overte.Theme.paletteActive.buttonText

            backgroundColor: checked ? transparent(priorityColors[2], 0.5) : Overte.Theme.paletteActive.button
            onToggled: refreshFilteredModel()

            Overte.ToolTip { text: qsTr("Show warning messages") }
        }

        Overte.RoundButton {
            id: filterInfo
            checkable: true
            checked: true

            icon.source: "../icons/log_info.svg"
            icon.width: 24
            icon.height: 24
            icon.color: Overte.Theme.paletteActive.buttonText

            backgroundColor: checked ? transparent(priorityColors[1], 0.5) : Overte.Theme.paletteActive.button
            onToggled: refreshFilteredModel()

            Overte.ToolTip { text: qsTr("Show info messages") }
        }

        Overte.RoundButton {
            id: filterDebug
            checkable: true
            checked: true

            icon.source: "../icons/pencil.svg"
            icon.width: 24
            icon.height: 24
            icon.color: Overte.Theme.paletteActive.buttonText

            backgroundColor: checked ? transparent(priorityColors[0], 0.5) : Overte.Theme.paletteActive.button
            onToggled: refreshFilteredModel()

            Overte.ToolTip { text: qsTr("Show plain messages") }
        }

        Item { Layout.fillWidth: true }
    }
}
