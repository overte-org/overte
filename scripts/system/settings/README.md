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

#### Examples

SettingBoolean.qml:
```qml
SettingBoolean {
	// Provide the label for the setting (String).
	settingText: "Rendering effects";

	// Pass a function that is executed and the resulting value is set to the internal variable "settingEnabled"
	// This function is executed when the component is loaded, setting the initial state of the boolean.
	// When setting a graphics preset, all SettingBoolean elements have their "update()" function executed.
	// The "update()" function executes the function provided here as the "settingEnabledCondition".
	settingEnabledCondition: function () { return Render.renderMethod === 0; }

	// When the value is changed, execute...
	onSettingEnabledChanged: {
		// Adjust the application setting to the current value of this boolean.
		Render.renderMethod = settingEnabled ? 0 : 1;
	}
}
```


SettingComboBox.qml:
```qml 
SettingComboBox {
	// Provide the label for the setting (String).
	settingText: "Refresh rate";

	// Options for the setting (Array of strings)
	options: ["Economical", "Interactive", "Real-Time", "Custom"];

	// Set the index of the combobox based on the current setting (int).
	optionIndex: Performance.getRefreshRateProfile();

	// When the value is changed, execute...
	onValueChanged: {						
		// Adjust the application setting to the current value of the combobox.
		// Note: the "index" variable provides the index of the provided options which is selected.
		Performance.setRefreshRateProfile(index);

		// If the index is 3 (Custom), show advanced settings, otherwise hide advanced settings.
		customFPSVaulesContainer.visible = index == 3;
	}
}
```

SettingNumber.qml:
```qml
SettingNumber {
	// Provide the label for the setting (String).
	settingText: "Focus Active";

	// Set the minimum value allowed for this input (real).
	minValue: 5;

	// Set the maximum value allowed (real).
	maxValue: 9999;

	// Extra text to add at the far right of the element.
	suffixText: "fps";

	// Set the initial value of the number based on the current setting (var).
	settingValue: Performance.getCustomRefreshRate(0);

	// When the value is changed, execute...
	onValueChanged: {
		// Adjust the application setting to the current value of this number.
		// Note: the "value" variable provides the current value of this element.
		Performance.setCustomRefreshRate(0, value);
	}
}
```

SettingSlider.qml:
```qml
SettingSlider {
	// Provide the label for the setting (String).
	settingText: "Resolution scale";

	// Set the step size for the slider (real).
	sliderStepSize: 0.1;

	// Set the minimum value allowed by the slider (real).
	minValue: 0.1;

	// Set the maximum value allowed by the slider (real).
	maxValue: 2;

	// Set the initial value based on the current setting (var).
	settingValue: Render.viewportResolutionScale.toFixed(1)

	// When the value is changed, execute...
	onSliderValueChanged: {
		// Adjust the application setting to the current value of this slider.
		// Note: the "value" variable provides the current value of this element.
		Render.viewportResolutionScale = value.toFixed(1)
	}
}
```