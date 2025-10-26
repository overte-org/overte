import "../../" as Overte
import "../"

SettingsPage {
	Header {
		text: qsTr("Desktop")

		// Hack to reduce the dead space on the top of the page
		height: implicitHeight
	}

	SwitchSetting {
        // FIXME: setting isn't exposed to script api
        enabled: false

		text: qsTr("Invert Y")

		// TODO
		value: false
	}

	SliderSetting {
        // FIXME: setting isn't exposed to script api
        enabled: false

		text: qsTr("Mouse Sensitivity")
		stepSize: 0.1
		from: 0.1
		to: 5.0
		valueToText: () => `${value.toFixed(1)}`;

		// TODO
		value: 1.0
	}

	Header { text: qsTr("VR User") }

	SliderSetting {
		text: qsTr("Height")
		fineTweakButtons: true
		stepSize: 0.01
		from: 0.8
		to: 2.5

		valueToText: () => {
			let meters = value.toFixed(2);
			let totalInches = Math.round(value * 39.37008);

			let feet = Math.floor(totalInches / 12);
			let inches = totalInches % 12;

			return `${feet}'${inches}"    ${meters.toLocaleString()}m`;
		}

        // FIXME: QML complains about userHeight not being bindable
		value: { value = MyAvatar.userHeight }
        onValueChanged: MyAvatar.userHeight = value
	}

	ComboSetting {
		text: qsTr("Dominant Hand")
		model: [
			qsTr("Left"),
			qsTr("Right"),
		]

        currentIndex: MyAvatar.getDominantHand() === "left" ? 0 : 1
        onCurrentIndexChanged: MyAvatar.setDominantHand(currentIndex === 0 ? "left" : "right")
	}

	Header { text: qsTr("VR Movement") }

	SliderSetting {
        // FIXME: setting isn't exposed to script api
        enabled: false

		text: qsTr("Turning Speed")
		stepSize: 10
		from: 0
		to: 400
		valueToText: () => value < 50 ? qsTr("Snap turning") : `${value.toFixed(1)}`;

        // FIXME: not exposed to scripts or QML
		/*value: MyAvatar.HMDYawSpeed
        onValueChanged: {
            MyAvatar.setSnapTurn(value < 50);
            MyAvatar.HMDYawSpeed = value;
        }*/
	}

	SliderSetting {
		id: walkingSpeed
		text: qsTr("Walking Speed")
		stepSize: 0.5
		from: 1.0
		to: 9
		valueToText: () => value < 1.5 ? qsTr("Teleport Only") : `${value.toFixed(1)} m/s`;
		//enabled: !useAvatarDefaultWalkingSpeed.value

		value: MyAvatar.vrWalkSpeed
        onValueChanged: {
            if (value === 0.0) {
                MyAvatar.useAdvancedMovementControls = false;
            } else {
                MyAvatar.useAdvancedMovementControls = true;
                MyAvatar.vrWalkSpeed = value;
            }
        }
	}

	ComboSetting {
		text: qsTr("Movement Relative To")
		enabled: walkingSpeed.value >= 1.5
		model: [
			qsTr("Head"),
			qsTr("Hand"),
		]

		// TODO
	}

	/*SwitchSetting {
		id: useAvatarDefaultWalkingSpeed
		text: qsTr("Use equipped avatar's default walking speed if available")

		// TODO
		value: false
    }*/

	Header { text: qsTr("VR UI") }

	ComboSetting {
        // FIXME: setting isn't exposed to script api
        enabled: false

		text: qsTr("Tablet Input")
		model: [
			qsTr("Laser"),
			qsTr("Stylus"),
			qsTr("Finger Touch"),
		]

		// TODO
	}

	ComboSetting {
        // FIXME: setting isn't exposed to script api
        enabled: false

		text: qsTr("Virtual Keyboard Input")
		model: [
			qsTr("Lasers"),
			qsTr("Mallets"),
		]

		// TODO
	}

	SliderSetting {
        // FIXME: setting isn't exposed to script api
        enabled: false

		text: qsTr("Laser Smoothing Delay")
		stepSize: 0.1
		from: 0.0
		to: 2.0
		valueToText: () => `${value.toFixed(1)}s`;

		// TODO
		value: 0.3
	}

	// for later, once the gesture scripts are stable and merged
	/*
	Header { text: qsTr("VR Gestures") }

	SwitchSetting {
		text: qsTr("Take Photo")
		value: true
	}

	SettingNote {
		text: qsTr("Double-click the trigger on your dominant hand near your ear to take a photo, or hold the trigger to take an animated screenshot.")
	}

	SwitchSetting {
		text: qsTr("Laser Toggle")
		value: true
	}

	SettingNote {
		text: qsTr("Click both triggers with your hands behind your head to toggle the interaction lasers.")
	}

	SwitchSetting {
		text: qsTr("Seated Mode Toggle")
		value: true
	}

	SettingNote {
		text: qsTr("Double-tap the controller grip on your non-dominant hand near your ear to switch between seated and standing mode.")
	}
	*/
}
