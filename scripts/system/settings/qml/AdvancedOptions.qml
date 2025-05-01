import QtQuick 2.7
import QtQuick.Controls 2.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3

Item {
    property var isEnabled: false;
    height: 0;
    width: parent.width;
    id: advancedOptionsRoot;
    clip: true;

    // Despite my best efforts, a timer is needed here to check to see if we need to expand on start.
    // After the children are all moved to the advancedOptionsList column, this timer should start and correctly set
    // the desired state.
    Timer {
        id: initExpandTimer;
        interval: 100;
        repeat: false;
        running: false;

        onTriggered: {
            if (isEnabled) advancedOptionsRoot.height = advancedOptionsList.height;
        }
    }

    // Expand Vertically on enabled animation.
    Behavior on height {
        NumberAnimation {
            duration: 200;
            easing.type: Easing.InOutCubic;
        }
    }

    onIsEnabledChanged: {
        if (isEnabled) advancedOptionsRoot.height = advancedOptionsList.height;
        else advancedOptionsRoot.height = 0;
    }

    // Content Container.
    Rectangle {
        color: "#222222";
        width: parent.width;
        height: advancedOptionsList.height;
        radius: 10;

        // Content List.
        ColumnLayout {
            width: parent.width - 10;
            id: advancedOptionsList;
        }
    }

    // Append all children created to this widget to the correct element.
    Component.onCompleted: {
        while (advancedOptionsRoot.children.length > 1){
            advancedOptionsRoot.children[1].parent = advancedOptionsList;

            // On the last child moved, run a timer to check to see if we need to expand
            initExpandTimer.running = true;
        }
    }
}