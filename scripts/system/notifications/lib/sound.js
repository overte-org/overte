const SOUND_EFFECTS = {
	SYSTEM: SoundCache.getSound(Script.resolvePath("../sound/systemNotification.mp3")),
	CONNECTION: SoundCache.getSound(Script.resolvePath("../sound/connectionNotification.mp3"))
}

const playSound = {
	system: () => {
		Audio.playSystemSound(SOUND_EFFECTS.SYSTEM, { volume: 0.5 })
	},
	connection: () => {
		Audio.playSystemSound(SOUND_EFFECTS.CONNECTION, { volume: 0.5 })
	}
}