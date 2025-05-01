"use strict";

const uiPath = Script.resolvePath("./qml/cryptographyDebugger.qml");

let tablet = Tablet.getTablet("com.highfidelity.interface.tablet.system");
let active = false;
let debugWindow = null;
let selection = null;

const rsa = Script.require('./modules/rsa.js');

function showCryptographyWindow() {
	active = true;
	appButton.editProperties({ isActive: true });

	debugWindow = new Desktop.createWindow(uiPath, {
		title: "Cryptography Debugger",
		size: { x: 300, y: 100 },
		visible: true,
		presentationMode: Desktop.PresentationMode.VIRTUAL,
	});

	debugWindow.closed.connect(closeCryptographyWindow);
	debugWindow.fromQml.connect(fromQML);

}

function closeCryptographyWindow() {
	active = false;
	if (debugWindow) {
		debugWindow.close();
		appButton.editProperties({ isActive: false });
	}
}

let appButton = tablet.addButton({
	icon: Script.resolvePath("./img/icon_white.svg"),
	activeIcon: Script.resolvePath("./img/icon_black.svg"),
	text: "CRYPTO",
	sortOrder: 7,
	isActive: active,
});

appButton.clicked.connect(toolbarButtonClicked);
Script.scriptEnding.connect(shutdownScript);

function toolbarButtonClicked() {
	if (active) closeCryptographyWindow();
	else showCryptographyWindow();
}

showCryptographyWindow();

function fromQML(event) {
	console.log(`Got event from QML:\n ${JSON.stringify(event, null, 4)}`);

	if (event.type == "action") {
		if (event.action === "startTest") runTest();
	}
}

function shutdownScript() {
	// Script has been removed.
	console.log("Shutting Down");
	tablet.removeButton(appButton);
}

function toQML(packet = { type: "" }) {
	debugWindow.sendToQml(packet);
}

async function runTest() {
	let testBits = [1024, 2048, 3072, 4096];
	testBits.forEach((bitLength) => rsa.runRSATest(bitLength));
}

function debugLog(message) {
	console.log(message);
}