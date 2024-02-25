# header-only library

vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO jkuhlmann/cgltf
        REF de399881c65c438a635627c749440eeea7e05599
        SHA512 753923116b92642848ff2bda70695ddd0e7be6db43ed3cfc37aff4cba90a29a92e3dbda139a5f2c80cad1d2cdaf81e1383e4ea7a12195f61fe8cfeb105e53ea2
        HEAD_REF master
)

file(COPY "${SOURCE_PATH}/cgltf.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include")
file(COPY "${SOURCE_PATH}/cgltf_write.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include")

# Handle copyright
configure_file("${SOURCE_PATH}/LICENSE" "${CURRENT_PACKAGES_DIR}/share/${PORT}/copyright" COPYONLY)