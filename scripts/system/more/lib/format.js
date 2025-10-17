//
//  format.js
//
//  Created by Armored Dragon on May 5th, 2025.
//  Copyright 2025, Overte e.V.
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
			return repositoryData;
		}
	}
}
