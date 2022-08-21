set(VERSION 1.2.12)

vcpkg_download_distfile(ARCHIVE_FILE
    URLS "http://www.zlib.net/zlib-${VERSION}.tar.xz" "https://downloads.sourceforge.net/project/libpng/zlib/${VERSION}/zlib-${VERSION}.tar.xz" "https://build-deps.overte.org/dependencies/zlib-${VERSION}.tar.xz"
    FILENAME "zlib1212.tar.xz"
    SHA512 12940e81e988f7661da52fa20bdc333314ae86a621fdb748804a20840b065a1d6d984430f2d41f3a057de0effc6ff9bcf42f9ee9510b88219085f59cbbd082bd
)

vcpkg_extract_source_archive_ex(
    OUT_SOURCE_PATH SOURCE_PATH
    ARCHIVE ${ARCHIVE_FILE}
    REF ${VERSION}
    PATCHES
        "cmake_dont_build_more_than_needed.patch"
)

# This is generated during the cmake build
file(REMOVE ${SOURCE_PATH}/zconf.h)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DSKIP_INSTALL_FILES=ON
        -DSKIP_BUILD_EXAMPLES=ON
    OPTIONS_DEBUG
        -DSKIP_INSTALL_HEADERS=ON
)

vcpkg_install_cmake()

# Both dynamic and static are built, so keep only the one needed
if(VCPKG_LIBRARY_LINKAGE STREQUAL static)
    if(EXISTS ${CURRENT_PACKAGES_DIR}/lib/zlibstatic.lib)
        file(RENAME ${CURRENT_PACKAGES_DIR}/lib/zlibstatic.lib ${CURRENT_PACKAGES_DIR}/lib/zlib.lib)
    endif()
    if(EXISTS ${CURRENT_PACKAGES_DIR}/debug/lib/zlibstaticd.lib)
        file(RENAME ${CURRENT_PACKAGES_DIR}/debug/lib/zlibstaticd.lib ${CURRENT_PACKAGES_DIR}/debug/lib/zlibd.lib)
    endif()
endif()

file(INSTALL ${CMAKE_CURRENT_LIST_DIR}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/zlib RENAME copyright)

vcpkg_copy_pdbs()

file(COPY ${CMAKE_CURRENT_LIST_DIR}/usage DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT})
