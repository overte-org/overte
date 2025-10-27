import "../../" as Overte
import "../"

SettingsPage {
    Header {
        text: qsTr("UI")

        // Hack to reduce the dead space on the top of the page
        height: implicitHeight
    }

    ComboSetting {
        text: qsTr("Color Scheme")
        model: [
            qsTr("Dark"),
            qsTr("Light"),
            qsTr("System"),
        ]

        onCurrentIndexChanged: {
            switch (currentIndex) {
                case 0:
                    Overte.useSystemColorScheme = false;
                    Overte.Theme.darkMode = true;
                    break;

                case 1:
                    Overte.useSystemColorScheme = false;
                    Overte.Theme.darkMode = false;
                    break;

                case 2:
                    Overte.useSystemColorScheme = true;
                    Overte.Theme.darkMode = true;
                    break;
            }
        }
    }

    ComboSetting {
        text: qsTr("Contrast")
        model: [
            qsTr("Standard"),
            qsTr("High"),
            qsTr("System"),
        ]

        onCurrentIndexChanged: {
            switch (currentIndex) {
                case 0:
                    Overte.useSystemContrastMode = false;
                    Overte.Theme.highContrast = false;
                    break;

                case 1:
                    Overte.useSystemContrastMode = false;
                    Overte.Theme.highContrast = true;
                    break;

                case 2:
                    Overte.useSystemContrastMode = true;
                    Overte.Theme.highContrast = false;
                    break;
            }
        }
    }

    SwitchSetting {
        text: qsTr("Reduced Motion")
        value: Overte.Theme.reducedMotion
        onValueChanged: () => Overte.Theme.reducedMotion = value
    }

    SettingNote {
        text: qsTr("This setting will disable UI animations that slide or scale.")
    }

    Header { text: qsTr("Screenshots") }

    FolderSetting {
        // TODO
        enabled: false

        text: qsTr("Folder")
        // TODO
        value: Snapshot.getSnapshotsLocation()
        onValueChanged: Snapshot.setSnapshotsLocation(value)
    }

    ComboSetting {
        text: qsTr("Format")
        model: [
            "png",
            "jpg",
            "webp",
        ]
        onCurrentIndexChanged: Snapshot.setSnapshotFormat(model[currentIndex])
    }

    SpinBoxSetting {
        // FIXME: setting isn't exposed to script api
        enabled: false

        text: qsTr("Animation Duration")
        from: 1
        to: 30

        // TODO
        value: 3
    }

    SettingNote {
        text: "Animated screenshots are saved as GIFs."
    }

    Header { text: qsTr("Privacy") }

    SwitchSetting {
        // FIXME: setting isn't exposed to script api
        enabled: false

        text: qsTr("Send Crash Reports")
        // TODO
        value: false
        onValueChanged: () => {}
    }

    SettingNote {
        text: qsTr("Sending crash reports helps Overte development.")
    }

    SwitchSetting {
        // FIXME: setting isn't exposed to script api
        enabled: false

        text: qsTr("Discord Rich Presence")
        // TODO
        value: false
        onValueChanged: () => {}
    }

    SettingNote {
        text: qsTr("If this setting is enabled, your current world will be shown on your Discord profile.")
    }
}
