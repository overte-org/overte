macro(TARGET_VHACD)
    find_package(v-hacd REQUIRED)
    target_link_libraries(${TARGET_NAME} v-hacd::v-hacd)
endmacro()
