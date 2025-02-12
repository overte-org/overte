# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/xorg-macros-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${xorg-macros_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${xorg-macros_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET xorg-macros::xorg-macros)
    add_library(xorg-macros::xorg-macros INTERFACE IMPORTED)
    message(${xorg-macros_MESSAGE_MODE} "Conan: Target declared 'xorg-macros::xorg-macros'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/xorg-macros-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()