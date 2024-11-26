//
//  translation.js
//
//  Created by Armored Dragon in November 2024
//  Copyright 2024, Overte e.V.
//
//  This module is used to request translations from webservers
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

// TODO: Libretranslate API key support
// TODO: Default service
// TODO: Override default service
// TODO: Detect language

const TranslationModule = {
	translate: async (text = "", targetLanguage = "", sourceLanguage = "auto") => {
		if (Settings.getValue('translate/enabled', false) == false) return; // Translation disabled

		let translationResponse = await libreTranslateRequest(sourceLanguage, targetLanguage, text);
		return translationResponse;
	}
}

/*
	When creating new translation services, always be sure you are returning the same exact response.
	The response should be something like this:

	{
		success: bool, 		// Success state of the whole translation request.
		text: string, 		// Translated text upon success, otherwise undefined.
		error: string,		// Error reason, otherwise undefined.
	}
*/

function libreTranslateRequest(sourceLanguage, targetLanguage, text){
	const url = Settings.getValue('translate/libreTranslate/serverURL');

	const packet = {
		q: text,
		source: sourceLanguage,
		target: targetLanguage,
		format: "text",
		alternatives: 0,
	}
	return new Promise((resolve, reject) => {
		if (sourceLanguage == targetLanguage) return reject({success: false, error: "Can not translate from and to the same language."}); // No need to translate
		if (text == "") return reject({success: false, error: "No text to translate provided."}); // Nothing to translate
		if (targetLanguage == "") return reject({success: false, error: "No target language provided."}); // We don't know what to translate to

		var req = new XMLHttpRequest();
		req.open("POST", url, true);
		req.setRequestHeader('Content-Type', 'application/json');
		req.onreadystatechange = function () {
			if (req.readyState === 4) {
				if (req.status === 200) {
					const parsed = JSON.parse(req.responseText);
					return resolve({success: true, text: parsed.translatedText});
				}
	
				return resolve({success: false, error: `Unexpected HTTP code: ${req.status}`});
			}
		};
		req.send(JSON.stringify(packet));
	})
}