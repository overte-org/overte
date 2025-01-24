# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/libxshmfence-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${libxshmfence_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${libxshmfence_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET libxshmfence::libxshmfence)
    add_library(libxshmfence::libxshmfence INTERFACE IMPORTED)
    message(${libxshmfence_MESSAGE_MODE} "Conan: Target declared 'libxshmfence::libxshmfence'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/libxshmfence-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()