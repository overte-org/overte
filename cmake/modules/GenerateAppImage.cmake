#
#  GenerateAppImage.cmake
#  cmake/modules
#
#  This gets called by cmake/macros/GenerateInstaller.cmake and is intended to package Interface AppImages only.
#
#  Created by Julian Groß on 2025-04-12.
#  Copyright 2025 Overte e.V.
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#

set(APPIMAGE_DESKTOP_FILE ${CPACK_CMAKE_SOURCE_DIR}/interface/org.overte.interface.desktop)
set(APPIMAGE_ICON_FILE ${CPACK_CMAKE_SOURCE_DIR}/interface/icon/interface.svg)

find_program(LINUXDEPLOY_EXECUTABLE
  NAMES linuxdeploy linuxdeploy-${CMAKE_SYSTEM_PROCESSOR}.AppImage
  PATHS ${CPACK_PACKAGE_DIRECTORY}/linuxdeploy)

if (NOT LINUXDEPLOY_EXECUTABLE)
  message(STATUS "Downloading linuxdeploy")
  set(LINUXDEPLOY_EXECUTABLE ${CPACK_PACKAGE_DIRECTORY}/linuxdeploy/linuxdeploy)
  file(DOWNLOAD
      https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-${CMAKE_SYSTEM_PROCESSOR}.AppImage
      ${LINUXDEPLOY_EXECUTABLE}
      LOG ${CPACK_PACKAGE_DIRECTORY}/linuxdeploy/linuxdeploy-download.log
      STATUS LINUXDEPLOY_DOWNLOAD)
  execute_process(COMMAND chmod +x ${LINUXDEPLOY_EXECUTABLE} COMMAND_ECHO STDOUT)
endif()

find_program(LINUXDEPLOY_PLUGIN_QT_EXECUTABLE
  NAMES linuxdeploy-plugin-qt linuxdeploy-plugin-qt-${CMAKE_SYSTEM_PROCESSOR}.AppImage
  PATHS ${CPACK_PACKAGE_DIRECTORY}/linuxdeploy)

if (NOT LINUXDEPLOY_PLUGIN_QT_EXECUTABLE)
  message(STATUS "Downloading linuxdeploy-plugin-qt")
  set(LINUXDEPLOY_PLUGIN_QT_EXECUTABLE ${CPACK_PACKAGE_DIRECTORY}/linuxdeploy/linuxdeploy-plugin-qt)
  file(DOWNLOAD
      https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-${CMAKE_SYSTEM_PROCESSOR}.AppImage
      ${LINUXDEPLOY_PLUGIN_QT_EXECUTABLE}
      LOG ${CPACK_PACKAGE_DIRECTORY}/linuxdeploy/linuxdeploy-plugin-qt-download.log
      STATUS LINUXDEPLOY-PLUGIN-QT_DOWNLOAD)
  execute_process(COMMAND chmod +x ${LINUXDEPLOY_PLUGIN_QT_EXECUTABLE} COMMAND_ECHO STDOUT)
endif()


file(COPY ${CPACK_CMAKE_SOURCE_DIR}/interface/org.overte.interface.appdata.xml DESTINATION ${CPACK_TEMPORARY_DIRECTORY}/usr/share/metainfo/)
file(COPY ${CPACK_PACKAGE_DIRECTORY}/interface/plugins DESTINATION ${CPACK_TEMPORARY_DIRECTORY}/usr/bin/)
file(COPY ${CPACK_PACKAGE_DIRECTORY}/interface/scripts DESTINATION ${CPACK_TEMPORARY_DIRECTORY}/usr/bin/)
file(COPY ${CPACK_PACKAGE_DIRECTORY}/interface/resources DESTINATION ${CPACK_TEMPORARY_DIRECTORY}/usr/bin/)
file(COPY ${CPACK_PACKAGE_DIRECTORY}/interface/resources.rcc DESTINATION ${CPACK_TEMPORARY_DIRECTORY}/usr/bin/)

execute_process(
  COMMAND
    ${CMAKE_COMMAND} -E env
      OUTPUT=${CPACK_PACKAGE_FILE_NAME}-${CMAKE_SYSTEM_PROCESSOR}.AppImage
      VERSION=${CPACK_PACKAGE_VERSION}
      QML_SOURCES_PATHS=${CPACK_CMAKE_SOURCE_DIR}/interface/resources/qml/
    ${LINUXDEPLOY_EXECUTABLE}
    --appdir=${CPACK_TEMPORARY_DIRECTORY}
    --executable=${CPACK_PACKAGE_DIRECTORY}/interface/interface
    --desktop-file=${APPIMAGE_DESKTOP_FILE}
    --icon-file=${APPIMAGE_ICON_FILE}
    --plugin qt
    --output=appimage
)
