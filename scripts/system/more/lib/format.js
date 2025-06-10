//
//  format.js
//
//  Created by Armored Dragon on 5 May 2025.
//  Copyright 2025 Overte e.V contributors.
//
//  ---
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

let versioning = {
	// Format the response and upgrade to a understood format as needed.
	// If we have a older version, upgrade the format of the response to prevent errors later 
	repository: (repositoryData) => {
		if (repositoryData.version === 2) {
			// Version 2 is the first version supported by this application due to version 1 not being in a valid JSON format.
			// We have to have a separate request for the legacy overte/community-apps repository.
			return repositoryData;
		}
	}
}

let legacy = {
	COMMUNITY_APPS_URL: "https://more.overte.org/applications/metadata.js",
	requestCommunityApps: async () => {
		const repoHeader = {
			title: "Overte Community Apps",
			version: 2,
			baseApiUrl: "https://more.overte.org/applications",
			baseRepositoryUrl: "https://github.com/overte-org/community-apps"
		}

		let response = await util.request(legacy.COMMUNITY_APPS_URL, "GET");
		response = _trimMetadataVariable(response);
		response = util.toJSON(response);

		for (let i = 0; response.applications.length > i; i++) {
			// For each application returned through the response...
			const app = response.applications[i];

			let formattedApplication = {
				appName: app.name,
				appBaseDirectory: null, 			// Set later
				appScriptVersions: null,			// Set later
				appIcon: app.icon,					// Adjusted / formatted later
				appDescription: app.description,
				appAuthor: "Overte",
				appAgeMaturity: null,				// null will be treaded as "unrated"
				appCategory: null,					// null will be treated as "uncategorized"
				appActive: app.isActive
			}

			// Format the base directory
			let splitDirectory = app.jsfile.split('/');
			splitDirectory.pop();
			formattedApplication.appBaseDirectory = splitDirectory.join('/');

			// Format the app Icon URL
			if (formattedApplication.appIcon.indexOf(formattedApplication.appBaseDirectory) > -1) {
				let appIconSplit = formattedApplication.appIcon.split('/');
				appIconSplit.shift();
				formattedApplication.appIcon = appIconSplit.join(`/`);
			}

			// Format the appScriptVersions
			let splitJavascriptFile = app.jsfile.split('/');
			splitJavascriptFile.shift();

			formattedApplication.appScriptVersions = {
				stable: splitJavascriptFile.join('/')
			};

			formattedApplication = repos._embedRepositoryConstants(formattedApplication, repoHeader);
			formattedApplication = repos._formatAppUrls(formattedApplication);
			formattedApplication = repos._checkIfInstalled(formattedApplication);

			repos.applications.push(formattedApplication);
		}

		return;

		function _trimMetadataVariable(response) {
			if (response.indexOf("var metadata") == -1) {
				debugLog(`Community Apps repo does not have the var head`);
				return response;
			}

			response = response.substring(15, response.length);
			response = response.slice(0, -1);

			return response;
		}
	},
	formatCommunityAppsUrl: (url) => {
		// If the url is targeting the community apps repo, change the url
		const urls = ["https://raw.githubusercontent.com/overte-org/community-apps/refs/heads/master/applications/metadata.js", legacy.COMMUNITY_APPS_URL]
		const isLegacyUrl = urls.indexOf(url) > -1;

		if (isLegacyUrl) {
			return legacy.COMMUNITY_APPS_URL;
		}

		return url;
	}
}