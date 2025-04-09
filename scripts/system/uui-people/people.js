"use strict";

const directoryBase = Account.metaverseServerURL;

// TODO: Connections
// FIXME: Check if focus user exists before issuing commands on them
// TODO: User join / leave notifications

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
let iAmAdmin = Users.getCanKick();
let adminUserData = {};
let adminUserDataTimestamp = 0;

let ignoredUsers = {};

appButton.clicked.connect(toolbarButtonClicked);

tablet.fromQml.connect(fromQML);
tablet.screenChanged.connect(onScreenChanged);
Script.scriptEnding.connect(shutdownScript);
Script.setInterval(updatePalData, 100);
Users.usernameFromIDReply.connect(onUsernameFromIDReply);

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
	if (event.type == "focusedUser") {
		if (Uuid.fromString(event.user) !== null) return highlightUser(event.user);
		else return removeHighlightUser();
	}

	if (event.type == "updateMyData") {
		return sendMyData();
	} 

	if (event.type == "ignoreUser"){
		if (ignoredUsers[event.user.sessionUUID]) {
			// User is ignored, unignore them
			delete ignoredUsers[event.sessionUUID];
			Users.ignore(event.sessionUUID, false);
			return;
		}
		else {
			ignoredUsers[event.user.sessionUUID] = event.user;
			Users.ignore(event.user.sessionUUID, true);
		}
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

	toQML({ type: "palList", data: [...palData, ...Object.values(ignoredUsers)] });
	toQML({ type: "adminUserData", data: adminUserData });

	palData.forEach((user) => domainUserUpdate(user.sessionUUID));

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
		canKick: Users.getCanKick(),
		findableBy: AccountServices.findableBy,
		username: AccountServices.username
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

function domainUserUpdate(sessionUUID){
	if (!iAmAdmin) return; 						// Not admin, not going to work
	if (adminUserData[sessionUUID]) return; 	// We already have that data!

	console.log(`Requesting ${sessionUUID}'s information`)
	adminUserData[sessionUUID] = {}; 			// Initialize to prevent multiple requests for a single user
	Users.requestUsernameFromID(sessionUUID);
}

function onUsernameFromIDReply(sessionUUID, userName, machineFingerprint, isAdmin) {
	console.log(`Got ${sessionUUID}'s information`)
	adminUserData[sessionUUID] = {
		username: userName,
		isAdmin: isAdmin
	};
	toQML({type: "adminUserData", data: adminUserData});
}