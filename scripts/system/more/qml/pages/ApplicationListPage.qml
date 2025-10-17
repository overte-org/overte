import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import "../widgets"

ColumnLayout {
	width: parent.width;
	height: parent.height;
	id: applicationListPage;
	spacing: 10;

	// Header
	RowLayout {
		Layout.margins: 10
		Layout.bottomMargin: 0;
		Layout.alignment: Qt.AlignVCenter;
		height: 60;

		Rectangle {
			color: "white";
			Layout.fillWidth: true;
			height: parent.height - 20;
			radius: 10;

            RowLayout {
                anchors.fill: parent
                anchors.margins: 5
                spacing: 5
                width: parent.width;
                
                TextField {
                    id: searchArea;
                    placeholderText: "Search...";
                    color: "black";
                    Layout.fillWidth: true;
                    font.pixelSize: 14;
                    onTextChanged: {
                        onSearchChanged(searchArea.text, statusFilter.checked);
                    }
                    background: Rectangle {
                        color: "transparent";
                    }
                    
                    onVisibleChanged: {
                        onSearchChanged(searchArea.text, statusFilter.checked);
                    }
                    onFocusChanged: {
                        if (HMD.active) {
                            if (focus) {
                                KeyboardScriptingInterface.raised = true;
                            } else {
                                KeyboardScriptingInterface.raised = false;
                            }
                        }
                    }
                }
                
                Button {
                    id: clearButton;
                    text: "\u{1F5D9}";
                    height: searchArea.height;
                    width: searchArea.height;

                    font.pixelSize: 16;
                    
                    background: Rectangle {
                        anchors.fill: parent
                        color: "#ffffff";
                        border.color: "#ffffff";
                        border.width: 0;
                        radius: 4;
                    }

                    onClicked: { searchArea.text = ""; }
                }
            }
		}

        CheckBox {
            id: statusFilter;
            text: "";
            checked: false;
            onCheckedChanged: {
                onSearchChanged(searchArea.text, checked);
            }
            
            indicator: Rectangle {
                implicitWidth: 20;
                implicitHeight: 20;
                radius: 10;
                border.color: parent.checked ? "#0bde54" : "gray";
                border.width: 0;
                color: parent.checked ? "#0bde54" : "gray";
                anchors.verticalCenter: parent.verticalCenter;
                anchors.centerIn: parent;
                
                Behavior on color {
                    ColorAnimation { duration: 150; }
                }
            }
        }

		CustomButton {
			height: parent.height - 20;
			implicitWidth: 55;
			buttonText: "";
			buttonIcon: "../../img/menu.svg";
			onClickedFunc: () => { showSettingsPage() }
		}
	}

	// Scroll area
	Flickable {
		Layout.margins: 10;
		Layout.topMargin: 0;
		Layout.fillHeight: true;
		Layout.fillWidth: true;
		contentHeight: appListColumn.height;
		clip: true;
		
		Column {
			id: appListColumn
			width: parent.width;

			Repeater {
				model: appList.length;
				delegate: ApplicationListEntry { }
			}
		}

		ScrollBar.vertical: ScrollBar {
			policy: Qt.ScrollBarAlwaysOn;

			background: Rectangle {
				color: "transparent";
				radius: 5;
				visible: parent.visible;
			}
		}
	}
}
