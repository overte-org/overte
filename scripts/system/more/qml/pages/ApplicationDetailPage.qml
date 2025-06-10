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

			CustomButton {
				width: parent.width;

				buttonText: "Back";

				onClickedFunc: () => { showAppListPage() }
			}

			Row {
				width: parent.width;
				height: 100;

				AppImage {
					imageSize: 100;
					icon: focusedApp && focusedApp.appIcon || "";
				}

				Item {
					width: 10;
					height: 1;
				}

				Column {
					width: parent.width;

					Text {
						text: focusedApp && focusedApp.appName || "";
						color: "white";
						font.pixelSize: 20;
					}
					Text {
						text: focusedApp && focusedApp.appCategory || "";
						color: "gray";
						font.pixelSize: 16;
					}
					Text {
						text: focusedApp && focusedApp.appAgeMaturity || "";
						color: "gray";
						font.pixelSize: 16;
					}
				}
			}

			Row {
				spacing: 10;
				CustomButton {
					visible: isAppInstalled === false;
					buttonText: "Install";
					buttonColor: colors.buttonSafe;
					onClickedFunc: () => { appVersionsElement.visible = true; appDetailsElement.visible = false; }
				}
				CustomButton {
					visible: isAppInstalled === true;
					buttonText: "Remove";
					buttonColor: colors.buttonDanger;
					onClickedFunc: () => { uninstallApp(focusedApp.installedUrl) }
				}
				CustomButton {
					buttonText: "View Repository";
					buttonColor: colors.button;
					onClickedFunc: () => { openAppRepository(focusedApp.appHomeUrl || focusedApp.repository.baseRepositoryUrl) }
				}
			}

			Column {
				width: parent.width;
				height: 1;

				Text {
					visible: isAppInstalled === true;
					text: focusedApp && focusedApp.isInstalled ? "Installed:\n" + focusedApp.installedUrl + "\n" : "";
					color: "gray";
					font.pixelSize: 16;
					wrapMode: Text.Wrap;
					width: parent.width;
				}

				Text {
					text: focusedApp && focusedApp.appDescription || "";
					color: "gray";
					font.pixelSize: 16;
					wrapMode: Text.Wrap;
					width: parent.width;
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
				buttonText: "Go Back";
				buttonColor: colors.button;
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
							height: 50;
							color: colors.darkBackground2;

							RowLayout {
								width: parent.width;
								height: 50;

								Text {
									text: appVersion;
									color: "white";
								}

								Text {
									text: appUrl;
									color: "orange";
									font.pixelSize: 20;
								}
							}

							MouseArea {
								anchors.fill: parent;
								hoverEnabled: true;
								propagateComposedEvents: true;	

								onPressed: {
									installApp(focusedApp, appVersion)
								}

								onEntered: {
									parent.color = colors.darkBackground3;
								}

								onExited: {
									parent.color = colors.darkBackground2;
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
