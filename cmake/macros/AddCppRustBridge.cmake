#
#  Copyright 2026 Overte e.V.
#  Created by RTUnreal on 2026/03/13
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#


# basic setup of a new crate:
#
# 1. create these files (replace `${TARGET_NAME}` with the library name):
# `libraries/${TARGET_NAME}/Cargo.toml`:
# ```toml
# [package]
# name = "ovlib-${TARGET_NAME}"
# version.workspace = true
# edition.workspace = true
# license.workspace = true
#
# [lib]
# crate-type = ["staticlib"]
# path = "src/${TARGET_NAME}.rs"
#
# [dependencies]
# cxx.workspace = true
#
# [build-dependencies]
# cxx-build.workspace = true
# ```
#
# `libraries/${TARGET_NAME}/build.rs`:
# ```rs
# #[allow(unused_must_use)]
# fn main() {
#     cxx_build::bridge("src/${TARGET_NAME}.rs");
#     println!("cargo:rerun-if-changed=src/${TARGET_NAME}.rs");
# }
# ```
#
# `libraries/${TARGET_NAME}/src/${TARGET_NAME}.rs`:
# ```rs
# #[cxx::bridge(namespace = "image::rust")]
# mod ffi {
#     // Rust types and signatures exposed to C++.
#     extern "Rust" {
#     }
#
#     // C++ types and signatures exposed to  Rust.
#     unsafe extern "C++" {}
# }
# ```
#
# 2. add rust bridge generator by adding the following line to `CMakeLists.txt`:
# `libraries/${TARGET_NAME}/CMakeLists.txt`:
# ```cmake
# add_cpp_rust_bridge()
# ```
#
# 3. add the new rust crate to to the rust workspace by adding Cargo.toml dir to the Cargo.toml in the root directory:
#
# ```toml
# [workspace]
# members = [
# ...
#    "libraries/${TARGET_NAME}",
# ...
# ]
# ```
#
# 3. use the rust code from C++ by including the header file from any cpp file:
#
# ```cpp
# #include <${TARGET_NAME}.rs.h>
# ```
#
macro(add_cpp_rust_bridge)
    set(${TARGET_NAME}_RUST_LIB_NAME ovlib-${TARGET_NAME})
    set(${TARGET_NAME}_RUST_SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/${TARGET_NAME}.rs)
    set(${TARGET_NAME}_RUST_SOURCE_DIR ${CARGO_TARGET_DIR}/cxxbridge/${${TARGET_NAME}_RUST_LIB_NAME}/src)
    set(${TARGET_NAME}_RUST_BRIDGE_CPP ${${TARGET_NAME}_RUST_SOURCE_DIR}/${TARGET_NAME}.rs.cc)
    # RUSTTODO: make use of release as well
    string(REGEX REPLACE "-" "_" ${TARGET_NAME}_RUST_LIB_FILE_NAME "${TARGET_NAME}")
    set(${TARGET_NAME}_RUST_LIB ${CARGO_TARGET_DIR}/debug/${CMAKE_STATIC_LIBRARY_PREFIX}ovlib_${${TARGET_NAME}_RUST_LIB_FILE_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX})

    # Add a custom command that builds the rust crate and generates C++ bridge code
    add_custom_command(
        OUTPUT ${${TARGET_NAME}_RUST_LIB} ${${TARGET_NAME}_RUST_BRIDGE_CPP}
        COMMAND cargo build --manifest-path ${CARGO_MANIFEST} -p ${${TARGET_NAME}_RUST_LIB_NAME}
        DEPENDS ${${TARGET_NAME}_RUST_SOURCE_FILE}
        USES_TERMINAL
        COMMENT "Running cargo for ${${TARGET_NAME}_RUST_LIB_NAME} ..."
    )

    target_sources(${TARGET_NAME} PRIVATE ${${TARGET_NAME}_RUST_BRIDGE_CPP})
    target_include_directories(${TARGET_NAME} PRIVATE ${${TARGET_NAME}_RUST_SOURCE_DIR}/)

    target_link_libraries(${TARGET_NAME} ${${TARGET_NAME}_RUST_LIB})

    # RUSTTODO: make these usable
    #set_target_properties(
    #    cpp_with_rust
    #    PROPERTIES ADDITIONAL_CLEAN_FILES ${CARGO_TARGET_DIR}
    #)

    ## Windows-only configuration
    #if(WIN32)
    #    target_link_libraries(cpp_with_rust userenv ws2_32 bcrypt)
    #    set_target_properties(
    #        cpp_with_rust
    #        PROPERTIES
    #        MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    #        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}
    #        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}
    #    )
    #endif()
endmacro()
