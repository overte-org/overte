import "../../" as Overte
import "../"

SettingsPage {
	SliderSetting {
		text: qsTr("Field of View")
		stepSize: 5
		from: 20
		to: 130
		valueToText: () => `${value}°`;

        value: Render.getVerticalFieldOfView()
        onValueChanged: Render.setVerticalFieldOfView(value)
	}

	SliderSetting {
		text: qsTr("Resolution Scale")
		stepSize: 10
		from: 10
		to: 200
		valueToText: () => `${value}%`;

        value: Render.viewportResolutionScale * 100
        onValueChanged: Render.viewportResolutionScale = value / 100
	}

	ComboSetting {
		text: qsTr("Anti-Aliasing")
		textRole: "text"
		valueRole: "mode"

		model: [
			{ text: qsTr("None"), mode: "none" },
			{ text: qsTr("4x MSAA"), mode: "msaa" },
			{ text: qsTr("TAA"), mode: "taa" },
			{ text: qsTr("FXAA"), mode: "fxaa" },
		]

		// TODO
	}

	ComboSetting {
		text: qsTr("LOD Culling")
		textRole: "text"
		valueRole: "mode"

		model: [
			{ text: qsTr("High Detail"), mode: 0 },
			{ text: qsTr("Medium Detail"), mode: 1 },
			{ text: qsTr("Low Detail"), mode: 2 },
		]

        currentIndex: { currentIndex = LODManager.worldDetailQuality }
        onCurrentIndexChanged: LODManager.worldDetailQuality = model[currentIndex].mode
	}

	SwitchSetting {
		text: qsTr("Custom Shaders")

        value: Render.proceduralMaterialsEnabled
        onValueChanged: Render.proceduralMaterialsEnabled = value
	}

	Header { text: qsTr("Advanced") }

	SettingNote {
		text: qsTr("These settings are incompatible with MSAA and may reduce performance.")
	}

	SwitchSetting {
		id: advRenderingEnabled
		text: qsTr("Rendering Effects")

        value: Render.renderMethod === 0
        onValueChanged: Render.renderMethod = value ? 0 : 1
	}

	SwitchSetting {
		enabled: advRenderingEnabled.value
		text: qsTr("Shadows")

		value: Render.shadowsEnabled
        onValueChanged: Render.shadowsEnabled = value
	}

	SwitchSetting {
		enabled: advRenderingEnabled.value
		text: qsTr("Bloom")

		value: Render.bloomEnabled
        onValueChanged: Render.bloomEnabled = value
	}

	SwitchSetting {
		enabled: advRenderingEnabled.value
		text: qsTr("Ambient Occlusion")

		value: Render.ambientOcclusionEnabled
        onValueChanged: Render.ambientOcclusionEnabled = value
	}
}

