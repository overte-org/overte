# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/xkeyboard-config-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${xkeyboard-config_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${xkeyboard-config_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET xkeyboard-config::xkeyboard-config)
    add_library(xkeyboard-config::xkeyboard-config INTERFACE IMPORTED)
    message(${xkeyboard-config_MESSAGE_MODE} "Conan: Target declared 'xkeyboard-config::xkeyboard-config'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/xkeyboard-config-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()