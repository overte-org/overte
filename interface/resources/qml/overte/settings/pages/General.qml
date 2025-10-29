import "../../" as Overte
import "../"

SettingsPage {
    id: page

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
        text: qsTr("Folder")
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

        currentIndex: {
            let current = Snapshot.getSnapshotFormat();
            for (let i in model) {
                if (model[i] === current) { return i; }
            }
            return 0;
        }
        onCurrentIndexChanged: Snapshot.setSnapshotFormat(model[currentIndex])
    }

    SpinBoxSetting {
        // FIXME
        enabled: false

        text: qsTr("Animation Duration")
        from: 1
        to: 30

        //value: Settings.getValue("snapshotAnimatedDuration", 3)
        //onValueChanged: Settings.setValue("snapshotAnimatedDuration", value)
    }

    SettingNote {
        text: "Animated screenshots are saved as GIFs."
    }

    Header { text: qsTr("Privacy") }

    SwitchSetting {
        text: qsTr("Send Crash Reports")

        // FIXME: MenuScriptingInterface is super cursed, we shouldn't be relying on
        // the *names* of the menu items to access them, especially once they're translated.
        // Why are these even menu items in the first place?
        value: MenuInterface.isOptionChecked("Enable Crash Reporting")
        onValueChanged: MenuInterface.setIsOptionChecked("Enable Crash Reporting", value)
    }

    SettingNote {
        text: qsTr("Sending crash reports helps Overte development.")
    }

    SwitchSetting {
        // FIXME
        enabled: false

        text: qsTr("Discord Rich Presence")
        //value: Settings.getValue("useDiscordPresence", true)
        //onValueChanged: Settings.setValue("useDiscordPresence", value)
    }

    SettingNote {
        text: qsTr("If this setting is enabled, your current world will be shown on your Discord profile.")
    }
}
