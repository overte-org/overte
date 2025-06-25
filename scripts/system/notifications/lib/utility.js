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
			util.debugLog(`Error parsing ${input} to JSON.`);
			util.debugLog(error);
			return null;
		}
	},

	debugLog: (content) => {
		if (typeof content === "object") content = JSON.stringify(content, null, 4);

		console.log(`[ Debug ] ${content}`);
	}
}
