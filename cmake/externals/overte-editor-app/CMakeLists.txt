include(FetchContent)

function(DOWNLOAD_AND_ADD_SCRIPT_EDITOR)
  FetchContent_Declare(
    overte-editor-app
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    URL      "${EXTERNAL_BUILD_ASSETS}/dependencies/script-editor/overte-editor-app-20241030_1.zip"
    URL_HASH MD5=3dbdfe5d68731c198619c4efc5b7f935
  )
endfunction()
