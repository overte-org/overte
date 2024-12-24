vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO KhronosGroup/SPIRV-Tools
    REF v2022.4
    SHA512 d93e97e168c50f545cc42418603ffc5fa6299bb3cc30d927444e4de0d955abc5dd481c9662a59cd49fc379da6bcc6df6fb747947e3dc144cee9b489aff7c4785
    HEAD_REF master
)

vcpkg_from_github(
    OUT_SOURCE_PATH SPIRV_HEADERS_PATH
    REPO KhronosGroup/SPIRV-Headers
    REF 85a1ed200d50660786c1a88d9166e871123cce39
    SHA512 1607cb8d3eaed838825f776b28219e6afe3524047f4d0670582fe757765369365a36dbf47abce71dde4cbd2448aeae4263fb29a4b6d034fbbf54bfab5e22b71c
    HEAD_REF master
)

vcpkg_find_acquire_program(PYTHON3)
get_filename_component(PYTHON3_DIR "${PYTHON3}" DIRECTORY)
vcpkg_add_to_path("${PYTHON3_DIR}")

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DSPIRV-Headers_SOURCE_DIR=${SPIRV_HEADERS_PATH}
        -DSPIRV_WERROR=OFF
)

vcpkg_install_cmake()

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
file(GLOB EXES "${CURRENT_PACKAGES_DIR}/bin/*")
file(COPY ${EXES} DESTINATION ${CURRENT_PACKAGES_DIR}/tools)
file(REMOVE ${EXES})
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/bin ${CURRENT_PACKAGES_DIR}/debug/bin)

# Handle copyright
file(COPY ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/spirv-tools)
file(RENAME ${CURRENT_PACKAGES_DIR}/share/spirv-tools/LICENSE ${CURRENT_PACKAGES_DIR}/share/spirv-tools/copyright)
