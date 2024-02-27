#
#  PackageLibrariesForDeployment.cmake
#  cmake/macros
#
#  Copyright 2015 High Fidelity, Inc.
#  Created by Stephen Birarda on February 17, 2014
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#

macro(PACKAGE_LIBRARIES_FOR_DEPLOYMENT)
    if (WIN32)
        set(PLUGIN_PATH "plugins")

        get_target_property(Qt_Core_Location Qt5::Core LOCATION)
        get_filename_component(QT_BIN_DIR ${Qt_Core_Location} DIRECTORY)
        find_program(WINDEPLOYQT_COMMAND windeployqt PATHS QT_BIN_DIR)

        if (NOT WINDEPLOYQT_COMMAND)
            message(FATAL_ERROR "Could not find windeployqt at ${QT_DIR}/bin. windeployqt is required.")
        endif ()

        # add a post-build command to call windeployqt to copy Qt plugins
        set(CMD "${WINDEPLOYQT_COMMAND} ${EXTRA_DEPLOY_OPTIONS} --no-compiler-runtime --no-opengl-sw --no-angle -no-system-d3d-compiler")

        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/windeploy-${TARGET_NAME}.bat" "${CMD} %*")
        add_custom_command(
                TARGET ${TARGET_NAME}
                POST_BUILD
                COMMAND "${CMAKE_CURRENT_BINARY_DIR}/windeploy-${TARGET_NAME}.bat" $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>,$<CONFIG:RelWithDebInfo>>:--release> \"$<TARGET_FILE:${TARGET_NAME}>\"
        )

    endif ()

    # Add a post-build command to copy the libraries beside the executable
    add_custom_command(
            TARGET ${TARGET_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND}
            -DBUNDLE_EXECUTABLE="$<TARGET_FILE:${TARGET_NAME}>"
            -DBUNDLE_PLUGIN_DIR="$<TARGET_FILE_DIR:${TARGET_NAME}>/${PLUGIN_PATH}"
            -DLIB_PATHS="${CMAKE_BINARY_DIR}/conanlibs/$<CONFIGURATION>"
            -P "${CMAKE_SOURCE_DIR}/cmake/FixupBundlePostBuild.cmake"
    )
endmacro()
