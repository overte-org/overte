"use strict";

const directoryBase = Account.metaverseServerURL;

// FIXME: Check if focus user exists before issuing commands on them.
// FIXME: Not having contacts breaks our own data?
// TODO: User join / leave notifications.
// FIXME: Better contacts page.
// TODO: Documentation.
// TODO: User selection distance based fallback.
// TODO: Highlight contact if they are in the session.

// Housekeeping TODO:
// TODO: Singular focused user. Create internal use object with all relevant information.
// TODO: Check to see if contact is present in world.

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
let contactsLib = Script.require("./libs/contacts.js");
let profilesLib = Script.require("./libs/profiles.js");
let helper = Script.require("./libs/helper.js");
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
	console.log(`Got event from QML:\n ${JSON.stringify(event, null, 4)}`);

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

	if (event.type == "addContact"){
		contactsLib.addContact(event.sessionId).then(helper.logJSON);
		return;
	}

	if (event.type == "removeContact") {
		contactsLib.removeContact(event.username).then(helper.logJSON);
		return;
	}

	if (event.type == "addFriend"){
		contactsLib.addFriend(event.username).then(helper.logJSON);
		return;
	}

	if (event.type == "removeFriend"){
		contactsLib.removeFriend(event.username).then(helper.logJSON);
		return;
	}

	if (event.type == "findContactByUsername") {
		contactsLib.getContactByUsername(event.username).then((response) => {
			if (response.success) toQML({type: "contactFromUsername", contact: response.contact})
		})
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

async function sendMyData() {
	// Send the current user to the QML UI.
	let data = {
		displayName: MyAvatar.displayName,
		icon: null,
		canKick: Users.getCanKick(),
		findableBy: AccountServices.findableBy,
		username: AccountServices.username
	}

	contactsLib.getContactList().then(() => {
		toQML({type: "connections", data: {connections: contactsLib.contacts, totalConnections: contactsLib.contacts.length}});
	}).catch((response) => {
		helper.logJSON(response);
	});

	data.icon = (await profilesLib.getProfile(data.displayName)).profile.images.thumbnail;
	toQML({ type: "myData", data: data });
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