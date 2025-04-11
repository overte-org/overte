
macro(TARGET_AFF)
    find_package(artery-font-format)
    target_link_libraries(${TARGET_NAME} artery-font-format::artery-font-format)
endmacro()


