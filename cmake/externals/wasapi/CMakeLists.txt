if (WIN32)

  set(EXTERNAL_NAME wasapi)
  string(TOUPPER ${EXTERNAL_NAME} EXTERNAL_NAME_UPPER)
    
  include(ExternalProject)
  ExternalProject_Add(
    ${EXTERNAL_NAME}
    URL "${EXTERNAL_BUILD_ASSETS}/dependencies/qtaudio_wasapi13.zip"
    URL_MD5 aa56a45f19c18caee13d29a40d1d7d28
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD 1
    DOWNLOAD_EXTRACT_TIMESTAMP 1
  )

  # Hide this external target (for ide users)
  set_target_properties(${EXTERNAL_NAME} PROPERTIES FOLDER "hidden/externals")

  ExternalProject_Get_Property(${EXTERNAL_NAME} SOURCE_DIR)

  set(${EXTERNAL_NAME_UPPER}_DLL_PATH ${SOURCE_DIR} CACHE FILEPATH "Location of wasapi DLL")

endif()
