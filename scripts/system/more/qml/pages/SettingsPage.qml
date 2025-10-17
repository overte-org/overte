import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11
import "../widgets/"
import "../widgets/CustomListView"

ColumnLayout {
	width: parent.width;
	height: parent.height;
	id: root;
	spacing: 10;

	property var entryList: [""];

	ColumnLayout {
		width: parent.width - 20;
		height: parent.height - 10;
		Layout.margins: 10
		Layout.bottomMargin: 0;
		Layout.alignment: Qt.AlignVCenter;

		CustomButton {
            Layout.preferredHeight: 40;
			Layout.fillWidth: true;

			buttonText: "Back";

			onClickedFunc: () => { showAppListPage() }
		}
        
        Text {
            text: "\nRepositories";
            color: colors.lightText3;
            font.pixelSize: 20;
        }
        
		CustomListView {
			Layout.fillHeight: true;
			onAddEntryButtonClickedFunc: () => {
                if (HMD.active) {
                    KeyboardScriptingInterface.raised = true;
                }
                toScript({type: "addNewRepositoryButtonClicked"});
            };
			entries: entryList;
		}
	}
}
