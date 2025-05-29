import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import "./widgets"


Rectangle {
    signal sendToScript(var message);
	color: Qt.rgba(0.1,0.1,0.1,1);
	width: parent.width;
	height: parent.height;
	anchors.centerIn: parent;
	anchors.horizontalCenter: parent.horizontalCenter;

    property int mode: 0; // 0 = Single, 1 = all
	property int bitLength: 512; 

	// Home page
	ColumnLayout {
		width: parent.width - 20;
		height: parent.height;
		spacing: 15;
		anchors.horizontalCenter: parent.horizontalCenter;

		// Controls
		RowLayout {
			width: parent.width;
			height: 50;
			Layout.alignment: Qt.AlignHCenter | Qt.AlignTop;
			Layout.topMargin: 10;
			
			SimpleButton {
				callback: () => {runTest()};
				buttonText: "Run"
			}
		}

		Item {
			Layout.fillHeight: true;
			Layout.fillWidth: true;
		}
	}

	function runTest() {
		toScript({type: "action", action: "startTest"});
	}

	function updateSelection(){
		var selection = {
			mode: mode,
			bitLength: bitLength
		}
		toScript({type: "update", selection: selection});
	}


	function fromScript(message) {
		if (message.type == "myData"){
			myData = message.data;
			canKick = message.data.canKick;
			return;
		}
	}

	// Send message to script
	function toScript(packet){
		sendToScript(packet)
	}
}

