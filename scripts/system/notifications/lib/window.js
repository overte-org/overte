const NOTIFICATION_CHANNEL = "overte.notification";

const windowFunc = {
	domainConnectionRefused: (reason, code, extra) => {
		receivedMessage(NOTIFICATION_CHANNEL, { type: 'system', message: `Domain Connection Refused`, details: `${reason}\nCODE: '${code}'\nEXTRA: ${extra}` }, null, true)
	},
	stillSnapshotTaken: (path, notify) => {
		// TODO: Check if snapshot notification is enabled
		if (notify !== true) return;
		receivedMessage(NOTIFICATION_CHANNEL, { type: 'system', message: `Snapshot saved`, details: `Directory:\n'${path}'` }, null, true)
	},
	processingGifStarted: (path) => {
		receivedMessage(NOTIFICATION_CHANNEL, { type: 'system', message: `Processing .gif snapshot...` }, null, true)
	},
	connectionAdded: (connectionName) => {
		notification.connection(`Added ${connectionName}`);
	},
	connectionError: (error) => {
		notification.connection(`Error adding connection`, `${error}`);
	},
	announcement: (message, details) => {
		receivedMessage(NOTIFICATION_CHANNEL, { type: 'system', message, details }, null, true)
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
		receivedMessage(NOTIFICATION_CHANNEL, { type: 'system', message: `Tablet needs your attention` }, null, true)
	},
}