import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Rectangle {
	property var onClickedFunc;
	property string buttonText: "Button";
	property color buttonColor: colors.button;
	property string buttonIcon: "";

	height: 30;
	radius: 5;
	implicitWidth: buttonTextComponent.width + 20;
	color: buttonColor;

	Text {
		text: buttonText;
		font.pixelSize: 20;
		color: "white";
		anchors.centerIn: parent;
		id: buttonTextComponent;
	}

	Image {
		visible: buttonIcon !== "";
		source: buttonIcon;
		width: 25;
		height: 25;
		sourceSize.width: 25;
		sourceSize.height: 25;
		anchors.centerIn: parent;
	}

	MouseArea {
		anchors.fill: parent;
		hoverEnabled: true;
		propagateComposedEvents: true;	

		onPressed: {
			if (onClickedFunc && typeof onClickedFunc === "function") {
				onClickedFunc();
			}
		}

		onEntered: {
			parent.color = colors.darkBackground3;
		}

		onExited: {
			parent.color = buttonColor;
		}
	}
}