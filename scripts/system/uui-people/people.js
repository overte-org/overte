"use strict";

const directoryBase = Account.metaverseServerURL;

// TODO: Get all user profile pictures?
// TODO: User status (Public / Friends only / Private)
// TODO: Focused user halo
// TODO: Teleport + Teleport permissions

let tablet = Tablet.getTablet("com.highfidelity.interface.tablet.system");
let active = false;

let appButton = tablet.addButton({
	icon: Script.resolvePath("./img/icon_white.png"),
	activeIcon: Script.resolvePath("./img/icon_black.png"),
	text: "PEOPLE",
	sortOrder: 7,
	isActive: active,
});

let palData = {};

const selectionListName = "people.focusedUser";
const selectionListStyle = {
	outlineUnoccludedColor: { red: 255, green: 0, blue: 0 },
	outlineUnoccludedAlpha: 1,
	fillUnoccludedColor: {red: 255, green: 255, blue: 255},
	fillUnoccludedAlpha: 0.0,
	outlineOccludedColor: { red: 255, green: 255, blue: 255 },
	outlineOccludedAlpha: 0.7,
	outlineWidth: 4,
	fillOccludedAlpha: 0.2
}; 

appButton.clicked.connect(toolbarButtonClicked);

tablet.fromQml.connect(fromQML);
tablet.screenChanged.connect(onScreenChanged);
Script.scriptEnding.connect(shutdownScript);
Script.setInterval(updatePalData, 100);

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
	}
}

function onScreenChanged(type, url) {
	if (url != Script.resolvePath("./qml/people.qml")) {
		active = false;
		removeHighlightUser();
		appButton.editProperties({
			isActive: active,
		});
	}
}

function fromQML(event) {
	console.log(`New QML event:\n${JSON.stringify(event)}`);
	if (event.type == "focusedUser") {
		if (Uuid.fromString(event.user) !== null) return highlightUser(event.user);
		else return removeHighlightUser();
	}
}

function shutdownScript() {
	// Script has been removed.
	console.log("Shutting Down");
	tablet.removeButton(appButton);
	removeHighlightUser();
}

function toQML(packet = { type: "" }) {
	tablet.sendToQml(packet);
}

function updatePalData() {
	// Updates the UI to the list of people in the session.
	palData = AvatarManager.getPalData().data;

	// Don't include ourself in the list
	palData = palData.filter((user) => user.sessionUUID !== "");

	// Set the audioLoudness value to a exponential value that fits within the bounds of the visual audio scale.
	palData.map((user) => {user.audioLoudness = scaleAudioExponential(user.audioLoudness)});

	toQML({ type: "palList", data: palData });

	function scaleAudioExponential(audioValue) {
		let normalizedValue = audioValue / 32768;
		let scaledValue = Math.pow(normalizedValue, 0.3);
		return scaledValue;
	}
}

function sendMyData() {
	// Send the current user to the QML UI.
	let data = {
		displayName: MyAvatar.displayName,
		icon: null,
		canKick: Users.getCanKick()
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

function highlightUser(sessionUUID){
	// We only ever highlight a single user. Delete the previous selection (if it exists)
	removeHighlightUser();

	// Create the selection highlight
	Selection.enableListHighlight(selectionListName, selectionListStyle);

	// Highlight the user
	Selection.addToSelectedItemsList(selectionListName, "avatar", sessionUUID);
}

function removeHighlightUser(){
	// Destroy the highlight list
	Selection.removeListFromMap(selectionListName);
}