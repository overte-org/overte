macro(TARGET_CGLTF)
    find_package(cgltf REQUIRED)
    target_link_libraries(${TARGET_NAME} cgltf::cgltf)
endmacro()
