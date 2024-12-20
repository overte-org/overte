# Translation Module

This module provides an easy way to preform translation tasks using http services.

## Including the module
You can use this translation module by using [Script.include](https://apidocs.overte.org/Script.html#.include) at the top of your file.

```javascript 
Script.include('/~/modules/translation/translation.js');
```

## Using the module
This module exposes the `TranslationModule` object.

Supported functions are:
```javascript
TranslationModule.translate(
	textToTranslate, 		// The text string to translate
	targetLanguageCode, 	// Target language to translate to, as a locale code. ("en" is English, "es" is Spanish, .etc)
	sourceLanguageCode		// Optional. The source language of the message. This ensures that the translation service knows what the source language is if it is supported.
)
``` 


## Supported services
`LibreTranslate` - [LibreTranslate](https://github.com/LibreTranslate/LibreTranslate) is a self hosted translation service

## Settings
`General`: 
- `enabled` - Whether to enable the translation functionality globally or not.

`LibreTranslate`:
- `serverURL` - The hostname of the LibreTranslate instance to be used when making a translation request

## Development
### Settings format
When interfacing with the Overte Settings api. make sure to follow the format `translate/<service>/<specific>`
For example, if you want to adjust a setting exposed to the "LibreTranslate" service, you would use `translate/libreTranslate/<specific>`. Please note that the settings are registered using standard camel case formatting. 

There may be specific settings that do not include a "service" and instead simply use the `translate/<specific>` format. For example: `translate/enabled`.

### Adding a service
The intended way to add services to this module is to create a function named after the service, followed by the word "request". 
Example: The service `LibreTranslate` turns into `libreTranslateRequest(...)`.

### Calling a service 
Each service is intended to get the same information as every other. The inputs will be the text to translate, the language intended to be translated to, and the source language. Some services may not accept a source language and it is ignored when calling the service. See "Using the module" for the specific inputs.

When returning data retrieved from a service, it should be returned in an object in the following format:
```javascript
{
	success: bool, 		// Success state of the whole translation request.
	text: string,		// Translated text upon success, otherwise undefined.
	error: string		// Error reason, otherwise undefined.
}
```