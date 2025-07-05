# Notifications
`notifications.js` will display important 'system' related notices to the user. It provides the overlay in the top right corner and the means to save notifications in the long term as desired.

- [Usage](#usage)
- [Development](#development)
- [Credits](#credits)

# Usage
This script takes advantage of the [Window.displayAnnouncement()](https://apidocs.overte.org/Window.html#.displayAnnouncement) function. When called, this script will listen for and execute its functions to save and display the announcement.

Refer to the [Window.displayAnnouncement()](https://apidocs.overte.org/Window.html#.displayAnnouncement) documentation for more information.

Usage example:
```javascript
Window.displayAnnouncement('Announcement message', 'Extra details about the announcement');
```

# Development
This script is branched into a few smaller scripts, see `/lib` for the important bits.
- `io.js` handles saving and persisting data.
- `sound.js` handles ui sound effects.
- `utility.js` provides helper functions.
- `window.js` provides signal listeners, mostly for the `Window.` events.

# Credits
- Sound effects
	- `/sound/systemNotification.mp3`
		- https://pixabay.com/sound-effects/new-notification-017-352293/
		- Universfield
		- https://pixabay.com/service/license-summary/