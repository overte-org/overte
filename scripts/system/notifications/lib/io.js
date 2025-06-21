const SETTING_SYSTEM_NOTIFICATION = "overte.notification.system";
const SETTING_CONNECTION_NOTIFICATION = "overte.notification.connection";

const io = {
	getNotifications: () => {
		app._data.systemNotifications = Settings.getValue(SETTING_SYSTEM_NOTIFICATION, new Array());
		app._data.connectionNotifications = Settings.getValue(SETTING_CONNECTION_NOTIFICATION, new Array());
	},
	saveNotification: (messageJSON) => {
		if (messageJSON.type === "system") {
			app._data.systemNotifications.push(messageJSON);
			app._data.systemNotifications = clampArrayLengthToMaximumLength(app._data.systemNotifications, app._config.maximumSavedSystemNotifications)
			Settings.setValue(SETTING_SYSTEM_NOTIFICATION, app._data.systemNotifications);
			return;
		}

		if (messageJSON.type === "connection") {
			app._data.connectionNotifications.push(messageJSON);
			app._data.connectionNotifications = clampArrayLengthToMaximumLength(app._data.connectionNotifications, app._config.maximumSavedConnectionNotifications)
			Settings.setValue(SETTING_CONNECTION_NOTIFICATION, app._data.connectionNotifications);
			return;
		}

		debugLog(`Unknown message type '${messageJSON.type}'. Not saving notification.`)
	}
}

function clampArrayLengthToMaximumLength(arr, length) {
	const arrayOverflowAmount = arr.length - length;

	if (arrayOverflowAmount > 0) {
		arr.splice(0, arrayOverflowAmount);
	}

	return arr;
}