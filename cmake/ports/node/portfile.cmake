# Copyright 2023 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

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
    # TODO
    vcpkg_download_distfile(
        NODE_SOURCE_ARCHIVE
        URLS "${EXTERNAL_BUILD_ASSETS}/dependencies/node/node-install-18.14.2-macOSXSDK10.14-macos-amd64-release.tar.xz"
        SHA512 TODO
        FILENAME node-install-18.14.2-macOSXSDK10.14-macos-amd64-release.tar.xz
    )
else ()
    # else Linux desktop
    if (VCPKG_TARGET_ARCHITECTURE STREQUAL "x64")
        #vcpkg_download_distfile(
        #    NODE_SOURCE_ARCHIVE
        #    URLS "https://daleglass.eu-central-1.linodeobjects.com/node-install-18.14.2-ubuntu-18.04-amd64-patched.tar.xz"
        #    SHA512 ecaf8139cd9e49528db349cf17b16b477b6153b8c8d9b08e7aea9cb378dd5126fa0a9a0f97bb6987da452c351047dec5abfede92404d7d940edeecb248b6b8b7
        #    FILENAME node-install-18.14.2-ubuntu-18.04-amd64-release.tar.xz
        #)
        vcpkg_from_github(
            OUT_SOURCE_PATH
            SOURCE_PATH
            REPO
            nodejs/node
            REF
            v18.16.0
            SHA512
            9b983b899acd02e7ed761bc3633fc56855e10335fcdb558a29d1cf068ce1125991c9a781616d82a9dc90be6e8ba1bf4a34a10a92c6b7db9cbe33ef7fa7dda67f
            HEAD_REF
            v18.16.0
        )
        file(COPY ${SOURCE_PATH} DESTINATION "${CURRENT_BUILDTREES_DIR}")
        vcpkg_execute_build_process(
            COMMAND ./configure --gdb --shared --v8-enable-object-print --shared-openssl --prefix=${CURRENT_BUILDTREES_DIR}/node-install/
            WORKING_DIRECTORY ${CURRENT_BUILDTREES_DIR}/v18.16.0-e8bd828d3a.clean
            LOGNAME "configure-node"
        )
        vcpkg_execute_build_process(
                COMMAND make -j20
                WORKING_DIRECTORY ${CURRENT_BUILDTREES_DIR}/v18.16.0-e8bd828d3a.clean
                LOGNAME "make-node"
        )
        vcpkg_execute_build_process(
                COMMAND make -j20 install
                WORKING_DIRECTORY ${CURRENT_BUILDTREES_DIR}/v18.16.0-e8bd828d3a.clean
                LOGNAME "install-node"
        )
        set(NODE_INSTALL_PATH ${CURRENT_BUILDTREES_DIR})
    elseif (VCPKG_TARGET_ARCHITECTURE STREQUAL "arm64")
            vcpkg_download_distfile(
            NODE_SOURCE_ARCHIVE
            URLS "${EXTERNAL_BUILD_ASSETS}/dependencies/node/node-install-18.16.0-ubuntu-20.04-aarch64-release.tar.xz"
            SHA512 aa4814c4ab1a922ec5afd4d7ef08479a32bfd23cb9a745102891bed5a2be13cc912e57e9bf80d856a15a5a9439b67c9a83963c605fdce349236795513090a426
            FILENAME node-install-18.16.0-ubuntu-22.04-aarch64-release.tar.xz
        )
    endif ()
endif ()

if (VCPKG_TARGET_ARCHITECTURE STREQUAL "x64")

else()
    vcpkg_extract_source_archive(SOURCE_PATH ARCHIVE ${NODE_SOURCE_ARCHIVE} NO_REMOVE_ONE_LEVEL)
    set(NODE_INSTALL_PATH ${MASTER_COPY_SOURCE_PATH})
endif()

# move WIN dll to /bin and WIN .lib to /lib

if (WIN32)
    file(COPY ${NODE_INSTALL_PATH}/node-install/include DESTINATION ${CURRENT_PACKAGES_DIR})
    file(COPY ${NODE_INSTALL_PATH}/node-install/libnode.lib DESTINATION ${CURRENT_PACKAGES_DIR}/lib)
    file(COPY ${NODE_INSTALL_PATH}/node-install/v8_libplatform.lib DESTINATION ${CURRENT_PACKAGES_DIR}/lib)
    file(COPY ${NODE_INSTALL_PATH}/node-install/libnode.dll DESTINATION ${CURRENT_PACKAGES_DIR}/bin)
else ()
    file(COPY ${NODE_INSTALL_PATH}/node-install/include DESTINATION ${CURRENT_PACKAGES_DIR})
    file(COPY ${NODE_INSTALL_PATH}/node-install/lib DESTINATION ${CURRENT_PACKAGES_DIR})
    file(COPY ${NODE_INSTALL_PATH}/node-install/share DESTINATION ${CURRENT_PACKAGES_DIR})
    file(COPY ${NODE_INSTALL_PATH}/node-install/bin DESTINATION ${CURRENT_PACKAGES_DIR})
endif ()
