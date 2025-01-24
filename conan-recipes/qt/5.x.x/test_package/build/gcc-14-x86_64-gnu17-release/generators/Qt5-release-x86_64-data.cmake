########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

list(APPEND qt_COMPONENT_NAMES Qt5::Core qt::WebEngineCore Qt5::Gui Qt5::EdidSupport Qt5::QSQLiteDriverPlugin Qt5::QPSQLDriverPlugin Qt5::QMySQLDriverPlugin Qt5::QODBCDriverPlugin Qt5::Network Qt5::Sql Qt5::Test Qt5::Concurrent Qt5::Xml Qt5::QSvgIconPlugin Qt5::QSvgPlugin Qt5::Positioning Qt5::QGeoServiceProviderFactoryMapbox Qt5::QGeoServiceProviderFactoryMapboxGL Qt5::GeoServiceProviderFactoryEsri Qt5::QGeoServiceProviderFactoryItemsOverlay Qt5::QGeoServiceProviderFactoryNokia Qt5::QGeoServiceProviderFactoryOsm Qt5::QGeoPositionInfoSourceFactoryGeoclue Qt5::QGeoPositionInfoSourceFactoryGeoclue2 Qt5::QGeoPositionInfoSourceFactoryPoll Qt5::QGeoPositionInfoSourceFactorySerialNmea Qt5::QM3uPlaylistPlugin Qt5::EventDispatcherSupport Qt5::FontDatabaseSupport Qt5::ThemeSupport Qt5::AccessibilitySupport Qt5::Widgets Qt5::ServiceSupport Qt5::XkbCommonSupport Qt5::OpenGL Qt5::OpenGLExtensions Qt5::Qml Qt5::QuickTest Qt5::Svg Qt5::Multimedia Qt5::WebSockets Qt5::XmlPatterns Qt5::PrintSupport Qt5::XcbQpa Qt5::QmlModels Qt5::QmlImportScanner Qt5::QmlWorkerScript Qt5::WebChannel Qt5::Scxml Qt5::MultimediaWidgets Qt5::QXcbIntegrationPlugin Qt5::Quick Qt5::QuickWidgets Qt5::QuickShapes Qt5::QuickControls2 Qt5::QuickTemplates2 Qt5::Location Qt5::WebEngineCore Qt5::MultimediaQuick Qt5::WebView Qt5::WebEngine Qt5::WebEngineWidgets)
list(REMOVE_DUPLICATES qt_COMPONENT_NAMES)
if(DEFINED qt_FIND_DEPENDENCY_NAMES)
  list(APPEND qt_FIND_DEPENDENCY_NAMES Fontconfig ODBC xkbcommon xorg opengl_system libxshmfence xorg-proto nss egl)
  list(REMOVE_DUPLICATES qt_FIND_DEPENDENCY_NAMES)
else()
  set(qt_FIND_DEPENDENCY_NAMES Fontconfig ODBC xkbcommon xorg opengl_system libxshmfence xorg-proto nss egl)
endif()
set(Fontconfig_FIND_MODE "NO_MODULE")
set(ODBC_FIND_MODE "NO_MODULE")
set(xkbcommon_FIND_MODE "NO_MODULE")
set(xorg_FIND_MODE "NO_MODULE")
set(opengl_system_FIND_MODE "NO_MODULE")
set(libxshmfence_FIND_MODE "NO_MODULE")
set(xorg-proto_FIND_MODE "NO_MODULE")
set(nss_FIND_MODE "NO_MODULE")
set(egl_FIND_MODE "NO_MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(qt_PACKAGE_FOLDER_RELEASE "/home/juliangro/.conan2/p/b/qt23376165b3dee/p")
set(qt_BUILD_MODULES_PATHS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib/cmake/Qt5Core/conan_qt_core_extras.cmake"
			"${qt_PACKAGE_FOLDER_RELEASE}/lib/cmake/Qt5Core/conan_qt_qt5_coreprivate.cmake"
			"${qt_PACKAGE_FOLDER_RELEASE}/lib/cmake/Qt5Core/Qt5CoreMacros.cmake"
			"${qt_PACKAGE_FOLDER_RELEASE}/lib/cmake/Qt5Gui/conan_qt_qt5_guiprivate.cmake"
			"${qt_PACKAGE_FOLDER_RELEASE}/lib/cmake/Qt5Widgets/conan_qt_qt5_widgetsprivate.cmake"
			"${qt_PACKAGE_FOLDER_RELEASE}/lib/cmake/Qt5Widgets/Qt5WidgetsMacros.cmake"
			"${qt_PACKAGE_FOLDER_RELEASE}/lib/cmake/Qt5Qml/conan_qt_qt5_qmlprivate.cmake"
			"${qt_PACKAGE_FOLDER_RELEASE}/lib/cmake/Qt5Quick/conan_qt_qt5_quickprivate.cmake"
			"${qt_PACKAGE_FOLDER_RELEASE}/lib/cmake/Qt5Scxml/Qt5ScxmlMacros.cmake")


set(qt_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtWebEngineWidgets"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtWebEngine"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtWebView"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtMultimediaQuick"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtWebEngineCore"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtLocation"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtQuickTemplates2"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtQuickControls2"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtQuickShapes"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtQuickWidgets"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtQuick"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtMultimediaWidgets"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtScxml"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtWebChannel"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtQmlWorkerScript"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtQmlModels"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtPrintSupport"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtXmlPatterns"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtWebSockets"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtMultimedia"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtSvg"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtQuickTest"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtQml"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtOpenGLExtensions"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtOpenGL"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtXkbCommonSupport"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtServiceSupport"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtWidgets"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtAccessibilitySupport"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtThemeSupport"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtFontDatabaseSupport"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtEventDispatcherSupport"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtPositioning"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtXml"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtConcurrent"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtTest"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtSql"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtNetwork"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtEdidSupport"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtGui"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtCore"
			"${qt_PACKAGE_FOLDER_RELEASE}/mkspecs/linux-g++")
set(qt_RES_DIRS_RELEASE )
set(qt_DEFINITIONS_RELEASE "-DQT_WEBENGINEWIDGETS_LIB"
			"-DQT_WEBENGINE_LIB"
			"-DQT_WEBVIEW_LIB"
			"-DQT_MULTIMEDIAQUICK_LIB"
			"-DQT_WEBENGINECORE_LIB"
			"-DQT_LOCATION_LIB"
			"-DQT_QUICKTEMPLATES2_LIB"
			"-DQT_QUICKCONTROLS2_LIB"
			"-DQT_QUICKSHAPES_LIB"
			"-DQT_QUICKWIDGETS_LIB"
			"-DQT_QUICK_LIB"
			"-DQT_MULTIMEDIAWIDGETS_LIB"
			"-DQT_SCXML_LIB"
			"-DQT_WEBCHANNEL_LIB"
			"-DQT_QMLWORKERSCRIPT_LIB"
			"-DQT_QMLMODELS_LIB"
			"-DQT_XCB_QPA_LIB_LIB"
			"-DQT_PRINT_SUPPORT_LIB"
			"-DQT_XMLPATTERNS_LIB"
			"-DQT_WEBSOCKETS_LIB"
			"-DQT_MULTIMEDIA_LIB"
			"-DQT_SVG_LIB"
			"-DQT_QUICKTEST_LIB"
			"-DQT_QML_LIB"
			"-DQT_OPENGLEXTENSIONS_LIB"
			"-DQT_OPENGL_LIB"
			"-DQT_XKBCOMMON_SUPPORT_LIB"
			"-DQT_SERVICE_SUPPORT_LIB"
			"-DQT_WIDGETS_LIB"
			"-DQT_ACCESSIBILITY_SUPPORT_LIB"
			"-DQT_THEME_SUPPORT_LIB"
			"-DQT_FONTDATABASE_SUPPORT_LIB"
			"-DQT_EVENTDISPATCHER_SUPPORT_LIB"
			"-DQT_POSITIONING_LIB"
			"-DQT_XML_LIB"
			"-DQT_CONCURRENT_LIB"
			"-DQT_TESTLIB_LIB"
			"-DQT_SQL_LIB"
			"-DQT_NETWORK_LIB"
			"-DQT_EDID_SUPPORT_LIB"
			"-DQT_GUI_LIB"
			"-DQT_CORE_LIB")
set(qt_SHARED_LINK_FLAGS_RELEASE )
set(qt_EXE_LINK_FLAGS_RELEASE )
set(qt_OBJECTS_RELEASE )
set(qt_COMPILE_DEFINITIONS_RELEASE "QT_WEBENGINEWIDGETS_LIB"
			"QT_WEBENGINE_LIB"
			"QT_WEBVIEW_LIB"
			"QT_MULTIMEDIAQUICK_LIB"
			"QT_WEBENGINECORE_LIB"
			"QT_LOCATION_LIB"
			"QT_QUICKTEMPLATES2_LIB"
			"QT_QUICKCONTROLS2_LIB"
			"QT_QUICKSHAPES_LIB"
			"QT_QUICKWIDGETS_LIB"
			"QT_QUICK_LIB"
			"QT_MULTIMEDIAWIDGETS_LIB"
			"QT_SCXML_LIB"
			"QT_WEBCHANNEL_LIB"
			"QT_QMLWORKERSCRIPT_LIB"
			"QT_QMLMODELS_LIB"
			"QT_XCB_QPA_LIB_LIB"
			"QT_PRINT_SUPPORT_LIB"
			"QT_XMLPATTERNS_LIB"
			"QT_WEBSOCKETS_LIB"
			"QT_MULTIMEDIA_LIB"
			"QT_SVG_LIB"
			"QT_QUICKTEST_LIB"
			"QT_QML_LIB"
			"QT_OPENGLEXTENSIONS_LIB"
			"QT_OPENGL_LIB"
			"QT_XKBCOMMON_SUPPORT_LIB"
			"QT_SERVICE_SUPPORT_LIB"
			"QT_WIDGETS_LIB"
			"QT_ACCESSIBILITY_SUPPORT_LIB"
			"QT_THEME_SUPPORT_LIB"
			"QT_FONTDATABASE_SUPPORT_LIB"
			"QT_EVENTDISPATCHER_SUPPORT_LIB"
			"QT_POSITIONING_LIB"
			"QT_XML_LIB"
			"QT_CONCURRENT_LIB"
			"QT_TESTLIB_LIB"
			"QT_SQL_LIB"
			"QT_NETWORK_LIB"
			"QT_EDID_SUPPORT_LIB"
			"QT_GUI_LIB"
			"QT_CORE_LIB")
set(qt_COMPILE_OPTIONS_C_RELEASE )
set(qt_COMPILE_OPTIONS_CXX_RELEASE -fPIC)
set(qt_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib"
			"${qt_PACKAGE_FOLDER_RELEASE}/plugins/platforms"
			"${qt_PACKAGE_FOLDER_RELEASE}/plugins/playlistformats"
			"${qt_PACKAGE_FOLDER_RELEASE}/plugins/position"
			"${qt_PACKAGE_FOLDER_RELEASE}/plugins/geoservices"
			"${qt_PACKAGE_FOLDER_RELEASE}/plugins/imageformats"
			"${qt_PACKAGE_FOLDER_RELEASE}/plugins/iconengines"
			"${qt_PACKAGE_FOLDER_RELEASE}/plugins/sqldrivers")
set(qt_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_LIBRARY_TYPE_RELEASE SHARED)
set(qt_IS_HOST_WINDOWS_RELEASE 0)
set(qt_LIBS_RELEASE Qt5WebEngineWidgets Qt5WebEngine Qt5WebView Qt5MultimediaQuick Qt5WebEngineCore Qt5Location Qt5QuickTemplates2 Qt5QuickControls2 Qt5QuickShapes Qt5QuickWidgets Qt5Quick Qt5MultimediaWidgets Qt5Scxml Qt5WebChannel Qt5QmlWorkerScript Qt5QmlModels Qt5XcbQpa Qt5PrintSupport Qt5XmlPatterns Qt5WebSockets Qt5Multimedia Qt5Svg Qt5QuickTest Qt5Qml Qt5OpenGLExtensions Qt5OpenGL Qt5XkbCommonSupport Qt5ServiceSupport Qt5Widgets Qt5AccessibilitySupport Qt5ThemeSupport Qt5FontDatabaseSupport Qt5EventDispatcherSupport Qt5Positioning Qt5Xml Qt5Concurrent Qt5Test Qt5Sql Qt5Network Qt5EdidSupport Qt5Gui Qt5Core)
set(qt_SYSTEM_LIBS_RELEASE resolv)
set(qt_FRAMEWORK_DIRS_RELEASE )
set(qt_FRAMEWORKS_RELEASE )
set(qt_BUILD_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib/cmake/Qt5Quick"
			"${qt_PACKAGE_FOLDER_RELEASE}/lib/cmake/Qt5Scxml"
			"${qt_PACKAGE_FOLDER_RELEASE}/lib/cmake/Qt5Qml"
			"${qt_PACKAGE_FOLDER_RELEASE}/lib/cmake/Qt5Widgets"
			"${qt_PACKAGE_FOLDER_RELEASE}/lib/cmake/Qt5Gui"
			"${qt_PACKAGE_FOLDER_RELEASE}/bin"
			"${qt_PACKAGE_FOLDER_RELEASE}/lib/cmake/Qt5Core")
set(qt_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(qt_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_COMPILE_OPTIONS_C_RELEASE}>")
set(qt_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_EXE_LINK_FLAGS_RELEASE}>")


set(qt_COMPONENTS_RELEASE Qt5::Core qt::WebEngineCore Qt5::Gui Qt5::EdidSupport Qt5::QSQLiteDriverPlugin Qt5::QPSQLDriverPlugin Qt5::QMySQLDriverPlugin Qt5::QODBCDriverPlugin Qt5::Network Qt5::Sql Qt5::Test Qt5::Concurrent Qt5::Xml Qt5::QSvgIconPlugin Qt5::QSvgPlugin Qt5::Positioning Qt5::QGeoServiceProviderFactoryMapbox Qt5::QGeoServiceProviderFactoryMapboxGL Qt5::GeoServiceProviderFactoryEsri Qt5::QGeoServiceProviderFactoryItemsOverlay Qt5::QGeoServiceProviderFactoryNokia Qt5::QGeoServiceProviderFactoryOsm Qt5::QGeoPositionInfoSourceFactoryGeoclue Qt5::QGeoPositionInfoSourceFactoryGeoclue2 Qt5::QGeoPositionInfoSourceFactoryPoll Qt5::QGeoPositionInfoSourceFactorySerialNmea Qt5::QM3uPlaylistPlugin Qt5::EventDispatcherSupport Qt5::FontDatabaseSupport Qt5::ThemeSupport Qt5::AccessibilitySupport Qt5::Widgets Qt5::ServiceSupport Qt5::XkbCommonSupport Qt5::OpenGL Qt5::OpenGLExtensions Qt5::Qml Qt5::QuickTest Qt5::Svg Qt5::Multimedia Qt5::WebSockets Qt5::XmlPatterns Qt5::PrintSupport Qt5::XcbQpa Qt5::QmlModels Qt5::QmlImportScanner Qt5::QmlWorkerScript Qt5::WebChannel Qt5::Scxml Qt5::MultimediaWidgets Qt5::QXcbIntegrationPlugin Qt5::Quick Qt5::QuickWidgets Qt5::QuickShapes Qt5::QuickControls2 Qt5::QuickTemplates2 Qt5::Location Qt5::WebEngineCore Qt5::MultimediaQuick Qt5::WebView Qt5::WebEngine Qt5::WebEngineWidgets)
########### COMPONENT Qt5::WebEngineWidgets VARIABLES ############################################

set(qt_Qt5_WebEngineWidgets_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtWebEngineWidgets")
set(qt_Qt5_WebEngineWidgets_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_WebEngineWidgets_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_WebEngineWidgets_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_WebEngineWidgets_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_WebEngineWidgets_RES_DIRS_RELEASE )
set(qt_Qt5_WebEngineWidgets_DEFINITIONS_RELEASE "-DQT_WEBENGINEWIDGETS_LIB")
set(qt_Qt5_WebEngineWidgets_OBJECTS_RELEASE )
set(qt_Qt5_WebEngineWidgets_COMPILE_DEFINITIONS_RELEASE "QT_WEBENGINEWIDGETS_LIB")
set(qt_Qt5_WebEngineWidgets_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_WebEngineWidgets_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_WebEngineWidgets_LIBS_RELEASE Qt5WebEngineWidgets)
set(qt_Qt5_WebEngineWidgets_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_WebEngineWidgets_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_WebEngineWidgets_FRAMEWORKS_RELEASE )
set(qt_Qt5_WebEngineWidgets_DEPENDENCIES_RELEASE Qt5::WebEngineCore Qt5::Quick Qt5::PrintSupport Qt5::Widgets Qt5::Gui Qt5::Network Qt5::Core)
set(qt_Qt5_WebEngineWidgets_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_WebEngineWidgets_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_WebEngineWidgets_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_WebEngineWidgets_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_WebEngineWidgets_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_WebEngineWidgets_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_WebEngineWidgets_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_WebEngineWidgets_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_WebEngineWidgets_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_WebEngineWidgets_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::WebEngine VARIABLES ############################################

set(qt_Qt5_WebEngine_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtWebEngine")
set(qt_Qt5_WebEngine_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_WebEngine_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_WebEngine_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_WebEngine_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_WebEngine_RES_DIRS_RELEASE )
set(qt_Qt5_WebEngine_DEFINITIONS_RELEASE "-DQT_WEBENGINE_LIB")
set(qt_Qt5_WebEngine_OBJECTS_RELEASE )
set(qt_Qt5_WebEngine_COMPILE_DEFINITIONS_RELEASE "QT_WEBENGINE_LIB")
set(qt_Qt5_WebEngine_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_WebEngine_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_WebEngine_LIBS_RELEASE Qt5WebEngine)
set(qt_Qt5_WebEngine_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_WebEngine_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_WebEngine_FRAMEWORKS_RELEASE )
set(qt_Qt5_WebEngine_DEPENDENCIES_RELEASE Qt5::WebEngineCore Qt5::Core)
set(qt_Qt5_WebEngine_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_WebEngine_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_WebEngine_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_WebEngine_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_WebEngine_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_WebEngine_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_WebEngine_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_WebEngine_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_WebEngine_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_WebEngine_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::WebView VARIABLES ############################################

set(qt_Qt5_WebView_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtWebView")
set(qt_Qt5_WebView_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_WebView_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_WebView_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_WebView_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_WebView_RES_DIRS_RELEASE )
set(qt_Qt5_WebView_DEFINITIONS_RELEASE "-DQT_WEBVIEW_LIB")
set(qt_Qt5_WebView_OBJECTS_RELEASE )
set(qt_Qt5_WebView_COMPILE_DEFINITIONS_RELEASE "QT_WEBVIEW_LIB")
set(qt_Qt5_WebView_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_WebView_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_WebView_LIBS_RELEASE Qt5WebView)
set(qt_Qt5_WebView_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_WebView_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_WebView_FRAMEWORKS_RELEASE )
set(qt_Qt5_WebView_DEPENDENCIES_RELEASE Qt5::Gui Qt5::Quick Qt5::Core)
set(qt_Qt5_WebView_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_WebView_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_WebView_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_WebView_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_WebView_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_WebView_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_WebView_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_WebView_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_WebView_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_WebView_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::MultimediaQuick VARIABLES ############################################

set(qt_Qt5_MultimediaQuick_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtMultimediaQuick")
set(qt_Qt5_MultimediaQuick_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_MultimediaQuick_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_MultimediaQuick_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_MultimediaQuick_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_MultimediaQuick_RES_DIRS_RELEASE )
set(qt_Qt5_MultimediaQuick_DEFINITIONS_RELEASE "-DQT_MULTIMEDIAQUICK_LIB")
set(qt_Qt5_MultimediaQuick_OBJECTS_RELEASE )
set(qt_Qt5_MultimediaQuick_COMPILE_DEFINITIONS_RELEASE "QT_MULTIMEDIAQUICK_LIB")
set(qt_Qt5_MultimediaQuick_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_MultimediaQuick_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_MultimediaQuick_LIBS_RELEASE Qt5MultimediaQuick)
set(qt_Qt5_MultimediaQuick_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_MultimediaQuick_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_MultimediaQuick_FRAMEWORKS_RELEASE )
set(qt_Qt5_MultimediaQuick_DEPENDENCIES_RELEASE Qt5::Multimedia Qt5::Quick Qt5::Core)
set(qt_Qt5_MultimediaQuick_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_MultimediaQuick_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_MultimediaQuick_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_MultimediaQuick_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_MultimediaQuick_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_MultimediaQuick_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_MultimediaQuick_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_MultimediaQuick_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_MultimediaQuick_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_MultimediaQuick_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::WebEngineCore VARIABLES ############################################

set(qt_Qt5_WebEngineCore_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtWebEngineCore")
set(qt_Qt5_WebEngineCore_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_WebEngineCore_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_WebEngineCore_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_WebEngineCore_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_WebEngineCore_RES_DIRS_RELEASE )
set(qt_Qt5_WebEngineCore_DEFINITIONS_RELEASE "-DQT_WEBENGINECORE_LIB")
set(qt_Qt5_WebEngineCore_OBJECTS_RELEASE )
set(qt_Qt5_WebEngineCore_COMPILE_DEFINITIONS_RELEASE "QT_WEBENGINECORE_LIB")
set(qt_Qt5_WebEngineCore_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_WebEngineCore_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_WebEngineCore_LIBS_RELEASE Qt5WebEngineCore)
set(qt_Qt5_WebEngineCore_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_WebEngineCore_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_WebEngineCore_FRAMEWORKS_RELEASE )
set(qt_Qt5_WebEngineCore_DEPENDENCIES_RELEASE Qt5::Gui Qt5::Quick Qt5::WebChannel Qt5::Positioning xorg-proto::xorg-proto libxshmfence::libxshmfence nss::nss egl::egl Qt5::Core)
set(qt_Qt5_WebEngineCore_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_WebEngineCore_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_WebEngineCore_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_WebEngineCore_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_WebEngineCore_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_WebEngineCore_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_WebEngineCore_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_WebEngineCore_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_WebEngineCore_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_WebEngineCore_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::Location VARIABLES ############################################

set(qt_Qt5_Location_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtLocation")
set(qt_Qt5_Location_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_Location_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_Location_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_Location_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_Location_RES_DIRS_RELEASE )
set(qt_Qt5_Location_DEFINITIONS_RELEASE "-DQT_LOCATION_LIB")
set(qt_Qt5_Location_OBJECTS_RELEASE )
set(qt_Qt5_Location_COMPILE_DEFINITIONS_RELEASE "QT_LOCATION_LIB")
set(qt_Qt5_Location_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_Location_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_Location_LIBS_RELEASE Qt5Location)
set(qt_Qt5_Location_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_Location_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_Location_FRAMEWORKS_RELEASE )
set(qt_Qt5_Location_DEPENDENCIES_RELEASE Qt5::Gui Qt5::Quick Qt5::Core)
set(qt_Qt5_Location_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_Location_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_Location_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_Location_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_Location_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_Location_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_Location_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_Location_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_Location_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_Location_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QuickTemplates2 VARIABLES ############################################

set(qt_Qt5_QuickTemplates2_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtQuickTemplates2")
set(qt_Qt5_QuickTemplates2_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_QuickTemplates2_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QuickTemplates2_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QuickTemplates2_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QuickTemplates2_RES_DIRS_RELEASE )
set(qt_Qt5_QuickTemplates2_DEFINITIONS_RELEASE "-DQT_QUICKTEMPLATES2_LIB")
set(qt_Qt5_QuickTemplates2_OBJECTS_RELEASE )
set(qt_Qt5_QuickTemplates2_COMPILE_DEFINITIONS_RELEASE "QT_QUICKTEMPLATES2_LIB")
set(qt_Qt5_QuickTemplates2_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QuickTemplates2_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QuickTemplates2_LIBS_RELEASE Qt5QuickTemplates2)
set(qt_Qt5_QuickTemplates2_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QuickTemplates2_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QuickTemplates2_FRAMEWORKS_RELEASE )
set(qt_Qt5_QuickTemplates2_DEPENDENCIES_RELEASE Qt5::Gui Qt5::Quick Qt5::Core)
set(qt_Qt5_QuickTemplates2_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QuickTemplates2_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QuickTemplates2_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QuickTemplates2_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QuickTemplates2_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QuickTemplates2_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QuickTemplates2_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QuickTemplates2_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QuickTemplates2_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QuickTemplates2_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QuickControls2 VARIABLES ############################################

set(qt_Qt5_QuickControls2_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtQuickControls2")
set(qt_Qt5_QuickControls2_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_QuickControls2_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QuickControls2_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QuickControls2_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QuickControls2_RES_DIRS_RELEASE )
set(qt_Qt5_QuickControls2_DEFINITIONS_RELEASE "-DQT_QUICKCONTROLS2_LIB")
set(qt_Qt5_QuickControls2_OBJECTS_RELEASE )
set(qt_Qt5_QuickControls2_COMPILE_DEFINITIONS_RELEASE "QT_QUICKCONTROLS2_LIB")
set(qt_Qt5_QuickControls2_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QuickControls2_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QuickControls2_LIBS_RELEASE Qt5QuickControls2)
set(qt_Qt5_QuickControls2_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QuickControls2_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QuickControls2_FRAMEWORKS_RELEASE )
set(qt_Qt5_QuickControls2_DEPENDENCIES_RELEASE Qt5::Gui Qt5::Quick Qt5::Core)
set(qt_Qt5_QuickControls2_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QuickControls2_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QuickControls2_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QuickControls2_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QuickControls2_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QuickControls2_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QuickControls2_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QuickControls2_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QuickControls2_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QuickControls2_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QuickShapes VARIABLES ############################################

set(qt_Qt5_QuickShapes_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtQuickShapes")
set(qt_Qt5_QuickShapes_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_QuickShapes_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QuickShapes_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QuickShapes_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QuickShapes_RES_DIRS_RELEASE )
set(qt_Qt5_QuickShapes_DEFINITIONS_RELEASE "-DQT_QUICKSHAPES_LIB")
set(qt_Qt5_QuickShapes_OBJECTS_RELEASE )
set(qt_Qt5_QuickShapes_COMPILE_DEFINITIONS_RELEASE "QT_QUICKSHAPES_LIB")
set(qt_Qt5_QuickShapes_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QuickShapes_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QuickShapes_LIBS_RELEASE Qt5QuickShapes)
set(qt_Qt5_QuickShapes_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QuickShapes_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QuickShapes_FRAMEWORKS_RELEASE )
set(qt_Qt5_QuickShapes_DEPENDENCIES_RELEASE Qt5::Gui Qt5::Qml Qt5::Quick Qt5::Core)
set(qt_Qt5_QuickShapes_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QuickShapes_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QuickShapes_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QuickShapes_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QuickShapes_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QuickShapes_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QuickShapes_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QuickShapes_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QuickShapes_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QuickShapes_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QuickWidgets VARIABLES ############################################

set(qt_Qt5_QuickWidgets_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtQuickWidgets")
set(qt_Qt5_QuickWidgets_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_QuickWidgets_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QuickWidgets_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QuickWidgets_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QuickWidgets_RES_DIRS_RELEASE )
set(qt_Qt5_QuickWidgets_DEFINITIONS_RELEASE "-DQT_QUICKWIDGETS_LIB")
set(qt_Qt5_QuickWidgets_OBJECTS_RELEASE )
set(qt_Qt5_QuickWidgets_COMPILE_DEFINITIONS_RELEASE "QT_QUICKWIDGETS_LIB")
set(qt_Qt5_QuickWidgets_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QuickWidgets_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QuickWidgets_LIBS_RELEASE Qt5QuickWidgets)
set(qt_Qt5_QuickWidgets_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QuickWidgets_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QuickWidgets_FRAMEWORKS_RELEASE )
set(qt_Qt5_QuickWidgets_DEPENDENCIES_RELEASE Qt5::Gui Qt5::Qml Qt5::Quick Qt5::Widgets Qt5::Core)
set(qt_Qt5_QuickWidgets_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QuickWidgets_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QuickWidgets_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QuickWidgets_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QuickWidgets_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QuickWidgets_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QuickWidgets_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QuickWidgets_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QuickWidgets_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QuickWidgets_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::Quick VARIABLES ############################################

set(qt_Qt5_Quick_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtQuick")
set(qt_Qt5_Quick_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_Quick_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_Quick_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_Quick_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_Quick_RES_DIRS_RELEASE )
set(qt_Qt5_Quick_DEFINITIONS_RELEASE "-DQT_QUICK_LIB")
set(qt_Qt5_Quick_OBJECTS_RELEASE )
set(qt_Qt5_Quick_COMPILE_DEFINITIONS_RELEASE "QT_QUICK_LIB")
set(qt_Qt5_Quick_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_Quick_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_Quick_LIBS_RELEASE Qt5Quick)
set(qt_Qt5_Quick_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_Quick_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_Quick_FRAMEWORKS_RELEASE )
set(qt_Qt5_Quick_DEPENDENCIES_RELEASE Qt5::Gui Qt5::Qml Qt5::QmlModels Qt5::Core)
set(qt_Qt5_Quick_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_Quick_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_Quick_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_Quick_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_Quick_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_Quick_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_Quick_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_Quick_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_Quick_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_Quick_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QXcbIntegrationPlugin VARIABLES ############################################

set(qt_Qt5_QXcbIntegrationPlugin_INCLUDE_DIRS_RELEASE )
set(qt_Qt5_QXcbIntegrationPlugin_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/plugins/platforms")
set(qt_Qt5_QXcbIntegrationPlugin_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QXcbIntegrationPlugin_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QXcbIntegrationPlugin_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QXcbIntegrationPlugin_RES_DIRS_RELEASE )
set(qt_Qt5_QXcbIntegrationPlugin_DEFINITIONS_RELEASE )
set(qt_Qt5_QXcbIntegrationPlugin_OBJECTS_RELEASE )
set(qt_Qt5_QXcbIntegrationPlugin_COMPILE_DEFINITIONS_RELEASE )
set(qt_Qt5_QXcbIntegrationPlugin_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QXcbIntegrationPlugin_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QXcbIntegrationPlugin_LIBS_RELEASE )
set(qt_Qt5_QXcbIntegrationPlugin_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QXcbIntegrationPlugin_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QXcbIntegrationPlugin_FRAMEWORKS_RELEASE )
set(qt_Qt5_QXcbIntegrationPlugin_DEPENDENCIES_RELEASE Qt5::Core Qt5::Gui Qt5::XcbQpa)
set(qt_Qt5_QXcbIntegrationPlugin_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QXcbIntegrationPlugin_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QXcbIntegrationPlugin_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QXcbIntegrationPlugin_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QXcbIntegrationPlugin_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QXcbIntegrationPlugin_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QXcbIntegrationPlugin_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QXcbIntegrationPlugin_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QXcbIntegrationPlugin_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QXcbIntegrationPlugin_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::MultimediaWidgets VARIABLES ############################################

set(qt_Qt5_MultimediaWidgets_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtMultimediaWidgets")
set(qt_Qt5_MultimediaWidgets_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_MultimediaWidgets_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_MultimediaWidgets_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_MultimediaWidgets_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_MultimediaWidgets_RES_DIRS_RELEASE )
set(qt_Qt5_MultimediaWidgets_DEFINITIONS_RELEASE "-DQT_MULTIMEDIAWIDGETS_LIB")
set(qt_Qt5_MultimediaWidgets_OBJECTS_RELEASE )
set(qt_Qt5_MultimediaWidgets_COMPILE_DEFINITIONS_RELEASE "QT_MULTIMEDIAWIDGETS_LIB")
set(qt_Qt5_MultimediaWidgets_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_MultimediaWidgets_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_MultimediaWidgets_LIBS_RELEASE Qt5MultimediaWidgets)
set(qt_Qt5_MultimediaWidgets_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_MultimediaWidgets_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_MultimediaWidgets_FRAMEWORKS_RELEASE )
set(qt_Qt5_MultimediaWidgets_DEPENDENCIES_RELEASE Qt5::Multimedia Qt5::Widgets Qt5::Gui Qt5::Core)
set(qt_Qt5_MultimediaWidgets_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_MultimediaWidgets_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_MultimediaWidgets_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_MultimediaWidgets_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_MultimediaWidgets_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_MultimediaWidgets_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_MultimediaWidgets_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_MultimediaWidgets_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_MultimediaWidgets_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_MultimediaWidgets_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::Scxml VARIABLES ############################################

set(qt_Qt5_Scxml_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtScxml")
set(qt_Qt5_Scxml_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_Scxml_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_Scxml_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_Scxml_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_Scxml_RES_DIRS_RELEASE )
set(qt_Qt5_Scxml_DEFINITIONS_RELEASE "-DQT_SCXML_LIB")
set(qt_Qt5_Scxml_OBJECTS_RELEASE )
set(qt_Qt5_Scxml_COMPILE_DEFINITIONS_RELEASE "QT_SCXML_LIB")
set(qt_Qt5_Scxml_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_Scxml_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_Scxml_LIBS_RELEASE Qt5Scxml)
set(qt_Qt5_Scxml_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_Scxml_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_Scxml_FRAMEWORKS_RELEASE )
set(qt_Qt5_Scxml_DEPENDENCIES_RELEASE Qt5::Qml Qt5::Core)
set(qt_Qt5_Scxml_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_Scxml_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_Scxml_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_Scxml_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_Scxml_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_Scxml_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_Scxml_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_Scxml_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_Scxml_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_Scxml_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::WebChannel VARIABLES ############################################

set(qt_Qt5_WebChannel_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtWebChannel")
set(qt_Qt5_WebChannel_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_WebChannel_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_WebChannel_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_WebChannel_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_WebChannel_RES_DIRS_RELEASE )
set(qt_Qt5_WebChannel_DEFINITIONS_RELEASE "-DQT_WEBCHANNEL_LIB")
set(qt_Qt5_WebChannel_OBJECTS_RELEASE )
set(qt_Qt5_WebChannel_COMPILE_DEFINITIONS_RELEASE "QT_WEBCHANNEL_LIB")
set(qt_Qt5_WebChannel_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_WebChannel_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_WebChannel_LIBS_RELEASE Qt5WebChannel)
set(qt_Qt5_WebChannel_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_WebChannel_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_WebChannel_FRAMEWORKS_RELEASE )
set(qt_Qt5_WebChannel_DEPENDENCIES_RELEASE Qt5::Qml Qt5::Core)
set(qt_Qt5_WebChannel_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_WebChannel_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_WebChannel_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_WebChannel_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_WebChannel_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_WebChannel_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_WebChannel_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_WebChannel_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_WebChannel_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_WebChannel_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QmlWorkerScript VARIABLES ############################################

set(qt_Qt5_QmlWorkerScript_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtQmlWorkerScript")
set(qt_Qt5_QmlWorkerScript_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_QmlWorkerScript_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QmlWorkerScript_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QmlWorkerScript_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QmlWorkerScript_RES_DIRS_RELEASE )
set(qt_Qt5_QmlWorkerScript_DEFINITIONS_RELEASE "-DQT_QMLWORKERSCRIPT_LIB")
set(qt_Qt5_QmlWorkerScript_OBJECTS_RELEASE )
set(qt_Qt5_QmlWorkerScript_COMPILE_DEFINITIONS_RELEASE "QT_QMLWORKERSCRIPT_LIB")
set(qt_Qt5_QmlWorkerScript_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QmlWorkerScript_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QmlWorkerScript_LIBS_RELEASE Qt5QmlWorkerScript)
set(qt_Qt5_QmlWorkerScript_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QmlWorkerScript_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QmlWorkerScript_FRAMEWORKS_RELEASE )
set(qt_Qt5_QmlWorkerScript_DEPENDENCIES_RELEASE Qt5::Qml Qt5::Core)
set(qt_Qt5_QmlWorkerScript_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QmlWorkerScript_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QmlWorkerScript_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QmlWorkerScript_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QmlWorkerScript_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QmlWorkerScript_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QmlWorkerScript_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QmlWorkerScript_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QmlWorkerScript_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QmlWorkerScript_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QmlImportScanner VARIABLES ############################################

set(qt_Qt5_QmlImportScanner_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include")
set(qt_Qt5_QmlImportScanner_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_QmlImportScanner_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QmlImportScanner_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QmlImportScanner_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QmlImportScanner_RES_DIRS_RELEASE )
set(qt_Qt5_QmlImportScanner_DEFINITIONS_RELEASE )
set(qt_Qt5_QmlImportScanner_OBJECTS_RELEASE )
set(qt_Qt5_QmlImportScanner_COMPILE_DEFINITIONS_RELEASE )
set(qt_Qt5_QmlImportScanner_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QmlImportScanner_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QmlImportScanner_LIBS_RELEASE )
set(qt_Qt5_QmlImportScanner_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QmlImportScanner_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QmlImportScanner_FRAMEWORKS_RELEASE )
set(qt_Qt5_QmlImportScanner_DEPENDENCIES_RELEASE Qt5::Qml)
set(qt_Qt5_QmlImportScanner_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QmlImportScanner_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QmlImportScanner_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QmlImportScanner_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QmlImportScanner_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QmlImportScanner_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QmlImportScanner_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QmlImportScanner_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QmlImportScanner_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QmlImportScanner_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QmlModels VARIABLES ############################################

set(qt_Qt5_QmlModels_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtQmlModels")
set(qt_Qt5_QmlModels_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_QmlModels_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QmlModels_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QmlModels_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QmlModels_RES_DIRS_RELEASE )
set(qt_Qt5_QmlModels_DEFINITIONS_RELEASE "-DQT_QMLMODELS_LIB")
set(qt_Qt5_QmlModels_OBJECTS_RELEASE )
set(qt_Qt5_QmlModels_COMPILE_DEFINITIONS_RELEASE "QT_QMLMODELS_LIB")
set(qt_Qt5_QmlModels_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QmlModels_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QmlModels_LIBS_RELEASE Qt5QmlModels)
set(qt_Qt5_QmlModels_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QmlModels_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QmlModels_FRAMEWORKS_RELEASE )
set(qt_Qt5_QmlModels_DEPENDENCIES_RELEASE Qt5::Qml Qt5::Core)
set(qt_Qt5_QmlModels_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QmlModels_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QmlModels_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QmlModels_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QmlModels_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QmlModels_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QmlModels_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QmlModels_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QmlModels_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QmlModels_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::XcbQpa VARIABLES ############################################

set(qt_Qt5_XcbQpa_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include")
set(qt_Qt5_XcbQpa_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_XcbQpa_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_XcbQpa_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_XcbQpa_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_XcbQpa_RES_DIRS_RELEASE )
set(qt_Qt5_XcbQpa_DEFINITIONS_RELEASE "-DQT_XCB_QPA_LIB_LIB")
set(qt_Qt5_XcbQpa_OBJECTS_RELEASE )
set(qt_Qt5_XcbQpa_COMPILE_DEFINITIONS_RELEASE "QT_XCB_QPA_LIB_LIB")
set(qt_Qt5_XcbQpa_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_XcbQpa_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_XcbQpa_LIBS_RELEASE Qt5XcbQpa)
set(qt_Qt5_XcbQpa_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_XcbQpa_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_XcbQpa_FRAMEWORKS_RELEASE )
set(qt_Qt5_XcbQpa_DEPENDENCIES_RELEASE Qt5::Core Qt5::Gui Qt5::ServiceSupport Qt5::ThemeSupport Qt5::FontDatabaseSupport Qt5::EdidSupport Qt5::XkbCommonSupport xorg::xorg)
set(qt_Qt5_XcbQpa_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_XcbQpa_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_XcbQpa_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_XcbQpa_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_XcbQpa_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_XcbQpa_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_XcbQpa_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_XcbQpa_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_XcbQpa_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_XcbQpa_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::PrintSupport VARIABLES ############################################

set(qt_Qt5_PrintSupport_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtPrintSupport")
set(qt_Qt5_PrintSupport_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_PrintSupport_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_PrintSupport_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_PrintSupport_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_PrintSupport_RES_DIRS_RELEASE )
set(qt_Qt5_PrintSupport_DEFINITIONS_RELEASE "-DQT_PRINT_SUPPORT_LIB")
set(qt_Qt5_PrintSupport_OBJECTS_RELEASE )
set(qt_Qt5_PrintSupport_COMPILE_DEFINITIONS_RELEASE "QT_PRINT_SUPPORT_LIB")
set(qt_Qt5_PrintSupport_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_PrintSupport_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_PrintSupport_LIBS_RELEASE Qt5PrintSupport)
set(qt_Qt5_PrintSupport_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_PrintSupport_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_PrintSupport_FRAMEWORKS_RELEASE )
set(qt_Qt5_PrintSupport_DEPENDENCIES_RELEASE Qt5::Gui Qt5::Widgets Qt5::Core)
set(qt_Qt5_PrintSupport_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_PrintSupport_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_PrintSupport_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_PrintSupport_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_PrintSupport_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_PrintSupport_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_PrintSupport_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_PrintSupport_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_PrintSupport_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_PrintSupport_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::XmlPatterns VARIABLES ############################################

set(qt_Qt5_XmlPatterns_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtXmlPatterns")
set(qt_Qt5_XmlPatterns_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_XmlPatterns_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_XmlPatterns_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_XmlPatterns_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_XmlPatterns_RES_DIRS_RELEASE )
set(qt_Qt5_XmlPatterns_DEFINITIONS_RELEASE "-DQT_XMLPATTERNS_LIB")
set(qt_Qt5_XmlPatterns_OBJECTS_RELEASE )
set(qt_Qt5_XmlPatterns_COMPILE_DEFINITIONS_RELEASE "QT_XMLPATTERNS_LIB")
set(qt_Qt5_XmlPatterns_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_XmlPatterns_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_XmlPatterns_LIBS_RELEASE Qt5XmlPatterns)
set(qt_Qt5_XmlPatterns_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_XmlPatterns_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_XmlPatterns_FRAMEWORKS_RELEASE )
set(qt_Qt5_XmlPatterns_DEPENDENCIES_RELEASE Qt5::Network Qt5::Core)
set(qt_Qt5_XmlPatterns_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_XmlPatterns_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_XmlPatterns_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_XmlPatterns_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_XmlPatterns_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_XmlPatterns_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_XmlPatterns_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_XmlPatterns_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_XmlPatterns_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_XmlPatterns_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::WebSockets VARIABLES ############################################

set(qt_Qt5_WebSockets_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtWebSockets")
set(qt_Qt5_WebSockets_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_WebSockets_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_WebSockets_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_WebSockets_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_WebSockets_RES_DIRS_RELEASE )
set(qt_Qt5_WebSockets_DEFINITIONS_RELEASE "-DQT_WEBSOCKETS_LIB")
set(qt_Qt5_WebSockets_OBJECTS_RELEASE )
set(qt_Qt5_WebSockets_COMPILE_DEFINITIONS_RELEASE "QT_WEBSOCKETS_LIB")
set(qt_Qt5_WebSockets_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_WebSockets_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_WebSockets_LIBS_RELEASE Qt5WebSockets)
set(qt_Qt5_WebSockets_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_WebSockets_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_WebSockets_FRAMEWORKS_RELEASE )
set(qt_Qt5_WebSockets_DEPENDENCIES_RELEASE Qt5::Network Qt5::Core)
set(qt_Qt5_WebSockets_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_WebSockets_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_WebSockets_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_WebSockets_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_WebSockets_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_WebSockets_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_WebSockets_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_WebSockets_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_WebSockets_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_WebSockets_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::Multimedia VARIABLES ############################################

set(qt_Qt5_Multimedia_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtMultimedia")
set(qt_Qt5_Multimedia_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_Multimedia_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_Multimedia_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_Multimedia_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_Multimedia_RES_DIRS_RELEASE )
set(qt_Qt5_Multimedia_DEFINITIONS_RELEASE "-DQT_MULTIMEDIA_LIB")
set(qt_Qt5_Multimedia_OBJECTS_RELEASE )
set(qt_Qt5_Multimedia_COMPILE_DEFINITIONS_RELEASE "QT_MULTIMEDIA_LIB")
set(qt_Qt5_Multimedia_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_Multimedia_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_Multimedia_LIBS_RELEASE Qt5Multimedia)
set(qt_Qt5_Multimedia_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_Multimedia_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_Multimedia_FRAMEWORKS_RELEASE )
set(qt_Qt5_Multimedia_DEPENDENCIES_RELEASE Qt5::Network Qt5::Gui Qt5::Core)
set(qt_Qt5_Multimedia_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_Multimedia_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_Multimedia_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_Multimedia_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_Multimedia_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_Multimedia_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_Multimedia_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_Multimedia_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_Multimedia_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_Multimedia_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::Svg VARIABLES ############################################

set(qt_Qt5_Svg_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtSvg")
set(qt_Qt5_Svg_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_Svg_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_Svg_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_Svg_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_Svg_RES_DIRS_RELEASE )
set(qt_Qt5_Svg_DEFINITIONS_RELEASE "-DQT_SVG_LIB")
set(qt_Qt5_Svg_OBJECTS_RELEASE )
set(qt_Qt5_Svg_COMPILE_DEFINITIONS_RELEASE "QT_SVG_LIB")
set(qt_Qt5_Svg_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_Svg_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_Svg_LIBS_RELEASE Qt5Svg)
set(qt_Qt5_Svg_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_Svg_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_Svg_FRAMEWORKS_RELEASE )
set(qt_Qt5_Svg_DEPENDENCIES_RELEASE Qt5::Gui Qt5::Core)
set(qt_Qt5_Svg_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_Svg_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_Svg_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_Svg_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_Svg_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_Svg_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_Svg_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_Svg_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_Svg_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_Svg_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QuickTest VARIABLES ############################################

set(qt_Qt5_QuickTest_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtQuickTest")
set(qt_Qt5_QuickTest_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_QuickTest_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QuickTest_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QuickTest_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QuickTest_RES_DIRS_RELEASE )
set(qt_Qt5_QuickTest_DEFINITIONS_RELEASE "-DQT_QUICKTEST_LIB")
set(qt_Qt5_QuickTest_OBJECTS_RELEASE )
set(qt_Qt5_QuickTest_COMPILE_DEFINITIONS_RELEASE "QT_QUICKTEST_LIB")
set(qt_Qt5_QuickTest_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QuickTest_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QuickTest_LIBS_RELEASE Qt5QuickTest)
set(qt_Qt5_QuickTest_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QuickTest_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QuickTest_FRAMEWORKS_RELEASE )
set(qt_Qt5_QuickTest_DEPENDENCIES_RELEASE Qt5::Test Qt5::Core)
set(qt_Qt5_QuickTest_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QuickTest_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QuickTest_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QuickTest_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QuickTest_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QuickTest_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QuickTest_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QuickTest_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QuickTest_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QuickTest_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::Qml VARIABLES ############################################

set(qt_Qt5_Qml_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtQml")
set(qt_Qt5_Qml_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_Qml_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_Qml_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_Qml_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_Qml_RES_DIRS_RELEASE )
set(qt_Qt5_Qml_DEFINITIONS_RELEASE "-DQT_QML_LIB")
set(qt_Qt5_Qml_OBJECTS_RELEASE )
set(qt_Qt5_Qml_COMPILE_DEFINITIONS_RELEASE "QT_QML_LIB")
set(qt_Qt5_Qml_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_Qml_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_Qml_LIBS_RELEASE Qt5Qml)
set(qt_Qt5_Qml_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_Qml_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_Qml_FRAMEWORKS_RELEASE )
set(qt_Qt5_Qml_DEPENDENCIES_RELEASE Qt5::Network Qt5::Core)
set(qt_Qt5_Qml_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_Qml_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_Qml_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_Qml_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_Qml_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_Qml_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_Qml_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_Qml_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_Qml_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_Qml_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::OpenGLExtensions VARIABLES ############################################

set(qt_Qt5_OpenGLExtensions_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtOpenGLExtensions")
set(qt_Qt5_OpenGLExtensions_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_OpenGLExtensions_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_OpenGLExtensions_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_OpenGLExtensions_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_OpenGLExtensions_RES_DIRS_RELEASE )
set(qt_Qt5_OpenGLExtensions_DEFINITIONS_RELEASE "-DQT_OPENGLEXTENSIONS_LIB")
set(qt_Qt5_OpenGLExtensions_OBJECTS_RELEASE )
set(qt_Qt5_OpenGLExtensions_COMPILE_DEFINITIONS_RELEASE "QT_OPENGLEXTENSIONS_LIB")
set(qt_Qt5_OpenGLExtensions_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_OpenGLExtensions_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_OpenGLExtensions_LIBS_RELEASE Qt5OpenGLExtensions)
set(qt_Qt5_OpenGLExtensions_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_OpenGLExtensions_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_OpenGLExtensions_FRAMEWORKS_RELEASE )
set(qt_Qt5_OpenGLExtensions_DEPENDENCIES_RELEASE Qt5::Gui Qt5::Core)
set(qt_Qt5_OpenGLExtensions_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_OpenGLExtensions_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_OpenGLExtensions_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_OpenGLExtensions_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_OpenGLExtensions_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_OpenGLExtensions_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_OpenGLExtensions_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_OpenGLExtensions_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_OpenGLExtensions_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_OpenGLExtensions_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::OpenGL VARIABLES ############################################

set(qt_Qt5_OpenGL_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtOpenGL")
set(qt_Qt5_OpenGL_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_OpenGL_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_OpenGL_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_OpenGL_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_OpenGL_RES_DIRS_RELEASE )
set(qt_Qt5_OpenGL_DEFINITIONS_RELEASE "-DQT_OPENGL_LIB")
set(qt_Qt5_OpenGL_OBJECTS_RELEASE )
set(qt_Qt5_OpenGL_COMPILE_DEFINITIONS_RELEASE "QT_OPENGL_LIB")
set(qt_Qt5_OpenGL_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_OpenGL_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_OpenGL_LIBS_RELEASE Qt5OpenGL)
set(qt_Qt5_OpenGL_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_OpenGL_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_OpenGL_FRAMEWORKS_RELEASE )
set(qt_Qt5_OpenGL_DEPENDENCIES_RELEASE Qt5::Gui Qt5::Core)
set(qt_Qt5_OpenGL_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_OpenGL_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_OpenGL_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_OpenGL_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_OpenGL_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_OpenGL_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_OpenGL_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_OpenGL_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_OpenGL_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_OpenGL_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::XkbCommonSupport VARIABLES ############################################

set(qt_Qt5_XkbCommonSupport_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtXkbCommonSupport")
set(qt_Qt5_XkbCommonSupport_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_XkbCommonSupport_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_XkbCommonSupport_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_XkbCommonSupport_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_XkbCommonSupport_RES_DIRS_RELEASE )
set(qt_Qt5_XkbCommonSupport_DEFINITIONS_RELEASE "-DQT_XKBCOMMON_SUPPORT_LIB")
set(qt_Qt5_XkbCommonSupport_OBJECTS_RELEASE )
set(qt_Qt5_XkbCommonSupport_COMPILE_DEFINITIONS_RELEASE "QT_XKBCOMMON_SUPPORT_LIB")
set(qt_Qt5_XkbCommonSupport_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_XkbCommonSupport_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_XkbCommonSupport_LIBS_RELEASE Qt5XkbCommonSupport)
set(qt_Qt5_XkbCommonSupport_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_XkbCommonSupport_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_XkbCommonSupport_FRAMEWORKS_RELEASE )
set(qt_Qt5_XkbCommonSupport_DEPENDENCIES_RELEASE Qt5::Core Qt5::Gui xkbcommon::libxkbcommon-x11)
set(qt_Qt5_XkbCommonSupport_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_XkbCommonSupport_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_XkbCommonSupport_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_XkbCommonSupport_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_XkbCommonSupport_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_XkbCommonSupport_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_XkbCommonSupport_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_XkbCommonSupport_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_XkbCommonSupport_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_XkbCommonSupport_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::ServiceSupport VARIABLES ############################################

set(qt_Qt5_ServiceSupport_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtServiceSupport")
set(qt_Qt5_ServiceSupport_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_ServiceSupport_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_ServiceSupport_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_ServiceSupport_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_ServiceSupport_RES_DIRS_RELEASE )
set(qt_Qt5_ServiceSupport_DEFINITIONS_RELEASE "-DQT_SERVICE_SUPPORT_LIB")
set(qt_Qt5_ServiceSupport_OBJECTS_RELEASE )
set(qt_Qt5_ServiceSupport_COMPILE_DEFINITIONS_RELEASE "QT_SERVICE_SUPPORT_LIB")
set(qt_Qt5_ServiceSupport_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_ServiceSupport_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_ServiceSupport_LIBS_RELEASE Qt5ServiceSupport)
set(qt_Qt5_ServiceSupport_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_ServiceSupport_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_ServiceSupport_FRAMEWORKS_RELEASE )
set(qt_Qt5_ServiceSupport_DEPENDENCIES_RELEASE Qt5::Core Qt5::Gui)
set(qt_Qt5_ServiceSupport_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_ServiceSupport_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_ServiceSupport_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_ServiceSupport_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_ServiceSupport_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_ServiceSupport_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_ServiceSupport_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_ServiceSupport_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_ServiceSupport_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_ServiceSupport_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::Widgets VARIABLES ############################################

set(qt_Qt5_Widgets_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtWidgets")
set(qt_Qt5_Widgets_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_Widgets_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_Widgets_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_Widgets_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_Widgets_RES_DIRS_RELEASE )
set(qt_Qt5_Widgets_DEFINITIONS_RELEASE "-DQT_WIDGETS_LIB")
set(qt_Qt5_Widgets_OBJECTS_RELEASE )
set(qt_Qt5_Widgets_COMPILE_DEFINITIONS_RELEASE "QT_WIDGETS_LIB")
set(qt_Qt5_Widgets_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_Widgets_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_Widgets_LIBS_RELEASE Qt5Widgets)
set(qt_Qt5_Widgets_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_Widgets_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_Widgets_FRAMEWORKS_RELEASE )
set(qt_Qt5_Widgets_DEPENDENCIES_RELEASE Qt5::Gui Qt5::Core)
set(qt_Qt5_Widgets_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_Widgets_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_Widgets_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_Widgets_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_Widgets_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_Widgets_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_Widgets_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_Widgets_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_Widgets_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_Widgets_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::AccessibilitySupport VARIABLES ############################################

set(qt_Qt5_AccessibilitySupport_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtAccessibilitySupport")
set(qt_Qt5_AccessibilitySupport_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_AccessibilitySupport_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_AccessibilitySupport_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_AccessibilitySupport_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_AccessibilitySupport_RES_DIRS_RELEASE )
set(qt_Qt5_AccessibilitySupport_DEFINITIONS_RELEASE "-DQT_ACCESSIBILITY_SUPPORT_LIB")
set(qt_Qt5_AccessibilitySupport_OBJECTS_RELEASE )
set(qt_Qt5_AccessibilitySupport_COMPILE_DEFINITIONS_RELEASE "QT_ACCESSIBILITY_SUPPORT_LIB")
set(qt_Qt5_AccessibilitySupport_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_AccessibilitySupport_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_AccessibilitySupport_LIBS_RELEASE Qt5AccessibilitySupport)
set(qt_Qt5_AccessibilitySupport_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_AccessibilitySupport_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_AccessibilitySupport_FRAMEWORKS_RELEASE )
set(qt_Qt5_AccessibilitySupport_DEPENDENCIES_RELEASE Qt5::Core Qt5::Gui)
set(qt_Qt5_AccessibilitySupport_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_AccessibilitySupport_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_AccessibilitySupport_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_AccessibilitySupport_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_AccessibilitySupport_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_AccessibilitySupport_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_AccessibilitySupport_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_AccessibilitySupport_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_AccessibilitySupport_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_AccessibilitySupport_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::ThemeSupport VARIABLES ############################################

set(qt_Qt5_ThemeSupport_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtThemeSupport")
set(qt_Qt5_ThemeSupport_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_ThemeSupport_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_ThemeSupport_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_ThemeSupport_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_ThemeSupport_RES_DIRS_RELEASE )
set(qt_Qt5_ThemeSupport_DEFINITIONS_RELEASE "-DQT_THEME_SUPPORT_LIB")
set(qt_Qt5_ThemeSupport_OBJECTS_RELEASE )
set(qt_Qt5_ThemeSupport_COMPILE_DEFINITIONS_RELEASE "QT_THEME_SUPPORT_LIB")
set(qt_Qt5_ThemeSupport_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_ThemeSupport_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_ThemeSupport_LIBS_RELEASE Qt5ThemeSupport)
set(qt_Qt5_ThemeSupport_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_ThemeSupport_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_ThemeSupport_FRAMEWORKS_RELEASE )
set(qt_Qt5_ThemeSupport_DEPENDENCIES_RELEASE Qt5::Core Qt5::Gui)
set(qt_Qt5_ThemeSupport_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_ThemeSupport_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_ThemeSupport_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_ThemeSupport_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_ThemeSupport_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_ThemeSupport_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_ThemeSupport_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_ThemeSupport_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_ThemeSupport_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_ThemeSupport_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::FontDatabaseSupport VARIABLES ############################################

set(qt_Qt5_FontDatabaseSupport_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtFontDatabaseSupport")
set(qt_Qt5_FontDatabaseSupport_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_FontDatabaseSupport_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_FontDatabaseSupport_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_FontDatabaseSupport_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_FontDatabaseSupport_RES_DIRS_RELEASE )
set(qt_Qt5_FontDatabaseSupport_DEFINITIONS_RELEASE "-DQT_FONTDATABASE_SUPPORT_LIB")
set(qt_Qt5_FontDatabaseSupport_OBJECTS_RELEASE )
set(qt_Qt5_FontDatabaseSupport_COMPILE_DEFINITIONS_RELEASE "QT_FONTDATABASE_SUPPORT_LIB")
set(qt_Qt5_FontDatabaseSupport_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_FontDatabaseSupport_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_FontDatabaseSupport_LIBS_RELEASE Qt5FontDatabaseSupport)
set(qt_Qt5_FontDatabaseSupport_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_FontDatabaseSupport_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_FontDatabaseSupport_FRAMEWORKS_RELEASE )
set(qt_Qt5_FontDatabaseSupport_DEPENDENCIES_RELEASE Qt5::Core Qt5::Gui Fontconfig::Fontconfig)
set(qt_Qt5_FontDatabaseSupport_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_FontDatabaseSupport_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_FontDatabaseSupport_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_FontDatabaseSupport_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_FontDatabaseSupport_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_FontDatabaseSupport_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_FontDatabaseSupport_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_FontDatabaseSupport_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_FontDatabaseSupport_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_FontDatabaseSupport_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::EventDispatcherSupport VARIABLES ############################################

set(qt_Qt5_EventDispatcherSupport_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtEventDispatcherSupport")
set(qt_Qt5_EventDispatcherSupport_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_EventDispatcherSupport_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_EventDispatcherSupport_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_EventDispatcherSupport_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_EventDispatcherSupport_RES_DIRS_RELEASE )
set(qt_Qt5_EventDispatcherSupport_DEFINITIONS_RELEASE "-DQT_EVENTDISPATCHER_SUPPORT_LIB")
set(qt_Qt5_EventDispatcherSupport_OBJECTS_RELEASE )
set(qt_Qt5_EventDispatcherSupport_COMPILE_DEFINITIONS_RELEASE "QT_EVENTDISPATCHER_SUPPORT_LIB")
set(qt_Qt5_EventDispatcherSupport_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_EventDispatcherSupport_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_EventDispatcherSupport_LIBS_RELEASE Qt5EventDispatcherSupport)
set(qt_Qt5_EventDispatcherSupport_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_EventDispatcherSupport_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_EventDispatcherSupport_FRAMEWORKS_RELEASE )
set(qt_Qt5_EventDispatcherSupport_DEPENDENCIES_RELEASE Qt5::Core Qt5::Gui)
set(qt_Qt5_EventDispatcherSupport_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_EventDispatcherSupport_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_EventDispatcherSupport_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_EventDispatcherSupport_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_EventDispatcherSupport_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_EventDispatcherSupport_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_EventDispatcherSupport_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_EventDispatcherSupport_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_EventDispatcherSupport_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_EventDispatcherSupport_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QM3uPlaylistPlugin VARIABLES ############################################

set(qt_Qt5_QM3uPlaylistPlugin_INCLUDE_DIRS_RELEASE )
set(qt_Qt5_QM3uPlaylistPlugin_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/plugins/playlistformats")
set(qt_Qt5_QM3uPlaylistPlugin_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QM3uPlaylistPlugin_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QM3uPlaylistPlugin_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QM3uPlaylistPlugin_RES_DIRS_RELEASE )
set(qt_Qt5_QM3uPlaylistPlugin_DEFINITIONS_RELEASE )
set(qt_Qt5_QM3uPlaylistPlugin_OBJECTS_RELEASE )
set(qt_Qt5_QM3uPlaylistPlugin_COMPILE_DEFINITIONS_RELEASE )
set(qt_Qt5_QM3uPlaylistPlugin_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QM3uPlaylistPlugin_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QM3uPlaylistPlugin_LIBS_RELEASE )
set(qt_Qt5_QM3uPlaylistPlugin_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QM3uPlaylistPlugin_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QM3uPlaylistPlugin_FRAMEWORKS_RELEASE )
set(qt_Qt5_QM3uPlaylistPlugin_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_QM3uPlaylistPlugin_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QM3uPlaylistPlugin_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QM3uPlaylistPlugin_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QM3uPlaylistPlugin_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QM3uPlaylistPlugin_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QM3uPlaylistPlugin_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QM3uPlaylistPlugin_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QM3uPlaylistPlugin_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QM3uPlaylistPlugin_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QM3uPlaylistPlugin_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QGeoPositionInfoSourceFactorySerialNmea VARIABLES ############################################

set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_INCLUDE_DIRS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/plugins/position")
set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_RES_DIRS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_DEFINITIONS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_OBJECTS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_COMPILE_DEFINITIONS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_LIBS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_FRAMEWORKS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QGeoPositionInfoSourceFactoryPoll VARIABLES ############################################

set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_INCLUDE_DIRS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/plugins/position")
set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_RES_DIRS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_DEFINITIONS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_OBJECTS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_COMPILE_DEFINITIONS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_LIBS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_FRAMEWORKS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QGeoPositionInfoSourceFactoryGeoclue2 VARIABLES ############################################

set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_INCLUDE_DIRS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/plugins/position")
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_RES_DIRS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_DEFINITIONS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_OBJECTS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_COMPILE_DEFINITIONS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_LIBS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_FRAMEWORKS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QGeoPositionInfoSourceFactoryGeoclue VARIABLES ############################################

set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_INCLUDE_DIRS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/plugins/position")
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_RES_DIRS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_DEFINITIONS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_OBJECTS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_COMPILE_DEFINITIONS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_LIBS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_FRAMEWORKS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QGeoServiceProviderFactoryOsm VARIABLES ############################################

set(qt_Qt5_QGeoServiceProviderFactoryOsm_INCLUDE_DIRS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryOsm_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/plugins/geoservices")
set(qt_Qt5_QGeoServiceProviderFactoryOsm_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QGeoServiceProviderFactoryOsm_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QGeoServiceProviderFactoryOsm_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QGeoServiceProviderFactoryOsm_RES_DIRS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryOsm_DEFINITIONS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryOsm_OBJECTS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryOsm_COMPILE_DEFINITIONS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryOsm_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QGeoServiceProviderFactoryOsm_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QGeoServiceProviderFactoryOsm_LIBS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryOsm_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryOsm_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryOsm_FRAMEWORKS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryOsm_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_QGeoServiceProviderFactoryOsm_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryOsm_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryOsm_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QGeoServiceProviderFactoryOsm_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QGeoServiceProviderFactoryOsm_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QGeoServiceProviderFactoryOsm_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QGeoServiceProviderFactoryOsm_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QGeoServiceProviderFactoryOsm_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QGeoServiceProviderFactoryOsm_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QGeoServiceProviderFactoryOsm_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QGeoServiceProviderFactoryNokia VARIABLES ############################################

set(qt_Qt5_QGeoServiceProviderFactoryNokia_INCLUDE_DIRS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryNokia_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/plugins/geoservices")
set(qt_Qt5_QGeoServiceProviderFactoryNokia_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QGeoServiceProviderFactoryNokia_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QGeoServiceProviderFactoryNokia_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QGeoServiceProviderFactoryNokia_RES_DIRS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryNokia_DEFINITIONS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryNokia_OBJECTS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryNokia_COMPILE_DEFINITIONS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryNokia_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QGeoServiceProviderFactoryNokia_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QGeoServiceProviderFactoryNokia_LIBS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryNokia_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryNokia_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryNokia_FRAMEWORKS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryNokia_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_QGeoServiceProviderFactoryNokia_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryNokia_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryNokia_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QGeoServiceProviderFactoryNokia_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QGeoServiceProviderFactoryNokia_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QGeoServiceProviderFactoryNokia_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QGeoServiceProviderFactoryNokia_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QGeoServiceProviderFactoryNokia_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QGeoServiceProviderFactoryNokia_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QGeoServiceProviderFactoryNokia_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QGeoServiceProviderFactoryItemsOverlay VARIABLES ############################################

set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_INCLUDE_DIRS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/plugins/geoservices")
set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_RES_DIRS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_DEFINITIONS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_OBJECTS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_COMPILE_DEFINITIONS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_LIBS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_FRAMEWORKS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::GeoServiceProviderFactoryEsri VARIABLES ############################################

set(qt_Qt5_GeoServiceProviderFactoryEsri_INCLUDE_DIRS_RELEASE )
set(qt_Qt5_GeoServiceProviderFactoryEsri_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/plugins/geoservices")
set(qt_Qt5_GeoServiceProviderFactoryEsri_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_GeoServiceProviderFactoryEsri_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_GeoServiceProviderFactoryEsri_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_GeoServiceProviderFactoryEsri_RES_DIRS_RELEASE )
set(qt_Qt5_GeoServiceProviderFactoryEsri_DEFINITIONS_RELEASE )
set(qt_Qt5_GeoServiceProviderFactoryEsri_OBJECTS_RELEASE )
set(qt_Qt5_GeoServiceProviderFactoryEsri_COMPILE_DEFINITIONS_RELEASE )
set(qt_Qt5_GeoServiceProviderFactoryEsri_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_GeoServiceProviderFactoryEsri_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_GeoServiceProviderFactoryEsri_LIBS_RELEASE )
set(qt_Qt5_GeoServiceProviderFactoryEsri_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_GeoServiceProviderFactoryEsri_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_GeoServiceProviderFactoryEsri_FRAMEWORKS_RELEASE )
set(qt_Qt5_GeoServiceProviderFactoryEsri_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_GeoServiceProviderFactoryEsri_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_GeoServiceProviderFactoryEsri_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_GeoServiceProviderFactoryEsri_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_GeoServiceProviderFactoryEsri_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_GeoServiceProviderFactoryEsri_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_GeoServiceProviderFactoryEsri_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_GeoServiceProviderFactoryEsri_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_GeoServiceProviderFactoryEsri_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_GeoServiceProviderFactoryEsri_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_GeoServiceProviderFactoryEsri_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QGeoServiceProviderFactoryMapboxGL VARIABLES ############################################

set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_INCLUDE_DIRS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/plugins/geoservices")
set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_RES_DIRS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_DEFINITIONS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_OBJECTS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_COMPILE_DEFINITIONS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_LIBS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_FRAMEWORKS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QGeoServiceProviderFactoryMapbox VARIABLES ############################################

set(qt_Qt5_QGeoServiceProviderFactoryMapbox_INCLUDE_DIRS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryMapbox_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/plugins/geoservices")
set(qt_Qt5_QGeoServiceProviderFactoryMapbox_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QGeoServiceProviderFactoryMapbox_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QGeoServiceProviderFactoryMapbox_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QGeoServiceProviderFactoryMapbox_RES_DIRS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryMapbox_DEFINITIONS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryMapbox_OBJECTS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryMapbox_COMPILE_DEFINITIONS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryMapbox_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QGeoServiceProviderFactoryMapbox_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QGeoServiceProviderFactoryMapbox_LIBS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryMapbox_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryMapbox_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryMapbox_FRAMEWORKS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryMapbox_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_QGeoServiceProviderFactoryMapbox_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryMapbox_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QGeoServiceProviderFactoryMapbox_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QGeoServiceProviderFactoryMapbox_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QGeoServiceProviderFactoryMapbox_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QGeoServiceProviderFactoryMapbox_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QGeoServiceProviderFactoryMapbox_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QGeoServiceProviderFactoryMapbox_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QGeoServiceProviderFactoryMapbox_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QGeoServiceProviderFactoryMapbox_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::Positioning VARIABLES ############################################

set(qt_Qt5_Positioning_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtPositioning")
set(qt_Qt5_Positioning_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_Positioning_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_Positioning_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_Positioning_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_Positioning_RES_DIRS_RELEASE )
set(qt_Qt5_Positioning_DEFINITIONS_RELEASE "-DQT_POSITIONING_LIB")
set(qt_Qt5_Positioning_OBJECTS_RELEASE )
set(qt_Qt5_Positioning_COMPILE_DEFINITIONS_RELEASE "QT_POSITIONING_LIB")
set(qt_Qt5_Positioning_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_Positioning_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_Positioning_LIBS_RELEASE Qt5Positioning)
set(qt_Qt5_Positioning_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_Positioning_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_Positioning_FRAMEWORKS_RELEASE )
set(qt_Qt5_Positioning_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_Positioning_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_Positioning_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_Positioning_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_Positioning_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_Positioning_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_Positioning_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_Positioning_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_Positioning_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_Positioning_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_Positioning_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QSvgPlugin VARIABLES ############################################

set(qt_Qt5_QSvgPlugin_INCLUDE_DIRS_RELEASE )
set(qt_Qt5_QSvgPlugin_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/plugins/imageformats")
set(qt_Qt5_QSvgPlugin_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QSvgPlugin_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QSvgPlugin_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QSvgPlugin_RES_DIRS_RELEASE )
set(qt_Qt5_QSvgPlugin_DEFINITIONS_RELEASE )
set(qt_Qt5_QSvgPlugin_OBJECTS_RELEASE )
set(qt_Qt5_QSvgPlugin_COMPILE_DEFINITIONS_RELEASE )
set(qt_Qt5_QSvgPlugin_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QSvgPlugin_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QSvgPlugin_LIBS_RELEASE )
set(qt_Qt5_QSvgPlugin_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QSvgPlugin_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QSvgPlugin_FRAMEWORKS_RELEASE )
set(qt_Qt5_QSvgPlugin_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_QSvgPlugin_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QSvgPlugin_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QSvgPlugin_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QSvgPlugin_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QSvgPlugin_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QSvgPlugin_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QSvgPlugin_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QSvgPlugin_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QSvgPlugin_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QSvgPlugin_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QSvgIconPlugin VARIABLES ############################################

set(qt_Qt5_QSvgIconPlugin_INCLUDE_DIRS_RELEASE )
set(qt_Qt5_QSvgIconPlugin_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/plugins/iconengines")
set(qt_Qt5_QSvgIconPlugin_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QSvgIconPlugin_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QSvgIconPlugin_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QSvgIconPlugin_RES_DIRS_RELEASE )
set(qt_Qt5_QSvgIconPlugin_DEFINITIONS_RELEASE )
set(qt_Qt5_QSvgIconPlugin_OBJECTS_RELEASE )
set(qt_Qt5_QSvgIconPlugin_COMPILE_DEFINITIONS_RELEASE )
set(qt_Qt5_QSvgIconPlugin_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QSvgIconPlugin_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QSvgIconPlugin_LIBS_RELEASE )
set(qt_Qt5_QSvgIconPlugin_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QSvgIconPlugin_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QSvgIconPlugin_FRAMEWORKS_RELEASE )
set(qt_Qt5_QSvgIconPlugin_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_QSvgIconPlugin_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QSvgIconPlugin_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QSvgIconPlugin_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QSvgIconPlugin_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QSvgIconPlugin_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QSvgIconPlugin_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QSvgIconPlugin_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QSvgIconPlugin_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QSvgIconPlugin_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QSvgIconPlugin_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::Xml VARIABLES ############################################

set(qt_Qt5_Xml_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtXml")
set(qt_Qt5_Xml_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_Xml_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_Xml_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_Xml_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_Xml_RES_DIRS_RELEASE )
set(qt_Qt5_Xml_DEFINITIONS_RELEASE "-DQT_XML_LIB")
set(qt_Qt5_Xml_OBJECTS_RELEASE )
set(qt_Qt5_Xml_COMPILE_DEFINITIONS_RELEASE "QT_XML_LIB")
set(qt_Qt5_Xml_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_Xml_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_Xml_LIBS_RELEASE Qt5Xml)
set(qt_Qt5_Xml_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_Xml_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_Xml_FRAMEWORKS_RELEASE )
set(qt_Qt5_Xml_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_Xml_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_Xml_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_Xml_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_Xml_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_Xml_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_Xml_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_Xml_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_Xml_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_Xml_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_Xml_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::Concurrent VARIABLES ############################################

set(qt_Qt5_Concurrent_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtConcurrent")
set(qt_Qt5_Concurrent_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_Concurrent_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_Concurrent_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_Concurrent_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_Concurrent_RES_DIRS_RELEASE )
set(qt_Qt5_Concurrent_DEFINITIONS_RELEASE "-DQT_CONCURRENT_LIB")
set(qt_Qt5_Concurrent_OBJECTS_RELEASE )
set(qt_Qt5_Concurrent_COMPILE_DEFINITIONS_RELEASE "QT_CONCURRENT_LIB")
set(qt_Qt5_Concurrent_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_Concurrent_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_Concurrent_LIBS_RELEASE Qt5Concurrent)
set(qt_Qt5_Concurrent_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_Concurrent_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_Concurrent_FRAMEWORKS_RELEASE )
set(qt_Qt5_Concurrent_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_Concurrent_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_Concurrent_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_Concurrent_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_Concurrent_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_Concurrent_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_Concurrent_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_Concurrent_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_Concurrent_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_Concurrent_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_Concurrent_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::Test VARIABLES ############################################

set(qt_Qt5_Test_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtTest")
set(qt_Qt5_Test_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_Test_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_Test_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_Test_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_Test_RES_DIRS_RELEASE )
set(qt_Qt5_Test_DEFINITIONS_RELEASE "-DQT_TESTLIB_LIB")
set(qt_Qt5_Test_OBJECTS_RELEASE )
set(qt_Qt5_Test_COMPILE_DEFINITIONS_RELEASE "QT_TESTLIB_LIB")
set(qt_Qt5_Test_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_Test_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_Test_LIBS_RELEASE Qt5Test)
set(qt_Qt5_Test_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_Test_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_Test_FRAMEWORKS_RELEASE )
set(qt_Qt5_Test_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_Test_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_Test_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_Test_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_Test_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_Test_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_Test_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_Test_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_Test_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_Test_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_Test_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::Sql VARIABLES ############################################

set(qt_Qt5_Sql_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtSql")
set(qt_Qt5_Sql_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_Sql_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_Sql_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_Sql_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_Sql_RES_DIRS_RELEASE )
set(qt_Qt5_Sql_DEFINITIONS_RELEASE "-DQT_SQL_LIB")
set(qt_Qt5_Sql_OBJECTS_RELEASE )
set(qt_Qt5_Sql_COMPILE_DEFINITIONS_RELEASE "QT_SQL_LIB")
set(qt_Qt5_Sql_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_Sql_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_Sql_LIBS_RELEASE Qt5Sql)
set(qt_Qt5_Sql_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_Sql_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_Sql_FRAMEWORKS_RELEASE )
set(qt_Qt5_Sql_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_Sql_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_Sql_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_Sql_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_Sql_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_Sql_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_Sql_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_Sql_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_Sql_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_Sql_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_Sql_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::Network VARIABLES ############################################

set(qt_Qt5_Network_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtNetwork")
set(qt_Qt5_Network_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_Network_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_Network_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_Network_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_Network_RES_DIRS_RELEASE )
set(qt_Qt5_Network_DEFINITIONS_RELEASE "-DQT_NETWORK_LIB")
set(qt_Qt5_Network_OBJECTS_RELEASE )
set(qt_Qt5_Network_COMPILE_DEFINITIONS_RELEASE "QT_NETWORK_LIB")
set(qt_Qt5_Network_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_Network_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_Network_LIBS_RELEASE Qt5Network)
set(qt_Qt5_Network_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_Network_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_Network_FRAMEWORKS_RELEASE )
set(qt_Qt5_Network_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_Network_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_Network_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_Network_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_Network_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_Network_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_Network_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_Network_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_Network_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_Network_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_Network_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QODBCDriverPlugin VARIABLES ############################################

set(qt_Qt5_QODBCDriverPlugin_INCLUDE_DIRS_RELEASE )
set(qt_Qt5_QODBCDriverPlugin_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/plugins/sqldrivers")
set(qt_Qt5_QODBCDriverPlugin_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QODBCDriverPlugin_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QODBCDriverPlugin_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QODBCDriverPlugin_RES_DIRS_RELEASE )
set(qt_Qt5_QODBCDriverPlugin_DEFINITIONS_RELEASE )
set(qt_Qt5_QODBCDriverPlugin_OBJECTS_RELEASE )
set(qt_Qt5_QODBCDriverPlugin_COMPILE_DEFINITIONS_RELEASE )
set(qt_Qt5_QODBCDriverPlugin_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QODBCDriverPlugin_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QODBCDriverPlugin_LIBS_RELEASE )
set(qt_Qt5_QODBCDriverPlugin_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QODBCDriverPlugin_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QODBCDriverPlugin_FRAMEWORKS_RELEASE )
set(qt_Qt5_QODBCDriverPlugin_DEPENDENCIES_RELEASE ODBC::ODBC Qt5::Core)
set(qt_Qt5_QODBCDriverPlugin_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QODBCDriverPlugin_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QODBCDriverPlugin_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QODBCDriverPlugin_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QODBCDriverPlugin_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QODBCDriverPlugin_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QODBCDriverPlugin_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QODBCDriverPlugin_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QODBCDriverPlugin_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QODBCDriverPlugin_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QMySQLDriverPlugin VARIABLES ############################################

set(qt_Qt5_QMySQLDriverPlugin_INCLUDE_DIRS_RELEASE )
set(qt_Qt5_QMySQLDriverPlugin_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/plugins/sqldrivers")
set(qt_Qt5_QMySQLDriverPlugin_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QMySQLDriverPlugin_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QMySQLDriverPlugin_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QMySQLDriverPlugin_RES_DIRS_RELEASE )
set(qt_Qt5_QMySQLDriverPlugin_DEFINITIONS_RELEASE )
set(qt_Qt5_QMySQLDriverPlugin_OBJECTS_RELEASE )
set(qt_Qt5_QMySQLDriverPlugin_COMPILE_DEFINITIONS_RELEASE )
set(qt_Qt5_QMySQLDriverPlugin_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QMySQLDriverPlugin_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QMySQLDriverPlugin_LIBS_RELEASE )
set(qt_Qt5_QMySQLDriverPlugin_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QMySQLDriverPlugin_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QMySQLDriverPlugin_FRAMEWORKS_RELEASE )
set(qt_Qt5_QMySQLDriverPlugin_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_QMySQLDriverPlugin_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QMySQLDriverPlugin_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QMySQLDriverPlugin_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QMySQLDriverPlugin_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QMySQLDriverPlugin_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QMySQLDriverPlugin_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QMySQLDriverPlugin_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QMySQLDriverPlugin_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QMySQLDriverPlugin_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QMySQLDriverPlugin_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QPSQLDriverPlugin VARIABLES ############################################

set(qt_Qt5_QPSQLDriverPlugin_INCLUDE_DIRS_RELEASE )
set(qt_Qt5_QPSQLDriverPlugin_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/plugins/sqldrivers")
set(qt_Qt5_QPSQLDriverPlugin_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QPSQLDriverPlugin_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QPSQLDriverPlugin_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QPSQLDriverPlugin_RES_DIRS_RELEASE )
set(qt_Qt5_QPSQLDriverPlugin_DEFINITIONS_RELEASE )
set(qt_Qt5_QPSQLDriverPlugin_OBJECTS_RELEASE )
set(qt_Qt5_QPSQLDriverPlugin_COMPILE_DEFINITIONS_RELEASE )
set(qt_Qt5_QPSQLDriverPlugin_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QPSQLDriverPlugin_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QPSQLDriverPlugin_LIBS_RELEASE )
set(qt_Qt5_QPSQLDriverPlugin_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QPSQLDriverPlugin_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QPSQLDriverPlugin_FRAMEWORKS_RELEASE )
set(qt_Qt5_QPSQLDriverPlugin_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_QPSQLDriverPlugin_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QPSQLDriverPlugin_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QPSQLDriverPlugin_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QPSQLDriverPlugin_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QPSQLDriverPlugin_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QPSQLDriverPlugin_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QPSQLDriverPlugin_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QPSQLDriverPlugin_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QPSQLDriverPlugin_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QPSQLDriverPlugin_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::QSQLiteDriverPlugin VARIABLES ############################################

set(qt_Qt5_QSQLiteDriverPlugin_INCLUDE_DIRS_RELEASE )
set(qt_Qt5_QSQLiteDriverPlugin_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/plugins/sqldrivers")
set(qt_Qt5_QSQLiteDriverPlugin_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_QSQLiteDriverPlugin_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_QSQLiteDriverPlugin_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_QSQLiteDriverPlugin_RES_DIRS_RELEASE )
set(qt_Qt5_QSQLiteDriverPlugin_DEFINITIONS_RELEASE )
set(qt_Qt5_QSQLiteDriverPlugin_OBJECTS_RELEASE )
set(qt_Qt5_QSQLiteDriverPlugin_COMPILE_DEFINITIONS_RELEASE )
set(qt_Qt5_QSQLiteDriverPlugin_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_QSQLiteDriverPlugin_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_QSQLiteDriverPlugin_LIBS_RELEASE )
set(qt_Qt5_QSQLiteDriverPlugin_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_QSQLiteDriverPlugin_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_QSQLiteDriverPlugin_FRAMEWORKS_RELEASE )
set(qt_Qt5_QSQLiteDriverPlugin_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_QSQLiteDriverPlugin_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_QSQLiteDriverPlugin_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_QSQLiteDriverPlugin_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_QSQLiteDriverPlugin_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_QSQLiteDriverPlugin_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_QSQLiteDriverPlugin_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_QSQLiteDriverPlugin_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_QSQLiteDriverPlugin_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_QSQLiteDriverPlugin_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_QSQLiteDriverPlugin_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::EdidSupport VARIABLES ############################################

set(qt_Qt5_EdidSupport_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtEdidSupport")
set(qt_Qt5_EdidSupport_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_EdidSupport_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_EdidSupport_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_EdidSupport_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_EdidSupport_RES_DIRS_RELEASE )
set(qt_Qt5_EdidSupport_DEFINITIONS_RELEASE "-DQT_EDID_SUPPORT_LIB")
set(qt_Qt5_EdidSupport_OBJECTS_RELEASE )
set(qt_Qt5_EdidSupport_COMPILE_DEFINITIONS_RELEASE "QT_EDID_SUPPORT_LIB")
set(qt_Qt5_EdidSupport_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_EdidSupport_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_EdidSupport_LIBS_RELEASE Qt5EdidSupport)
set(qt_Qt5_EdidSupport_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_EdidSupport_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_EdidSupport_FRAMEWORKS_RELEASE )
set(qt_Qt5_EdidSupport_DEPENDENCIES_RELEASE Qt5::Core)
set(qt_Qt5_EdidSupport_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_EdidSupport_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_EdidSupport_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_EdidSupport_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_EdidSupport_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_EdidSupport_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_EdidSupport_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_EdidSupport_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_EdidSupport_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_EdidSupport_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::Gui VARIABLES ############################################

set(qt_Qt5_Gui_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtGui")
set(qt_Qt5_Gui_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_Gui_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_Gui_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_Gui_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_Gui_RES_DIRS_RELEASE )
set(qt_Qt5_Gui_DEFINITIONS_RELEASE "-DQT_GUI_LIB")
set(qt_Qt5_Gui_OBJECTS_RELEASE )
set(qt_Qt5_Gui_COMPILE_DEFINITIONS_RELEASE "QT_GUI_LIB")
set(qt_Qt5_Gui_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_Gui_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_Qt5_Gui_LIBS_RELEASE Qt5Gui)
set(qt_Qt5_Gui_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_Gui_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_Gui_FRAMEWORKS_RELEASE )
set(qt_Qt5_Gui_DEPENDENCIES_RELEASE Fontconfig::Fontconfig xkbcommon::xkbcommon xorg::xorg opengl::opengl Qt5::Core)
set(qt_Qt5_Gui_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_Gui_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_Gui_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_Gui_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_Gui_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_Gui_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_Gui_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_Gui_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_Gui_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_Gui_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT qt::WebEngineCore VARIABLES ############################################

set(qt_qt_WebEngineCore_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include")
set(qt_qt_WebEngineCore_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_qt_WebEngineCore_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_qt_WebEngineCore_LIBRARY_TYPE_RELEASE SHARED)
set(qt_qt_WebEngineCore_IS_HOST_WINDOWS_RELEASE 0)
set(qt_qt_WebEngineCore_RES_DIRS_RELEASE )
set(qt_qt_WebEngineCore_DEFINITIONS_RELEASE )
set(qt_qt_WebEngineCore_OBJECTS_RELEASE )
set(qt_qt_WebEngineCore_COMPILE_DEFINITIONS_RELEASE )
set(qt_qt_WebEngineCore_COMPILE_OPTIONS_C_RELEASE "")
set(qt_qt_WebEngineCore_COMPILE_OPTIONS_CXX_RELEASE "")
set(qt_qt_WebEngineCore_LIBS_RELEASE )
set(qt_qt_WebEngineCore_SYSTEM_LIBS_RELEASE resolv)
set(qt_qt_WebEngineCore_FRAMEWORK_DIRS_RELEASE )
set(qt_qt_WebEngineCore_FRAMEWORKS_RELEASE )
set(qt_qt_WebEngineCore_DEPENDENCIES_RELEASE )
set(qt_qt_WebEngineCore_SHARED_LINK_FLAGS_RELEASE )
set(qt_qt_WebEngineCore_EXE_LINK_FLAGS_RELEASE )
set(qt_qt_WebEngineCore_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_qt_WebEngineCore_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_qt_WebEngineCore_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_qt_WebEngineCore_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_qt_WebEngineCore_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_qt_WebEngineCore_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_qt_WebEngineCore_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_qt_WebEngineCore_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT Qt5::Core VARIABLES ############################################

set(qt_Qt5_Core_INCLUDE_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/include"
			"${qt_PACKAGE_FOLDER_RELEASE}/include/QtCore"
			"${qt_PACKAGE_FOLDER_RELEASE}/mkspecs/linux-g++")
set(qt_Qt5_Core_LIB_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/lib")
set(qt_Qt5_Core_BIN_DIRS_RELEASE "${qt_PACKAGE_FOLDER_RELEASE}/bin")
set(qt_Qt5_Core_LIBRARY_TYPE_RELEASE SHARED)
set(qt_Qt5_Core_IS_HOST_WINDOWS_RELEASE 0)
set(qt_Qt5_Core_RES_DIRS_RELEASE )
set(qt_Qt5_Core_DEFINITIONS_RELEASE "-DQT_CORE_LIB")
set(qt_Qt5_Core_OBJECTS_RELEASE )
set(qt_Qt5_Core_COMPILE_DEFINITIONS_RELEASE "QT_CORE_LIB")
set(qt_Qt5_Core_COMPILE_OPTIONS_C_RELEASE "")
set(qt_Qt5_Core_COMPILE_OPTIONS_CXX_RELEASE "-fPIC")
set(qt_Qt5_Core_LIBS_RELEASE Qt5Core)
set(qt_Qt5_Core_SYSTEM_LIBS_RELEASE )
set(qt_Qt5_Core_FRAMEWORK_DIRS_RELEASE )
set(qt_Qt5_Core_FRAMEWORKS_RELEASE )
set(qt_Qt5_Core_DEPENDENCIES_RELEASE )
set(qt_Qt5_Core_SHARED_LINK_FLAGS_RELEASE )
set(qt_Qt5_Core_EXE_LINK_FLAGS_RELEASE )
set(qt_Qt5_Core_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(qt_Qt5_Core_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${qt_Qt5_Core_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${qt_Qt5_Core_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${qt_Qt5_Core_EXE_LINK_FLAGS_RELEASE}>
)
set(qt_Qt5_Core_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${qt_Qt5_Core_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${qt_Qt5_Core_COMPILE_OPTIONS_C_RELEASE}>")