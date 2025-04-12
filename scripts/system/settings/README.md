# Settings
This application allows users to adjust multiple aspects of the Overte application.

## Developing

### QML Widgets
This application provides several QML widgets to make adding additional settings easy.
The important settings related widgets are as follows:

| Name                | Description                                                            |
|---------------------|------------------------------------------------------------------------|
| SettingBoolean.qml  | An enable/disable toggle for a provided setting                        |
| SettingComboBox.qml | A drop down element which provides a list of options                   |
| SettingNumber.qml   | An input which only allows numerical data entry                        |
| SettingSlider.qml   | An input which only allows numerical data entry, presented as a slider |
| SettingSlider.qml   | An input which only allows numerical data entry, presented as a slider |

#### Examples

SettingComboBox.qml:
```qml 
SettingComboBox {
	// Provide the label for the setting (String)
	settingText: "Refresh rate";

	// Options for the setting (Array of strings)
	options: ["Economical", "Interactive", "Real-Time", "Custom"];

	// Set the index of the combobox based on the current setting (int)
	optionIndex: Performance.getRefreshRateProfile();

	// When the value is changed, execute...
	onValueChanged: {						
		// Adjust the application setting to the current value of the combobox
		Performance.setRefreshRateProfile(index);

		// If the index is 3 (Custom), show advanced settings, otherwise hide advanced settings.
		customFPSVaulesContainer.visible = index == 3;
	}
}
```
