import "../../" as Overte
import "../"

SettingsPage {
    SwitchSetting {
        text: "Mute Microphone"

        value: AudioScriptingInterface.muted
        onValueChanged: AudioScriptingInterface.muted = value
    }

    SwitchSetting {
        text: "Push-to-Talk"

        value: AudioScriptingInterface.pushToTalk
        onValueChanged: AudioScriptingInterface.pushToTalk = value
    }

    SettingNote {
        text: "Push [T] or squeeze both controller grips to talk."
    }

    Header { text: qsTr("Audio Devices") }

    WideComboSetting {
        text: "Output Device"
        model: AudioScriptingInterface.devices.output

        // TODO: how do these work???
        //currentIndex: 0
        //onCurrentIndexChanged: AudioScriptingInterface.setOutputDevice(currentIndex, false)
    }

    WideComboSetting {
        text: "Input Device"
        model: AudioScriptingInterface.devices.input

        // TODO: how do these work???
        //currentIndex: 0
        //onCurrentIndexChanged: AudioScriptingInterface.setInputDevice(currentIndex, false)
    }

    SettingNote {
        text: qsTr("Most VR runtimes will automatically switch the default audio devices to your headset.")
    }
}
