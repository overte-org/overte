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
        enabled: advRenderingEnabled.value

        text: qsTr("Anti-Aliasing")

        model: [
            // TODO: separate none and MSAA?
            // RenderMainView.PreparePrimaryBufferForward.numSamples is a massive hack
            advRenderingEnabled.value ? qsTr("None") : qsTr("4x MSAA"),
            qsTr("TAA"),
            qsTr("FXAA"),
        ]

        currentIndex: advRenderingEnabled.value ? Render.getAntialiasingMode() : 0
        onCurrentIndexChanged: Render.setAntialiasingMode(currentIndex)
    }

    ComboSetting {
        text: qsTr("Level-of-Detail")
        textRole: "text"
        valueRole: "mode"

        model: [
            { text: qsTr("High"), mode: 0 },
            { text: qsTr("Medium"), mode: 1 },
            { text: qsTr("Low"), mode: 2 },
        ]

        currentIndex: LODManager.worldDetailQuality
        onCurrentIndexChanged: LODManager.worldDetailQuality = model[currentIndex].mode
    }

    SwitchSetting {
        text: qsTr("Custom Shaders")

        value: Render.proceduralMaterialsEnabled
        onValueChanged: Render.proceduralMaterialsEnabled = value
    }

    SwitchSetting {
        text: qsTr("Allow third-person camera to pass through walls")

        value: !Render.getCameraClippingEnabled()
        onValueChanged: !Render.setCameraClippingEnabled(value)
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

    Header { text: qsTr("Desktop Window") }

    ComboSetting {
        text: qsTr("Fullscreen Monitor")
        model: Render.getScreens()

        currentIndex: {
            const index = model.indexOf(Render.getFullScreenScreen());
            return index !== -1 ? index : 0;
        }
        onCurrentIndexChanged: Render.setFullScreenScreen(model[currentIndex])
    }

    ComboSetting {
        id: fpsLimit
        text: qsTr("Framerate Limit")
        model: [
            // see Performance.RefreshRateProfile
            qsTr("20 FPS"),
            qsTr("30 FPS"),
            qsTr("60 FPS"),
            qsTr("Custom"),
        ]

        currentIndex: Performance.getRefreshRateProfile()
        onCurrentIndexChanged: Performance.setRefreshRateProfile(currentIndex)
    }

    SettingNote {
        text: qsTr("Higher settings may increase battery usage. VR is always run at your headset's native framerate when possible.")
    }

    Header {
        text: qsTr("Custom Framerate Limit")
        visible: fpsLimit.currentIndex === 3
    }

    SpinBoxSetting {
        visible: fpsLimit.currentIndex === 3
        text: qsTr("Focused Active")
        from: 5
        to: 500
        value: Performance.getCustomRefreshRate(0 /* FOCUS_ACTIVE */)
        onValueChanged: Performance.setCustomRefreshRate(0 /* FOCUS_ACTIVE */, value)
    }

    SpinBoxSetting {
        visible: fpsLimit.currentIndex === 3
        text: qsTr("Focused AFK")
        from: 1
        to: 500
        value: Performance.getCustomRefreshRate(1 /* FOCUS_INACTIVE */)
        onValueChanged: Performance.setCustomRefreshRate(1 /* FOCUS_INACTIVE */, value)
    }

    SpinBoxSetting {
        visible: fpsLimit.currentIndex === 3
        text: qsTr("Unfocused")
        from: 1
        to: 500
        value: Performance.getCustomRefreshRate(2 /* UNFOCUS */)
        onValueChanged: Performance.setCustomRefreshRate(2 /* FOCUS */, value)
    }

    SpinBoxSetting {
        visible: fpsLimit.currentIndex === 3
        text: qsTr("Minimized")
        from: 1
        to: 500
        value: Performance.getCustomRefreshRate(3 /* MINIMIZED */)
        onValueChanged: Performance.setCustomRefreshRate(3 /* MINIMIZED */, value)
    }

    // FIXME: Does anybody actually care about these? Are they useful?
    SpinBoxSetting {
        visible: fpsLimit.currentIndex === 3
        text: qsTr("Startup")
        from: 5
        to: 500
        value: Performance.getCustomRefreshRate(4 /* STARTUP */)
        onValueChanged: Performance.setCustomRefreshRate(4 /* STARTUP */, value)
    }

    SpinBoxSetting {
        visible: fpsLimit.currentIndex === 3
        text: qsTr("Shutdown")
        from: 5
        to: 500
        value: Performance.getCustomRefreshRate(5 /* SHUTDOWN */)
        onValueChanged: Performance.setCustomRefreshRate(5 /* SHUTDOWN */, value)
    }
}

