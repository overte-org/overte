import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Component {
    id: template_chat_message

    Rectangle {
        property int index: delegateIndex

        height: Math.max(65, children[1].height + 30)
        color: index % 2 === 0 ? "transparent" : Qt.rgba(0.15,0.15,0.15,1)
        width: listview.parent.parent.width
        Layout.fillWidth: true

        Item {
            width: parent.width - 10
            anchors.horizontalCenter: parent.horizontalCenter
            height: 22

            TextEdit {
                text: delegateUsername;
                color: "lightgray";
                readOnly: true;
                selectByMouse: true;
                selectByKeyboard: true;
            }

            Text {
                anchors.right: parent.right;
                text: delegateDate;
                color: "lightgray";
            }
        }

        Flow {
            anchors.top: parent.children[0].bottom;
            width: parent.width;
            x: 5
            id: messageBoxFlow

            Repeater {
                model: delegateText;

                Item {
                    width: parent.width;
                    height: children[0].contentHeight;

                    TextEdit {
                        text: model.value || ""
                        font.pointSize: 12
                        wrapMode: TextEdit.WordWrap
                        width: parent.width * 0.8
                        visible: model.type === 'text' || model.type === 'mention';
                        readOnly: true
                        selectByMouse: true
                        selectByKeyboard: true

                        color: {
                            switch (model.type) {
                                case "mention":
                                    return "purple";
                                default:
                                    return "white";
                            }
                        }
                    }

                    Flow {
                        width: parent.width * 0.8;
                        height: 20
                        visible: model.type === 'url';

                        TextEdit {
                            id: urlTypeTextDisplay;
                            text: model.value || "";
                            font.pointSize: 12;
                            wrapMode: Text.Wrap;
                            color: "#4EBAFD";
                            font.underline: true;
                            readOnly: true
                            selectByMouse: true
                            selectByKeyboard: true
                            width: Math.min(parent.width - 20, textMetrics.tightBoundingRect.width) ;

                            MouseArea {
                                anchors.fill: parent;

                                onClicked: {
                                    Window.openWebBrowser(model.value);
                                }
                            }
                        }

                        TextMetrics {
                            id: textMetrics
                            font: urlTypeTextDisplay.font
                            text: urlTypeTextDisplay.text
                        }

                        Text {
                            width: 20;
                            text: "🗗";
                            font.pointSize: 10;
                            color: "white";
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            
                            MouseArea {
                                anchors.fill: parent;

                                onClicked: {
                                    Qt.openUrlExternally(model.value);
                                }
                            }
                        }
                    }

                    RowLayout {
                        visible: model.type === 'overteLocation';
                        width: Math.min(messageBoxFlow.width, children[0].children[1].contentWidth + 35);
                        height: 20;
                        Layout.leftMargin: 5
                        Layout.rightMargin: 5

                        Rectangle {
                            width: parent.width;
                            height: 20;
                            color: "lightgray"
                            radius: 2;

                            Image {
                                source: "../img/ui/world_black.png"
                                width: 18;
                                height: 18;
                                sourceSize.width: 18
                                sourceSize.height: 18
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter 
                                anchors.leftMargin: 2
                                anchors.rightMargin: 10
                            }

                            TextEdit {
                                text: model.type === 'overteLocation' ? model.value.split('hifi://')[1].split('/')[0] : '';
                                color: "black"
                                font.pointSize: 12
                                x: parent.children[0].width + 5;
                                anchors.verticalCenter: parent.verticalCenter 
                                readOnly: true
                                selectByMouse: true
                                selectByKeyboard: true
                            }

                            MouseArea {
                                anchors.fill: parent;

                                onClicked: {
                                    Window.openUrl(model.value);
                                }
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true;
                        visible: model.type === 'messageEnd';
                    }

                    Item {
                        visible: model.type === 'imageEmbed';
                        width: messageBoxFlow.width;
                        height: 200

                        AnimatedImage {
                            source: model.type === 'imageEmbed' ? model.value : ''
                            height: Math.min(sourceSize.height, 200);
                            fillMode: Image.PreserveAspectFit
                        }
                    }


                }
            }
        }
    }
}