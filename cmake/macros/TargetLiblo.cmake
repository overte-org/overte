macro(target_liblo)
    find_package(liblo REQUIRED)
    target_link_libraries(${TARGET_NAME} liblo::liblo)
endmacro()