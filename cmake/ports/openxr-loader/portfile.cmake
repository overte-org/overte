
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO KhronosGroup/OpenXR-SDK
    REF "release-${VERSION}"
    SHA512 f5f02857036d14c3894bee979bf108c4066ff5551393bc9bdde85dced5c5007148880c6174174dfe3b844e00baeb66106afbf18be069958128404d6a9bdc96ce
    HEAD_REF master
    PATCHES
        fix-openxr-sdk-jsoncpp.patch
)

vcpkg_from_github(
    OUT_SOURCE_PATH SDK_SOURCE_PATH
    REPO KhronosGroup/OpenXR-SDK-Source
    REF "release-${VERSION}"
    SHA512 29155f5cd6104a479ce25ea090020001a01652ce42823ddad3e2569d7d2d513a0339c084d90acd3a00b220f7ba1cf68af1ac4b4c01f0a949aa9d919a1914d6c9
    HEAD_REF master
    PATCHES
        fix-openxr-sdk-jsoncpp.patch
)

# Weird behavior inside the OpenXR loader.  On Windows they force shared libraries to use static crt, and
# vice-versa. Might be better in future iterations to patch the CMakeLists.txt for OpenXR
if (VCPKG_TARGET_IS_UWP OR VCPKG_TARGET_IS_WINDOWS)
    if(VCPKG_LIBRARY_LINKAGE STREQUAL static)
        set(DYNAMIC_LOADER OFF)
        set(VCPKG_CRT_LINKAGE dynamic)
    else()
        set(DYNAMIC_LOADER ON)
        set(VCPKG_CRT_LINKAGE static)
    endif()
endif()

vcpkg_find_acquire_program(PYTHON3)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_API_LAYERS=OFF
        -DBUILD_TESTS=OFF
        -DBUILD_CONFORMANCE_TESTS=OFF
        -DDYNAMIC_LOADER=${DYNAMIC_LOADER}
        -DPYTHON_EXECUTABLE="${PYTHON3}"
        -DBUILD_WITH_SYSTEM_JSONCPP=ON
)

vcpkg_cmake_install()

if(VCPKG_TARGET_IS_WINDOWS)
    vcpkg_cmake_config_fixup(PACKAGE_NAME OpenXR CONFIG_PATH cmake)
else()
    vcpkg_cmake_config_fixup(PACKAGE_NAME OpenXR CONFIG_PATH lib/cmake/openxr)
endif()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

vcpkg_fixup_pkgconfig()
vcpkg_copy_pdbs()
file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
