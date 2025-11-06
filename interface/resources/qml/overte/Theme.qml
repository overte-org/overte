import QtQuick

pragma Singleton
QtObject {
    property bool useSystemColorScheme: true
    property bool useSystemContrastMode: true

    // https://github.com/overte-org/overte/issues/1733
    property bool darkMode: (
        useSystemColorScheme ?
        Qt.application.styleHints.colorScheme !== Qt.ColorScheme.Light :
        true
    )
    property bool highContrast: (
        useSystemContrastMode ?
        Qt.application.styleHints.accessibility.contrastPreference === Qt.ContrastPreference.HighContrast :
        false
    )
    property bool reducedMotion: false

    // font face for UI elements
    readonly property string fontFamily: "Roboto"

    // font face for document text
    readonly property string bodyFontFamily: "Roboto"

    // font face for code editors
    readonly property string monoFontFamily: "Roboto Mono"

    readonly property int fontPixelSize: 18
    readonly property int fontPixelSizeSmall: 14
    readonly property real borderRadius: 4.0
    readonly property real borderWidth: 2.0
    readonly property real borderWidthFocused: highContrast ? borderWidth * 2 : borderWidth

    // TODO: set these on mobile where scroll buttons aren't useful
    readonly property int scrollbarWidth: 24
    readonly property bool scrollbarButtons: true

    // Qt.lighten and Qt.darken constants for subtle 3D effect
    readonly property real borderDarker: darkMode ? 2.5 : 1.5
    readonly property real depthLighter: highContrast ? 1.0 : 1.2
    readonly property real depthDarker: highContrast ? 1.0 : 1.3
    readonly property real checkedDarker: highContrast ? 1.5 : 1.2
    readonly property real hoverLighter: {
        if (darkMode) {
            // don't apply hover lightness on dark high contrast
            return highContrast ? 1.0 : 1.3;
        } else {
            // 1.3 blows out the button colors to white on the light theme
            return 1.1;
        }
    }

    readonly property var paletteActive: (
        highContrast ?
        (darkMode ? paletteDarkContrast : paletteLightContrast) :
        (darkMode ? paletteDark : paletteLight)
    )

    readonly property var paletteDark: QtObject {
        readonly property color alternateBase:   Qt.darker(base, 1.1)
        readonly property color base:            "#403849"
        readonly property color text:            "#eeeeee"
        readonly property color button:          "#605868"
        readonly property color buttonText:      "#eeeeee"
        readonly property color window:          "#524c59"
        readonly property color windowText:      "#eeeeee"
        readonly property color highlight:       "#0a9dce"
        readonly property color highlightedText: "#ffffff"

        readonly property color focusRing:       "#f4801a"
        readonly property color placeholderText: "#80eeeeee"

        readonly property color tooltip:         "#524c59"
        readonly property color tooltipText:     "#eeeeee"

        readonly property color buttonDestructive: "#823d3d"
        readonly property color buttonAdd:         "#3a753a"
        readonly property color buttonInfo:        "#1e6591"

        readonly property color statusOffline:   "#808080"
        readonly property color statusFriendsOnly: "orange"
        readonly property color statusContacts:    "lime"
        readonly property color statusEveryone:    "cyan"

        readonly property color link: highlight
        readonly property color dialogShade: "#d0000000"

        readonly property color activeWindowTitleBg: Qt.darker("#403849", 1.2)
        readonly property color activeWindowTitleFg: text

        readonly property color userCountEmpty: "#b0b0b0"
        readonly property color userCountActive: "#22ef22"
        readonly property color userCountFull: "#ef2f1f"

        readonly property color appIconBackground: "#202020"
        readonly property color appInstalledRunning: statusContacts
        readonly property color appInstalledNotRunning: statusFriendsOnly
        readonly property color appNotInstalledRunning: "red"
        readonly property color appNotInstalled: statusOffline
    }

    readonly property var paletteLight: QtObject {
        readonly property color alternateBase:   Qt.darker(base, 1.05)
        readonly property color base:            "#f5f5f5"
        readonly property color text:            "#111111"
        readonly property color button:          "#f3f2f4"
        readonly property color buttonText:      "#111111"
        readonly property color window:          "#eeeeee"
        readonly property color windowText:      "#111111"
        readonly property color highlight:       "#0b3ebf"
        readonly property color highlightedText: "#ffffff"

        readonly property color focusRing:       "#f4801a"
        readonly property color placeholderText: "#60000000"

        readonly property color tooltip:         "#fffecc"
        readonly property color tooltipText:     "#111111"

        readonly property color buttonDestructive: "#fccccc"
        readonly property color buttonAdd:         "#bef4c5"
        readonly property color buttonInfo:        "#bfe5fc"

        readonly property color statusOffline:   "#808080"
        readonly property color statusFriendsOnly: "brown"
        readonly property color statusContacts:    "green"
        readonly property color statusEveryone:    "teal"

        readonly property color link: highlight
        readonly property color dialogShade: "#d0808080"

        readonly property color activeWindowTitleBg: "#000080"
        readonly property color activeWindowTitleFg: "white"

        readonly property color userCountEmpty: "#303030"
        readonly property color userCountActive: "#008000"
        readonly property color userCountFull: "#800000"

        readonly property color appIconBackground: "#202020"
        readonly property color appInstalledRunning: "#00ff00"
        readonly property color appInstalledNotRunning: "#ffaf00"
        readonly property color appNotInstalledRunning: "red"
        readonly property color appNotInstalled: statusOffline
    }

    readonly property var paletteDarkContrast: QtObject {
        readonly property color alternateBase:   Qt.darker(base, 1.1)
        readonly property color base:            "#000000"
        readonly property color text:            "#f0f0f0"
        readonly property color button:          "#000000"
        readonly property color buttonText:      "#f0f0f0"
        readonly property color window:          "#000000"
        readonly property color windowText:      "#f0f0f0"
        readonly property color highlight:       "#ffffff"
        readonly property color highlightedText: "#000000"

        readonly property color focusRing:       "#ff00ff"
        readonly property color placeholderText: "#00ff00"

        readonly property color tooltip:         "#000000"
        readonly property color tooltipText:     "#ffff00"

        readonly property color buttonDestructive: "#600000"
        readonly property color buttonAdd:         "#006000"
        readonly property color buttonInfo:        "#000080"

        readonly property color statusOffline:   "#808080"
        readonly property color statusFriendsOnly: "orange"
        readonly property color statusContacts:    "lime"
        readonly property color statusEveryone:    "cyan"

        readonly property color link: "#ffff00"
        readonly property color dialogShade: "#e8000000"

        readonly property color activeWindowTitleBg: base
        readonly property color activeWindowTitleFg: "white"

        readonly property color userCountEmpty: text
        readonly property color userCountActive: "#00ff00"
        readonly property color userCountFull: "#ff00ff"

        readonly property color appIconBackground: "black"
        readonly property color appInstalledRunning: statusContacts
        readonly property color appInstalledNotRunning: statusFriendsOnly
        readonly property color appNotInstalledRunning: "red"
        readonly property color appNotInstalled: statusOffline
    }

    readonly property var paletteLightContrast: QtObject {
        readonly property color alternateBase:   Qt.darker(base, 1.05)
        readonly property color base:            "#f5f5f5"
        readonly property color text:            "#000000"
        readonly property color button:          "#f5f5f5"
        readonly property color buttonText:      "#000000"
        readonly property color window:          "#f0f0f0"
        readonly property color windowText:      "#000000"
        readonly property color highlight:       "#600060"
        readonly property color highlightedText: "#ffffff"

        readonly property color focusRing:       "#ff00ff"
        readonly property color placeholderText: "#007000"

        readonly property color tooltip:         "#fffeee"
        readonly property color tooltipText:     "#111111"

        readonly property color buttonDestructive: "#ffdddd"
        readonly property color buttonAdd:         "#ddffdd"
        readonly property color buttonInfo:        "#ddffff"

        readonly property color statusOffline:   "#808080"
        readonly property color statusFriendsOnly: "brown"
        readonly property color statusContacts:    "green"
        readonly property color statusEveryone:    "teal"

        readonly property color link: "#000080"
        readonly property color dialogShade: "#fad0d0d0"

        readonly property color activeWindowTitleBg: base
        readonly property color activeWindowTitleFg: "black"

        readonly property color userCountEmpty: text
        readonly property color userCountActive: "#006000"
        readonly property color userCountFull: "#600060"

        readonly property color appIconBackground: "black"
        readonly property color appInstalledRunning: "#00ff00"
        readonly property color appInstalledNotRunning: "#ffaf00"
        readonly property color appNotInstalledRunning: "red"
        readonly property color appNotInstalled: statusOffline
    }
}
