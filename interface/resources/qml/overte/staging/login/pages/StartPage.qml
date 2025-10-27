import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../../" as Overte

Item {
    readonly property url codeOfConduct: "https://overte.org/code_of_conduct.html"

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 32
        spacing: Overte.Theme.fontPixelSize * 2

        Overte.Label {
            Layout.fillWidth: true

            text: qsTr("Welcome to Overte!\n\nAn account gives you a contacts list and lets you publish to the public worlds list.\n\nAccounts are entirely optional.")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.Wrap
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            spacing: 16

            Overte.Button {
                Layout.alignment: Qt.AlignCenter
                Layout.fillWidth: true

                // force equally sized buttons
                Layout.preferredWidth: 1

                text: qsTr("Register")

                onClicked: {
                    stack.push("./RegisterPage.qml");
                }
            }

            Overte.Button {
                Layout.alignment: Qt.AlignCenter
                Layout.fillWidth: true

                // force equally sized buttons
                Layout.preferredWidth: 1

                text: qsTr("Log in")

                onClicked: {
                    stack.push("./LoginPage.qml");
                }
            }
        }

        Overte.Button {
            Layout.alignment: Qt.AlignCenter
            Layout.fillWidth: true

            text: qsTr("Code of Conduct")

            onClicked: () => Qt.openUrlExternally(codeOfConduct)
        }
    }
}
