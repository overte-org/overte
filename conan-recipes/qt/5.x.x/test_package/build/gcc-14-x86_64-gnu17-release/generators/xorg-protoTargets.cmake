# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/xorg-proto-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${xorg-proto_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${xorg-proto_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET xorg-proto::xorg-proto)
    add_library(xorg-proto::xorg-proto INTERFACE IMPORTED)
    message(${xorg-proto_MESSAGE_MODE} "Conan: Target declared 'xorg-proto::xorg-proto'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/xorg-proto-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()