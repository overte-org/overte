vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO oneapi-src/oneTBB
    REF 3df08fe234f23e732a122809b40eb129ae22733f
    SHA512 078b0aef93fb49a974adc365a4147cd2d12704e59d448fa2e510cd4ac8fa77cc4c83eebc5612684ed36a907449c876e5717eba581c195e1d9a7faf0ae832cb00
    HEAD_REF v2021.5.0
        fix-static-build.patch
        add-find-dependency.patch
)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DTBB_TEST=OFF
        -DTBB_EXAMPLES=OFF
        -DTBB_STRICT=OFF
        -DTBB_WINDOWS_DRIVER=OFF
        -DTBB_NO_APPCONTAINER=OFF
        -DTBB4PY_BUILD=OFF
        -DTBB_CPF=OFF
        -DTBB_FIND_PACKAGE=OFF
)

vcpkg_install_cmake()
vcpkg_copy_pdbs()

vcpkg_fixup_cmake_targets(CONFIG_PATH lib/cmake TARGET_PATH share)

# Delete unnamed libraries, they are deprecated
file(REMOVE "${CURRENT_PACKAGES_DIR}/debug/tbb${VCPKG_TARGET_STATIC_LIBRARY_SUFFIX}"
    "${CURRENT_PACKAGES_DIR}/tbb${VCPKG_TARGET_STATIC_LIBRARY_SUFFIX}"
)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

# Handle copyright
file(COPY ${SOURCE_PATH}/LICENSE ${CMAKE_CURRENT_LIST_DIR}/usage DESTINATION ${CURRENT_PACKAGES_DIR}/share/tbb)
file(RENAME ${CURRENT_PACKAGES_DIR}/share/tbb/LICENSE ${CURRENT_PACKAGES_DIR}/share/tbb/copyright)
#file(INSTALL "${SOURCE_PATH}/LICENSE.txt" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
