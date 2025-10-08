import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11
import "../widgets/"

Rectangle {
	width: parent.width;
	height: parent.height;
	color: "transparent";

	property var focusedApp: appList[focusedAppIndex];
	property bool isAppInstalled: focusedAppIndex > -1 && focusedApp.isInstalled || false;
	property string appInstalledUrl: focusedAppIndex > -1 && focusedApp.installedUrl || "";

	Item {
		visible: true;
		width: parent.width - 10;
		height: parent.height - 10;
		anchors.centerIn: parent;
		id: appDetailsElement;

		Column {
			width: parent.width;
			height: parent.height;
			spacing: 10;

            Row {
                id: navigationContainer;
                width: parent.width;
                
                CustomButton {
                    id: detailsBackButton;
                    width: parent.width;
                    buttonText: "Back";
                    onClickedFunc: () => { showAppListPage() }
                }
            }
            
            Row {
                id: statusContainer;
                anchors.right: parent.right;
                spacing: 6;
				height: 24;
                
                Text {
                    visible: focusedApp.isInstalled;
                    text: focusedApp.isRunning ? "\u2B24 INSTALLED / RUNNING" : "\u2BC0 INSTALLED / NOT RUNNING";
                    color: focusedApp.isRunning ? colors.greenIndicatorText : colors.redIndicatorText;
                    font.pixelSize: 14;
                }

                Text {
                    visible: !focusedApp.isInstalled && focusedApp.isRunning;
                    text: "\u25B2 NOT INSTALLED / RUNNING";
                    color: colors.yellowIndicatorText;
                    font.pixelSize: 14;
                }
                
                Text {
                    visible: !focusedApp.isInstalled && !focusedApp.isRunning;
                    text: "NOT INSTALLED / NOT RUNNING";
                    color: colors.lightText3;
                    font.pixelSize: 14;
                }
            }
            
			Row {
                id: descriptionContainer;
                anchors.left: parent.left;
				width: parent.width;

                Column {
                    width:100;
                    AppImage {
                        imageSize: 100;
                        icon: focusedApp && focusedApp.appIcon || "";
                    }
                }
                
				Column {
					width: 10;
					height: 1;
				}

				Column {
					width: parent.width - 110;

					Text {
						text: focusedApp && focusedApp.appName || "";
						color: colors.lightText1;
						font.pixelSize: 20;
					}
                    Text {
                        text: focusedApp && focusedApp.appDescription || "";
                        color: colors.lightText2;
                        font.pixelSize: 14;
                        wrapMode: Text.Wrap;
                        width: parent.width;
                    }
				}
			}

			Row {
                id: specificationContainer;
				width: parent.width;
                anchors.left: parent.left;
                
                Column {
                    width: parent.width;

                    Text {
                        id: category;
                        text: focusedApp && focusedApp.appCategory ? "Category: " + focusedApp.appCategory || "Category: -" : "";
                        color: colors.lightText2;
                        font.pixelSize: 16;
                        anchors.left: parent.left;
                    }
                    
                    Text {
                        id: maturity;
                        text: focusedApp && focusedApp.appAgeMaturity ? "Maturity: " + focusedApp.appAgeMaturity || "Maturity: -" : "";
                        color: colors.lightText2;
                        font.pixelSize: 16;
                        anchors.left: parent.left;
                    }
                    
                    Text {
                        id: version;
                        text: focusedApp && focusedApp.isInstalled ? "Installed version: " + focusedApp.installedVersion : "";
                        color: colors.lightText2;
                        font.pixelSize: 16;
                        wrapMode: Text.Wrap;
                        width: parent.width;
                        anchors.left: parent.left;
                    }
                }
			}

			RowLayout {
				spacing: 10;
				width: parent.width;
                anchors.left: parent.left;

				CustomButton {
					visible: !focusedApp.isInstalled;
					buttonText: "Install";
					buttonColor: colors.buttonSafe;
					Layout.fillWidth: true;
					onClickedFunc: () => { appVersionsElement.visible = true; appDetailsElement.visible = false; }
				}
				CustomButton {
					visible: focusedApp.isInstalled;
					buttonText: "Remove";
					buttonColor: colors.buttonDanger;
					Layout.fillWidth: true;
					onClickedFunc: () => { uninstallApp(focusedApp.installedUrl); }
				}
				CustomButton {
					visible: !focusedApp.isInstalled && focusedApp.isRunning;
					buttonText: "Stop";
					buttonColor: colors.buttonDanger;
					Layout.fillWidth: true;
					onClickedFunc: () => { stopApp(focusedApp.installedUrl); }
				}
				CustomButton {
					visible: focusedApp.isInstalled && focusedApp.isRunning;
					buttonText: "Stop";
					buttonColor: colors.buttonUtility;
					Layout.fillWidth: true;
					onClickedFunc: () => { stopApp(focusedApp.installedUrl); }
				}
				CustomButton {
					visible: focusedApp.isInstalled;
					buttonText: "Reload";
					buttonColor: colors.buttonUtility;
					Layout.fillWidth: true;
					onClickedFunc: () => { reloadApp(focusedApp.installedUrl); }
				}
				CustomButton {
					buttonText: focusedApp.isInstalled && focusedApp.isRunning ? "Repository" : "View Repository";
					buttonColor: colors.button;
					Layout.fillWidth: true;
					onClickedFunc: () => { openAppRepository(focusedApp.appHomeUrl || focusedApp.repository.baseRepositoryUrl); }
				}
			}

		}
	}

	// App version selection
	Item {
		visible: false;
		width: parent.width - 10;
		height: parent.height - 10;
		anchors.centerIn: parent;
		id: appVersionsElement;

		ColumnLayout {
			width: parent.width;
			height: parent.height;

			CustomButton {
				width: parent.width;
				buttonText: "Back";
				buttonColor: colors.button;
				Layout.fillWidth: true;
				onClickedFunc: () => { appVersionsElement.visible = false; appDetailsElement.visible = true; }
			}
			
			Item {
				width: parent.width;
				Layout.fillHeight: true;

				Column {
					width: parent.width;
					height: parent.height;

					Repeater {
						model: Object.keys(focusedApp && focusedApp.appScriptVersions || []).length;
						delegate: Rectangle {
							property string appVersion: Object.keys(focusedApp.appScriptVersions)[index];
							property string appUrl: focusedApp.appScriptVersions[Object.keys(focusedApp.appScriptVersions)[index]];
							width: parent.width;
							height: appUrlElement.visible ? children[0].children[0].height + children[0].children[1].height + 10 : children[0].children[0].height + 10  ;
							color: colors.darkBackground2;

							ColumnLayout {
								width: parent.width - 10;
								height: parent.height - 10;
								spacing: 0;
								anchors.centerIn: parent;

								Item {
									width: parent.width - 10;
									height: children[0].contentHeight;
									Text {
										text: appVersion;
										color: "white";
										font.pixelSize: 24;
										wrapMode: Text.Wrap;
										width: parent.width;
									}
								}

								Item {
									id: appUrlElement;
									width: parent.width - 10;
									height: children[0].contentHeight;
									visible: false;
									clip: true;
									Text {
										text: appUrl;
										color: colors.lightText2;
										font.pixelSize: 16;
										wrapMode: Text.Wrap;
										width: parent.width;
									}
								}
							}

							MouseArea {
								anchors.fill: parent;
								hoverEnabled: true;
								propagateComposedEvents: true;	

								onPressed: {
									installApp(focusedApp, appVersion);
									appVersionsElement.visible = false; 
									appDetailsElement.visible = true; 
								}

								onEntered: {
									parent.color = colors.darkBackground3;
									appUrlElement.visible = true;
								}

								onExited: {
									parent.color = colors.darkBackground2;
									appUrlElement.visible = false;
								}
							}
						}
					}
				}
			}
		}
	}

	function getVersionsCount() {
		// print(Object.keys(appVersions))
		return Object.keys(focusedApp.appVersions).length;
	}
	function getVersionInformation(index) {
		return {
			name: Object.keys(appVersions)[index],
			url: appVersions[Object.keys(appVersions)[index]]
		}
	}
}
