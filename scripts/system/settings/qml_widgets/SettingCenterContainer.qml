import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

ScrollView {
	width: parent.width
	height: parent.height
	y: header.height
	id: root

	ColumnLayout {
		width: parent.width
		anchors.horizontalCenter: parent.horizontalCenter
		spacing: 0
	}

	// For each child of index not of 0 or 1, append as child of index of 0. 
	// Append children of the custom element to the ColumnLayout.
	Component.onCompleted: {
		for (var i = 2; i < root.contentChildren.length; i++) {
			root.contentChildren[i].parent = root.contentChildren[0]
        }
	}
}
