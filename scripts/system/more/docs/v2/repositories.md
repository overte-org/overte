# Repositories

- [Repository formatting](#repository-formatting-how-to-make-your-own-repository)
- [Note on legacy community-apps](#note-on-legacy-community-apps)

# Repository formatting: How to make your own repository

More.js works by requesting and receiving a JSON response from the net. The response received is expected to follow a specific format. 
The following table lists all valid fields and expected value types.  

## Header
### Valid fields
| Field | Value | Description |
| --- | --- | --- |
version | int | The version number of the metadata.json file. 
title | string | The title of the repository. This makes identifying your specific repository easier.
baseApiUrl | string | The base of a API request url that will be used when downloading apps.
baseRepositoryUrl | string | A human readable homepage of the repository. 
applicationList | array | The list of applications this repository supplies, an array of Objects.

### Example
Here is an example implementation of the above table. This example is non-functional and serves only to illustrate the expected structure.

```json
{
	"version": 2,
	"title": "Example Repository",
	"baseApiUrl": "https://raw.githubusercontent.com/Overte/community-apps/master",
	"baseRepositoryUrl": "https://more.overte.org",
	"applicationList": [
		{
			// An example will be provided later for valid applicationList entries.	
		}
	]
}
```

## Application List
### Valid fields

| Field | Value | Description |
| --- | --- | --- |
appName | string | The name of the application.
appDescription | string | The description of the application.
appBaseDirectory | string | The base directory of the application. Computed with the repositories "baseApiUrl".
appScriptVersions | object | An object containing all versions of the application.
appIcon | string | A directory that contains an application icon for the app listing. Can either be relative or a direct HTTPS link.
appAuthor | string | The author(s) of the application. Supplied as a string.
appAgeMaturity | string | An age maturity rating of the application.
appCategory | string | The category of the application.
appHomeUrl | string | A human readable homepage for the application.
appActive | bool | A boolean that determines whether to display the application in the application list. Normally this should always be 'true'. 

### Example
Here is an example implementation of the above table. This example is non-functional and serves only to illustrate the expected structure.

```json
	{
		"appName": "My Application",
		"appDescription": "A very cool application that does magical things!",
		"appBaseDirectory": "myapp",
		"appScriptVersions": {
			"stable": "myapp.js",
			"testVersion": "myapp-test.js"
		},
		"appIcon": "img/icon_white.png",
		"appAuthor": "John Doe",
		"appAgeMaturity": "EVERYONE",
		"appCategory": "UTILITY",
		"appHomeUrl": "https://example.com",
		"appActive": true
	}
```

More.js is capable of reading files from either a relative position from the "appBaseDirectory" field above or through HTTPS links.
An example folder structure that would work for this would be the following:
```
ROOT/
┗ myapp/
  ┣ myapp.js        
  ┣ myapp-test.js   
  ┗ img/
    ┗ icon_white.png
```

When constructing the intended URL to be used, More.js will combine the "baseApiUrl" field provided in the header area of the repository response, along with the apps "appBaseDirectory" and iterate through the "appScriptVersions" (and every other url for that matter) to compose the full valid url.
The provided example for both the repository metadata.json file and this application object would compose to the following:

`https://raw.githubusercontent.com/Overte/community-apps/master` (baseApiUrl) + <br>
`myapp` (appBaseDirectory) + <br>
`myapp.js` (appScriptVersions.stable) <br>

Which will be interpreted as `https://raw.githubusercontent.com/Overte/community-apps/master/myapp/myapp.js`.
The provided example will also use a url from the `testVersion` field: `https://raw.githubusercontent.com/Overte/community-apps/master/myapp/myapp-test.js`, and it will be listed along side the stable version.


# Note on Legacy Community apps

This application has support for the existing community apps repository offered by Overte through https://more.overte.org. This support is very limited and hard coded. The community-apps repository is automatically converted to a format that follows the expected format for third-party repositories. However since version 2 contains more information about each application than what is provided from the metadata.js file, there is some information that may be slightly inaccurate, or misleading. 