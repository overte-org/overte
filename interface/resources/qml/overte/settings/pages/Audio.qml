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
        model: {
            let tmp = [];
            const source = AudioScriptingInterface.devices.output;

            for (let i = 0; i < source.rowCount(); i++) {
                tmp.push(source.data(source.index(i, 0), /* DeviceNameRole */ 0x100));
            }

            return tmp;
        }

        // TODO: how do these work???
        //currentIndex: 0
        //onCurrentIndexChanged: AudioScriptingInterface.setOutputDevice(currentIndex, false)
    }

    WideComboSetting {
        text: "Input Device"
        model: {
            let tmp = [];
            const source = AudioScriptingInterface.devices.input;

            for (let i = 0; i < source.rowCount(); i++) {
                tmp.push(source.data(source.index(i, 0), /* DeviceNameRole */ 0x100));
            }

            return tmp;
        }

        // TODO: how do these work???
        //currentIndex: 0
        //onCurrentIndexChanged: AudioScriptingInterface.setInputDevice(currentIndex, false)
    }

    SettingNote {
        text: qsTr("Most VR runtimes will automatically switch the default audio devices to your headset.")
    }
}
