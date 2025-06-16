const playSound = {
	system: () => {
		playSound._playSound(Script.resolvePath("../sound/systemNotification.mp3"));
	},
	connection: () => {
		playSound._playSound(Script.resolvePath("../sound/connectionNotification.mp3"));
	},
	_playSound: (url) => {
		const sound = SoundCache.getSound(url);
		Audio.playSystemSound(sound, { volume: 0.5 });
	}
}