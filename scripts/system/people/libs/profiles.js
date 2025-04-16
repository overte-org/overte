//
//  contacts.js
//
//  A small library to help with viewing and parsing user profiles
//
//  Created by Armored Dragon, 2025.
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
"use strict";

var helper = Script.require("./helper.js");

const directoryBase = Account.metaverseServerURL;

let profilesLib = {
	getProfile: (username) => {
		return new Promise((resolve, reject) => {
			print(`Getting profile for '${username}'.`);

			const url = `${directoryBase}/api/v1/users?filter=connections&per_page=10&search=${encodeURIComponent(username)}`
			
			helper.request(url).then(onResponse);

			function onResponse(response) {
				response = helper.makeJSON(response);

				if (response.status !== "success") {
					print(`Error requesting profile for ${username}.`);
					return reject({success: false, message: "Unknown error", response: response});
				}

				return resolve({success: true, message: `Found ${username}'s profile.`, profile: response.data.users[0]});
			}
		})
	}
}

module.exports = profilesLib;