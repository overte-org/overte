import QtQuick 2.7
import QtWebEngine 1.5

Item {
    id: root

    property bool webViewProfileSetup: false
    property string currentUrl: ""
    property string downloadUrl: ""
    property string adaptedPath: ""
    property string tempDir: ""
    function setupWebEngineSettings() {
        WebEngine.settings.javascriptCanOpenWindows = true;
        WebEngine.settings.javascriptCanAccessClipboard = false;
        WebEngine.settings.spatialNavigationEnabled = false;
        WebEngine.settings.localContentCanAccessRemoteUrls = true;
    }


    function initWebviewProfileHandlers(profile) {
    }
}
