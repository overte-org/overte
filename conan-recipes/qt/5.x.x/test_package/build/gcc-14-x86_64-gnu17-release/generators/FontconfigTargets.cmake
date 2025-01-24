# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/Fontconfig-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${fontconfig_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${Fontconfig_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET Fontconfig::Fontconfig)
    add_library(Fontconfig::Fontconfig INTERFACE IMPORTED)
    message(${Fontconfig_MESSAGE_MODE} "Conan: Target declared 'Fontconfig::Fontconfig'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/Fontconfig-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()