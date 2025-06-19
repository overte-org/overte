

const windowFunc = {
	domainConnectionRefused: (reason, code, extra) => {
		notification.system(`Domain Connection Refused`, `${reason}\nCODE: '${code}'\nEXTRA: ${extra}`);
	},
	stillSnapshotTaken: (path, notify) => {
		// TODO: Check if snapshot notification is enabled
		if (notify !== true) return;

		notification.system(`Snapshot saved`, `Directory:\n'${path}'`);
	},
	processingGifStarted: (path) => {
		notification.system(`Processing .gif snapshot...`)
	},
	connectionAdded: (connectionName) => {
		notification.connection(`Added ${connectionName}`);
	},
	connectionError: (error) => {
		notification.connection(`Error adding connection`, `${error}`);
	},
	announcement: (message) => {
		notification.system(`${message}`)
	},
	notifyEditError: (message) => {
		// Seems to only be for edit.js. Deprecate but allow use.
		debugLog(`window.notifyEditError is deprecated. Please use 'window.announcement()'`);
		windowFunc.announcement(message);
	},
	notify: (message) => {
		// Not sure what this one is used for, only for edit.js?
		// If so, log deprecation notice to console, but use anyways.
		debugLog(`window.notify is deprecated. Please use 'window.announcement()'`);
		windowFunc.announcement(message);
	},
	tabletNotification: (message = null) => {
		// Currently hardcoded value.
		notification.system(`Tablet needs your attention`);
	},
}