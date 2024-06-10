import QtQuick 2.7
import QtQuick.Controls 2.2
import QtWebChannel 1.0
import controls 1.0
import hifi.toolbars 1.0
import QtGraphicalEffects 1.0
import controlsUit 1.0 as HifiControls
import stylesUit 1.0

TabBar {
    id: editTabView
    width: parent.width
    contentWidth: parent.width
    padding: 0
    spacing: 0

    readonly property QtObject tabIndex: QtObject {
        readonly property int create: 0
        readonly property int properties: 1
        readonly property int grid: 2
    }

    readonly property HifiConstants hifi: HifiConstants {}

    EditTabButton {
        title: "CREATE"
        active: true
        enabled: true
        property string originalUrl: ""

        property Component visualItem: Component {

            Rectangle {
                color: "#404040"
                id: container

                Flickable {
                    height: parent.height
                    width: parent.width
                    clip: true

                    contentHeight: createEntitiesFlow.height + importButton.height + assetServerButton.height +
                                   header.anchors.topMargin + createEntitiesFlow.anchors.topMargin +
                                   assetServerButton.anchors.topMargin + importButton.anchors.topMargin +
                                   header.paintedHeight

                    contentWidth: width

                    ScrollBar.vertical : ScrollBar {
                        visible: parent.contentHeight > parent.height
                        width: 20
                        background: Rectangle {
                            color: hifi.colors.tableScrollBackgroundDark
                        }
                    }

                    Text {
                        id: header
                        color: "#ffffff"
                        text: "Choose an Entity Type to Create:"
                        font.pixelSize: 14
                        font.bold: true
                        anchors.top: parent.top
                        anchors.topMargin: 28
                        anchors.left: parent.left
                        anchors.leftMargin: 28
                    }

                    Flow {
                        id: createEntitiesFlow
                        spacing: 35
                        anchors.right: parent.right
                        anchors.rightMargin: 55
                        anchors.left: parent.left
                        anchors.leftMargin: 55
                        anchors.top: parent.top
                        anchors.topMargin: 70


                        NewEntityButton {
                            icon: "icons/94-model-01.svg"
                            text: "MODEL"
                            onClicked: {
                                editRoot.sendToScript({
                                    method: "newEntityButtonClicked",
                                    params: { buttonName: "newModelButton" }
                                });
                                editTabView.currentIndex = tabIndex.properties
                            }
                        }

                        NewEntityButton {
                            icon: "icons/21-cube-01.svg"
                            text: "SHAPE"
                            onClicked: {
                                editRoot.sendToScript({
                                    method: "newEntityButtonClicked",
                                    params: { buttonName: "newShapeButton" }
                                });
                                editTabView.currentIndex = tabIndex.properties
                            }
                        }

                        NewEntityButton {
                            icon: "icons/24-light-01.svg"
                            text: "LIGHT"
                            onClicked: {
                                editRoot.sendToScript({
                                    method: "newEntityButtonClicked",
                                    params: { buttonName: "newLightButton" }
                                });
                                editTabView.currentIndex = tabIndex.properties
                            }
                        }

                        NewEntityButton {
                            icon: "icons/20-text-01.svg"
                            text: "TEXT"
                            onClicked: {
                                editRoot.sendToScript({
                                    method: "newEntityButtonClicked",
                                    params: { buttonName: "newTextButton" }
                                });
                                editTabView.currentIndex = tabIndex.properties
                            }
                        }

                        NewEntityButton {
                            icon: "icons/image.svg"
                            text: "IMAGE"
                            onClicked: {
                                editRoot.sendToScript({
                                    method: "newEntityButtonClicked",
                                    params: { buttonName: "newImageButton" }
                                });
                                editTabView.currentIndex = tabIndex.properties
                            }
                        }

                        NewEntityButton {
                            icon: "icons/25-web-1-01.svg"
                            text: "WEB"
                            onClicked: {
                                editRoot.sendToScript({
                                    method: "newEntityButtonClicked",
                                    params: { buttonName: "newWebButton" }
                                });
                                editTabView.currentIndex = tabIndex.properties
                            }
                        }

                        NewEntityButton {
                            icon: "icons/23-zone-01.svg"
                            text: "ZONE"
                            onClicked: {
                                editRoot.sendToScript({
                                    method: "newEntityButtonClicked",
                                    params: { buttonName: "newZoneButton" }
                                });
                                editTabView.currentIndex = tabIndex.properties
                            }
                        }

                        NewEntityButton {
                            icon: "icons/90-particles-01.svg"
                            text: "PARTICLE"
                            onClicked: {
                                editRoot.sendToScript({
                                    method: "newEntityButtonClicked",
                                    params: { buttonName: "newParticleButton" }
                                });
                                editTabView.currentIndex = tabIndex.properties
                            }
                        }

                        NewEntityButton {
                            icon: "icons/126-material-01.svg"
                            text: "MATERIAL"
                            onClicked: {
                                editRoot.sendToScript({
                                    method: "newEntityButtonClicked",
                                    params: { buttonName: "newMaterialButton" }
                                });
                                editTabView.currentIndex = tabIndex.properties
                            }
                        }

                        NewEntityButton {
                            icon: "icons/voxels.svg"
                            text: "VOXELS"
                            onClicked: {
                                editRoot.sendToScript({
                                    method: "newEntityButtonClicked",
                                    params: { buttonName: "newPolyVoxButton" }
                                });
                                editTabView.currentIndex = tabIndex.properties
                            }
                        }
                    }

                    HifiControls.Button {
                        id: assetServerButton
                        text: "Open This Domain's Asset Server"
                        color: hifi.buttons.black
                        colorScheme: hifi.colorSchemes.dark
                        anchors.right: parent.right
                        anchors.rightMargin: 55
                        anchors.left: parent.left
                        anchors.leftMargin: 55
                        anchors.top: createEntitiesFlow.bottom
                        anchors.topMargin: 35
                        onClicked: {
                            editRoot.sendToScript({
                                method: "newEntityButtonClicked",
                                params: { buttonName: "openAssetBrowserButton" }
                            });
                        }
                    }

                    HifiControls.Button {
                        id: importButton
                        text: "Import Entities (.json) from a File"
                        color: hifi.buttons.black
                        colorScheme: hifi.colorSchemes.dark
                        anchors.right: parent.right
                        anchors.rightMargin: 55
                        anchors.left: parent.left
                        anchors.leftMargin: 55
                        anchors.top: assetServerButton.bottom
                        anchors.topMargin: 20
                        onClicked: {
                            editRoot.sendToScript({
                                method: "newEntityButtonClicked",
                                params: { buttonName: "importEntitiesButton" }
                            });
                        }
                    }

                    HifiControls.Button {
                        id: importButtonFromUrl
                        text: "Import Entities (.json) from a URL"
                        color: hifi.buttons.black
                        colorScheme: hifi.colorSchemes.dark
                        anchors.right: parent.right
                        anchors.rightMargin: 55
                        anchors.left: parent.left
                        anchors.leftMargin: 55
                        anchors.top: importButton.bottom
                        anchors.topMargin: 20
                        onClicked: {
                            editRoot.sendToScript({
                                method: "newEntityButtonClicked",
                                params: { buttonName: "importEntitiesFromUrlButton" }
                            });
                        }
                    }
                }
            } // Flickable
        }
    }

    EditTabButton {
        title: "PROPERTIES"
        active: true
        enabled: true
        property string originalUrl: ""

        property Component visualItem: Component {
            WebView {
                id: entityPropertiesWebView
                url: Qt.resolvedUrl("../entityProperties/html/entityProperties.html")
                enabled: true
                blurOnCtrlShift: false
            }
        }
    }

    EditTabButton {
        title: "TOOLS"
        active: true
        enabled: true
        property string originalUrl: ""

        property Component visualItem: Component {
            WebView {
                id: gridControlsWebView
                url: Qt.resolvedUrl("../../html/gridControls.html")
                enabled: true
                blurOnCtrlShift: false
            }
        }
    }

    EditTabButton {
        title: "IMPORT"
        active: true
        enabled: true
        property string originalUrl: ""

        property Component visualItem: Component {
            WebView {
                id: advancedImportWebView
                url: Qt.resolvedUrl("../importEntities/html/importEntities.html")
                enabled: true
                blurOnCtrlShift: false
            }
        }
    }

    function fromScript(message) {
        switch (message.method) {
            case 'selectTab':
                selectTab(message.params.id);
                break;
            default:
                console.warn('EditToolsTabView.qml: Unrecognized message');
        }
    }

    // Changes the current tab based on tab index or title as input
    function selectTab(id) {
        if (typeof id === 'number') {
            if (id >= tabIndex.create && id <= tabIndex.import) {
                editTabView.currentIndex = id;
            } else {
                console.warn('Attempt to switch to invalid tab:', id);
            }
        } else if (typeof id === 'string'){
            switch (id.toLowerCase()) {
                case 'create':
                    editTabView.currentIndex = tabIndex.create;
                    break;
                case 'properties':
                    editTabView.currentIndex = tabIndex.properties;
                    break;
                case 'grid':
                    editTabView.currentIndex = tabIndex.grid;
                    break;
                case 'import':
                    editTabView.currentIndex = tabIndex.import;
                    break;
                default:
                    console.warn('Attempt to switch to invalid tab:', id);
            }
        } else {
            console.warn('Attempt to switch tabs with invalid input:', JSON.stringify(id));
        }
    }
}
