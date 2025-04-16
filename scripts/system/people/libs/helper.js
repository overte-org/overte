//
//  helper.js
//
//  A small library that provides helper functions used throughout this application.
//
//  Created by Armored Dragon, 2025.
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
"use strict";

let helper = {
	removeCurlyBracesFromUuid: (Uuid) => {
		return guidWithCurlyBraces.slice(1, -1);
	},
	request: (url, method = "GET", body) => {
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
	},
	makeJSON: (string) => {
		if (typeof string === "object") return string;
		try {
			return JSON.parse(string);
		} catch {
			print(`Could not turn into JSON:\n${string}`);
			return {};
		}
	},
	logJSON: (obj) => {
		print(JSON.stringify(obj, null, 4));

	}
}

module.exports = helper;