"use strict";

const directoryBase = Account.metaverseServerURL;

// FIXME: Check if focus user exists before issuing commands on them
// TODO: User join / leave notifications
// TODO: Dedicated "Edit Persona" button
// FIXME: Better contacts page
// TODO: Documentation
// TODO: User selection distance based fallback

let tablet = Tablet.getTablet("com.highfidelity.interface.tablet.system");
let active = false;

let appButton = tablet.addButton({
	icon: Script.resolvePath("./img/icon_white.svg"),
	activeIcon: Script.resolvePath("./img/icon_black.svg"),
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
		destroyHighlightSelection();
		appButton.editProperties({
			isActive: active,
		});
	}
}

function fromQML(event) {
	if (event.type == "focusedUser") {
		if (Uuid.fromString(event.user) !== null) return highlightUser(event.user);
		else return destroyHighlightSelection();
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

	if (event.type == "addFriend"){
		addFriend(event.username);
		return;
	}

	if (event.type == "removeFriend"){
		removeFriend(event.username);
		return;
	}

}

function shutdownScript() {
	// Script has been removed.
	console.log("Shutting Down");
	tablet.removeButton(appButton);
	destroyHighlightSelection();
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

	updateConnections();

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

function request(url, method = "GET", body) {
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
		if (method == `POST`) req.setRequestHeader("Content-Type", "application/json");
		req.send(JSON.stringify(body));
	})
}

function highlightUser(sessionUUID){
	destroyHighlightSelection();

	Selection.enableListHighlight(selectionListName, selectionListStyle);

	Selection.addToSelectedItemsList(selectionListName, "avatar", sessionUUID);

	const childEntitiesOfAvatar = recursivelyGetAllEntitiesOfAvatar(sessionUUID);
	childEntitiesOfAvatar.forEach((id) => Selection.addToSelectedItemsList(selectionListName, "entity", id));
}

function recursivelyGetAllEntitiesOfAvatar(avatarId) {
    let entityIds = [];

    recurse(avatarId);
    return entityIds;

    function recurse(id) {
        const children = Entities.getChildrenIDs(id);
		
        children.forEach((childId) => {
            entityIds.push(childId);
            recurse(childId);
        });
    }
}

function destroyHighlightSelection(){
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

async function updateConnections(){
	console.log(`Updating connections`);
	let req = await request(`https://mv.overte.org/server/api/v1/users/connections`);
	// We have a response we expect
	if (!req.includes('{')) return; 
	
	req = JSON.parse(req);

	if (req.status == "success") {
		const totalConnections = req.total_entries;
		req = req.data.users; // Now we only have the contact data and not any of the extra

		toQML({type: "connections", data: {connections: req, totalConnections: totalConnections}});
	}
}

function addFriend(username){
	request(`${directoryBase}/api/v1/user/friends`, `POST`, {username: username});
}
function removeFriend(username) {
	request(`${directoryBase}/api/v1/user/friends/${username}`, `DELETE`);
}