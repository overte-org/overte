//
//  formatting.js
//
//  Created by Armored Dragon, 2024.
//  Copyright 2024 Overte e.V.
//
//	This just does some basic formatting and minor housekeeping for the domainChat.js application
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

const formatting = {
	toJSON: function(data) {
		if (typeof data == "object") return data; // Already JSON
		
		try {
			const parsedData = JSON.parse(data);
			return parsedData;
		} catch (e) {
			console.log('Failed to convert data to JSON.')
			return null; // Could not convert to json, some error;
		}
	},
	addTimeAndDateStringToPacket: function(packet) {
		// Gets the current time and adds it to a given packet
		const timeArray = formatting.helpers._timestampArray(packet.timestamp);
		packet.timeString = timeArray[0];
		packet.dateString = timeArray[1];
		return packet;
	},
	trimPacketToSave: function(packet) {
		// Takes a packet, and returns a packet containing only what is needed to save.
		let newPacket = {
			channel: packet.channel || "",
			displayName: packet.displayName || "",
			message: packet.message || "",
			timestamp: packet.timestamp || formatting.helpers.getTimestamp(),
		};
		return newPacket;
	},
	parseMessage: async function(message, enableEmbedding) {
		const urlRegex = /https?:\/\/(www\.)?[-a-zA-Z0-9@:%._\+~#=]{1,256}\.[a-zA-Z0-9()]{1,6}\b([-a-zA-Z0-9()@:%_\+.~#?&//=]*)/;
		const overteLocationRegex = /hifi:\/\/[a-zA-Z0-9_-]+\/[-+]?\d*\.?\d+,[+-]?\d*\.?\d+,[+-]?\d*\.?\d+\/[-+]?\d*\.?\d+,[+-]?\d*\.?\d+,[+-]?\d*\.?\d+,[+-]?\d*\.?\d+/;

		let runningMessage = message;			// The remaining message that will be parsed
		let messageArray = [];					// An array of messages that are split up by the formatting functions

		const regexPatterns = [
			{ type: "url", regex: urlRegex },
			{ type: "overteLocation", regex: overteLocationRegex }
		]

		while (true) {
			let firstMatch = _findFirstMatch();

			if (firstMatch == null) {
				// If there is no more text to parse, break out of the loop and return the message array.
				// Format any remaining text as a basic 'text' type.
				if (runningMessage.trim() != "") messageArray.push({type: 'text', value: runningMessage});

				// Append a final 'fill width' to the message text.
				messageArray.push({type: 'messageEnd'});
				break;
			}

			_formatMessage(firstMatch);
		}

		// Embed images in the message array.
		if (enableEmbedding) {
			for (dataChunk of messageArray){
				if (dataChunk.type == 'url'){
					let url = dataChunk.value;

					const res = await formatting.helpers.fetch(url, {method: 'GET'});      // TODO: Replace with 'HEAD' method. https://github.com/overte-org/overte/issues/1273
					const contentType = res.getResponseHeader("content-type");

					if (contentType.startsWith('image/')) {
						messageArray.push({type: 'imageEmbed', value: url});
						continue;
					}
					if (contentType.startsWith('video/')){ 
						messageArray.push({type: 'videoEmbed', value: url});
						continue;
					}
				}
			}
		}

		return messageArray;

		function _formatMessage(firstMatch){
			let indexOfFirstMatch = firstMatch[0];
			let regex = regexPatterns[firstMatch[1]].regex;

			let foundMatch = runningMessage.match(regex)[0];

			if (runningMessage.substring(0, indexOfFirstMatch) != "") messageArray.push({type: 'text', value: runningMessage.substring(0, indexOfFirstMatch)});
			messageArray.push({type: regexPatterns[firstMatch[1]].type, value: runningMessage.substring(indexOfFirstMatch, indexOfFirstMatch + foundMatch.length)});
			
			runningMessage = runningMessage.substring(indexOfFirstMatch + foundMatch.length);   // Remove the part of the message we have worked with
		}

		function _findFirstMatch(){
			let indexOfFirstMatch = Infinity;
			let indexOfRegexPattern = Infinity;

			for (let i = 0; regexPatterns.length > i; i++){
				let indexOfMatch = runningMessage.search(regexPatterns[i].regex);

				if (indexOfMatch == -1) continue;                                              // No match found

				if (indexOfMatch < indexOfFirstMatch) {
					indexOfFirstMatch = indexOfMatch;
					indexOfRegexPattern = i;
				}
			}

			if (indexOfFirstMatch !== Infinity) return [indexOfFirstMatch, indexOfRegexPattern];    // If there was a found match
			return null;                                                                            // No found match
		}
	},

	helpers: {
		// Small functions that are used often in the other functions.
		_timestampArray: function(timestamp) {
			const currentDate = timestamp || formatting.helpers.getTimestamp();
			let timeArray = [];

			timeArray.push(new Date(currentDate).toLocaleTimeString(undefined, {
				hour12: false,
			}));
	
			timeArray.push(new Date(currentDate).toLocaleDateString(undefined, {
				year: "numeric",
				month: "long",
				day: "numeric",
			}));
	
			return timeArray;
		},
		getTimestamp: function(){
			return Date.now();
		},
		fetch: function (url, options = {method: "GET"}) {
			return new Promise((resolve, reject) => {
				let req = new XMLHttpRequest();
	
				req.onreadystatechange = function () {
	
					if (req.readyState === req.DONE) {
						if (req.status === 200) {
							resolve(req);
				
						} else {
							console.log("Error", req.status, req.statusText);
							reject();
						}
					}
				};
	
				req.open(options.method, url);
				req.send();
			});
		}
	}
}
