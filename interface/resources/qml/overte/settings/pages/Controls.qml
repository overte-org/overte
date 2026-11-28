import "../../" as Overte
import "../"

// NOTE: There's a lot of "x depends on non-bindable properties" warnings,
// there's not much that can be done about them and I don't think there's
// a way of explicitly telling QML to not try binding to stuff.
SettingsPage {
    Header {
        text: qsTr("Desktop")

        // Hack to reduce the dead space on the top of the page
        height: implicitHeight
    }

    SwitchSetting {
        id: invertMouseY
        text: qsTr("Invert Y")

        value: MyAvatar.pitchSpeed < 0
        onValueChanged: {
            MyAvatar.pitchSpeed = mouseSensitivity.value * (value ? -75 : 75);
        }
    }

    SliderSetting {
        id: mouseSensitivity
        text: qsTr("Mouse Sensitivity")
        stepSize: 0.1
        from: 0.1
        to: 5.0
        valueToText: () => `${value.toLocaleString()}`;

        value: MyAvatar.yawSpeed / 75
        onValueChanged: {
            MyAvatar.yawSpeed = value * 75;
            MyAvatar.pitchSpeed = value * (invertMouseY.value ? -75 : 75);
        }
    }

    Header { text: qsTr("VR User") }

    SliderSetting {
        text: qsTr("Height")
        fineTweakButtons: true
        stepSize: 0.01
        from: 0.8
        to: 2.5

        valueToText: () => {
            let meters = value.toLocaleString();
            let totalInches = Math.round(value * 39.37008);

            let feet = Math.floor(totalInches / 12);
            let inches = totalInches % 12;

            return `${feet}'${inches}"    ${meters.toLocaleString()}m`;
        }

        value: MyAvatar.userHeight
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

    SwitchSetting {
        text: qsTr("Seated Mode")
        value: MyAvatar.standingMode !== 0
        onValueChanged: {
            MyAvatar.standingMode = (
                value ?
                2 : // ForcedHeight
                0   // Standing
            );
        }
    }

    Header { text: qsTr("VR Movement") }

    SliderSetting {
        text: qsTr("Turning Speed")
        stepSize: 10
        from: 40
        to: 400
        valueToText: () => value < 50 ? qsTr("Snap turning") : `${value.toLocaleString()}`;

        value: MyAvatar.hmdYawSpeed
        onValueChanged: {
            MyAvatar.setSnapTurn(value < 50);
            MyAvatar.HMDYawSpeed = value;
        }
    }

    SliderSetting {
        id: walkingSpeed
        text: qsTr("Walking Speed")
        stepSize: 0.5
        from: 1.0
        to: 9
        valueToText: () => value < 1.5 ? qsTr("Teleport Only") : `${value.toLocaleString()} m/s`;
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

        currentIndex: MyAvatar.getMovementReference()
        onCurrentIndexChanged: MyAvatar.setMovementReference(currentIndex)
    }

    /*SwitchSetting {
        id: useAvatarDefaultWalkingSpeed
        text: qsTr("Use equipped avatar's default walking speed if available")

        // TODO
        value: false
    }*/

    Header { text: qsTr("VR UI") }

    ComboSetting {
        text: qsTr("Virtual Keyboard Input")
        model: [
            qsTr("Lasers"),
            qsTr("Mallets"),
        ]

        currentIndex: KeyboardScriptingInterface.preferMalletsOverLasers ? 1 : 0
        onCurrentIndexChanged: {
            KeyboardScriptingInterface.preferMalletsOverLasers = currentIndex == 1;
        }
    }

    SliderSetting {
        text: qsTr("Laser Smoothing Delay")
        stepSize: 0.05
        from: 0.0
        to: 2.0
        valueToText: () => `${value.toLocaleString()}s`;

        value: PickScriptingInterface.handLaserDelay
        onValueChanged: PickScriptingInterface.handLaserDelay = value
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
