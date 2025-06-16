const util = {
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
	}
}
