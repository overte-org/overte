#
#  InstallExternalLibrary.cmake
#  cmake/macros
#
#  Copyright 2024 Overte e.V.
#  Created by Dale Glass on May 1, 2024
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

# Installs external libraries from places like vcpkg




function(install_external_library LIBRARY_FILES)
  if (WIN32 OR APPLE)
    # TODO
  else()
    # Linux


    # Linux libraries can be symlinks to the real .so
    foreach(LIB_FILE IN LISTS LIBRARY_FILES)
        message(STATUS "Installing library: ${LIB_FILE}")

        install(
            PROGRAMS "${LIB_FILE}"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/overte"
        )

        while (IS_SYMLINK "${LIB_FILE}")
            file(READ_SYMLINK ${LIB_FILE} real_file)
            if(NOT IS_ABSOLUTE "${real_file}")
                get_filename_component(dir "${LIB_FILE}" DIRECTORY)
                set(real_file "${dir}/${real_file}")
            endif()

            install(
                PROGRAMS "${real_file}"
                DESTINATION "${CMAKE_INSTALL_LIBDIR}/overte"
            )

            set(LIB_FILE "${real_file}")
        endwhile()
    endforeach()
  endif()
endfunction()
