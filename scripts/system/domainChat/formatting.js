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
		}
	}
}
