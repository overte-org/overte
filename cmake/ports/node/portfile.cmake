# Copyright 2023 Overte e.V.
# SPDX-License-Identifier: MIT

set(NODE_VERSION 18.14.2)
set(MASTER_COPY_SOURCE_PATH ${CURRENT_BUILDTREES_DIR}/src)

file(READ "${VCPKG_ROOT_DIR}/_env/EXTERNAL_BUILD_ASSETS.txt" EXTERNAL_BUILD_ASSETS)

if (ANDROID)
   # TODO
elseif (WIN32)
    vcpkg_download_distfile(
        NODE_SOURCE_ARCHIVE
        URLS "${EXTERNAL_BUILD_ASSETS}/dependencies/node/node-install-18.15.1-win-x64-release.tar.xz"
        SHA512 892608a43ae32b0a82a0e3c7994934d0ce85639ea372c8e7feb7de44220211fa91878bd0744e1488054777807dd5b0c0677b59b44ab5e9fd35ecf222b38d8046
        FILENAME node-install-18.15.1-win-x64-release.tar.xz
    )
elseif (APPLE)
    vcpkg_download_distfile(
        NODE_SOURCE_ARCHIVE
        URLS "${EXTERNAL_BUILD_ASSETS}/dependencies/node/node-install-18.14.2-macOSXSDK10.14-macos-amd64-release.tar.xz"
        SHA512 TODO
        FILENAME node-install-18.14.2-macOSXSDK10.14-macos-amd64-release.tar.xz
    )
else ()
    # else Linux desktop
    vcpkg_download_distfile(
        NODE_SOURCE_ARCHIVE
        URLS "${EXTERNAL_BUILD_ASSETS}/dependencies/node/node-install-18.14.2-ubuntu-18.04-amd64-release.tar.xz"
        SHA512 ff5ca5c27b811d20ac524346ee122bcd72e9e85c6de6f4799f620bb95dac959ce910cc5bb2162ed741a7f65043aa78173ecd2ce5b92f5a4d91ecb07ce71fa560
        FILENAME node-install-18.14.2-ubuntu-18.04-amd64-release.tar.xz
    )
endif ()

vcpkg_extract_source_archive(MASTER_COPY_SOURCE_PATH ARCHIVE ${NODE_SOURCE_ARCHIVE} NO_REMOVE_ONE_LEVEL)

# move WIN dll to /bin and WIN .lib to /lib

if (WIN32)
    file(COPY ${MASTER_COPY_SOURCE_PATH}/node-install/include DESTINATION ${CURRENT_PACKAGES_DIR})
    file(COPY ${MASTER_COPY_SOURCE_PATH}/node-install/libnode.lib DESTINATION ${CURRENT_PACKAGES_DIR}/lib)
    file(COPY ${MASTER_COPY_SOURCE_PATH}/node-install/v8_libplatform.lib DESTINATION ${CURRENT_PACKAGES_DIR}/lib)
    file(COPY ${MASTER_COPY_SOURCE_PATH}/node-install/libnode.dll DESTINATION ${CURRENT_PACKAGES_DIR}/bin)
else ()
    file(COPY ${MASTER_COPY_SOURCE_PATH}/node-install/include DESTINATION ${CURRENT_PACKAGES_DIR})
    file(COPY ${MASTER_COPY_SOURCE_PATH}/node-install/lib DESTINATION ${CURRENT_PACKAGES_DIR})
    file(COPY ${MASTER_COPY_SOURCE_PATH}/node-install/share DESTINATION ${CURRENT_PACKAGES_DIR})
    file(COPY ${MASTER_COPY_SOURCE_PATH}/node-install/bin DESTINATION ${CURRENT_PACKAGES_DIR})
endif ()
