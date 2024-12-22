macro(target_openldap)
    find_path(OPENLDAP_INCLUDE_DIR ldap.h)
    find_library(LDAP_LIBRARY ldap)
    find_library(LBER_LIBRARY lber)

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(OpenLdap DEFAULT_MSG
            OPENLDAP_INCLUDE_DIR LDAP_LIBRARY LBER_LIBRARY)

    set(OPENLDAP_LIBRARIES ${LDAP_LIBRARY} ${LBER_LIBRARY})

    mark_as_advanced(OPENLDAP_INCLUDE_DIR LDAP_LIBRARY LBER_LIBRARY)
    target_link_libraries(${TARGET_NAME} ${OPENLDAP_LIBRARIES})
endmacro()