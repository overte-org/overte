//
//  contacts.js
//
//  A small library to help with managing user contacts
//
//  Created by Armored Dragon, 2025.
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
"use strict";

const directoryBase = Account.metaverseServerURL;

let contactsLib = {
	contacts: [],
	
	addContact: (uuid) => {
		return new Promise((resolve, reject) => {
			print(`Adding contact '${uuid}'`);
			
			const requestUrl = `${directoryBase}/api/v1/user/connection_request`;
			const requestBody = {'node_id': removeCurlyBracesFromUuid(MyAvatar.sessionUUID), 'proposed_node_id': removeCurlyBracesFromUuid(uuid)};
			
			request(requestUrl, "POST", {'user_connection_request': requestBody}).then(onResponse);
		
			function onResponse(response){
				const responseJSON = makeJSON(response);
		
				// Error check
				if (responseJSON.status !== "success") {
					print(`Error sending contact request.`)
					return reject({success: false, message: "Unknown error", response: responseJSON});
				}
			

				// We sent a request, but the recipient does not have an outgoing request to us.
				if (responseJSON.data.connection === "pending") {
					print(`Contact request is pending.`);
					return resolve({success: true, message: "Contact request sent.", accepted: false});
				}

				// We sent a request, and the recipient has a outgoing request for us.
				// We are now contacts
				if (responseJSON.data.connection.new_connection) {
					print(`Contact request is accepted.`);
					return resolve({success: true, message: `Contact request for ${uuid} accepted.`, accepted: true});
				}
			}
		})
	},
	removeContact: (username) => {
		return new Promise((resolve, reject) => {
			print(`Removing contact '${username}'.`);

			request(`${directoryBase}/api/v1/user/connections/${username}`, `DELETE`).then(onResponse);

			function onResponse(response){
				const responseJSON = makeJSON(response);

				logJSON(responseJSON);

				if (responseJSON.status !== "success") {
					print(`Error sending contact removal request.`)
					return reject({success: false, message: "Unknown error", response: responseJSON});
				}

				resolve({success: true, message: `Contact '${username}' was removed.`})
			}
		});
	},
	addFriend: (username) => {
		return new Promise((resolve, reject) => {
			print(`Adding friend '${username}'.`);

			request(`${directoryBase}/api/v1/user/friends`, `POST`, {username: username}).then(onResponse);

			function onResponse(response) {
				const responseJSON = makeJSON(response);
				logJSON(responseJSON);
			}
		})
	},
	removeFriend: (username) => {
		return new Promise((resolve, reject) => {
			print(`Removing friend '${username}'.`);
			request(`${directoryBase}/api/v1/user/friends/${username}`, `DELETE`).then(onResponse);

			function onResponse(response) {
				const responseJSON = makeJSON(response);
				logJSON(responseJSON);
			}
		})
	},
	getContactList: () => {
		return new Promise((resolve, reject) => {
			print(`Getting contact list.`);

			request(`https://mv.overte.org/server/api/v1/users/connections`).then(onResponse);

			function onResponse(response) {
				const responseJSON = makeJSON(response);

				if (responseJSON.status !== "success") {
					print(`Error requesting contacts.`);
					return reject({success: false, message: "Unknown error", response: responseJSON});
				}

				contactsLib.contacts = responseJSON.data.users;
				resolve({success: true, message: "Contacts have been updated", contacts: contactsLib.contacts});
			}
		});
	},
	getContactByUsername: (username, refreshContactList = false) => {
		return new Promise(async (resolve, reject) => {
			if (refreshContactList) await contactsLib.getContactList();

			const contactSingle = contactsLib.contacts.find((contact) => contact.username === username);

			return resolve({success: true, contact: contactSingle});
		});
	}
}

function removeCurlyBracesFromUuid(guidWithCurlyBraces) {
	return guidWithCurlyBraces.slice(1, -1);
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

function makeJSON(string){
	if (typeof string === "object") return string;
	try {
		return JSON.parse(string);
	} catch {
		print(`Could not turn into JSON:\n${string}`);
		return {};
	}
}

// Debug functions ---
function logJSON(obj){
	print(JSON.stringify(obj, null, 4));
}

module.exports = contactsLib;