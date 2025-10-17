//
//  util.js
//
//  Created by Armored Dragon on May 5th, 2025.
//  Copyright 2025, Overte e.V.
//
//  ---
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

let util = {
	toJSON: (input) => {
		if (!input) {
			// Nothing.
			return null;
		}

		if (typeof input === "object") {
			// Already JSON.
			return input;
		}

		try {
			// Convert to JSON.
			let inputJSON = JSON.parse(input);
			return inputJSON;
		}
		catch (error) {
			// Failed to convert to JSON, fail gracefully.
			debugLog(`Error parsing ${input} to JSON.`);
			debugLog(error);
			return null;
		}
	},
	request: (url, method = "GET") => {
		return new Promise((resolve) => {
			debugLog(`Making "${method}" request to "${url}"`);
			//if (util.isValidUrl(url) === false) return resolve(null);
			if (util.isValidUrl(url) === false) return reject();
			let req = new XMLHttpRequest();

			req.onreadystatechange = function () {
				if (req.readyState === req.DONE) {
					if (req.status === 200) {
						debugLog(`${method} request to ${url} succeeded.`)
						resolve(req.responseText);
					}
					else {
						debugLog("Error", req.status, req.statusText);
						return resolve(null);
					}
				}
			};

			req.open(method, url);
			req.send();
		})
	},
	isValidUrl: (string) => {
		const urlFromString = util.extractUrlFromString(string);
		if (!urlFromString) return false;

		const isHttpProtocol = urlFromString.substring(0, 4) === "http";

		return isHttpProtocol;
	},
	extractUrlFromString: (string) => {
		if (!string) {
			debugLog(`String is null. Can not extract URL.`)
			return;
		}

		string = string.trim();

		const urlRegex = /[-a-zA-Z0-9@:%_\+.~#?&//=]{2,256}\.[a-z]{2,4}\b(\/[-a-zA-Z0-9@:%_\+.~#?&//=]*)?/g;
		const doesStringHaveUrl = urlRegex.test(string);
		if (doesStringHaveUrl === false) return null;

		const urlFromString = string.match(urlRegex)[0];

		return urlFromString;
	}
}