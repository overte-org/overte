//
//  GraphicsSettings.qml
//  qml\hifi\dialogs\graphics
//
//  Created by Zach Fox on 2019-07-10
//  Copyright 2019 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

import Hifi 1.0 as Hifi
import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.12
import stylesUit 1.0 as HifiStylesUit
import controlsUit 1.0 as HifiControlsUit
import "qrc:////qml//controls" as HifiControls
import PerformanceEnums 1.0

Flickable {
    HifiStylesUit.HifiConstants { id: hifi; }

    contentHeight: graphicsSettingsColumnLayout.height;

    ScrollBar.vertical : ScrollBar {
        policy: ScrollBar.AlwaysOn
        visible: true
        width: 20
        background: Rectangle {
            color: hifi.colors.tableScrollBackgroundDark
        }
    }

    id: root;
    anchors.fill: parent

    ColumnLayout {
        id: graphicsSettingsColumnLayout
        anchors.left: parent.left
        anchors.leftMargin: 26
        anchors.right: parent.right
        anchors.rightMargin: 26
        anchors.top: parent.top
        anchors.topMargin: HMD.active ? 80 : 0
        spacing: 8

        ColumnLayout {
            Layout.preferredWidth: parent.width
            Layout.topMargin: 18
            spacing: 0

            HifiStylesUit.RalewayRegular {
                text: "GRAPHICS SETTINGS"
                Layout.maximumWidth: parent.width
                height: 30
                size: 16
                color: "#FFFFFF"
            }

            ColumnLayout {
                Layout.topMargin: 10
                Layout.preferredWidth: parent.width
                spacing: 0

                HifiControlsUit.RadioButton {
                    id: performanceLowPower
                    colorScheme: hifi.colorSchemes.dark
                    height: 18
                    fontSize: 16
                    leftPadding: 0
                    text: "Low Power"
                    checked: Performance.getPerformancePreset() === PerformanceEnums.LOW_POWER
                    onClicked: {
                        Performance.setPerformancePreset(PerformanceEnums.LOW_POWER);
                        root.refreshAllDropdowns();
                    }
                }

                HifiControlsUit.RadioButton {
                    id: performanceLow
                    colorScheme: hifi.colorSchemes.dark
                    height: 18
                    fontSize: 16
                    leftPadding: 0
                    text: "Low"
                    checked: Performance.getPerformancePreset() === PerformanceEnums.LOW
                    onClicked: {
                        Performance.setPerformancePreset(PerformanceEnums.LOW);
                        root.refreshAllDropdowns();
                    }
                }

                HifiControlsUit.RadioButton {
                    id: performanceMedium
                    colorScheme: hifi.colorSchemes.dark
                    height: 18
                    fontSize: 16
                    leftPadding: 0
                    text: "Medium"
                    checked: Performance.getPerformancePreset() === PerformanceEnums.MID
                    onClicked: {
                        Performance.setPerformancePreset(PerformanceEnums.MID);
                        root.refreshAllDropdowns();
                    }
                }

                HifiControlsUit.RadioButton {
                    id: performanceHigh
                    colorScheme: hifi.colorSchemes.dark
                    height: 18
                    fontSize: 16
                    leftPadding: 0
                    text: "High"
                    checked: Performance.getPerformancePreset() === PerformanceEnums.HIGH
                    onClicked: {
                        Performance.setPerformancePreset(PerformanceEnums.HIGH);
                        root.refreshAllDropdowns();
                    }
                }

                HifiControlsUit.RadioButton {
                    id: performanceCustom
                    colorScheme: hifi.colorSchemes.dark
                    height: 18
                    fontSize: 16
                    leftPadding: 0
                    text: "Custom"
                    checked: Performance.getPerformancePreset() === PerformanceEnums.CUSTOM
                    onClicked: {
                        Performance.setPerformancePreset(PerformanceEnums.CUSTOM);
                    }
                }
            }

            ColumnLayout {
                Layout.topMargin: 10
                Layout.preferredWidth: parent.width
                spacing: 0

                Item {
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 35

                    HifiStylesUit.RalewayRegular {
                        id: worldDetailHeader
                        text: "Target frame rate"
                        anchors.left: parent.left
                        anchors.top: parent.top
                        width: 130
                        height: parent.height
                        size: 16
                        color: "#FFFFFF"
                    }

                    ListModel {
                        id: worldDetailModel

                        ListElement {
                            text: "High Frame Rate/Low Detail"
                        }
                        ListElement {
                            text: "Medium Frame Rate/Medium Detail"
                        }
                        ListElement {
                            text: "Low Frame Rate/High Detail"
                        }
                    }

                    HifiControlsUit.ComboBox {
                        id: worldDetailDropdown
                        enabled: performanceCustom.checked
                        anchors.left: worldDetailHeader.right
                        anchors.leftMargin: 20
                        anchors.top: parent.top
                        width: 280
                        height: parent.height
                        colorScheme: hifi.colorSchemes.dark
                        model: worldDetailModel
                        currentIndex: -1

                        function refreshWorldDetailDropdown() {
                            worldDetailDropdown.currentIndex = LODManager.worldDetailQuality;
                        }

                        Component.onCompleted: {
                            worldDetailDropdown.refreshWorldDetailDropdown();
                        }

                        onCurrentIndexChanged: {
                            LODManager.worldDetailQuality = currentIndex;
                            worldDetailDropdown.displayText = model.get(currentIndex).text;
                        }
                    }
                }

                ColumnLayout {
                    Layout.preferredWidth: parent.width
                    Layout.topMargin: 20

                    HifiStylesUit.RalewayRegular {
                        id: renderingEffectsHeader
                        text: "Rendering Effects"
                        anchors.left: parent.left
                        anchors.top: parent.top
                        width: 130
                        height: parent.height
                        size: 16
                        color: "#FFFFFF"
                    }

                    ColumnLayout {
                        anchors.left: renderingEffectsHeader.right
                        anchors.leftMargin: 20
                        Layout.preferredWidth: parent.width
                        spacing: 0
                        enabled: performanceCustom.checked

                        HifiControlsUit.RadioButton {
                            id: renderingEffectsDisabled
                            colorScheme: hifi.colorSchemes.dark
                            height: 18
                            fontSize: 16
                            leftPadding: 0
                            text: "Disabled"
                            checked: Render.renderMethod === 1
                            onClicked: {
                                Render.renderMethod = 1; // "FORWARD"
                                //refreshRenderingEffectCheckboxes();
                            }
                        }

                        HifiControlsUit.RadioButton {
                            id: renderingEffectsEnabled
                            enabled: PlatformInfo.isRenderMethodDeferredCapable()
                            colorScheme: hifi.colorSchemes.dark
                            height: 18
                            fontSize: 16
                            leftPadding: 0
                            text: "Enabled"
                            checked: Render.renderMethod === 0
                            onClicked: {
                                Render.renderMethod = 0; // "DEFERRED"
                            }
                        }

                        ColumnLayout {
                            id: renderingEffectCheckboxes
                            Layout.preferredWidth: parent.width
                            anchors.left: parent.left
                            anchors.leftMargin: 24
                            anchors.topMargin: 8
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: Layout.topMargin
                            enabled: performanceCustom.checked && renderingEffectsEnabled.checked

                            HifiControlsUit.CheckBox {
                                id: renderingEffectShadows
                                checked: Render.shadowsEnabled
                                boxSize: 16
                                text: "Shadows"
                                spacing: -1
                                colorScheme: hifi.colorSchemes.dark
                                anchors.left: parent.left
                                anchors.top: parent.top
                                onCheckedChanged: {
                                    Render.shadowsEnabled = renderingEffectShadows.checked;
                                }
                            }
                            HifiControlsUit.CheckBox {
                                id: renderingEffectHaze
                                checked: Render.hazeEnabled
                                boxSize: 16
                                text: "Haze"
                                spacing: -1
                                colorScheme: hifi.colorSchemes.dark
                                anchors.left: parent.left
                                anchors.top: renderingEffectShadows.bottom
                                onCheckedChanged: {
                                    Render.hazeEnabled = renderingEffectHaze.checked;
                                }
                            }
                            HifiControlsUit.CheckBox {
                                id: renderingEffectBloom
                                checked: Render.bloomEnabled
                                boxSize: 16
                                text: "Bloom"
                                spacing: -1
                                colorScheme: hifi.colorSchemes.dark
                                anchors.left: parent.left
                                anchors.top: renderingEffectHaze.bottom
                                onCheckedChanged: {
                                    Render.bloomEnabled = renderingEffectBloom.checked;
                                }
                            }
                            HifiControlsUit.CheckBox {
                                id: renderingEffectAO
                                checked: Render.ambientOcclusionEnabled
                                boxSize: 16
                                text: "AO"
                                spacing: -1
                                colorScheme: hifi.colorSchemes.dark
                                anchors.left: parent.left
                                anchors.top: renderingEffectBloom.bottom
                                onCheckedChanged: {
                                    Render.ambientOcclusionEnabled = renderingEffectAO.checked;
                                }
                            }
                            HifiControlsUit.CheckBox {
                                id: renderingEffectLocalLights
                                enabled: false
                                //checked: Render.localLightsEnabled
                                checked: renderingEffectsEnabled.checked
                                boxSize: 16
                                text: "Local lights"
                                spacing: -1
                                colorScheme: hifi.colorSchemes.dark
                                anchors.left: parent.left
                                anchors.top: renderingEffectAO.bottom
                                //onCheckedChanged: {
                                //    Render.localLightsEnabled = renderingEffectLocalLightsEnabled.checked;
                                //}
                            }
                        }
                    }
                }

                Item {
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 35
                    Layout.topMargin: 10

                    HifiStylesUit.RalewayRegular {
                        id: refreshRateHeader
                        text: "Refresh Rate"
                        anchors.left: parent.left
                        anchors.top: parent.top
                        width: 130
                        height: parent.height
                        size: 16
                        color: "#FFFFFF"
                    }

                    ListModel {
                        id: refreshRateModel

                        ListElement {
                            text: "Economical"
                            refreshRatePreset: 0 // RefreshRateProfile::ECO
                        }
                        ListElement {
                            text: "Interactive"
                            refreshRatePreset: 1 // RefreshRateProfile::INTERACTIVE
                        }
                        ListElement {
                            text: "Real-Time"
                            refreshRatePreset: 2 // RefreshRateProfile::REALTIME
                        }
                        ListElement {
                            text: "Custom"
                            refreshRatePreset: 3 // RefreshRateProfile::CUSTOM
                        }
                    }

                    HifiControlsUit.ComboBox {
                        id: refreshRateDropdown
                        enabled: performanceCustom.checked
                        anchors.left: refreshRateHeader.right
                        anchors.leftMargin: 20
                        anchors.top: parent.top
                        width: 280
                        height: parent.height
                        colorScheme: hifi.colorSchemes.dark
                        model: refreshRateModel
                        currentIndex: -1

                        function refreshRefreshRateDropdownDisplay() {
                            refreshRateDropdown.currentIndex = Performance.getRefreshRateProfile();
                        }

                        Component.onCompleted: {
                            refreshRateDropdown.refreshRefreshRateDropdownDisplay();
                        }

                        onCurrentIndexChanged: {
                            Performance.setRefreshRateProfile(model.get(currentIndex).refreshRatePreset);
                            refreshRateDropdown.displayText = model.get(currentIndex).text;
                        }
                    }
                }

                ColumnLayout {
                    width: parent.width
                    Layout.topMargin: 32
                    visible: refreshRateDropdown.currentIndex == 3

                    RowLayout {
                        Layout.margins: 8

                        HifiControlsUit.SpinBox {
                            id: refreshRateCustomFocusActive
                            decimals: 0
                            width: 160
                            height: 32
                            suffix: " FPS"
                            label: "Focus Active"
                            realFrom: 1
                            realTo: 1000
                            realStepSize: 15
                            realValue: 60
                            colorScheme: hifi.colorSchemes.dark
                            property var loaded: false

                            Component.onCompleted: {
                                realValue = Performance.getCustomRefreshRate(0)
                                loaded = true
                            }

                            onRealValueChanged: {
                                if (loaded) {
                                    Performance.setCustomRefreshRate(0, realValue)
                                }
                            }
                        }

                        HifiControlsUit.SpinBox {
                            id: refreshRateCustomFocusInactive
                            decimals: 0
                            width: 160
                            height: 32
                            suffix: " FPS"
                            label: "Focus Inactive"
                            realFrom: 1
                            realTo: 1000
                            realStepSize: 15
                            realValue: 60
                            colorScheme: hifi.colorSchemes.dark
                            property var loaded: false

                            Component.onCompleted: {
                                realValue = Performance.getCustomRefreshRate(1)
                                loaded = true
                            }

                            onRealValueChanged: {
                                if (loaded) {
                                    Performance.setCustomRefreshRate(1, realValue)
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.margins: 8

                        HifiControlsUit.SpinBox {
                            id: refreshRateCustomUnfocus
                            decimals: 0
                            width: 160
                            height: 32
                            suffix: " FPS"
                            label: "Unfocus"
                            realFrom: 1
                            realTo: 1000
                            realStepSize: 15
                            realValue: 60
                            colorScheme: hifi.colorSchemes.dark
                            property var loaded: false

                            Component.onCompleted: {
                                realValue = Performance.getCustomRefreshRate(2)
                                loaded = true
                            }

                            onRealValueChanged: {
                                if (loaded) {
                                    Performance.setCustomRefreshRate(2, realValue);
                                }
                            }
                        }

                        HifiControlsUit.SpinBox {
                            id: refreshRateCustomMinimized
                            decimals: 0
                            width: 160
                            height: 32
                            suffix: " FPS"
                            label: "Minimized"
                            realFrom: 1
                            realTo: 1000
                            realStepSize: 1
                            realValue: 60
                            colorScheme: hifi.colorSchemes.dark
                            property var loaded: false

                            Component.onCompleted: {
                                realValue = Performance.getCustomRefreshRate(3)
                                loaded = true
                            }

                            onRealValueChanged: {
                                if (loaded) {
                                    Performance.setCustomRefreshRate(3, realValue)
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.margins: 8

                        HifiControlsUit.SpinBox {
                            id: refreshRateCustomStartup
                            decimals: 0
                            width: 160
                            height: 32
                            suffix: " FPS"
                            label: "Startup"
                            realFrom: 1
                            realTo: 1000
                            realStepSize: 15
                            realValue: 60
                            colorScheme: hifi.colorSchemes.dark
                            property var loaded: false

                            Component.onCompleted: {
                                realValue = Performance.getCustomRefreshRate(4)
                                loaded = true
                            }

                            onRealValueChanged: {
                                if (loaded) {
                                    Performance.setCustomRefreshRate(4, realValue)
                                }
                            }
                        }

                        HifiControlsUit.SpinBox {
                            id: refreshRateCustomShutdown
                            decimals: 0
                            width: 160
                            height: 32
                            suffix: " FPS"
                            label: "Shutdown"
                            realFrom: 1
                            realTo: 1000
                            realStepSize: 15
                            realValue: 60
                            colorScheme: hifi.colorSchemes.dark
                            property var loaded: false

                            Component.onCompleted: {
                                realValue = Performance.getCustomRefreshRate(5)
                                loaded = true
                            }

                            onRealValueChanged: {
                                if (loaded) {
                                    Performance.setCustomRefreshRate(5, realValue)
                                }
                            }
                        }
                    }
                }

                Item {
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 35
                    Layout.topMargin: 16

                    HifiStylesUit.RalewayRegular {
                        id: resolutionHeader
                        text: "Resolution Scale (" + Number.parseFloat(Render.viewportResolutionScale).toPrecision(3) + ")"
                        anchors.left: parent.left
                        anchors.top: parent.top
                        width: 130
                        height: parent.height
                        size: 16
                        color: "#FFFFFF"
                    }

                    HifiControlsUit.Slider {
                        id: resolutionScaleSlider
                        enabled: performanceCustom.checked
                        anchors.left: resolutionHeader.right
                        anchors.leftMargin: 57
                        anchors.top: parent.top
                        width: 150
                        height: parent.height
                        colorScheme: hifi.colorSchemes.dark
                        minimumValue: 0.25
                        maximumValue: 2.0
                        stepSize: 0.05
                        value: Render.viewportResolutionScale
                        live: true

                        function updateResolutionScale(sliderValue) {
                            if (Render.viewportResolutionScale !== sliderValue) {
                                Render.viewportResolutionScale = sliderValue;
                            }
                        }

                        onValueChanged: {
                            updateResolutionScale(value);
                        }
                        onPressedChanged: {
                            if (!pressed) {
                                updateResolutionScale(value);
                            }
                        }
                    }
                }
                Item {
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 35
                    Layout.topMargin: 16

                    HifiStylesUit.RalewayRegular {
                        id: fieldOfViewHeader
                        text: "Vertical FOV (" + Number(Math.round(Render.verticalFieldOfView)) + ")"
                        anchors.left: parent.left
                        anchors.top: parent.top
                        width: 130
                        height: parent.height
                        size: 16
                        color: "#FFFFFF"
                    }

                    HifiControlsUit.Slider {
                        id: fieldOfViewSlider
                        enabled: true
                        anchors.left: fieldOfViewHeader.right
                        anchors.leftMargin: 57
                        anchors.top: parent.top
                        width: 150
                        height: parent.height
                        colorScheme: hifi.colorSchemes.dark
                        minimumValue: 20
                        maximumValue: 130
                        stepSize: 0.05
                        value: Render.verticalFieldOfView
                        live: true

                        function updateFieldOfView(sliderValue) {
                            if (Render.verticalFieldOfView !== sliderValue) {
                                Render.verticalFieldOfView = sliderValue;
                            }
                        }

                        onValueChanged: {
                            updateFieldOfView(value);
                        }
                        onPressedChanged: {
                            if (!pressed) {
                                updateFieldOfView(value);
                            }
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.topMargin: 20
                Layout.preferredWidth: parent.width
                spacing: 0

                Item {
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 35

                    HifiStylesUit.RalewayRegular {
                        id: antialiasingHeader
                        text: "Anti-aliasing"
                        anchors.left: parent.left
                        anchors.top: parent.top
                        width: 130
                        height: parent.height
                        size: 16
                        color: "#FFFFFF"
                    }

                    ListModel {
                        id: antialiasingModel

                        // Maintain same order as "AntialiasingConfig::Mode".
                        ListElement {
                            text: "None"
                        }
                        ListElement {
                            text: "TAA"
                        }
                        ListElement {
                            text: "FXAA"
                        }
                    }

                    HifiControlsUit.ComboBox {
                        id: antialiasingDropdown
                        anchors.left: antialiasingHeader.right
                        anchors.leftMargin: 20
                        anchors.top: parent.top
                        width: 280
                        height: parent.height
                        colorScheme: hifi.colorSchemes.dark
                        model: antialiasingModel
                        currentIndex: -1

                        function refreshAntialiasingDropdown() {
                            antialiasingDropdown.currentIndex = Render.antialiasingMode;
                        }

                        Component.onCompleted: {
                            antialiasingDropdown.refreshAntialiasingDropdown();
                        }

                        onCurrentIndexChanged: {
                            Render.antialiasingMode = currentIndex;
                            antialiasingDropdown.displayText = model.get(currentIndex).text;
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.topMargin: 20
                Layout.preferredWidth: parent.width
                spacing: 0

                Item {
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 35

                    HifiStylesUit.RalewayRegular {
                        id: fullScreenDisplayHeader
                        text: "Full screen display"
                        anchors.left: parent.left
                        anchors.top: parent.top
                        width: 130
                        height: parent.height
                        size: 16
                        color: "#FFFFFF"
                    }

                    ListModel {
                        id: fullScreenDisplayModel

                        ListElement {
                            text: "Screen 1"
                        }

                        function refreshScreens() {
                            fullScreenDisplayModel.clear();
                            Render.getScreens().forEach(function(screen) {
                                fullScreenDisplayModel.append({"text" : screen});
                            });
                        }

                        Component.onCompleted: {
                            fullScreenDisplayModel.refreshScreens();
                        }
                    }

                    HifiControlsUit.ComboBox {
                        id: fullScreenDisplayDropdown
                        anchors.left: fullScreenDisplayHeader.right
                        anchors.leftMargin: 20
                        anchors.top: parent.top
                        width: 280
                        height: parent.height
                        colorScheme: hifi.colorSchemes.dark
                        model: fullScreenDisplayModel
                        currentIndex: 0

                        function refreshFullScreenDisplayDropdown() {
                            var screens = Render.getScreens();
                            var selected = Render.getFullScreenScreen();

                            for(let idx = 0; idx < screens.length; idx++) {
                                if (screens[idx] == selected) {
                                    fullScreenDisplayDropdown.currentIndex = idx;
                                    return;
                                }
                            }

                            console.log("Selected full screen screen", selected, "not found, falling back to primary screen");
                            console.log("List of screens is:", screens);
                            fullScreenDisplayDropdown.currentIndex = 0;
                        }

                        Component.onCompleted: {
                            model.refreshScreens();
                            fullScreenDisplayDropdown.refreshFullScreenDisplayDropdown();
                            fullScreenDisplayDropdown.displayText = model.get(currentIndex).text;
                        }

                        onCurrentIndexChanged: {
                            if (currentIndex >= 0) {
                                // Somehow, we end up going through here twice on every change of the combo box.
                                // The first one is with the new selected index, and the second one is with the
                                // index at -1.
                                //
                                // The first one comes from a sensible stack of:
                                //     onCurrentIndexChanged (qrc:/qml/hifi/dialogs/graphics/GraphicsSettings.qml:559)
                                //     refreshScreens (qrc:/qml/hifi/dialogs/graphics/GraphicsSettings.qml:514)
                                //     onCompleted (qrc:/qml/hifi/dialogs/graphics/GraphicsSettings.qml:553)
                                //     load (qrc:/qml/hifi/tablet/WindowRoot.qml:170)
                                //     loadSource (qrc:/qml/hifi/tablet/WindowRoot.qml:63)
                                //
                                // The second seems to be called out of nowhere. This likely indicates some sort of bug.
                                // Might be related to Wayland?

                                Render.setFullScreenScreen(model.get(currentIndex).text);
                                fullScreenDisplayDropdown.displayText = model.get(currentIndex).text;
                            } else {
                                console.log("Called with currentIndex =", currentIndex);
                                console.trace();
                            }
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.topMargin: 20
                Layout.preferredWidth: parent.width
                spacing: 0

                Item {
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 35

                    HifiStylesUit.RalewayRegular {
                        id: proceduralMaterialsHeader
                        text: "Procedural Materials"
                        anchors.left: parent.left
                        anchors.top: parent.top
                        width: 150
                        height: parent.height
                        size: 16
                        color: "#FFFFFF"
                    }

                    HifiControlsUit.CheckBox {
                        id: renderingEffectProceduralMaterials
                        checked: Render.proceduralMaterialsEnabled
                        boxSize: 16
                        spacing: -1
                        colorScheme: hifi.colorSchemes.dark
                        anchors.left: proceduralMaterialsHeader.right
                        anchors.leftMargin: 20
                        anchors.top: parent.top
                        onCheckedChanged: {
                            Render.proceduralMaterialsEnabled = renderingEffectProceduralMaterials.checked;
                        }
                    }
                }
            }

        }
    }

    function refreshAllDropdowns() {
        worldDetailDropdown.refreshWorldDetailDropdown();
        refreshRateDropdown.refreshRefreshRateDropdownDisplay();
        antialiasingDropdown.refreshAntialiasingDropdown();
    }
}
