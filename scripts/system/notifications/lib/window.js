const NOTIFICATION_CHANNEL = "overte.notification";

const windowFunc = {
	domainConnectionRefused: (reason, code, extra) => {
		Window.displayAnnouncement(`Domain Connection Refused`, `${reason}\nCODE: '${code}'\nEXTRA: ${extra}`);
	},
	stillSnapshotTaken: (path, notify) => {
		// TODO: Check if snapshot notification is enabled
		if (notify !== true) return;
		Window.displayAnnouncement(`Snapshot saved`, `Directory:\n'${path}'`);
	},
	processingGifStarted: (path) => {
		Window.displayAnnouncement(`Processing .gif snapshot...`);
	},
	connectionAdded: (connectionName) => {
		notification.system(`Added ${connectionName}`);
	},
	connectionError: (error) => {
		notification.system(`Error adding connection`, `${error}`);
	},
	announcement: (message, details) => {
		notification.system(message, details);
	},
	notifyEditError: (message) => {
		// Seems to only be for edit.js. Deprecate but allow use.
		util.debugLog(`window.notifyEditError is deprecated. Please use 'window.announcement()'`);
		Window.displayAnnouncement(message);
	},
	notify: (message) => {
		// Not sure what this one is used for, only for edit.js?
		// If so, log deprecation notice to console, but use anyways.
		util.debugLog(`window.notify is deprecated. Please use 'window.announcement()'`);
		Window.displayAnnouncement(message);
	},
	tabletNotification: (message = null) => {
		// Currently hardcoded value.
		windowFunc.announcement(`Tablet needs your attention`);
	},
}