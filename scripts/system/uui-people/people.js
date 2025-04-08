"use strict";

const directoryBase = Account.metaverseServerURL;

// Get list of users
// Get information about users (blocked/ volume, etc)

let tablet = Tablet.getTablet("com.highfidelity.interface.tablet.system");
let active = false;

let appButton = tablet.addButton({
	icon: Script.resolvePath("./img/icon_white.png"),
	activeIcon: Script.resolvePath("./img/icon_black.png"),
	text: "PEOPLE",
	sortOrder: 7,
	isActive: active,
});

appButton.clicked.connect(toolbarButtonClicked);

tablet.fromQml.connect(fromQML);
tablet.screenChanged.connect(onScreenChanged);
Script.scriptEnding.connect(shutdownScript);

function toolbarButtonClicked() {
	if (active) {
		tablet.gotoHomeScreen();
		active = !active;
		appButton.editProperties({ isActive: active });
	} else {
		tablet.loadQMLSource(Script.resolvePath("./qml/people.qml"));
		active = !active;
		appButton.editProperties({ isActive: active });
		sendMyData();
		updatePalList();
	}
}

function onScreenChanged(type, url) {
	if (url != Script.resolvePath("./qml/people.qml")) {
		active = false;
		appButton.editProperties({
			isActive: active,
		});
	}
}

function fromQML(event) {
	console.log(`New QML event:\n${JSON.stringify(event)}`);
	if (event.type == "getUserData") {
		getFocusedUserData(event.user);
		return;
	}
}

function shutdownScript() {
	// Script has been removed.
	console.log("Shutting Down");
	tablet.removeButton(appButton);
}

function toQML(packet = { type: "" }) {
	tablet.sendToQml(packet);
}

function updatePalList() {
	// Updates the UI to the list of people in the session.
	const palData = AvatarManager.getPalData().data;
	toQML({ type: "palList", data: palData });
}


function sendMyData() {
	// Send the current user to the QML UI.
	let data = {
		displayName: MyAvatar.displayName,
		icon: null,
	}

	// Get the current user's avatar icon.
	var url = directoryBase + '/api/v1/users?filter=connections&per_page=10&search=' + encodeURIComponent(data.displayName);

	request(url).then((res) => {
		let parsedResponse = JSON.parse(res);

		if (parsedResponse.status !== `success`) return;

		parsedResponse = parsedResponse.data.users[0]; // My user

		data.icon = parsedResponse.images.thumbnail;

		toQML({ type: "myData", data: data });
	});
}

function getFocusedUserData(sessionUUID) {
	const palData = AvatarManager.getPalData().data;
	const singleUserPalData = palData.filter((user) => user.sessionUUID === sessionUUID)[0];
	toQML({ type: "focusedUserData", data: singleUserPalData });
}

function request(url, method = "GET") {
	return new Promise((resolve) => {

		var req = new XMLHttpRequest();
		req.onreadystatechange = function () {
			if (req.readyState === req.DONE) {
				if (req.status === 200) {
					resolve(req.responseText);
				}
				else {
					print("Error", req.status, req.statusText);
				}
			}
		};

		req.open(method, url);
		req.send();
	})
}
