# Copyright 2023-2025 Overte e.V.
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
    vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO nodejs/node
        REF v18.20.7
        SHA512 0d7000937b9b5089affc23daa7222938213bd5d77b8ed872d8cb424570fbc3e1567362c18ee8ef99413be098f7ff9fb82d49b9fc92fc93589628b07d1464b3ff
        HEAD_REF v18.20.7
        PATCHES node-cstdint.patch
    )
    # node cannot configure out of source, which VCPKG expects. So we copy the source to the configure directory.
    file(COPY ${SOURCE_PATH}/ DESTINATION "${CURRENT_BUILDTREES_DIR}")
    if (VCPKG_TARGET_ARCHITECTURE STREQUAL "arm64")
        # --gdb fails on aarch64
        vcpkg_execute_build_process(
            COMMAND ./configure --shared --v8-enable-object-print --shared-openssl --prefix=${CURRENT_BUILDTREES_DIR}/node-install/
            WORKING_DIRECTORY ${CURRENT_BUILDTREES_DIR}
            LOGNAME "configure-node"
        )
    else () # amd64
        vcpkg_execute_build_process(
            COMMAND ./configure --gdb --shared --v8-enable-object-print --shared-openssl --prefix=${CURRENT_BUILDTREES_DIR}/node-install/
            WORKING_DIRECTORY ${CURRENT_BUILDTREES_DIR}
            LOGNAME "configure-node"
        )
    endif ()
    if(VCPKG_MAX_CONCURRENCY GREATER 0)
        vcpkg_execute_build_process(
            COMMAND make -j${VCPKG_MAX_CONCURRENCY}
            WORKING_DIRECTORY ${CURRENT_BUILDTREES_DIR}
            LOGNAME "make-node"
        )
        vcpkg_execute_build_process(
                COMMAND make -j${VCPKG_MAX_CONCURRENCY} install
                WORKING_DIRECTORY ${CURRENT_BUILDTREES_DIR}
                LOGNAME "install-node"
        )
    elseif (VCPKG_CONCURRENCY GREATER 0)
        vcpkg_execute_build_process(
            COMMAND make -j${VCPKG_CONCURRENCY}
            WORKING_DIRECTORY ${CURRENT_BUILDTREES_DIR}
            LOGNAME "make-node"
        )
        vcpkg_execute_build_process(
                COMMAND make -j${VCPKG_CONCURRENCY} install
                WORKING_DIRECTORY ${CURRENT_BUILDTREES_DIR}
                LOGNAME "install-node"
        )
    else ()
        vcpkg_execute_build_process(
            COMMAND make -j$(nproc)
            WORKING_DIRECTORY ${CURRENT_BUILDTREES_DIR}
            LOGNAME "make-node"
        )
        vcpkg_execute_build_process(
                COMMAND make -j$(nproc) install
                WORKING_DIRECTORY ${CURRENT_BUILDTREES_DIR}
                LOGNAME "install-node"
        )
    endif ()
    set(NODE_INSTALL_PATH ${CURRENT_BUILDTREES_DIR})
endif ()

if (NODE_INSTALL_PATH)

else()
    vcpkg_extract_source_archive(MASTER_COPY_SOURCE_PATH ARCHIVE ${NODE_SOURCE_ARCHIVE} NO_REMOVE_ONE_LEVEL)
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
