macro(target_liblo)
    find_package(liblo QUIET REQUIRED)
    target_link_libraries(${TARGET_NAME} liblo::liblo)
endmacro()
