# header-only library

vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO Chlumsky/artery-font-format
        REF 34134bde3cea35a93c2ae5703fa8d3d463793400
        SHA512 6b2fc0de9ca7b367c9b98f829ce6cee858f1252b12a49b6f1e89a5a2fdb109e20ef812f0b30495195ca0b177adae32d5e238fdc305724857ced098be2d29a6af
        HEAD_REF master
)

file(COPY "${SOURCE_PATH}/artery-font" DESTINATION "${CURRENT_PACKAGES_DIR}/include")

# Handle copyright
configure_file("${SOURCE_PATH}/LICENSE.txt" "${CURRENT_PACKAGES_DIR}/share/${PORT}/copyright" COPYONLY)