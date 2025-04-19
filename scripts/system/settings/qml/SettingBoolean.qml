import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import TabletScriptingInterface 1.0

Item {
	id: root;
	property string settingText: "";
	property bool settingEnabled: false;

	height: 50;
	width: parent.width;

	RowLayout {
		width: parent.width - 10;
		height: parent.height;
		anchors.horizontalCenter: parent.horizontalCenter;

		TextEdit {
			id: settingTextElem
			height: parent.height;
			text: settingText;
			color: "white";
			font.pixelSize: 22;
			selectByMouse: true;
			readOnly: true;
		}

        Switch {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight;
            checked: settingEnabled;
            implicitHeight: 20;

            indicator: Item {
                implicitWidth: 70;
                implicitHeight: parent.implicitHeight;

                Rectangle {
                    anchors.fill: parent
                    radius: height / 2
                    color: parent.parent.checked ? "#5153bd" : "gray";

                    Behavior on color {
                        ColorAnimation {
                            duration: 200
                            easing.type: Easing.InOutCubic
                        }
                    }
                }


                Rectangle {
                    width: 30
                    height: 30
                    radius: height;
                    color: "white"
                    x: parent.parent.checked ? parent.width - width : 0;
                    y: (parent.implicitHeight - height) / 2

                    // Movement animation
                    Behavior on x {
                        NumberAnimation {
                            duration: 100;
                            easing.type: Easing.InOutCubic;
                        }
                    }
                }
            }

            onCheckedChanged: {
			    Tablet.playSound(TabletEnums.ButtonClicked);
                settingEnabled = checked;
            }
        }
	}
}